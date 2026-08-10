// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words amqpconnection amqpconnectionoptions amqpconnectionoptionsbuilder

#include "azure/core/amqp/internal/connection.hpp"

#include "azure/core/amqp/internal/common/global_state.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"
#include "claims_based_security_impl.hpp"
#include "connection_impl.hpp"
#include "private/token_refresh.hpp"
#include "session_impl.hpp"

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
    // same. The caller holds the token mutex.
    void PutTokenForAudience(
        std::shared_ptr<SessionImpl> session,
        CbsTokenType tokenType,
        std::string const& audienceUrl,
        std::string const& token,
        Azure::DateTime const& expiresOn,
        Azure::Core::Context const& context)
    {
      auto claimsBasedSecurity = std::make_shared<ClaimsBasedSecurityImpl>(session);
      auto cbsOpenStatus = claimsBasedSecurity->Open(context);
      if (cbsOpenStatus != CbsOpenResult::Ok)
      {
        throw std::runtime_error("Could not open Claims Based Security object.");
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
        claimsBasedSecurity->Close(context);
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

      std::unique_lock<std::mutex> lock(m_tokenMutex);
      // If we have authenticated this audience, we're done and can return success.
      // A cached token is only good while it has enough life left to use. A token
      // that is at or near its expiry is discarded here, so the audience is
      // authenticated again below.
      auto token = m_tokenStore.find(audienceUrl);
      if (token != m_tokenStore.end())
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
          m_tokenSessions[audienceUrl] = session;
#endif
          return token->second;
        }
        if (m_options.EnableTrace)
        {
          Log::Stream(Logger::Level::Verbose) << "Cached token for " << audienceUrl
                                              << " is at or near expiry, authenticating again.";
        }
        m_tokenStore.erase(token);
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
            context);
      }

      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose)
            << "Authenticated connection for audience " << audienceUrl << " successfully.";
      }

      // Assign, do not emplace. A refreshed token must replace the token that is
      // already in the cache.
      m_tokenStore[audienceUrl] = accessToken;
#if ENABLE_UAMQP
      // Remember the session that authenticated this audience, so the refresh
      // thread can put a new token on the same session. The pointer is weak, so
      // the refresh thread never keeps a session alive.
      m_tokenSessions[audienceUrl] = session;
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
  void ConnectionImpl::StartTokenRefresh()
  {
    if (m_tokenRefreshStop)
    {
      return;
    }
    if (!m_tokenRefreshThread.joinable())
    {
      m_tokenRefreshThread = std::thread([this]() { TokenRefreshThread(); });
    }
    m_tokenRefreshCv.notify_all();
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
      std::unique_lock<std::mutex> lock(m_tokenMutex);
      m_tokenRefreshStop = true;
      m_tokenRefreshCv.notify_all();
      threadToJoin = std::move(m_tokenRefreshThread);
    }

    // Join outside the lock, because the refresh thread takes the token mutex.
    if (threadToJoin.joinable())
    {
      if (threadToJoin.get_id() == std::this_thread::get_id())
      {
        // The refresh thread is running this call, which means it released the
        // last reference to this connection. It cannot join itself.
        //
        // This path only happens when the connection is destroyed while it is
        // still open, and the destructor stops the process on that condition a
        // moment after this call returns. So the detached thread does not
        // outlive the connection today. If those asserts ever go away, this
        // thread comes back to a destroyed mutex, so give it a way to stop
        // before you relax them.
        threadToJoin.detach();
      }
      else
      {
        threadToJoin.join();
      }
    }
  }

  void ConnectionImpl::TokenRefreshThread()
  {
    // A background thread must not let an exception escape, because that ends
    // the process. Every failure of one refresh is handled inside
    // RefreshTokenForAudience, so this catch is for the unexpected.
    try
    {
      std::unique_lock<std::mutex> lock(m_tokenMutex);
      while (!m_tokenRefreshStop)
      {
        auto const now = std::chrono::system_clock::now();

        // Idle wake time when no token is close to its expiry.
        auto nextWake = now + IdleTokenRefreshPoll;
        std::vector<std::string> dueAudiences;
        for (auto const& entry : m_tokenStore)
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

        for (auto const& audience : dueAudiences)
        {
          if (m_tokenRefreshStop)
          {
            break;
          }
          RefreshTokenForAudience(audience, lock);
        }

        if (!dueAudiences.empty())
        {
          // Keep a minimum time between two refresh passes. A token with a
          // lifetime shorter than the refresh buffer is due as soon as it
          // arrives, and this wait stops the thread from spinning on it.
          nextWake = std::chrono::system_clock::now() + MinimumTokenRefreshInterval;
        }

        if (m_tokenStore.empty())
        {
          // Nothing to refresh. Wait until an audience is authenticated, or
          // until shutdown.
          m_tokenRefreshCv.wait(
              lock, [this]() { return m_tokenRefreshStop || !m_tokenStore.empty(); });
        }
        else
        {
          m_tokenRefreshCv.wait_until(lock, nextWake, [this]() { return m_tokenRefreshStop; });
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
  void ConnectionImpl::RefreshTokenForAudience(
      std::string const& audienceUrl,
      std::unique_lock<std::mutex>& lock)
  {
    std::shared_ptr<SessionImpl> session;
    auto sessionEntry = m_tokenSessions.find(audienceUrl);
    if (sessionEntry != m_tokenSessions.end())
    {
      session = sessionEntry->second.lock();
    }
    if (!session)
    {
      // The session that authenticated this audience is gone. Drop the entry,
      // and the next link open authenticates the audience again.
      m_tokenStore.erase(audienceUrl);
      m_tokenSessions.erase(audienceUrl);
      return;
    }

    // Remember the token that this refresh replaces, so the result does not
    // overwrite a newer token that a caller stored while the mutex was free.
    auto const previousEntry = m_tokenStore.find(audienceUrl);
    if (previousEntry == m_tokenStore.end())
    {
      return;
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
            session, tokenType, audienceUrl, accessToken.Token, accessToken.ExpiresOn, context);
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
    // Release the session before the mutex is taken again. This reference can be
    // the last one, and a session destructor takes other locks.
    session.reset();
    lock.lock();

    auto currentEntry = m_tokenStore.find(audienceUrl);
    if (currentEntry == m_tokenStore.end() || currentEntry->second.Token != previousToken)
    {
      // A caller replaced or dropped this token while the mutex was free. That
      // token is newer than this one, so keep it.
      return;
    }

    if (refreshed)
    {
      currentEntry->second = accessToken;
      if (traceEnabled)
      {
        Log::Stream(Logger::Level::Verbose)
            << "Refreshed the token for audience " << audienceUrl << " before its expiry.";
      }
    }
    else
    {
      // Drop the cache entry, so the next link open authenticates again.
      Log::Stream(Logger::Level::Warning)
          << "Could not refresh the token for audience " << audienceUrl << ": " << failureMessage;
      m_tokenStore.erase(currentEntry);
      m_tokenSessions.erase(audienceUrl);
    }
  }
#endif // ENABLE_UAMQP
}}}} // namespace Azure::Core::Amqp::_detail
