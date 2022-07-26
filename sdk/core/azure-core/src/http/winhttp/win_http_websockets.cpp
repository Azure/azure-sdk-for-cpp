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
#include <shared_mutex>

namespace Azure { namespace Core { namespace Http { namespace WebSockets {

  void WinHttpWebSocketTransport::OnUpgradedConnection(
      Azure::Core::Http::_detail::unique_HINTERNET const& requestHandle)
  {
    // Convert the request handle into a WebSocket handle for us to use later.
    m_socketHandle = Azure::Core::Http::_detail::unique_HINTERNET(
        WinHttpWebSocketCompleteUpgrade(requestHandle.get(), 0),
        Azure::Core::Http::_detail::HINTERNET_deleter{});
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

  /**
   * @brief  Close the WebSocket cleanly.
   */
  void WinHttpWebSocketTransport::NativeClose() { m_socketHandle.reset(); }

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

    auto err = WinHttpWebSocketClose(
        m_socketHandle.get(),
        status,
        disconnectReason.empty()
            ? nullptr
            : reinterpret_cast<PVOID>(const_cast<char*>(disconnectReason.c_str())),
        static_cast<DWORD>(disconnectReason.size()));
    if (err != 0)
    {
      GetErrorAndThrow("WinHttpWebSocketClose() failed", err);
    }

    context.ThrowIfCancelled();

    // Make sure that the server responds gracefully to the close request.
    auto closeInformation = NativeGetCloseSocketInformation(context);

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
    context.ThrowIfCancelled();
    uint16_t closeStatus = 0;
    char closeReason[WINHTTP_WEB_SOCKET_MAX_CLOSE_REASON_LENGTH]{};
    DWORD closeReasonLength;

    auto err = WinHttpWebSocketQueryCloseStatus(
        m_socketHandle.get(),
        &closeStatus,
        closeReason,
        WINHTTP_WEB_SOCKET_MAX_CLOSE_REASON_LENGTH,
        &closeReasonLength);
    if (err != 0)
    {
      GetErrorAndThrow("WinHttpGetCloseStatus() failed", err);
    }
    return NativeWebSocketCloseInformation{closeStatus, std::string(closeReason)};
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
    // Lock the socket to prevent concurrent writes. WinHTTP gets annoyed if
    // there are multiple WinHttpWebSocketSend requests outstanding.
    std::lock_guard<std::mutex> lock(m_sendMutex);
    auto err = WinHttpWebSocketSend(
        m_socketHandle.get(),
        bufferType,
        reinterpret_cast<PVOID>(const_cast<uint8_t*>(frameData.data())),
        static_cast<DWORD>(frameData.size()));
    if (err != 0)
    {
      GetErrorAndThrow("WinHttpWebSocketSend() failed", err);
    }
  }

  WinHttpWebSocketTransport::NativeWebSocketReceiveInformation
  WinHttpWebSocketTransport::NativeReceiveFrame(Azure::Core::Context const& context)
  {
    WINHTTP_WEB_SOCKET_BUFFER_TYPE bufferType;
    NativeWebSocketFrameType frameTypeReceived;
    DWORD bufferBytesRead;
    std::vector<uint8_t> buffer(128);
    context.ThrowIfCancelled();
    std::lock_guard<std::mutex> lock(m_receiveMutex);

    auto err = WinHttpWebSocketReceive(
        m_socketHandle.get(),
        reinterpret_cast<PVOID>(buffer.data()),
        static_cast<DWORD>(buffer.size()),
        &bufferBytesRead,
        &bufferType);
    if (err != 0 && err != ERROR_INSUFFICIENT_BUFFER)
    {
      GetErrorAndThrow("WinHttpWebSocketReceive() failed", err);
    }
    buffer.resize(bufferBytesRead);

    switch (bufferType)
    {
      case WINHTTP_WEB_SOCKET_UTF8_MESSAGE_BUFFER_TYPE:
        frameTypeReceived = NativeWebSocketFrameType::Text;
        break;
      case WINHTTP_WEB_SOCKET_BINARY_MESSAGE_BUFFER_TYPE:
        frameTypeReceived = NativeWebSocketFrameType::Binary;
        break;
      case WINHTTP_WEB_SOCKET_BINARY_FRAGMENT_BUFFER_TYPE:
        frameTypeReceived = NativeWebSocketFrameType::BinaryFragment;
        break;
      case WINHTTP_WEB_SOCKET_UTF8_FRAGMENT_BUFFER_TYPE:
        frameTypeReceived = NativeWebSocketFrameType::TextFragment;
        break;
      case WINHTTP_WEB_SOCKET_CLOSE_BUFFER_TYPE:
        frameTypeReceived = NativeWebSocketFrameType::Closed;
        break;
      default:
        throw std::runtime_error("Unknown frame type: " + std::to_string(bufferType));
        break;
    }
    return NativeWebSocketReceiveInformation{frameTypeReceived, buffer};
  }

}}}} // namespace Azure::Core::Http::WebSockets
