// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words amqpconnection amqpconnectionoptions amqpconnectionoptionsbuilder

#include "azure/core/amqp/internal/connection.hpp"

#include "azure/core/amqp/internal/common/global_state.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"
#include "claims_based_security_impl.hpp"
#include "connection_impl.hpp"
#include "private/cbs_open_failure.hpp"
#include "private/token_refresh.hpp"
#include "session_impl.hpp"

#include <azure/core/azure_assert.hpp>
#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <chrono>
#include <memory>
#include <vector>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  // Create a connection with an existing networking Transport.
#if ENABLE_UAMQP
  Connection::Connection(
      Network::_internal::Transport const& transport,
      ConnectionOptions const& options,
      ConnectionEvents* eventHandler,
      ConnectionEndpointEvents* endpointEventHandler)
      : m_impl{std::make_shared<_detail::ConnectionImpl>(
          transport.GetImpl(),
          options,
          eventHandler,
          endpointEventHandler)}
  {
    m_impl->FinishConstruction();
  }
#endif

  // Create a connection with a request URI and options.
  Connection::Connection(
      std::string const& hostName,
      std::shared_ptr<const Credentials::TokenCredential> credential,
      ConnectionOptions const& options
#if ENABLE_UAMQP
      ,
      ConnectionEvents* eventHandler
#endif
      )
      : m_impl
  {
    std::make_shared<_detail::ConnectionImpl>(
        hostName,
        credential,
        options
#if ENABLE_UAMQP
        ,
        eventHandler
#endif
    )
  }
  {
    m_impl->FinishConstruction();
  }

  Connection::~Connection() {}

  Session Connection::CreateSession(
      SessionOptions const& sessionOptions
#if ENABLE_UAMQP
      ,
      SessionEvents* sessionEvents
#endif
  ) const
  {
    return Azure::Core::Amqp::_detail::SessionFactory::CreateFromInternal(
        std::make_shared<_detail::SessionImpl>(
            m_impl,
            sessionOptions
#if ENABLE_UAMQP
            ,
            sessionEvents
#endif
            ));
  }

#if ENABLE_UAMQP
  Session Connection::CreateSession(
      Endpoint& endpoint,
      SessionOptions const& sessionOptions,
      SessionEvents* sessionEvents) const
  {
    return Azure::Core::Amqp::_detail::SessionFactory::CreateFromInternal(
        std::make_shared<_detail::SessionImpl>(m_impl, endpoint, sessionOptions, sessionEvents));
  }
  void Connection::Poll() { m_impl->Poll(); }

  void Connection::Listen() { m_impl->Listen(); }
#endif // ENABLE_UAMQP

  void Connection::Open(Azure::Core::Context const& context) { m_impl->Open(context); }
  void Connection::Close(Azure::Core::Context const& context) { m_impl->Close(context); }
  void Connection::Close(
      std::string const& condition,
      std::string const& description,
      Models::AmqpValue value,
      Azure::Core::Context const& context)
  {
    m_impl->Close(condition, description, value, context);
  }
  uint32_t Connection::GetMaxFrameSize() const { return m_impl->GetMaxFrameSize(); }
#if ENABLE_UAMQP
  uint32_t Connection::GetRemoteMaxFrameSize() const { return m_impl->GetRemoteMaxFrameSize(); }
#endif
  uint16_t Connection::GetMaxChannel() const { return m_impl->GetMaxChannel(); }
  std::string Connection::GetHost() const { return m_impl->GetHost(); }
  uint16_t Connection::GetPort() const { return m_impl->GetPort(); }
  std::chrono::milliseconds Connection::GetIdleTimeout() const { return m_impl->GetIdleTimeout(); }
  Models::AmqpMap Connection::GetProperties() const { return m_impl->GetProperties(); }
#if ENABLE_UAMQP
  void Connection::SetIdleEmptyFrameSendPercentage(double ratio)
  {
    m_impl->SetIdleEmptyFrameSendPercentage(ratio);
  }
#endif
}}}} // namespace Azure::Core::Amqp::_internal

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  namespace {
    // Put a token for one audience on the wire. Both the first authentication
    // and the proactive refresh use this function, so the two paths stay the
    // same. The caller must hold the CBS mutex, because this function makes a
    // claims based security object. This function takes no lock of its own. The
    // token mutex is not necessary here, and the refresh path releases it
    // before this call.
    void PutTokenForAudience(
        std::shared_ptr<SessionImpl> session,
        CbsTokenType tokenType,
        std::string const& audienceUrl,
        std::string const& token,
        Azure::DateTime const& expiresOn,
        CbsOpenCaller caller,
        Azure::Core::Context const& context)
    {
      auto claimsBasedSecurity = std::make_shared<ClaimsBasedSecurityImpl>(session);
      auto cbsOpenStatus = claimsBasedSecurity->Open(context);
      if (cbsOpenStatus != CbsOpenResult::Ok)
      {
        Log::Stream(Logger::Level::Warning)
            << FormatCbsOpenFailureLog(cbsOpenStatus, audienceUrl, tokenType, expiresOn, caller);
        throw std::runtime_error(DescribeCbsOpenFailure(cbsOpenStatus, audienceUrl, caller));
      }

      try
      {
        auto result
            = claimsBasedSecurity->PutToken(tokenType, audienceUrl, token, expiresOn, context);
        if (std::get<0>(result) != CbsOperationResult::Ok)
        {
          throw Azure::Core::Credentials::AuthenticationException(
              "Could not authenticate client. Error Status: " + std::to_string(std::get<1>(result))
              + " reason: " + std::get<2>(result));
        }
        Log::Stream(Logger::Level::Verbose) << "Close CBS object";
        claimsBasedSecurity->Close(context);
      }
      catch (...)
      {
        // Ensure that the claims based security object is closed before we leave this scope.
        //
        // Keep the exception that brought us here. A close that throws in this
        // handler replaces that exception, and the caller then reads a close
        // failure in place of the authentication failure that caused it. Issue
        // #7323 makes a close that fails leave the object closed, so this
        // handler does not need the exception to keep the object safe.
        try
        {
          claimsBasedSecurity->Close(context);
        }
        catch (...)
        {
        }
        throw;
      }
    }
  } // namespace

  bool ConnectionImpl::IsSasCredential() const
  {
    if (GetCredential())
    {
      return GetCredential()->GetCredentialName() == "ServiceBusSasConnectionStringCredential";
    }
    return false;
  }

  // Ensure that we have a token for the provided audience.
  // If we don't, authenticate the audience with the service using the provided session.
  // Note that the granularity of
  Credentials::AccessToken ConnectionImpl::AuthenticateAudience(
      std::shared_ptr<SessionImpl> session,
      std::string const& audience,
      Azure::Core::Context const& context)
  {
    if (GetCredential())
    {
      std::string audienceUrl = audience;
      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose) << "Authenticate connection for audience " << audience;
      }
      // If the audience looks like a URL for AMQP, AMQPS, or SB, we can use the URL as
      // provided.
      if ((audience.find("amqps://") != 0) && (audience.find("amqp://") != 0)
          && (audience.find("sb://") != 0))
      {
        audienceUrl = "amqps://" + GetHost();
        // The provided audience may begin with a /, if not, we need to add the separator.
        if (audience.front() != '/')
        {
          audienceUrl += "/";
        }
        audienceUrl += audience;
        if (m_options.EnableTrace)
        {
          Log::Stream(Logger::Level::Verbose)
              << "Initial audience is not URL, using " << audienceUrl;
        }
      }

      std::unique_lock<std::mutex> lock(m_tokenState->Mutex);
      // If we have authenticated this audience, we're done and can return success.
      // A cached token is only good while it has enough life left to use. A token
      // that is at or near its expiry is discarded here, so the audience is
      // authenticated again below.
      auto token = m_tokenState->TokenStore.find(audienceUrl);
      if (token != m_tokenState->TokenStore.end())
      {
        if (IsCachedTokenUsable(token->second, std::chrono::system_clock::now()))
        {
          if (m_options.EnableTrace)
          {
            Log::Stream(Logger::Level::Verbose) << "Using cached token for " << audienceUrl;
          }
#if ENABLE_UAMQP
          // Point the refresh thread at a session that is in use now. The
          // session that first authenticated this audience can be gone while
          // another session still uses the token.
          m_tokenState->TokenSessions[audienceUrl] = session;
#endif
          return token->second;
        }
        if (m_options.EnableTrace)
        {
          Log::Stream(Logger::Level::Verbose) << "Cached token for " << audienceUrl
                                              << " is at or near expiry, authenticating again.";
        }
        m_tokenState->TokenStore.erase(token);
        ++m_tokenState->Generation;
#if ENABLE_UAMQP
        // Remove the session with the token. The authentication below puts both
        // of them back. If that authentication throws, the two maps stay in
        // step. Erase by the key, because the iterator above belongs to the
        // other map.
        m_tokenState->TokenSessions.erase(audienceUrl);
#endif
      }
      // We've not authenticated this audience.
      // Authenticate it with the server

      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose)
            << "No cached token for " << audienceUrl << ", Authenticating.";
      }

      Credentials::TokenRequestContext requestContext;

      requestContext.Scopes = m_options.AuthenticationScopes;
      auto accessToken{GetCredential()->GetToken(requestContext, context)};

      {
#if ENABLE_UAMQP
        // Only one claims based security object may exist on this connection at
        // a time. See m_cbsMutex.
        std::lock_guard<std::mutex> cbsLock(m_cbsMutex);
#endif
        PutTokenForAudience(
            session,
            (IsSasCredential() ? CbsTokenType::Sas : CbsTokenType::Jwt),
            audienceUrl,
            accessToken.Token,
            accessToken.ExpiresOn,
            CbsOpenCaller::Authenticate,
            context);
      }

      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose)
            << "Authenticated connection for audience " << audienceUrl << " successfully.";
      }

      // Assign, do not emplace. A refreshed token must replace the token that is
      // already in the cache.
      m_tokenState->TokenStore[audienceUrl] = accessToken;
      ++m_tokenState->Generation;
#if ENABLE_UAMQP
      // Remember the session that authenticated this audience, so the refresh
      // thread can put a new token on the same session. The pointer is weak, so
      // the refresh thread never keeps a session alive.
      m_tokenState->TokenSessions[audienceUrl] = session;
      StartTokenRefresh();
#endif
      return accessToken;
    }
    else
    {
      Log::Stream(Logger::Level::Verbose) << "No credential, returning empty token.";
      // If the connection is unauthenticated, then just return an empty access token.
      return {};
    }
  }

#if ENABLE_UAMQP
  // Start the refresh thread if it is not running yet. The caller holds the
  // token mutex.
  //
  // A thread that stopped on an error stays joinable, so this function does not
  // start a second one. That is deliberate. The connection then refreshes each
  // token when a caller uses it, which is the behavior the error log describes.
  //
  // The refresh runs at most one time for the life of a connection. Read the
  // note on Stop below before you make it restart.
  void ConnectionImpl::StartTokenRefresh()
  {
    // Stop is terminal. StopTokenRefresh sets it, and nothing clears it, so
    // this function does nothing for the rest of the life of this connection.
    // A caller then authenticates each audience on use, and the near expiry
    // test in the cache keeps that correct.
    //
    // Do not make the refresh restart by clearing this flag. Stop is also the
    // lifetime guard for a detached thread. StopTokenRefresh detaches when the
    // refresh thread runs the call itself, and that thread reads Stop before it
    // touches the raw connection pointer again. A clear on a connection that is
    // gone gives undefined behavior, not a failed refresh.
    //
    // Two more members stay in their stopped state. m_tokenRefreshContext is
    // cancelled and cannot go back, so every refresh would fail at once. The
    // thread member is left not joinable, so the test below is not the guard
    // that stops a second thread.
    //
    // A restart needs a new state block and a new context, which is a new
    // connection. That is what the clients do. A closed connection is dead on
    // both transports, so nothing here reopens one. Note that the token maps
    // live through a close, so a closed connection answers from the cache until
    // a token falls under MinimumTokenLifetimeToUse, and the full
    // authentication after that fails because the connection no longer polls.
    if (m_tokenState->Stop)
    {
      return;
    }
    if (!m_tokenRefreshThread.joinable())
    {
      // The thread co-owns the shared state, so the state outlives this
      // connection. The raw `this` pointer is good only while the state says
      // the connection is alive. See TokenRefreshState.
      m_tokenRefreshThread
          = std::thread([this, state = m_tokenState]() { TokenRefreshThread(this, state); });
    }
    m_tokenState->Cv.notify_all();
  }

  void ConnectionImpl::StopTokenRefresh()
  {
    // Cancel before the lock, not after it. The refresh thread does its network
    // work without the token mutex, but it can hold that mutex at other times.
    // A cancel that waits for the mutex could not stop an operation that is in
    // flight, which is the operation this call must stop. Context::Cancel is
    // safe to call from any thread.
    m_tokenRefreshContext.Cancel();

    std::thread threadToJoin;
    {
      std::unique_lock<std::mutex> lock(m_tokenState->Mutex);
      m_tokenState->Stop = true;
      m_tokenState->Cv.notify_all();
      threadToJoin = std::move(m_tokenRefreshThread);
    }

    // Join outside the lock, because the refresh thread takes the token mutex.
    if (threadToJoin.joinable())
    {
      if (threadToJoin.get_id() == std::this_thread::get_id())
      {
        // The refresh thread is running this call, which means it released the
        // last reference to this connection. It cannot join itself, so detach
        // it.
        //
        // The detached thread outlives this connection, and that is safe. It
        // holds a shared_ptr to the state block, so the mutex, the condition
        // variable, the stop flag, and the token maps all stay alive. Stop is
        // set above, so the thread comes back from the release, takes the mutex
        // in the state block, sees Stop, and leaves without touching this
        // connection.
        threadToJoin.detach();
      }
      else
      {
        threadToJoin.join();
      }
    }
  }

  // The body of the refresh thread.
  //
  // This is a static function, and `connection` is a raw pointer on purpose.
  // The thread must not own the connection, because a thread that owns the
  // object it refreshes keeps that object alive forever. The `state` block is
  // shared instead. The thread uses `connection` only while it knows the
  // connection is alive, which means until RefreshTokenForAudience reports that
  // the connection can be gone.
  void ConnectionImpl::TokenRefreshThread(
      ConnectionImpl* connection,
      std::shared_ptr<TokenRefreshState> state)
  {
    // A background thread must not let an exception escape, because that ends
    // the process. Every failure of one refresh is handled inside
    // RefreshTokenForAudience, so this catch is for the unexpected.
    try
    {
      std::unique_lock<std::mutex> lock(state->Mutex);

      // The earliest time the next refresh pass may run. This lives outside
      // the loop, because a wake for a new audience starts a new pass, and
      // that new pass must still obey the floor.
      auto refreshFloor = std::chrono::system_clock::time_point::min();

      while (!state->Stop)
      {
        // Read before the scan. The refresh below releases the mutex, and a
        // read after it would take an audience added in that window as seen.
        auto const observed = state->Generation;

        auto const now = std::chrono::system_clock::now();

        // Idle wake time when no token is close to its expiry.
        auto nextWake = now + IdleTokenRefreshPoll;
        std::vector<std::string> dueAudiences;
        for (auto const& entry : state->TokenStore)
        {
          if (IsTokenRefreshDue(entry.second, now))
          {
            dueAudiences.push_back(entry.first);
          }
          else if (IsTokenRefreshDue(entry.second, now + IdleTokenRefreshPoll))
          {
            // This token is due inside the idle poll time, so wake for it. The
            // expiry is close to now, which makes the cast safe.
            auto const expiry
                = static_cast<std::chrono::system_clock::time_point>(entry.second.ExpiresOn);
            nextWake = (std::min)(nextWake, expiry - TokenRefreshBuffer);
          }
        }

        if (ShouldDeferTokenRefreshPass(!dueAudiences.empty(), now, refreshFloor))
        {
          dueAudiences.clear();
          // Take the earlier of the scan deadline and the floor. The scan
          // deadline can belong to a token that is not the deferred one, and
          // a wait to the later time would miss it.
          nextWake = (std::min)(nextWake, refreshFloor);
        }

        bool keptTokenAfterFailedRefresh = false;
        for (auto const& audience : dueAudiences)
        {
          if (state->Stop)
          {
            break;
          }
          if (connection->RefreshTokenForAudience(
                  *state, audience, lock, keptTokenAfterFailedRefresh))
          {
            // That call released a session reference, which can have destroyed
            // this connection. `connection` is not safe to use now, so leave.
            // The state block stays alive until this thread returns.
            return;
          }
        }

        if (!dueAudiences.empty())
        {
          // Keep a minimum time between two refresh passes. A token with a
          // lifetime shorter than the refresh buffer is due as soon as it
          // arrives, and this floor stops the thread from spinning on it.
          refreshFloor = std::chrono::system_clock::now() + MinimumTokenRefreshInterval;
          nextWake = NextWakeAfterRefreshPass(nextWake, refreshFloor, keptTokenAfterFailedRefresh);
        }

        if (state->TokenStore.empty())
        {
          // Nothing to refresh. Wait until an audience is authenticated, or
          // until shutdown.
          state->Cv.wait(lock, [&state]() { return state->Stop || !state->TokenStore.empty(); });
        }
        else
        {
          // This thread's own refresh bumps the counter, so this wait
          // returns at once, one time, per pass. That cannot spin, because the
          // next pass finds the floor still in force, defers its work, and
          // writes nothing to the map, leaving the counter equal to what it
          // observed.
          state->Cv.wait_until(lock, nextWake, [&state, &lock, observed]() {
            return ShouldWakeTokenRefresh(*state, observed, lock);
          });
        }
      }
    }
    catch (std::exception const& e)
    {
      Log::Stream(Logger::Level::Error)
          << "The token refresh thread stopped on an error: " << e.what()
          << ". Tokens are refreshed on use instead.";
    }
    catch (...)
    {
      Log::Stream(Logger::Level::Error)
          << "The token refresh thread stopped on an unknown error. Tokens are refreshed on use "
             "instead.";
    }
  }

  // Replace the token for one audience.
  //
  // The caller holds the token mutex through `lock`. This function releases that
  // mutex for the credential call and for the CBS operation, because both go to
  // the network. Holding the mutex there would block every caller that opens a
  // link, and it would stop a shutdown from cancelling this work.
  //
  // This function holds a strong session pointer while the mutex is free, so it
  // can hold the last reference to the session and, through the session, the
  // last reference to this connection. ReleaseOutsideLock releases that
  // pointer, always with the mutex free. The return value tells the caller
  // whether this connection can be gone.
  bool ConnectionImpl::RefreshTokenForAudience(
      TokenRefreshState& state,
      std::string const& audienceUrl,
      std::unique_lock<std::mutex>& lock,
      bool& keptTokenAfterFailedRefresh)
  {
    AZURE_ASSERT(lock.owns_lock() && lock.mutex() == &state.Mutex);
    std::shared_ptr<SessionImpl> promotedSession;
    auto sessionEntry = state.TokenSessions.find(audienceUrl);
    if (sessionEntry != state.TokenSessions.end())
    {
      promotedSession = sessionEntry->second.lock();
    }
    if (!promotedSession)
    {
      // The session that authenticated this audience is gone. Drop the entry,
      // and the next link open authenticates the audience again.
      state.TokenStore.erase(audienceUrl);
      state.TokenSessions.erase(audienceUrl);
      ++state.Generation;
      return false;
    }

    // The hold owns this reference from here on, on every path out of this
    // function.
    ReleaseOutsideLock<SessionImpl> sessionHold{std::move(promotedSession), lock};

    // Remember the token that this refresh replaces, so the result does not
    // overwrite a newer token that a caller stored while the mutex was free.
    auto const previousEntry = state.TokenStore.find(audienceUrl);
    if (previousEntry == state.TokenStore.end())
    {
      // A caller dropped this token while this pass was in flight. There is
      // nothing to replace. Release the session here, so the flag that this
      // function returns is read after the release.
      sessionHold.Release();
      return state.Stop;
    }
    std::string const previousToken{previousEntry->second.Token};

    auto const tokenType = (IsSasCredential() ? CbsTokenType::Sas : CbsTokenType::Jwt);
    auto const credential = GetCredential();
    auto const scopes = m_options.AuthenticationScopes;
    auto const traceEnabled = m_options.EnableTrace;
    auto const parentContext = m_tokenRefreshContext;

    Credentials::AccessToken accessToken;
    bool refreshed{false};
    std::string failureMessage;

    lock.unlock();
    try
    {
      auto context = parentContext.WithDeadline(
          std::chrono::system_clock::now() + TokenRefreshOperationTimeout);

      Credentials::TokenRequestContext requestContext;
      requestContext.Scopes = scopes;
      accessToken = credential->GetToken(requestContext, context);

      {
        // Only one claims based security object may exist on this connection at
        // a time, so this waits for a caller that authenticates right now. See
        // m_cbsMutex. This mutex is released before the token mutex is taken
        // again, which keeps the lock order acyclic.
        std::lock_guard<std::mutex> cbsLock(m_cbsMutex);
        PutTokenForAudience(
            sessionHold.Get(),
            tokenType,
            audienceUrl,
            accessToken.Token,
            accessToken.ExpiresOn,
            CbsOpenCaller::Refresh,
            context);
      }
      refreshed = true;
    }
    catch (std::exception const& e)
    {
      failureMessage = e.what();
    }
    catch (...)
    {
      failureMessage = "unknown error";
    }
    // This is the one point where this thread can destroy the connection. A
    // session destructor releases the session's reference to the connection, so
    // this release can run the connection destructor on this thread. That
    // destructor calls StopTokenRefresh, which sets Stop in the state block and
    // detaches this thread. Every line after this one uses `state`, which this
    // thread co-owns, and no member of this connection.
    sessionHold.Release();
    lock.lock();

    if (state.Stop)
    {
      // Either a shutdown is in progress, or the release above destroyed this
      // connection. Do not touch this connection again.
      return true;
    }

    auto currentEntry = state.TokenStore.find(audienceUrl);
    if (currentEntry == state.TokenStore.end() || currentEntry->second.Token != previousToken)
    {
      // A caller replaced or dropped this token while the mutex was free. That
      // token is newer than this one, so keep it.
      return false;
    }

    if (refreshed)
    {
      currentEntry->second = accessToken;
      ++state.Generation;
      if (traceEnabled)
      {
        Log::Stream(Logger::Level::Verbose)
            << "Refreshed the token for audience " << audienceUrl << " before its expiry.";
      }
    }
    else
    {
      Log::Stream(Logger::Level::Warning)
          << "Could not refresh the token for audience " << audienceUrl << ": " << failureMessage;

      // Keep the token while it still has life. A sender or a receiver that is
      // already open never authenticates again, because only the Open methods
      // call AuthenticateAudience. So a token that goes away here never comes
      // back, and the link dies at the expiry. The thread tries again on its
      // next pass instead, which is about one attempt each 20 seconds.
      //
      // Drop the token when it has no life left. The next link open then
      // authenticates the audience again.
      if (ShouldDropTokenAfterFailedRefresh(currentEntry->second, std::chrono::system_clock::now()))
      {
        state.TokenStore.erase(currentEntry);
        state.TokenSessions.erase(audienceUrl);
        ++state.Generation;
      }
      else
      {
        keptTokenAfterFailedRefresh = true;
      }
    }
    return false;
  }
#endif // ENABLE_UAMQP
}}}} // namespace Azure::Core::Amqp::_detail
