// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Utilities to be used by HTTP transport implementations.
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

  enum class WebSocketResultType : int
  {
    Unknown,
    TextFrameReceived,
    BinaryFrameReceived,
    PeerClosed,
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

  struct WebSocketResult
  {
    WebSocketResultType ResultType;
    bool IsFinalFrame{false};
    std::shared_ptr<WebSocketTextFrame> AsTextFrame();
    std::shared_ptr<WebSocketBinaryFrame> AsBinaryFrame();
    std::shared_ptr<WebSocketPeerCloseFrame> AsPeerCloseFrame();
  };

  class WebSocketTextFrame : public WebSocketResult,
                             public std::enable_shared_from_this<WebSocketTextFrame> {
  private:
  public:
    WebSocketTextFrame() = default;
    WebSocketTextFrame(bool isFinalFrame, unsigned char const* body, size_t size)
        : WebSocketResult{WebSocketResultType::TextFrameReceived, isFinalFrame},
          Text(body, body + size)
    {
    }
    std::string Text;
  };
  class WebSocketBinaryFrame : public WebSocketResult,
                               public std::enable_shared_from_this<WebSocketBinaryFrame> {
  private:
  public:
    WebSocketBinaryFrame() = default;
    WebSocketBinaryFrame(bool isFinal, unsigned char const* body, size_t size)
        : WebSocketResult{WebSocketResultType::BinaryFrameReceived, isFinal},
          Data(body, body + size)
    {
    }
    std::vector<uint8_t> Data;
  };

  class WebSocketPeerCloseFrame : public WebSocketResult,
                                  public std::enable_shared_from_this<WebSocketPeerCloseFrame> {
  public:
    WebSocketPeerCloseFrame() = default;
    WebSocketPeerCloseFrame(uint16_t remoteStatusCode, std::string const& remoteCloseReason)
        : WebSocketResult{WebSocketResultType::PeerClosed}, RemoteStatusCode(remoteStatusCode),
          RemoteCloseReason(remoteCloseReason)
    {
    }
    uint16_t RemoteStatusCode;
    std::string RemoteCloseReason;
  };

  struct WebSocketOptions : Azure::Core::_internal::ClientOptions
  {
    /**
     * @brief Enable masking for this WebSocket.
     *
     * @detail Masking is needed to block [certain infrastructure
     * attacks](https://www.rfc-editor.org/rfc/rfc6455.html#section-10.3) and is strongly
     * recommended.
     */
    bool EnableMasking{true};
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
     * @brief Construct an instance of a WebSocketOptions type.
     *
     * @param enableMasking If true, enable masking for the websocket
     * @param protocols Supported protocols for this websocket client.
     */
    explicit WebSocketOptions(bool enableMasking, std::vector<std::string> protocols)
        : Azure::Core::_internal::ClientOptions{}, EnableMasking(enableMasking),
          Protocols(protocols)
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
        bool isFinalFrame,
        Azure::Core::Context const& context = Azure::Core::Context{});

    /** @brief Sends a Binary frame to the remote server.
     *
     * @param binaryFrame Binary data to send.
     * @param isFinalFrame if True, this is the final frame in a multi-frame message.
     * @param context Context for the operation.
     */
    void SendFrame(
        std::vector<uint8_t> const& binaryFrame,
        bool isFinalFrame,
        Azure::Core::Context const& context = Azure::Core::Context{});

    /** @brief Receive a frame from the remote server.
     *
     * @param context Context for the operation.
     *
     * @returns The received WebSocket frame.
     *
     */
    std::shared_ptr<WebSocketResult> ReceiveFrame(
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
    bool IsOpen();

    /** @brief Returns the protocol chosen by the remote server during the initial handshake
     */
    std::string const& GetChosenProtocol() const;

  private:
    std::unique_ptr<_detail::WebSocketImplementation> m_socketImplementation;
  };
}}}} // namespace Azure::Core::Http::WebSockets
