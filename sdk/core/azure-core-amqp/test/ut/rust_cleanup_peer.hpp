// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <future>
#include <thread>

extern "C"
{
  struct CleanupPeer;
  CleanupPeer* cleanup_peer_start(std::uint32_t mode);
  std::uint16_t cleanup_peer_port(CleanupPeer const* peer);
  bool cleanup_peer_wait(CleanupPeer const* peer, std::uint32_t event, std::uint64_t timeoutMs);
  void cleanup_peer_resume(CleanupPeer const* peer);
  void cleanup_peer_stop(CleanupPeer* peer);
}
namespace Azure { namespace Core { namespace Amqp { namespace Tests {
  class RustCleanupPeer final {
  public:
    enum Mode : std::uint32_t
    {
      HoldQuery = 1,
      ServiceError = 2
    };
    enum Event : std::uint32_t
    {
      Request = 1
    };
    explicit RustCleanupPeer(std::uint32_t mode = 0) : m_peer(cleanup_peer_start(mode)) {}
    ~RustCleanupPeer() { cleanup_peer_stop(m_peer); }
    RustCleanupPeer(RustCleanupPeer const&) = delete;
    RustCleanupPeer& operator=(RustCleanupPeer const&) = delete;
    std::uint16_t Port() const { return cleanup_peer_port(m_peer); }
    bool Wait(Event event) const { return cleanup_peer_wait(m_peer, event, 5000); }
    void Resume() { cleanup_peer_resume(m_peer); }

  private:
    CleanupPeer* m_peer;
  };

  // Each protocol regression runs in a death-test child. The watchdog exits the
  // child before a stuck synchronous FFI call or std::future destructor can hang.
  class CleanupWatchdog final {
  public:
    CleanupWatchdog()
        : m_done(m_finished.get_future()), m_thread([this] {
            if (m_done.wait_for(std::chrono::seconds(15)) != std::future_status::ready)
            {
              std::_Exit(124);
            }
          })
    {
    }
    ~CleanupWatchdog()
    {
      m_finished.set_value();
      m_thread.join();
    }

  private:
    std::promise<void> m_finished;
    std::future<void> m_done;
    std::thread m_thread;
  };
}}}} // namespace Azure::Core::Amqp::Tests
