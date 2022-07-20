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
  class WebSocketTransport {
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
     * @brief Destructs `%WebSocketTransport`.
     *
     */
    virtual ~WebSocketTransport() {}

    /**
     * @brief Determines if the transport natively supports WebSockets or not.
     *
     * @returns true if the transport has native websocket support, false otherwise.
     */
    virtual bool HasNativeWebsocketSupport() = 0;

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
     * @param frameType Frame type sent to the server, Text or Binary.
     * @param frameData Frame data to be sent to the server.
     * @param context Context for the operation.
     */
    virtual void SendFrame(
        WebSocketFrameType frameType,
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
    virtual std::pair<WebSocketFrameType, std::vector<uint8_t>> ReceiveFrame(
        Azure::Core::Context const& context)
        = 0;

    /**************/
    /* Non Native WebSocket support functions */
    /**************/

    // Implement a buffered stream reader
    class BufferedStreamReader {
      std::shared_ptr<Azure::Core::Http::WebSockets::WebSocketTransport> m_transport;
      std::unique_ptr<Azure::Core::IO::BodyStream> m_initialBodyStream;
      constexpr static size_t m_bufferSize = 1024;
      uint8_t m_buffer[m_bufferSize]{};
      size_t m_bufferPos = 0;
      size_t m_bufferLen = 0;
      bool m_eof = false;

    public:
      explicit BufferedStreamReader() = default;
      ~BufferedStreamReader() = default;

      void SetInitialStream(std::unique_ptr<Azure::Core::IO::BodyStream>& stream)
      {
        m_initialBodyStream = std::move(stream);
      }
      void SetTransport(
          std::shared_ptr<Azure::Core::Http::WebSockets::WebSocketTransport>& transport)
      {
        m_transport = transport;
      }

      uint8_t ReadByte(Azure::Core::Context const& context)
      {
        if (m_bufferPos >= m_bufferLen)
        {
          // Start by reading data from our initial body stream.
          m_bufferLen = m_initialBodyStream->ReadToCount(m_buffer, m_bufferSize, context);
          if (m_bufferLen == 0)
          {
            // If we run out of the initial stream, we need to read from the transport.
            m_bufferLen = m_transport->ReadFromSocket(m_buffer, m_bufferSize, context);
          }
          m_bufferPos = 0;
          if (m_bufferLen == 0)
          {
            m_eof = true;
            return 0;
          }
        }
        return m_buffer[m_bufferPos++];
      }
      uint16_t ReadShort(Azure::Core::Context const& context)
      {
        uint16_t result = ReadByte(context);
        result <<= 8;
        result |= ReadByte(context);
        return result;
      }
      uint64_t ReadInt64(Azure::Core::Context const& context)
      {
        uint64_t result = 0;

        result |= (static_cast<uint64_t>(ReadByte(context)) << 56 & 0xff00000000000000);
        result |= (static_cast<uint64_t>(ReadByte(context)) << 48 & 0x00ff000000000000);
        result |= (static_cast<uint64_t>(ReadByte(context)) << 40 & 0x0000ff0000000000);
        result |= (static_cast<uint64_t>(ReadByte(context)) << 32 & 0x000000ff00000000);
        result |= (static_cast<uint64_t>(ReadByte(context)) << 24 & 0x00000000ff000000);
        result |= (static_cast<uint64_t>(ReadByte(context)) << 16 & 0x0000000000ff0000);
        result |= (static_cast<uint64_t>(ReadByte(context)) << 8 & 0x000000000000ff00);
        result |= static_cast<uint64_t>(ReadByte(context));
        return result;
      }
      std::vector<uint8_t> ReadBytes(size_t readLength, Azure::Core::Context const& context)
      {
        std::vector<uint8_t> result;
        size_t index = 0;
        while (index < readLength)
        {
          uint8_t byte = ReadByte(context);
          result.push_back(byte);
          index += 1;
        }
        return result;
      }

      bool IsEof() const { return m_eof; }
    };

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
