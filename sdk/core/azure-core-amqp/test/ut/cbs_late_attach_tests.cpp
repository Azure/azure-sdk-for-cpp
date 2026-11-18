// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Regression tests for issue #7557: a CBS open that ends before the service's ATTACH reply is
// processed must not leave an ATTACHED link endpoint without a frame callback.

#include <azure/core/amqp/internal/claims_based_security.hpp>
#include <azure/core/amqp/internal/connection.hpp>
#include <azure/core/amqp/internal/message_sender.hpp>
#include <azure/core/amqp/internal/session.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/platform.hpp>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#if ENABLE_UAMQP && !defined(AZ_PLATFORM_MAC) && !defined(AZ_PLATFORM_WINDOWS)
#include "mock_amqp_server.hpp"

#include <poll.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

namespace Azure { namespace Core { namespace Amqp { namespace Tests {
  namespace {
    struct HeldFrame
    {
      std::size_t Offset;
      std::size_t Size;
      std::string Performative;
    };

    // Splits held bytes into AMQP frames. The relay holds only after the session is mapped and
    // the connection is quiet, so the held bytes start on a frame boundary.
    std::vector<HeldFrame> SplitFrames(std::vector<char> const& bytes)
    {
      auto const byteAt = [&bytes](std::size_t index) {
        return static_cast<std::uint32_t>(static_cast<unsigned char>(bytes[index]));
      };
      std::vector<HeldFrame> frames;
      std::size_t offset = 0;
      while (offset + 8 <= bytes.size())
      {
        std::uint32_t const size = (byteAt(offset) << 24) | (byteAt(offset + 1) << 16)
            | (byteAt(offset + 2) << 8) | byteAt(offset + 3);
        if (size < 8 || offset + size > bytes.size())
        {
          break;
        }
        std::size_t const body = offset + byteAt(offset + 4) * 4;
        std::string name{"empty"};
        if (body + 3 <= offset + size && byteAt(body) == 0x00 && byteAt(body + 1) == 0x53)
        {
          static char const* const names[]
              = {"OPEN",
                 "BEGIN",
                 "ATTACH",
                 "FLOW",
                 "TRANSFER",
                 "DISPOSITION",
                 "DETACH",
                 "END",
                 "CLOSE"};
          auto const code = byteAt(body + 2);
          name = (code >= 0x10 && code <= 0x18) ? names[code - 0x10] : "other";
        }
        frames.push_back({offset, size, name});
        offset += size;
      }
      return frames;
    }

    // A one-connection TCP relay between the client and the mock server. While it holds, it
    // forwards client bytes at once but keeps every server byte back. That lets the client give
    // up on a CBS open while the service's ATTACH reply is already written but not yet read.
    class HoldingRelay final {
    public:
      explicit HoldingRelay(std::uint16_t serverPort) : m_serverPort{serverPort}
      {
        m_listenFd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (m_listenFd < 0)
        {
          throw std::runtime_error("relay: socket failed");
        }
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        socklen_t length = sizeof(address);
        if (::bind(m_listenFd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0
            || ::listen(m_listenFd, 1) != 0
            || ::getsockname(m_listenFd, reinterpret_cast<sockaddr*>(&address), &length) != 0)
        {
          ::close(m_listenFd);
          throw std::runtime_error("relay: bind or listen failed");
        }
        m_port = ntohs(address.sin_port);
        m_thread = std::thread([this]() { Run(); });
      }

      ~HoldingRelay()
      {
        m_stop = true;
        if (m_thread.joinable())
        {
          m_thread.join();
        }
        for (int fd : {m_listenFd, m_clientFd, m_serverFd})
        {
          if (fd >= 0)
          {
            ::close(fd);
          }
        }
      }

      HoldingRelay(HoldingRelay const&) = delete;
      HoldingRelay& operator=(HoldingRelay const&) = delete;

      std::uint16_t GetPort() const { return m_port; }
      void Hold() { m_hold = true; }
      void Release() { m_hold = false; }

      std::size_t HeldBytes()
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_held.size();
      }

      std::string HeldPerformatives()
      {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string names;
        for (auto const& frame : SplitFrames(m_held))
        {
          names += frame.Performative + " ";
        }
        return names;
      }

    private:
      static bool SendAll(int fd, char const* data, std::size_t size)
      {
        while (size > 0)
        {
          auto const sent = ::send(fd, data, size, MSG_NOSIGNAL);
          if (sent <= 0)
          {
            return false;
          }
          data += sent;
          size -= static_cast<std::size_t>(sent);
        }
        return true;
      }

      void Run()
      {
        pollfd acceptPoll{m_listenFd, POLLIN, 0};
        while (!m_stop && ::poll(&acceptPoll, 1, 50) <= 0)
        {
        }
        if (m_stop)
        {
          return;
        }
        m_clientFd = ::accept(m_listenFd, nullptr, nullptr);
        m_serverFd = ::socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in server{};
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        server.sin_port = htons(m_serverPort);
        if (m_clientFd < 0 || m_serverFd < 0
            || ::connect(m_serverFd, reinterpret_cast<sockaddr*>(&server), sizeof(server)) != 0)
        {
          return;
        }

        char buffer[16384];
        while (!m_stop)
        {
          if (!m_hold)
          {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_held.empty())
            {
              if (!SendAll(m_clientFd, m_held.data(), m_held.size()))
              {
                return;
              }
              m_held.clear();
            }
          }
          pollfd fds[2]{{m_clientFd, POLLIN, 0}, {m_serverFd, POLLIN, 0}};
          if (::poll(fds, 2, 20) <= 0)
          {
            continue;
          }
          if (fds[0].revents & (POLLIN | POLLHUP | POLLERR))
          {
            auto const read = ::recv(m_clientFd, buffer, sizeof(buffer), 0);
            if (read <= 0 || !SendAll(m_serverFd, buffer, static_cast<std::size_t>(read)))
            {
              return;
            }
          }
          if (fds[1].revents & (POLLIN | POLLHUP | POLLERR))
          {
            auto const read = ::recv(m_serverFd, buffer, sizeof(buffer), 0);
            if (read <= 0)
            {
              return;
            }
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_hold || !m_held.empty())
            {
              m_held.insert(m_held.end(), buffer, buffer + read);
            }
            else if (!SendAll(m_clientFd, buffer, static_cast<std::size_t>(read)))
            {
              return;
            }
          }
        }
      }

      std::uint16_t m_serverPort;
      std::uint16_t m_port{};
      int m_listenFd{-1};
      int m_clientFd{-1};
      int m_serverFd{-1};
      std::atomic<bool> m_hold{false};
      std::atomic<bool> m_stop{false};
      std::mutex m_mutex;
      std::vector<char> m_held;
      std::thread m_thread;
    };

    class StaticTokenCredential final : public Azure::Core::Credentials::TokenCredential {
    public:
      StaticTokenCredential() : Azure::Core::Credentials::TokenCredential("StaticToken") {}
      Azure::Core::Credentials::AccessToken GetToken(
          Azure::Core::Credentials::TokenRequestContext const&,
          Azure::Core::Context const&) const override
      {
        Azure::Core::Credentials::AccessToken token;
        token.Token = "static-test-token";
        token.ExpiresOn = std::chrono::system_clock::now() + std::chrono::hours(1);
        return token;
      }
    };

    bool WaitForHeldBytes(HoldingRelay& relay, std::chrono::milliseconds timeout)
    {
      auto const deadline = std::chrono::steady_clock::now() + timeout;
      while (std::chrono::steady_clock::now() < deadline)
      {
        if (relay.HeldBytes() > 0)
        {
          return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
      }
      return false;
    }
  } // namespace

  using namespace Azure::Core::Amqp::_internal;
  using namespace Azure::Core::Amqp::_detail;

  // The production path: a sender open authenticates its audience through
  // ConnectionImpl::AuthenticateAudience and PutTokenForAudience, the same CBS open and cleanup
  // that a token refresh runs. The caller's deadline ends the CBS open while the ATTACH reply and
  // the FLOW that grants credit to $cbs-sender are held in the relay. When the relay lets them
  // through, the session sees an ATTACH for a link that it has already freed.
  TEST(TestCbsLateAttach, LateAttachAndFlowAfterCancelledCbsOpen)
  {
    MessageTests::AmqpServerMock mockServer;
    mockServer.StartListening();
    HoldingRelay relay(mockServer.GetPort());

    {
      ConnectionOptions options;
      options.ContainerId = "cbs-late-attach";
      options.EnableTrace = true;
      options.Port = relay.GetPort();
      Connection connection("localhost", std::make_shared<StaticTokenCredential>(), options);
      Session session{connection.CreateSession()};

      // The long-lived sender of a producer. Its open runs the first CBS cycle, and its open link
      // keeps the connection on the polling thread, so late frames are read when they arrive.
      // The mock routes a link by its target, and "$cbs" is a target that it accepts.
      MessageSenderOptions telemetryOptions;
      telemetryOptions.Name = "telemetry-sender";
      telemetryOptions.MessageSource = "telemetry-source";
      telemetryOptions.EnableTrace = true;
      MessageSender telemetrySender{session.CreateMessageSender("$cbs", telemetryOptions, nullptr)};
      ASSERT_FALSE(telemetrySender.Open());

      relay.Hold();
      {
        MessageSender sender{session.CreateMessageSender("late-attach-entity", {}, nullptr)};
        Azure::Core::Context openContext{Azure::Core::Context{}.WithDeadline(
            std::chrono::system_clock::now() + std::chrono::seconds(3))};
        bool openFailed = false;
        try
        {
          auto const openError = sender.Open(openContext);
          openFailed = static_cast<bool>(openError);
        }
        catch (std::exception const& ex)
        {
          openFailed = true;
          GTEST_LOG_(INFO) << "Sender open failed as expected: " << ex.what();
        }
        EXPECT_TRUE(openFailed) << "The held relay must make the CBS open miss its deadline.";
      }
      ASSERT_TRUE(WaitForHeldBytes(relay, std::chrono::seconds(5)))
          << "The service never answered the CBS ATTACH frames.";
      GTEST_LOG_(INFO) << "Releasing " << relay.HeldBytes()
                       << " held bytes to the client: " << relay.HeldPerformatives();

      // Before a fix, a FLOW in these bytes calls a NULL frame callback at session.c:694 on the
      // polling thread, and the process stops with SIGSEGV.
      relay.Release();
      std::this_thread::sleep_for(std::chrono::seconds(2));

      // The session must still attach a new link after the late frames. The mock keeps its side
      // of the cancelled $cbs-sender link, so it cannot serve a second CBS open, and this link
      // skips authentication.
      {
        MessageSenderOptions laterOptions;
        laterOptions.Name = "later-sender";
        laterOptions.MessageSource = "later-source";
        laterOptions.AuthenticationRequired = false;
        MessageSender laterSender{session.CreateMessageSender("$cbs", laterOptions, nullptr)};
        EXPECT_FALSE(laterSender.Open());
        laterSender.Close();
      }
      telemetrySender.Close();
    }
    mockServer.StopListening();
  }
}}}} // namespace Azure::Core::Amqp::Tests
#endif
