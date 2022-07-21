// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief #Azure::Core::Http::WebSockets::WebSocketTransport implementation via WInHTTP.
 */

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/transport.hpp"
#include "azure/core/http/websockets/websockets_transport.hpp"
#include "azure/core/http/win_http_transport.hpp"
#include <memory>
#include <mutex>

namespace Azure { namespace Core { namespace Http { namespace WebSockets {

  /**
   * @brief Concrete implementation of a WebSocket Transport that uses WinHTTP.
   */
  class WinHttpWebSocketTransport : public WebSocketTransport, public WinHttpTransport {

    Azure::Core::Http::_detail::unique_HINTERNET m_socketHandle;
    std::mutex m_sendMutex;
    std::mutex m_receiveMutex;

    // Called by the
    void OnResponseReceived(Azure::Core::Http::_detail::unique_HINTERNET& requestHandle) override;

  public:
    /**
     * @brief Construct a new WinHTTP WebSocket Transport.
     *
     * @param options Optional parameter to override the default options.
     */
    WinHttpWebSocketTransport(WinHttpTransportOptions const& options = WinHttpTransportOptions())
        : WinHttpTransport(options)
    {
    }
    /**
     * @brief Implements interface to send an HTTP Request and produce an HTTP RawResponse
     *
     * @param request an HTTP Request to be send.
     * @param context A context to control the request lifetime.
     *
     * @return unique ptr to an HTTP RawResponse.
     */
    virtual std::unique_ptr<RawResponse> Send(Request& request, Context const& context) override;

    /**
     * @brief Indicates if the transports supports native websockets or not.
     *
     * @details For the WinHTTP websocket transport, the transport supports native websockets.
     */
    virtual bool HasNativeWebsocketSupport() override { return true; }

    /**
     * @brief Gracefully closes the WebSocket, notifying the remote node of the close reason.
     *
     */
    virtual void NativeClose() override;

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
    virtual void NativeCloseSocket(uint16_t, std::string const&, Azure::Core::Context const&)
        override;

    /**
     * @brief Retrieve the information associated with a WebSocket close response.
     *
     * Should only be called when a Receive operation returns WebSocketFrameType::CloseFrameType
     *
     * @param context Context for the operation.
     *
     * @returns a tuple containing the status code and string.
     */
    virtual NativeWebSocketCloseInformation NativeGetCloseSocketInformation(
        Azure::Core::Context const& context) override;

    /**
     * @brief Send a frame of data to the remote node.
     *
     * @details Not implemented for CURL websockets because CURL does not support native
     * websockets.
     *
     * @brief frameType Frame type sent to the server, Text or Binary.
     * @brief frameData Frame data to be sent to the server.
     */
    virtual void NativeSendFrame(
        NativeWebSocketFrameType,
        std::vector<uint8_t> const&,
        Azure::Core::Context const&) override;

    virtual NativeWebSocketReceiveInformation NativeReceiveFrame(
        Azure::Core::Context const&) override;

    // Non-Native WebSocket support.
    /**
     * @brief This function is used when working with streams to pull more data from the wire.
     * Function will try to keep pulling data from socket until the buffer is all written or
     * until there is no more data to get from the socket.
     *
     * @details Not implemented for WinHTTP websockets because WinHTTP implements websockets
     * natively.
     */
    virtual size_t ReadFromSocket(uint8_t*, size_t, Context const&) override
    {
      throw std::runtime_error("Not implemented.");
    }

    /**
     * @brief This method will use sockets to write all the bytes from buffer.
     *
     * @details Not implemented for WinHTTP websockets because WinHTTP implements websockets
     * natively.
     *
     */
    virtual int SendBuffer(uint8_t const*, size_t, Context const&) override
    {
      throw std::runtime_error("Not implemented.");
    }

    bool SupportsWebSockets() const override { return true; }
  };

}}}} // namespace Azure::Core::Http::WebSockets
