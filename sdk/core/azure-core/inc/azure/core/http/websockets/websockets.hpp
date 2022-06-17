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
    ContinuationReceived,
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
  class WebSocketContinuationFrame;
  class WebSocketPeerCloseFrame;

  struct WebSocketResult
  {
    WebSocketResultType ResultType;
    std::shared_ptr<WebSocketTextFrame> AsTextFrame();
    std::shared_ptr<WebSocketBinaryFrame> AsBinaryFrame();
    std::shared_ptr<WebSocketPeerCloseFrame> AsPeerCloseFrame();
    std::shared_ptr<WebSocketContinuationFrame> AsContinuationFrame();
  };

  class WebSocketTextFrame : public WebSocketResult,
                             public std::enable_shared_from_this<WebSocketTextFrame> {
  private:
  public:
    WebSocketTextFrame() = default;
    WebSocketTextFrame(bool isFinalFrame, unsigned char const* body, size_t size)
        : WebSocketResult{WebSocketResultType::TextFrameReceived}, Text(body, body + size),
          IsFinalFrame(isFinalFrame)
    {
    }
    std::string Text;
    bool IsFinalFrame;
  };
  class WebSocketBinaryFrame : public WebSocketResult,
                               public std::enable_shared_from_this<WebSocketBinaryFrame> {
  private:
  public:
    WebSocketBinaryFrame() = default;
    WebSocketBinaryFrame(bool isFinal, unsigned char const* body, size_t size)
        : WebSocketResult{WebSocketResultType::BinaryFrameReceived}, Data(body, body + size),
          IsFinalFrame(isFinal)
    {
    }
    std::vector<uint8_t> Data;
    bool IsFinalFrame;
  };

  class WebSocketContinuationFrame : public WebSocketResult,
                                public std::enable_shared_from_this<WebSocketContinuationFrame> {
  public:
    WebSocketContinuationFrame() = default;
    WebSocketContinuationFrame(bool isFinal, unsigned char const* body, size_t size)
        : WebSocketResult{WebSocketResultType::ContinuationReceived},
          ContinuationData(body, body + size), IsFinalFrame(isFinal)
    {
    }
    std::vector<uint8_t> ContinuationData;
    bool IsFinalFrame;
  };

  class WebSocketPeerCloseFrame : public WebSocketResult,
                                  public std::enable_shared_from_this<WebSocketPeerCloseFrame> {
    std::vector<uint8_t> frameData_;

  public:
    WebSocketPeerCloseFrame() = default;
    WebSocketPeerCloseFrame(
        uint16_t remoteStatusCode,
        unsigned char const* closeData,
        size_t closeSize)
        : WebSocketResult{WebSocketResultType::PeerClosed}, RemoteStatusCode(remoteStatusCode),
          frameData_(closeData, closeData + closeSize), BodyStream(frameData_)
    {
    }
    uint16_t RemoteStatusCode;
    Azure::Core::IO::MemoryBodyStream BodyStream;
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
    std::string ServiceName;
    std::string ServiceVersion;

    explicit WebSocketOptions(bool enableMasking, std::vector<std::string> protocols)
        : Azure::Core::_internal::ClientOptions{}, EnableMasking(enableMasking),
          Protocols(protocols)
    {
    }
    WebSocketOptions() = default;
  };

  class WebSocket {
  public:
    explicit WebSocket(
        Azure::Core::Url const& remoteUrl,
        WebSocketOptions const& options = WebSocketOptions{});

    ~WebSocket();

    void Open(Azure::Core::Context const& context = Azure::Core::Context{});
    void Close(Azure::Core::Context const& context = Azure::Core::Context{});
    void Close(
        uint16_t closeStatus,
        std::string const& closeReason = {},
        Azure::Core::Context const& context = Azure::Core::Context{});
    void SendFrame(
        std::string const& textFrame,
        bool isFinalFrame,
        Azure::Core::Context const& context = Azure::Core::Context{});
    void SendFrame(
        std::vector<uint8_t> const& binaryFrame,
        bool isFinalFrame,
        Azure::Core::Context const& context = Azure::Core::Context{});

    std::shared_ptr<WebSocketResult> ReceiveFrame(
        Azure::Core::Context const& context = Azure::Core::Context{});

    void AddHeader(std::string const& headerName, std::string const& headerValue);

    bool IsOpen();

    std::string const& GetChosenProtocol() const;

  private:
    std::unique_ptr<_detail::WebSocketImplementation> m_socketImplementation;
  };
}}}} // namespace Azure::Core::Http::WebSockets
