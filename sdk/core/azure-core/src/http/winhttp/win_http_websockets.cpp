// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/http.hpp"
#include "azure/core/http/policies/policy.hpp"
#include "azure/core/http/transport.hpp"
#include "azure/core/http/websockets/win_http_websockets_transport.hpp"
#include "azure/core/internal/diagnostics/log.hpp"
#include "azure/core/platform.hpp"
#include "azure/core/request_failed_exception.hpp"

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
#include <shared_mutex>

namespace Azure { namespace Core { namespace Http { namespace WebSockets {

  void WinHttpWebSocketTransport::OnUpgradedConnection(
      std::unique_ptr<_detail::WinHttpRequest> const& request) const
  {
    // TODO: Need to get HINTERNET handle from WinHttpRequest
    // For now, this is a placeholder implementation to fix compilation
    // The actual implementation needs to:
    // 1. Get HINTERNET handle from request object
    // 2. Call WinHttpWebSocketCompleteUpgrade
    // 3. Store the WebSocket handle
    
    throw Azure::Core::RequestFailedException("WebSocket upgrade not yet implemented on Windows");
  }

  std::unique_ptr<Azure::Core::Http::RawResponse> WinHttpWebSocketTransport::Send(
      Azure::Core::Http::Request& request,
      Azure::Core::Context const& context)
  {
    return WinHttpTransport::Send(request, context);
  }

  /**
   * @brief  Close the WebSocket cleanly.
   */
  void WinHttpWebSocketTransport::Close() { 
    // TODO: Implement proper WebSocket close when Windows integration is complete
    // m_socketHandle.reset(); 
  }

  // Native WebSocket support methods.
  /**
   * @brief Gracefully closes the WebSocket, notifying the remote node of the close reason.
   *
   * @details Not implemented for CURL websockets because CURL does not support native websockets.
   *
   * @param status Status value to be sent to the remote node. Application defined.
   * @param disconnectReason UTF-8 encoded reason for the disconnection. Optional.
   * @param context Context for the operation.
   *
   */
  void WinHttpWebSocketTransport::NativeCloseSocket(
      uint16_t status,
      std::string const& disconnectReason,
      Azure::Core::Context const& context)
  {
    context.ThrowIfCancelled();

    // TODO: Windows WebSocket API integration needed
    // auto err = WinHttpWebSocketClose(
    //     m_socketHandle.get(),
    //     status,
    //     disconnectReason.empty()
    //         ? nullptr
    //         : reinterpret_cast<PVOID>(const_cast<char*>(disconnectReason.c_str())),
    //     static_cast<DWORD>(disconnectReason.size()));
    // if (err != 0)
    // {
    //   throw Azure::Core::RequestFailedException("WinHttpWebSocketClose() failed");
    // }
    throw Azure::Core::RequestFailedException("Windows WebSocket native close not yet implemented");

    context.ThrowIfCancelled();

    // TODO: Windows WebSocket API integration needed  
    // auto closeInformation = NativeGetCloseSocketInformation(context);
    (void)context;

    // The server should return the same status we sent.
    if (closeInformation.CloseReason != status)
    {
      throw std::runtime_error(
          "Close status mismatch, got " + std::to_string(closeInformation.CloseReason)
          + " expected " + std::to_string(status));
    }
  }
  /**
   * @brief Retrieve the information associated with a WebSocket close response.
   *
   * Should only be called when a Receive operation returns WebSocketFrameType::CloseFrameType
   *
   * @param context Context for the operation.
   *
   * @returns a tuple containing the status code and string.
   */
  WinHttpWebSocketTransport::NativeWebSocketCloseInformation
  WinHttpWebSocketTransport::NativeGetCloseSocketInformation(Azure::Core::Context const& context)
  {
    (void)context;
    // TODO: Windows WebSocket API integration needed
    throw Azure::Core::RequestFailedException("Windows WebSocket get close info not yet implemented");
  }
  }

  /**
   * @brief Send a frame of data to the remote node.
   *
   * @details Not implemented for CURL websockets because CURL does not support native
   * websockets.
   *
   * @brief frameType Frame type sent to the server, Text or Binary.
   * @brief frameData Frame data to be sent to the server.
   */
  void WinHttpWebSocketTransport::NativeSendFrame(
      NativeWebSocketFrameType frameType,
      std::vector<uint8_t> const& frameData,
      Azure::Core::Context const& context)
  {
    context.ThrowIfCancelled();
    WINHTTP_WEB_SOCKET_BUFFER_TYPE bufferType;
    switch (frameType)
    {
      case NativeWebSocketFrameType::Text:
        bufferType = WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE;
        break;
      case NativeWebSocketFrameType::Binary:
        bufferType = WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE;
        break;
      case NativeWebSocketFrameType::BinaryFragment:
        bufferType = WINHTTP_WEB_SOCKET_BINARY_FRAGMENT_BUFFER_TYPE;
        break;
      case NativeWebSocketFrameType::TextFragment:
        bufferType = WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE;
        break;
      default:
        throw std::runtime_error(
            "Unknown frame type: " + std::to_string(static_cast<uint32_t>(frameType)));
        break;
    }
    // TODO: Windows WebSocket API integration needed
    (void)frameType;
    (void)frameData;
    (void)context;
    throw Azure::Core::RequestFailedException("Windows WebSocket send frame not yet implemented");
  }

  WinHttpWebSocketTransport::NativeWebSocketReceiveInformation
  WinHttpWebSocketTransport::NativeReceiveFrame(Azure::Core::Context const& context)
  {
    (void)context;
    // TODO: Windows WebSocket API integration needed
    throw Azure::Core::RequestFailedException("Windows WebSocket receive frame not yet implemented");
  }

}}}} // namespace Azure::Core::Http::WebSockets
