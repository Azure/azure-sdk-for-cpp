// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include "azure/core/http/websockets/websockets.hpp"
#include "azure/core/http/websockets/websockets_transport.hpp"
#include "azure/core/internal/http/pipeline.hpp"

// Implementation of WebSocket protocol.
namespace Azure { namespace Core { namespace Http { namespace WebSockets { namespace _detail {

  class WebSocketImplementation {
    enum class SocketState
    {
      Invalid,
      Closed,
      Opening,
      Open,
      Closing,
    };

  public:
    WebSocketImplementation(Azure::Core::Url const& remoteUrl, WebSocketOptions const& options);

    void Open(Azure::Core::Context const& context);
    void Close(Azure::Core::Context const& context);
    void Close(
        uint16_t closeStatus,
        std::string const& closeReason,
        Azure::Core::Context const& context);
    void SendFrame(
        std::string const& textFrame,
        bool isFinalFrame,
        Azure::Core::Context const& context);
    void SendFrame(
        std::vector<uint8_t> const& binaryFrame,
        bool isFinalFrame,
        Azure::Core::Context const& context);

    std::shared_ptr<WebSocketResult> ReceiveFrame(Azure::Core::Context const& context);

    void AddHeader(std::string const& headerName, std::string const& headerValue);

    std::string const& GetChosenProtocol() const;
    bool IsOpen() { return m_state == SocketState::Open; }

  private:
    SocketState m_state{SocketState::Invalid};
    std::array<uint8_t, 16> GenerateRandomKey();
    void VerifySocketAccept(std::string const& encodedKey, std::string const& acceptHeader);
    Azure::Core::Url m_remoteUrl;
    WebSocketOptions m_options;
    std::map<std::string, std::string> m_headers;
    std::string m_chosenProtocol;
    std::shared_ptr<Azure::Core::Http::WebSockets::WebSocketTransport> m_transport;
  };
}}}}} // namespace Azure::Core::Http::WebSockets::_detail
