// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief #Azure::Core::Http::HttpTransport implementation via CURL.
 */

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/http/curl_transport.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/transport.hpp"
#include "azure/core/http/websockets/websockets_transport.hpp"
#include <memory>

namespace Azure { namespace Core { namespace Http { namespace WebSockets {

  /**
   * @brief Concrete implementation of a WebSocket Transport that uses libcurl.
   */
  class CurlWebSocketTransport : public WebSocketTransport, public CurlTransport {
    // std::unique_ptr cannot be constructed on an incomplete type (CurlNetworkConnection), but
    // std::shared_ptr can be.
    std::shared_ptr<Azure::Core::Http::CurlNetworkConnection> m_upgradedConnection;
    void OnUpgradedConnection(
        std::unique_ptr<Azure::Core::Http::CurlNetworkConnection>& upgradedConnection) override;

  public:
    /**
     * @brief Construct a new CurlTransport object.
     *
     * @param options Optional parameter to override the default options.
     */
    CurlWebSocketTransport(CurlTransportOptions const& options = CurlTransportOptions())
        : CurlTransport(options)
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
     * @detail For the CURL websocket transport, the transport does NOT support native websockets -
     * it is the responsibility of the client of the WebSocketTransport to format WebSocket protocol
     * elements.
     */
    virtual bool NativeWebsocketSupport() override { return false; }

    virtual void CompleteUpgrade() override;

    /**
     * @brief Gracefully closes the WebSocket, notifying the remote node of the close reason.
     *
     */
    virtual void Close() override;

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
    virtual void CloseSocket(uint16_t, std::string const&, Azure::Core::Context const&) override
    {
      throw std::runtime_error("Not implemented.");
    }

    /**
     * @brief Send a frame of data to the remote node.
     *
     * @detail Not implemented for CURL websockets because CURL does not support native websockets.
     *
     * @brief frameType Frame type sent to the server, Text or Binary.
     * @brief frameData Frame data to be sent to the server.
     */
    virtual void SendFrame(WebSocketFrameType, std::vector<uint8_t>, Azure::Core::Context const&)
        override
    {
      throw std::runtime_error("Not implemented.");
    }

    // Non-Native WebSocket support.
    /**
     * @brief This function is used when working with streams to pull more data from the wire.
     * Function will try to keep pulling data from socket until the buffer is all written or until
     * there is no more data to get from the socket.
     *
     */
    virtual size_t ReadFromSocket(uint8_t* buffer, size_t bufferSize, Context const& context)
        override;

    /**
     * @brief This method will use libcurl socket to write all the bytes from buffer.
     *
     */
    virtual int SendBuffer(uint8_t const* buffer, size_t bufferSize, Context const& context)
        override;
  };

}}}} // namespace Azure::Core::Http::WebSockets
