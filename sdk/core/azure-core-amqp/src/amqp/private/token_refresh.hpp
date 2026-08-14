// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/datetime.hpp>

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  class SessionImpl;

  // The state that the connection shares with the token refresh thread.
  //
  // The refresh thread promotes a weak session pointer to a strong one, so it
  // can hold the last reference to a session and, through that session, the
  // last reference to the connection. When it releases that reference, the
  // connection destructor runs on the refresh thread. That destructor sets
  // Stop, signals Cv, and detaches the thread, because a thread cannot join
  // itself.
  //
  // The thread holds a shared_ptr to this block, so the block stays alive after
  // the connection is gone. The thread comes back from the release, takes
  // Mutex, sees Stop, and leaves without touching the connection.
  struct TokenRefreshState final
  {
    // Protects every other member of this block.
    std::mutex Mutex;
    std::condition_variable Cv;
    bool Stop{false};
    // The cached token for each audience.
    std::map<std::string, Azure::Core::Credentials::AccessToken> TokenStore;
    // The session that authenticated each audience. The pointer is weak, so the
    // refresh thread never keeps a session alive.
    std::map<std::string, std::weak_ptr<SessionImpl>> TokenSessions;
    // Counts the changes to TokenStore. Every write to that map increments this
    // counter under Mutex. The refresh thread reads the counter before it scans
    // the map, and it compares the two values after it wakes. A different value
    // means the map changed, so the thread scans again. The same value means the
    // wake was spurious, so the thread sleeps again on the same deadline.
    std::uint64_t Generation{0};
  };

  // Holds a shared pointer that a thread must not drop while it holds a lock,
  // and releases it with that lock free.
  //
  // The token refresh thread promotes a weak session pointer for the length of
  // one refresh. A session holds the connection, so that promoted pointer can
  // be the last reference to both, and the release then runs the connection
  // destructor on the refresh thread. That destructor takes the token mutex, so
  // a release under the token mutex would lock a mutex that this thread already
  // holds, and the mutex is not recursive. The release also runs other
  // destructors that take other locks.
  //
  // Every exit from the scope runs this destructor, so no return path and no
  // exception leaves the release under the lock.
  template <typename T> class ReleaseOutsideLock final {
  public:
    ReleaseOutsideLock(std::shared_ptr<T> held, std::unique_lock<std::mutex>& lock)
        : m_held{std::move(held)}, m_lock{lock}
    {
    }

    ~ReleaseOutsideLock() { Release(); }

    ReleaseOutsideLock(ReleaseOutsideLock const&) = delete;
    ReleaseOutsideLock& operator=(ReleaseOutsideLock const&) = delete;
    ReleaseOutsideLock(ReleaseOutsideLock&&) = delete;
    ReleaseOutsideLock& operator=(ReleaseOutsideLock&&) = delete;

    std::shared_ptr<T> const& Get() const { return m_held; }

    // Release the pointer with the lock free, then put the lock back in the
    // state it was in. This is safe to call more than once.
    void Release()
    {
      if (!m_held)
      {
        return;
      }
      bool const wasLocked = m_lock.owns_lock();
      if (wasLocked)
      {
        m_lock.unlock();
      }
      m_held.reset();
      if (wasLocked)
      {
        m_lock.lock();
      }
    }

  private:
    std::shared_ptr<T> m_held;
    std::unique_lock<std::mutex>& m_lock;
  };

  // The .NET client refreshes a CBS token seven minutes before the token
  // expires. See AmqpConnectionScope.cs in Azure.Messaging.EventHubs. Use the
  // same buffer here.
  constexpr std::chrono::minutes TokenRefreshBuffer{7};

  // The smallest time between two refresh passes. A service that issues tokens
  // with a lifetime shorter than the buffer makes every token due immediately.
  // This interval stops the refresh thread from spinning in that case.
  constexpr std::chrono::seconds MinimumTokenRefreshInterval{20};

  // The refresh thread does the early refresh. The cache only has to stop
  // itself from giving a caller a token that is about to die, so its margin is
  // small. A larger margin here would make the cache authenticate again on
  // every call for a token with a short lifetime.
  constexpr std::chrono::seconds MinimumTokenLifetimeToUse{30};

  // How long the refresh thread sleeps when no token is close to its expiry.
  constexpr std::chrono::minutes IdleTokenRefreshPoll{1};

  // The deadline for one refresh, which covers the credential call and the CBS
  // operation.
  constexpr std::chrono::seconds TokenRefreshOperationTimeout{60};

  // These functions compare in the Azure::DateTime domain on purpose. The cast
  // from Azure::DateTime to std::chrono::system_clock::time_point is explicit
  // and it throws when the value is outside the range of the system clock. A
  // credential can return any value in ExpiresOn, and a default constructed
  // ExpiresOn is year 1, so a cast here could throw on a caller thread or on
  // the refresh thread. The conversion in the other direction does not throw.

  // Return true while the cached token has enough life left to give to a
  // caller.
  inline bool IsCachedTokenUsable(
      Azure::Core::Credentials::AccessToken const& token,
      std::chrono::system_clock::time_point now)
  {
    return token.ExpiresOn > Azure::DateTime(now + MinimumTokenLifetimeToUse);
  }

  // Return true when the refresh thread must replace this token by the given
  // time. The refresh is due one buffer before the token expires, so the test
  // adds the buffer to the time instead of subtracting it from the expiry.
  // Adding avoids an underflow for a token that reports a very early expiry.
  inline bool IsTokenRefreshDue(
      Azure::Core::Credentials::AccessToken const& token,
      std::chrono::system_clock::time_point now)
  {
    return token.ExpiresOn <= Azure::DateTime(now + TokenRefreshBuffer);
  }

  // Return true when the refresh thread must leave its wait.
  //
  // The thread sleeps on a deadline that it computed from TokenStore. A change
  // to that map makes the deadline wrong, because a new audience can hold a
  // token that is due now. The caller gives the counter value that it read
  // before its scan, and a different value means the map changed. The same
  // value means the wake was spurious, so the thread sleeps again.
  //
  // The caller holds Mutex.
  inline bool ShouldWakeTokenRefresh(TokenRefreshState const& state, std::uint64_t observed)
  {
    return state.Stop || state.Generation != observed;
  }

  // Return true when a refresh that failed must drop the cached token.
  //
  // A refresh can fail for a short time, for example when the credential cannot
  // reach the identity service. The cached token still works until it expires,
  // so the connection keeps the token and the refresh thread tries again on its
  // next pass. The thread gets about one attempt each 20 seconds through the
  // buffer.
  //
  // A token with no life left is different. A caller cannot use it, and a token
  // that reaches the service after the expiry cannot save a link that the
  // service already dropped. So the connection drops the token, and the next
  // link open authenticates the audience again.
  inline bool ShouldDropTokenAfterFailedRefresh(
      Azure::Core::Credentials::AccessToken const& token,
      std::chrono::system_clock::time_point now)
  {
    return !IsCachedTokenUsable(token, now);
  }

}}}} // namespace Azure::Core::Amqp::_detail
