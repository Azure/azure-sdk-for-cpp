// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Utilities to be used by HTTP WebSocket transport implementations.
 */

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/http/http.hpp"

namespace Azure { namespace Core { namespace Http { namespace WebSockets {

  /**
   * @brief Base class for all WebSocket transport implementations.
   */
  class WebSocketTransport : public HttpTransport {
  public:
    /**
     * @brief Web Socket Frame type, one of Text or Binary.
     */
    enum class WebSocketFrameType
    {
      /**
       * @brief Indicates that the frame is a partial UTF-8 encoded text frame - it is NOT the
       * complete frame to be sent to the remote node.
       */
      FrameTypeTextFragment,
      /**
       * @brief Indicates that the frame is either the complete UTF-8 encoded text frame to be sent
       * to the remote node or the final frame of a multipart message.
       */
      FrameTypeText,
      /**
       * @brief Indicates that the frame is either the complete binary frame to be sent
       * to the remote node or the final frame of a multipart message.
       */
      FrameTypeBinary,
      /**
       * @brief Indicates that the frame is a partial binary frame - it is NOT the
       * complete frame to be sent to the remote node.
       */
      FrameTypeBinaryFragment,

      FrameTypeClosed,
    };
    /**
     * @brief Destructs `%HttpTransport`.
     *
     */
    virtual ~WebSocketTransport() {}

    /**
     * @brief Determines if the transport natively supports WebSockets or not.
     *
     * @returns true iff the transport has native websocket support, false otherwise.
     */
    virtual bool NativeWebsocketSupport() = 0;
    /**
     * @brief Complete the WebSocket upgrade.
     *
     * @detail Called by the WebSocket client after the HTTP server responds with a
     * SwitchingProtocols response. This method performs whatever operations are needed to
     * transfer the protocol from HTTP to WebSockets.
     */
    virtual void CompleteUpgrade() = 0;

    /**************/
    /* Native WebSocket support functions*/
    /**************/
    /**
     * @brief Gracefully closes the WebSocket, notifying the remote node of the close reason.
     *
     * @param status Status value to be sent to the remote node. Application defined.
     * @param disconnectReason UTF-8 encoded reason for the disconnection. Optional.
     * @param context Context for the operation.
     */
    virtual void CloseSocket(
        uint16_t status,
        std::string const& disconnectReason,
        Azure::Core::Context const& context)
        = 0;

    /**
     * @brief Closes the WebSocket.
     *
     * Does not notify the remote endpoint that the socket is being closed.
     *
     */
    virtual void Close() = 0;

    /**
     * @brief Retrieve the information associated with a WebSocket close response.
     *
     * @param context Context for the operation.
     *
     * @returns a tuple containing the status code and string.
     */
    virtual std::pair<uint16_t, std::string> GetCloseSocketInformation(
        Azure::Core::Context const& context)
        = 0;

    /**
     * @brief Send a frame of data to the remote node.
     *
     * @brief frameType Frame type sent to the server, Text or Binary.
     * @brief frameData Frame data to be sent to the server.
     */
    virtual void SendFrame(
        WebSocketFrameType frameType,
        std::vector<uint8_t> const& frameData,
        Azure::Core::Context const& context)
        = 0;

    /**
     * @brief Receive a frame from the remote WebSocket server.
     *
     * @param frameTypeReceived frame type received from the remote server.
     *
     * @returns Frame data received from the remote server.
     */
    virtual std::vector<uint8_t> ReceiveFrame(
        WebSocketFrameType& frameTypeReceived,
        Azure::Core::Context const& context)
        = 0;

    /**************/
    /* Non Native WebSocket support functions */
    /**************/

    /**
     * @brief This function is used when working with streams to pull more data from the wire.
     * Function will try to keep pulling data from socket until the buffer is all written or until
     * there is no more data to get from the socket.
     *
     */
    virtual size_t ReadFromSocket(uint8_t* buffer, size_t bufferSize, Context const& context) = 0;

    /**
     * @brief This method will use the raw socket to write all the bytes from buffer.
     *
     */
    virtual int SendBuffer(uint8_t const* buffer, size_t bufferSize, Context const& context) = 0;

  protected:
    /**
     * @brief Constructs a default instance of `%HttpTransport`.
     *
     */
    WebSocketTransport() = default;

    /**
     * @brief Constructs `%HttpTransport` by copying another instance of `%HttpTransport`.
     *
     * @param other An instance to copy.
     */
    WebSocketTransport(const WebSocketTransport& other) = default;

    /**
     * @brief Constructs `%HttpTransport` by moving another instance of `%HttpTransport`.
     *
     * @param other An instance to move in.
     */
    WebSocketTransport(WebSocketTransport&& other) = default;

    /**
     * @brief Assigns `%HttpTransport` to another instance of `%HttpTransport`.
     *
     * @param other An instance to assign.
     *
     * @return A reference to this instance.
     */
    WebSocketTransport& operator=(const WebSocketTransport& other) = default;
  };

}}}} // namespace Azure::Core::Http::WebSockets
