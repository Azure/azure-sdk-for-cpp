// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Azure Core APIs implementing the WebSocket protocol [RFC 6455]
 * (https://www.rfc-editor.org/rfc/rfc6455.html).
 */

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/transport.hpp"
#include "azure/core/internal/client_options.hpp"
#include <string>
#include <vector>

namespace Azure { namespace Core { namespace Http { namespace WebSockets {
  namespace _detail {
    class WebSocketImplementation;
  }
  namespace _internal {

    enum class WebSocketFrameType : int
    {
      Unknown,
      TextFrameReceived,
      BinaryFrameReceived,
      PeerClosedReceived,
    };

    enum class WebSocketErrorCode : uint16_t
    {
      OK = 1000,
      EndpointDisappearing = 1001,
      ProtocolError = 1002,
      UnknownDataType = 1003,
      Reserved1 = 1004,
      NoStatusCodePresent = 1005,
      ConnectionClosedWithoutCloseFrame = 1006,
      InvalidMessageData = 1007,
      PolicyViolation = 1008,
      MessageTooLarge = 1009,
      ExtensionNotFound = 1010,
      UnexpectedError = 1011,
      TlsHandshakeFailure = 1015,
    };

    class WebSocketTextFrame;
    class WebSocketBinaryFrame;
    class WebSocketPeerCloseFrame;

    namespace _detail {
      class WebSocketImplementation;
    }
    /** @brief Statistics about data sent and received by the WebSocket.
     *
     * @remarks This class is primarily intended for test collateral and debugging to allow
     * a caller to determine information about the status of a WebSocket.
     *
     * Note: Some of these statistics are not available if the underlying transport supports native
     * websockets.
     */
    struct WebSocketStatistics
    {
      /** @brief The number of WebSocket frames sent on this WebSocket. */
      uint32_t FramesSent;
      /** @brief The number of bytes of data sent to the peer on this WebSocket. */
      uint32_t BytesSent;
      /** @brief The number of WebSocket frames received from the peer. */
      uint32_t FramesReceived;
      /** @brief The number of bytes received from the peer. */
      uint32_t BytesReceived;
      /** @brief The number of "Ping" frames received from the peer. */
      uint32_t PingFramesReceived;
      /** @brief The number of "Ping" frames sent to the peer. */
      uint32_t PingFramesSent;
      /** @brief The number of "Pong" frames received from the peer. */
      uint32_t PongFramesReceived;
      /** @brief The number of "Pong" frames sent to the peer. */
      uint32_t PongFramesSent;
      /** @brief The number of "Text" frames received from the peer. */
      uint32_t TextFramesReceived;
      /** @brief The number of "Text" frames sent to the peer. */
      uint32_t TextFramesSent;
      /** @brief The number of "Binary" frames received from the peer. */
      uint32_t BinaryFramesReceived;
      /** @brief The number of "Binary" frames sent to the peer. */
      uint32_t BinaryFramesSent;
      /** @brief The number of "Continuation" frames sent to the peer. */
      uint32_t ContinuationFramesSent;
      /** @brief The number of "Continuation" frames received from the peer. */
      uint32_t ContinuationFramesReceived;
      /** @brief The number of "Close" frames received from the peer. */
      uint32_t CloseFramesReceived;
      /** @brief The number of frames received which were not processed. */
      uint32_t FramesDropped;
      /** @brief The number of frames received which were not returned because they were received
       * after the Close() method was called. */
      uint32_t FramesDroppedByClose;
      /** @brief The number of frames dropped because they were over the maximum payload size. */
      uint32_t FramesDroppedByPayloadSizeLimit;
      /** @brief The number of frames dropped because they were out of compliance with the protocol.
       */
      uint32_t FramesDroppedByProtocolError;
      /** @brief The number of reads performed on the transport.*/
      uint32_t TransportReads;
      /** @brief The number of bytes read from the transport. */
      uint32_t TransportReadBytes;
    };

    /** @brief A frame of data received from a WebSocket.
     */
    class WebSocketFrame {
    public:
      /** @brief The type of frame received: Text, Binary or Close. */
      WebSocketFrameType FrameType{};
      /** @brief True if the frame received is a "final" frame */
      bool IsFinalFrame{false};
      /** @brief Returns the contents of the frame as a Text frame.
       * @returns A WebSocketTextFrame containing the contents of the frame.
       */
      std::shared_ptr<WebSocketTextFrame> AsTextFrame();
      /** @brief Returns the contents of the frame as a Binary frame.
       * @returns A WebSocketBinaryFrame containing the contents of the frame.
       */
      std::shared_ptr<WebSocketBinaryFrame> AsBinaryFrame();
      /** @brief Returns the contents of the frame as a Peer Close frame.
       * @returns A WebSocketPeerCloseFrame containing the contents of the frame.
       */
      std::shared_ptr<WebSocketPeerCloseFrame> AsPeerCloseFrame();

      /** @brief Construct a new instance of a WebSocketFrame.*/
      WebSocketFrame() = default;

      /** @brief Construct a new instance of a WebSocketFrame with a specific frame type.
       * @param frameType The type of frame received.
       */
      WebSocketFrame(WebSocketFrameType frameType) : FrameType{frameType} {}
      WebSocketFrame(WebSocketFrameType frameType, bool isFinalFrame)
          : FrameType{frameType}, IsFinalFrame{isFinalFrame}
      {
      }
    };

    /** @brief Contains the contents of a WebSocket Text frame.*/
    class WebSocketTextFrame : public WebSocketFrame,
                               public std::enable_shared_from_this<WebSocketTextFrame> {
      friend Azure::Core::Http::WebSockets::_detail::WebSocketImplementation;

    private:
    public:
      /** @brief Constructs a new WebSocketTextFrame */
      WebSocketTextFrame() : WebSocketFrame(WebSocketFrameType::TextFrameReceived){};
      /** @brief Text of the frame received from the remote peer. */
      std::string Text;

    private:
      /** @brief Constructs a new WebSocketTextFrame
       * @param isFinalFrame True if this is the final frame in a multi-frame message.
       * @param body UTF-8 encoded text of the frame data.
       * @param size Length in bytes of the frame body.
       */
      WebSocketTextFrame(bool isFinalFrame, uint8_t const* body, size_t size)
          : WebSocketFrame{WebSocketFrameType::TextFrameReceived, isFinalFrame},
            Text(body, body + size)
      {
      }
    };

    /** @brief Contains the contents of a WebSocket Binary frame.*/
    class WebSocketBinaryFrame : public WebSocketFrame,
                                 public std::enable_shared_from_this<WebSocketBinaryFrame> {
      friend Azure::Core::Http::WebSockets::_detail::WebSocketImplementation;

    private:
    public:
      /** @brief Constructs a new WebSocketBinaryFrame */
      WebSocketBinaryFrame() : WebSocketFrame(WebSocketFrameType::BinaryFrameReceived){};
      /** @brief Binary frame data received from the remote peer. */
      std::vector<uint8_t> Data;

      /** @brief Constructs a new WebSocketBinaryFrame
       * @param isFinal True if this is the final frame in a multi-frame message.
       * @param body binary of the frame data.
       * @param size Length in bytes of the frame body.
       */
    private:
      WebSocketBinaryFrame(bool isFinal, uint8_t const* body, size_t size)
          : WebSocketFrame{WebSocketFrameType::BinaryFrameReceived, isFinal},
            Data(body, body + size)
      {
      }
    };

    /** @brief Contains the contents of a WebSocket Close frame.*/
    class WebSocketPeerCloseFrame : public WebSocketFrame,
                                    public std::enable_shared_from_this<WebSocketPeerCloseFrame> {
      friend Azure::Core::Http::WebSockets::_detail::WebSocketImplementation;

    public:
      /** @brief Constructs a new WebSocketPeerCloseFrame */
      WebSocketPeerCloseFrame() : WebSocketFrame(WebSocketFrameType::PeerClosedReceived){};
      /** @brief Status code sent from the remote peer. Typically a member of the WebSocketErrorCode
       * enumeration */
      uint16_t RemoteStatusCode;
      /** @brief Optional text sent from the remote peer. */
      std::string RemoteCloseReason;

    private:
      /** @brief Constructs a new WebSocketBinaryFrame
       * @param remoteStatusCode Status code sent by the remote peer.
       * @param remoteCloseReason Optional reason sent by the remote peer.
       */
      WebSocketPeerCloseFrame(uint16_t remoteStatusCode, std::string const& remoteCloseReason)
          : WebSocketFrame{WebSocketFrameType::PeerClosedReceived},
            RemoteStatusCode(remoteStatusCode), RemoteCloseReason(remoteCloseReason)
      {
      }
    };

    struct WebSocketOptions : Azure::Core::_internal::ClientOptions
    {
      /**
       * @brief The set of protocols which are supported by this client
       */
      std::vector<std::string> Protocols = {};

      /**
       * @brief The protocol name of the service client. Used for the User-Agent header
       * in the initial WebSocket handshake.
       */
      std::string ServiceName;
      /**
       * @brief The version of the service client. Used for the User-Agent header in the
       * initial WebSocket handshake
       */
      std::string ServiceVersion;

      /**
       * @brief The period of time between ping operations, default is 60 seconds.
       */
      std::chrono::duration<int64_t> PingInterval{std::chrono::seconds{60}};

      /**
       * @brief Construct an instance of a WebSocketOptions type.
       *
       * @param protocols Supported protocols for this websocket client.
       */
      explicit WebSocketOptions(std::vector<std::string> protocols)
          : Azure::Core::_internal::ClientOptions{}, Protocols(protocols)
      {
      }
      WebSocketOptions() = default;
    };

    class WebSocket {
    public:
      /** @brief Constructs a new instance of a WebSocket with the specified WebSocket options.
       *
       * @param remoteUrl The URL of the remote WebSocket server.
       * @param options The options to use for the WebSocket.
       */
      explicit WebSocket(
          Azure::Core::Url const& remoteUrl,
          WebSocketOptions const& options = WebSocketOptions{});

      /** @brief Destroys an instance of a WebSocket.
       */
      ~WebSocket();

      /** @brief Opens a WebSocket connection to a remote server.
       *
       * @param context Context for the operation, used for cancellation and timeout.
       */
      void Open(Azure::Core::Context const& context = Azure::Core::Context{});

      /** @brief Closes a WebSocket connection to the remote server gracefully.
       *
       * @param context Context for the operation.
       */
      void Close(Azure::Core::Context const& context = Azure::Core::Context{});

      /** @brief Closes a WebSocket connection to the remote server with additional context.
       *
       * @param closeStatus 16 bit WebSocket error code.
       * @param closeReason String describing the reason for closing the socket.
       * @param context Context for the operation.
       */
      void Close(
          uint16_t closeStatus,
          std::string const& closeReason = {},
          Azure::Core::Context const& context = Azure::Core::Context{});

      /** @brief Sends a String frame to the remote server.
       *
       * @param textFrame UTF-8 encoded text to send.
       * @param isFinalFrame if True, this is the final frame in a multi-frame message.
       * @param context Context for the operation.
       */
      void SendFrame(
          std::string const& textFrame,
          bool isFinalFrame = false,
          Azure::Core::Context const& context = Azure::Core::Context{});

      /** @brief Sends a Binary frame to the remote server.
       *
       * @param binaryFrame Binary data to send.
       * @param isFinalFrame if True, this is the final frame in a multi-frame message.
       * @param context Context for the operation.
       */
      void SendFrame(
          std::vector<uint8_t> const& binaryFrame,
          bool isFinalFrame = false,
          Azure::Core::Context const& context = Azure::Core::Context{});

      /** @brief Receive a frame from the remote server.
       *
       * @param context Context for the operation.
       *
       * @returns The received WebSocket frame.
       *
       */
      std::shared_ptr<WebSocketFrame> ReceiveFrame(
          Azure::Core::Context const& context = Azure::Core::Context{});

      /** @brief AddHeader - Adds a header to the initial handshake.
       *
       * @note This API is ignored after the WebSocket is opened.
       *
       * @param headerName Name of header to add to the initial handshake request.
       * @param headerValue Value of header to add.
       */
      void AddHeader(std::string const& headerName, std::string const& headerValue);

      /** @brief Determine if the WebSocket is open.
       *
       * @returns true if the WebSocket is open, false otherwise.
       */
      bool IsOpen() const;

      /** @brief Returns "true" if the configured websocket transport
       * supports websockets in the transport, or if the websocket implementation
       * is providing websocket protocol support.
       *
       * @returns true if the websocket transport supports websockets natively.
       */
      bool HasNativeWebSocketSupport() const;

      /** @brief Returns the protocol chosen by the remote server during the initial handshake
       *
       * @returns The protocol negotiated between client and server.
       */
      std::string const& GetChosenProtocol() const;

      /** @brief Returns statistics about the WebSocket.
       *
       * @returns The statistics about the WebSocket.
       */
      WebSocketStatistics GetStatistics() const;

    private:
      std::unique_ptr<Azure::Core::Http::WebSockets::_detail::WebSocketImplementation>
          m_socketImplementation;
    };
  } // namespace _internal
}}}} // namespace Azure::Core::Http::WebSockets
