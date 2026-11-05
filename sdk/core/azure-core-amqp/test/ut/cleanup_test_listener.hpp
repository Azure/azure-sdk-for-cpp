// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/amqp/internal/common/global_state.hpp>
#include <azure/core/platform.hpp>

#if defined(AZ_PLATFORM_WINDOWS)
#include <winsock2.h>
#else
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include <cstdint>
#include <stdexcept>

namespace Azure { namespace Core { namespace Amqp { namespace Tests {
  // Allows Begin to connect over TCP without starting a peer AMQP session. Cleanup
  // fault tests do not need peer callbacks, and must not race a mock server's session teardown.
  class CleanupTestListener final {
  public:
    CleanupTestListener()
    {
      Common::_detail::GlobalStateHolder::GlobalStateInstance();
      m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      if (m_socket == InvalidSocket)
      {
        throw std::runtime_error("Could not create cleanup test listener");
      }
      sockaddr_in address{};
      address.sin_family = AF_INET;
      address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
#if defined(AZ_PLATFORM_WINDOWS)
      int size = sizeof(address);
#else
      socklen_t size = sizeof(address);
#endif
      if (bind(m_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0
          || listen(m_socket, 1) != 0
          || getsockname(m_socket, reinterpret_cast<sockaddr*>(&address), &size) != 0)
      {
        Close();
        throw std::runtime_error("Could not listen on cleanup test port");
      }
      m_port = ntohs(address.sin_port);
    }

    ~CleanupTestListener() { Close(); }
    CleanupTestListener(CleanupTestListener const&) = delete;
    CleanupTestListener& operator=(CleanupTestListener const&) = delete;
    std::uint16_t GetPort() const { return m_port; }

  private:
    using Socket = decltype(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    static constexpr Socket InvalidSocket = static_cast<Socket>(-1);
    Socket m_socket{InvalidSocket};
    std::uint16_t m_port{};

    void Close()
    {
      if (m_socket != InvalidSocket)
      {
#if defined(AZ_PLATFORM_WINDOWS)
        closesocket(m_socket);
#else
        close(m_socket);
#endif
        m_socket = InvalidSocket;
      }
    }
  };
}}}} // namespace Azure::Core::Amqp::Tests
