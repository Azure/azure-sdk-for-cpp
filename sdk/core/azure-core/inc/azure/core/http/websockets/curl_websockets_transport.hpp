// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief #Azure::Core::Http::WebSockets::WebSocketTransport implementation via CURL.
 */

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/http/curl_transport.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/transport.hpp"
#include "azure/core/http/websockets/websockets_transport.hpp"
#include <memory>

namespace Azure { namespace Core { namespace Http { namespace WebSockets {

  struct CurlWebSocketTransportOptions : public Azure::Core::Http::CurlTransportOptions
  {
  };
  /**
   * @brief Concrete implementation of a WebSocket Transport that uses libcurl.
   */
  class CurlWebSocketTransport : public CurlTransport, public WebSocketTransport {
  public:
    /**
     * @brief Construct a new CurlWebSocketTransport object.
     *
     * @param options Optional parameter to override the default options.
     */
    CurlWebSocketTransport(
        CurlWebSocketTransportOptions const& options = CurlWebSocketTransportOptions())
        : CurlTransport(options)
    {
    }
    /**
     * @brief Construct a new CurlWebSocketTransport object.
     *
     * @param options Optional parameter to override the default options.
     */
    CurlWebSocketTransport(Azure::Core::Http::Policies::TransportOptions const& options = {})
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
     * @brief Indicates if the transport natively supports websockets or not.
     *
     * @details For the CURL websocket transport, the transport does NOT support native
     * websockets - it is the responsibility of the client of the WebSocketTransport to format
     * WebSocket protocol elements.
     */
    virtual bool HasBuiltInWebSocketSupport() override { return false; }

    /**
     * @brief Closes the WebSocket handle.
     *
     */
    virtual void Close() override;

    // Native WebSocket support methods.
    /**
     * @brief Gracefully closes the WebSocket, notifying the remote node of the close reason.
     *
     * @details Not implemented for CURL websockets because CURL does not support native
     * websockets.
     *
     * The first param is the close reason, the second is descriptive text.
     */
    virtual void NativeCloseSocket(uint16_t, std::string const&, Azure::Core::Context const&)
        override
    {
      throw std::runtime_error("Not implemented.");
    }

    /**
     * @brief Retrieve the status of the close socket operation.
     *
     * @details Not implemented for CURL websockets because CURL does not support native
     * websockets.
     *
     */
    NativeWebSocketCloseInformation NativeGetCloseSocketInformation(
        const Azure::Core::Context&) override
    {
      throw std::runtime_error("Not implemented");
    }

    /**
     * @brief Send a frame of data to the remote node.
     *
     * @details Not implemented for CURL websockets because CURL does not support native
     * websockets.
     *
     */
    virtual void NativeSendFrame(
        NativeWebSocketFrameType,
        std::vector<uint8_t> const&,
        Azure::Core::Context const&) override
    {
      throw std::runtime_error("Not implemented.");
    }

    /**
     * @brief Receive a frame of data from the remote node.
     *
     * @details Not implemented for CURL websockets because CURL does not support native
     * websockets.
     *
     */
    virtual NativeWebSocketReceiveInformation NativeReceiveFrame(
        Azure::Core::Context const&) override
    {
      throw std::runtime_error("Not implemented");
    }

    // Non-Native WebSocket support.
    /**
     * @brief This function is used when working with streams to pull more data from the wire.
     * Function will try to keep pulling data from socket until the buffer is all written or
     * until there is no more data to get from the socket.
     *
     * @param buffer Buffer to fill with data.
     * @param bufferSize Size of buffer.
     * @param context Context to control the request lifetime.
     *
     * @returns Buffer data received.
     *
     */
    virtual size_t ReadFromSocket(uint8_t* buffer, size_t bufferSize, Context const& context)
        override;

    /**
     * @brief This method will use libcurl socket to write all the bytes from buffer.
     *
     * @param buffer Buffer to send.
     * @param bufferSize Number of bytes to write.
     * @param context Context for the operation.
     */
    virtual int SendBuffer(uint8_t const* buffer, size_t bufferSize, Context const& context)
        override;

    /**
     * @brief returns true if this transport supports WebSockets, false otherwise.
     */
    bool HasWebSocketSupport() const override { return true; }

  private:
    // std::unique_ptr cannot be constructed on an incomplete type (CurlNetworkConnection), but
    // std::shared_ptr can be.
    std::shared_ptr<Azure::Core::Http::CurlNetworkConnection> m_upgradedConnection;
    void OnUpgradedConnection(
        std::unique_ptr<Azure::Core::Http::CurlNetworkConnection>&& upgradedConnection) override;
  };
}}}} // namespace Azure::Core::Http::WebSockets
