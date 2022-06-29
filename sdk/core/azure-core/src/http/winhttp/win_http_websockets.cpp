// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/http.hpp"
#include "azure/core/http/policies/policy.hpp"
#include "azure/core/http/transport.hpp"
#include "azure/core/http/websockets/win_http_websockets_transport.hpp"
#include "azure/core/internal/diagnostics/log.hpp"
#include "azure/core/platform.hpp"

#if defined(AZ_PLATFORM_POSIX)
#include <poll.h> // for poll()
#include <sys/socket.h> // for socket shutdown
#elif defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <winapifamily.h>
#include <winsock2.h> // for WSAPoll();
#endif

namespace Azure { namespace Core { namespace Http { namespace WebSockets {

  void WinHttpWebSocketTransport::OnResponseReceived(
      Azure::Core::Http::_detail::unique_HINTERNET& requestHandle)
  {
    // Convert the request handle into a WebSocket handle for us to use later.
    m_socketHandle = Azure::Core::Http::_detail::unique_HINTERNET(
        WinHttpWebSocketCompleteUpgrade(requestHandle.get(), 0),
        Azure::Core::Http::_detail::HINTERNET_deleter());
    if (!m_socketHandle)
    {
      GetErrorAndThrow("Error Upgrading HttpRequest handle to WebSocket handle.");
    }
  }

  std::unique_ptr<Azure::Core::Http::RawResponse> WinHttpWebSocketTransport::Send(
      Azure::Core::Http::Request& request,
      Azure::Core::Context const& context)
  {
    return WinHttpTransport::Send(request, context);
  }

  void WinHttpWebSocketTransport::CompleteUpgrade() {}

  // Close the WebSocket operation cleanly. Does not specify
  void WinHttpWebSocketTransport::Close()
  {
    auto err = WinHttpWebSocketClose(
        m_socketHandle.get(), WINHTTP_WEB_SOCKET_ENDPOINT_TERMINATED_CLOSE_STATUS, 0, 0);
    if (err != 0)
    {
      GetErrorAndThrow("WinHttpWebSocketClose() failed", err);
    }
  }

  // Native WebSocket support methods.
  /**
   * @brief Gracefully closes the WebSocket, notifying the remote node of the close reason.
   *
   * @detail Not implemented for CURL websockets because CURL does not support native websockets.
   *
   * @param status Status value to be sent to the remote node. Application defined.
   * @param disconnectReason UTF-8 encoded reason for the disconnection. Optional.
   * @param context Context for the operation.
   *
   */
  void WinHttpWebSocketTransport::CloseSocket(
      uint16_t status,
      std::string const& disconnectReason,
      Azure::Core::Context const& context)
  {
    context.ThrowIfCancelled();

    auto err = WinHttpWebSocketClose(
        m_socketHandle.get(),
        status,
        reinterpret_cast<PVOID>(const_cast<char*>(disconnectReason.c_str())),
        static_cast<DWORD>(disconnectReason.size()));
    if (err != 0)
    {
      GetErrorAndThrow("WinHttpWebSocketClose() failed", err);
    }
  }

  /**
   * @brief Send a frame of data to the remote node.
   *
   * @detail Not implemented for CURL websockets because CURL does not support native
   * websockets.
   *
   * @brief frameType Frame type sent to the server, Text or Binary.
   * @brief frameData Frame data to be sent to the server.
   */
  void WinHttpWebSocketTransport::SendFrame(
      WebSocketFrameType,
      std::vector<uint8_t>,
      Azure::Core::Context const&)
  {
    throw std::runtime_error("Not implemented");
  }

}}}} // namespace Azure::Core::Http::WebSockets
