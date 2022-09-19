// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Utilities to be used by HTTP WebSocket transport implementations.
 */

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/http/http.hpp"

namespace Azure { namespace Core { namespace Http { namespace _detail { namespace WebSockets {

  /**
   * @brief Base class for all WebSocket transport implementations.
   */
  class WebSocketTransport {
  public:
    /**
     * @brief Web Socket Frame type, one of Text or Binary.
     */
    enum class NativeWebSocketFrameType
    {
      /**
       * @brief Indicates that the frame is a partial UTF-8 encoded text frame - it is NOT the
       * complete frame to be sent to the remote node.
       */
      TextFragment,
      /**
       * @brief Indicates that the frame is either the complete UTF-8 encoded text frame to be sent
       * to the remote node or the final frame of a multipart message.
       */
      Text,
      /**
       * @brief Indicates that the frame is either the complete binary frame to be sent
       * to the remote node or the final frame of a multipart message.
       */
      Binary,
      /**
       * @brief Indicates that the frame is a partial binary frame - it is NOT the
       * complete frame to be sent to the remote node.
       */
      BinaryFragment,

      /**
       * @brief Indicates that the frame is a "close" frame - the remote node
       * sent a close frame.
       */
      Closed,
    };

    /** @brief Close information returned from a WebSocket transport that has builtin support
     * for WebSockets.
     */
    struct NativeWebSocketCloseInformation
    {
      /**
       * @brief Close response code.
       */
      uint16_t CloseReason;
      /**
       * @brief Close reason.
       */
      std::string CloseReasonDescription;
    };
    /** @brief Frame information returned from a WebSocket transport that has builtin support
     * for WebSockets.
     */
    struct NativeWebSocketReceiveInformation
    {
      /**
       * @brief Type of frame received.
       */
      NativeWebSocketFrameType FrameType;
      /**
       * @brief Data received.
       */
      std::vector<uint8_t> FrameData;
    };
    /**
     * @brief Destructs `%WebSocketTransport`.
     *
     */
    virtual ~WebSocketTransport() {}

    /**
     * @brief Indicates whether the transport natively supports WebSockets.
     *
     * @returns true if the transport has native websocket support, false otherwise.
     */
    virtual bool HasBuiltInWebSocketSupport() = 0;

    /**
     * @brief Closes the WebSocket.
     *
     * Does not notify the remote endpoint that the socket is being closed.
     *
     */
    virtual void Close() = 0;

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
    virtual void NativeCloseSocket(
        uint16_t status,
        std::string const& disconnectReason,
        Azure::Core::Context const& context)
        = 0;

    /**
     * @brief Retrieve the information associated with a WebSocket close response.
     *
     * @param context Context for the operation.
     *
     * @returns a tuple containing the status code and string.
     */
    virtual NativeWebSocketCloseInformation NativeGetCloseSocketInformation(
        Azure::Core::Context const& context)
        = 0;

    /**
     * @brief Send a frame of data to the remote node.
     *
     * @param frameType Frame type sent to the server, Text or Binary.
     * @param frameData Frame data to be sent to the server.
     * @param context Context for the operation.
     */
    virtual void NativeSendFrame(
        NativeWebSocketFrameType frameType,
        std::vector<uint8_t> const& frameData,
        Azure::Core::Context const& context)
        = 0;

    /**
     * @brief Receive a frame from the remote WebSocket server.
     *
     * @param context Context for the operation.
     *
     * @returns a tuple containing the Frame data received from the remote server and the type of
     * data returned from the remote endpoint
     */
    virtual NativeWebSocketReceiveInformation NativeReceiveFrame(
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
     * @brief Constructs a default instance of `%WebSocketTransport`.
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
     * @brief Constructs a WebSocketTransport from another WebSocketTransport.
     *
     * @param other An instance to move in.
     */
    WebSocketTransport(WebSocketTransport&& other) = default;

    /**
     * @brief Assigns one WebSocketTransport to another.
     *
     * @param other An instance to assign.
     *
     * @return A reference to this instance.
     */
    WebSocketTransport& operator=(const WebSocketTransport& other) = default;
  };

}}}}} // namespace Azure::Core::Http::_detail::WebSockets
