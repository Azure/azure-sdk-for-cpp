// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include "websocketsimpl.hpp"
#include "azure/core/base64.hpp"
#include "azure/core/http/policies/policy.hpp"
#include "azure/core/internal/cryptography/sha_hash.hpp"
#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
#include "azure/core/http/websockets/win_http_websockets_transport.hpp"
#elif defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include "azure/core/http/websockets/curl_websockets_transport.hpp"
#endif
#include "websocket_frame.hpp"

#include <array>
#include <random>

namespace Azure { namespace Core { namespace Http { namespace WebSockets { namespace _detail {

  WebSocketImplementation::WebSocketImplementation(
      Azure::Core::Url const& remoteUrl,
      WebSocketOptions const& options)
      : m_remoteUrl(remoteUrl), m_options(options)
  {
  }

  void WebSocketImplementation::Open(Azure::Core::Context const& context)
  {
    if (m_state != SocketState::Invalid && m_state != SocketState::Closed)
    {
      throw std::runtime_error("Socket is not closed.");
    }
    m_state = SocketState::Opening;

#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
    auto winHttpTransport
        = std::make_shared<Azure::Core::Http::WebSockets::WinHttpWebSocketTransport>();
    m_options.Transport.Transport = winHttpTransport;
#elif defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
    CurlTransportOptions transportOptions;
    transportOptions.HttpKeepAlive = false;
    m_transport
        = std::make_shared<Azure::Core::Http::WebSockets::CurlWebSocketTransport>(transportOptions);
    m_options.Transport.Transport = m_transport;
#endif

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perCallPolicies{};
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies{};
    // If the caller has told us a service name, add the telemetry policy to the pipeline to add a
    // user agent header to the request.
    if (!m_options.ServiceName.empty())
    {
      perCallPolicies.push_back(
          std::make_unique<Azure::Core::Http::Policies::_internal::TelemetryPolicy>(
              m_options.ServiceName, m_options.ServiceVersion, m_options.Telemetry));
    }
    Azure::Core::Http::_internal::HttpPipeline openPipeline(
        m_options, std::move(perRetryPolicies), std::move(perCallPolicies));

    Azure::Core::Http::Request openSocketRequest(
        Azure::Core::Http::HttpMethod::Get, m_remoteUrl, false);

    // Set the standardized WebSocket upgrade headers.
    openSocketRequest.SetHeader("Upgrade", "websocket");
    openSocketRequest.SetHeader("Connection", "Upgrade");
    openSocketRequest.SetHeader("Sec-WebSocket-Version", "13");
    // Generate the random request key
    auto randomKey = GenerateRandomKey();
    auto encodedKey = Azure::Core::Convert::Base64Encode(randomKey);
    openSocketRequest.SetHeader("Sec-WebSocket-Key", encodedKey);
    std::string protocols;
    for (auto const& protocol : m_options.Protocols)
    {
      protocols += protocol;
      protocols += ", ";
    }
    protocols = protocols.substr(0, protocols.size() - 2);
    openSocketRequest.SetHeader("Sec-WebSocket-Protocol", protocols);
    for (auto const& additionalHeader : m_headers)
    {
      openSocketRequest.SetHeader(additionalHeader.first, additionalHeader.second);
    }
    std::string remoteOrigin;
    remoteOrigin = m_remoteUrl.GetScheme();
    remoteOrigin += "://";
    remoteOrigin += m_remoteUrl.GetHost();
    openSocketRequest.SetHeader("Origin", remoteOrigin);

    // Send the connect request to the WebSocket server.
    auto response = openPipeline.Send(openSocketRequest, context);

    // Ensure that the server thinks we're switching protocols. If it doesn't,
    // fail immediately.
    if (response->GetStatusCode() != Azure::Core::Http::HttpStatusCode::SwitchingProtocols)
    {
      throw Azure::Core::Http::TransportException("Unexpected handshake response");
    }

    // Prove that the server received this socket request.
    auto& responseHeaders = response->GetHeaders();
    auto socketAccept(responseHeaders.find("Sec-WebSocket-Accept"));
    if (socketAccept != responseHeaders.end())
    {
      VerifySocketAccept(encodedKey, socketAccept->second);
    }

    // Remember the protocol that the client chose.
    auto chosenProtocol = responseHeaders.find("Sec-WebSocket-Protocol");
    if (chosenProtocol != responseHeaders.end())
    {
      m_chosenProtocol = chosenProtocol->second;
    }

    // Inform the transport that the upgrade is complete and that the WebSockets layer is taking
    // over the HTTP connection.
    m_transport->CompleteUpgrade();
    m_state = SocketState::Open;
  }

  std::string const& WebSocketImplementation::GetChosenProtocol() const
  {
    if (m_state != SocketState::Open)
    {
      throw std::runtime_error("Socket is not open.");
    }
    return m_chosenProtocol;
  }

  void WebSocketImplementation::Close(Azure::Core::Context const& context)
  {
    if (m_state != SocketState::Open)
    {
      throw std::runtime_error("Socket is not open.");
    }
    m_state = SocketState::Closing;
    if (m_transport->NativeWebsocketSupport())
    {
      m_transport->CloseSocket(0, "", context);
    }
    else
    {
      // Send a going away message to the server.
      uint16_t closeReason = static_cast<uint16_t>(WebSocketErrorCode::EndpointDisappearing);
      std::vector<uint8_t> closePayload;
      closePayload.push_back(closeReason >> 8);
      closePayload.push_back(closeReason & 0xff);
      std::vector<uint8_t> closeFrame = WebSocketFrameEncoder::EncodeFrame(
          SocketOpcode::Close, m_options.EnableMasking, true, closePayload);
      m_transport->SendBuffer(closeFrame.data(), closeFrame.size(), context);

      auto closeResponse = ReceiveFrame(context);
      if (closeResponse->ResultType != WebSocketResultType::PeerClosed)
      {
        throw std::runtime_error("Unexpected result type received during close().");
      }
    }
    m_transport->Close();
    m_state = SocketState::Closed;
  }

  void WebSocketImplementation::Close(
      uint16_t closeStatus,
      std::string const& closeReason,
      Azure::Core::Context const& context)
  {
    if (m_state != SocketState::Open)
    {
      throw std::runtime_error("Socket is not open.");
    }

    m_state = SocketState::Closing;
    if (m_transport->NativeWebsocketSupport())
    {
      m_transport->CloseSocket(closeStatus, closeReason, context);
    }
    else
    {
      std::vector<uint8_t> closePayload;
      closePayload.push_back(closeStatus >> 8);
      closePayload.push_back(closeStatus & 0xff);
      closePayload.insert(closePayload.end(), closeReason.begin(), closeReason.end());

      std::vector<uint8_t> closeFrame = WebSocketFrameEncoder::EncodeFrame(
          SocketOpcode::Close, m_options.EnableMasking, true, closePayload);
      m_transport->SendBuffer(closeFrame.data(), closeFrame.size(), context);

      auto closeResponse = ReceiveFrame(context);
      if (closeResponse->ResultType != WebSocketResultType::PeerClosed)
      {
        throw std::runtime_error("Unexpected result type received during close().");
      }
    }
    m_transport->Close();

    m_state = SocketState::Closed;
  }

  void WebSocketImplementation::AddHeader(std::string const& header, std::string const& headerValue)
  {
    m_headers.emplace(std::make_pair(header, headerValue));
  }
  void WebSocketImplementation::SendFrame(
      std::string const& textFrame,
      bool isFinalFrame,
      Azure::Core::Context const& context)
  {
    if (m_state != SocketState::Open)
    {
      throw std::runtime_error("Socket is not open.");
    }
    if (m_transport->NativeWebsocketSupport())
    {
      throw std::runtime_error("Not implemented");
    }
    else
    {
      std::vector<uint8_t> utf8text(textFrame.begin(), textFrame.end());
      std::vector<uint8_t> sendFrame = WebSocketFrameEncoder::EncodeFrame(
          SocketOpcode::TextFrame, m_options.EnableMasking, isFinalFrame, utf8text);

      m_transport->SendBuffer(sendFrame.data(), sendFrame.size(), context);
    }
  }

  void WebSocketImplementation::SendFrame(
      std::vector<uint8_t> const& binaryFrame,
      bool isFinalFrame,
      Azure::Core::Context const& context)
  {
    if (m_state != SocketState::Open)
    {
      throw std::runtime_error("Socket is not open.");
    }
    if (m_transport->NativeWebsocketSupport())
    {
      throw std::runtime_error("Not implemented");
    }
    else
    {
      std::vector<uint8_t> sendFrame = WebSocketFrameEncoder::EncodeFrame(
          SocketOpcode::BinaryFrame, m_options.EnableMasking, isFinalFrame, binaryFrame);

      m_transport->SendBuffer(sendFrame.data(), sendFrame.size(), context);
    }
  }

  std::shared_ptr<WebSocketResult> WebSocketImplementation::ReceiveFrame(
      Azure::Core::Context const& context)
  {
    if (m_state != SocketState::Open && m_state != SocketState::Closing)
    {
      throw std::runtime_error("Socket is not open.");
    }
    if (m_transport->NativeWebsocketSupport())
    {
      throw std::runtime_error("Not implemented");
    }
    else
    {
      uint8_t payloadByte;
      // Read the first byte from the socket (the opcode and final bit).
      auto bytesRead = m_transport->ReadFromSocket(&payloadByte, sizeof(payloadByte), context);
      if (bytesRead == 0)
      {
        return nullptr;
      }
      if (bytesRead != sizeof(payloadByte))
      {
        throw std::runtime_error("Could not read opcode from socket.");
      }
      SocketOpcode opcode = static_cast<SocketOpcode>(payloadByte & 0x7f);
      bool isFinal = (payloadByte & 0x80) != 0;

      // Read the next byte from the socket (the size
      bytesRead = m_transport->ReadFromSocket(&payloadByte, sizeof(payloadByte), context);
      if (bytesRead != sizeof(payloadByte))
      {
        throw std::runtime_error("Could not read size and mask from socket.");
      }

      bool isMasked = false;
      if (payloadByte & 0x80)
      {
        isMasked = true;
      }
      uint64_t payloadLength = payloadByte & 0x7f;
      if (payloadLength == 126)
      {
        uint8_t shortSize[sizeof(uint16_t)];
        bytesRead = m_transport->ReadFromSocket(shortSize, sizeof(shortSize), context);
        if (bytesRead != sizeof(shortSize))
        {
          throw std::runtime_error("Could not read short size from socket.");
        }
        payloadLength = 0;
        payloadLength |= (static_cast<uint64_t>(shortSize[0]) << 8) & 0xff;
        payloadLength |= (static_cast<uint64_t>(shortSize[1]) & 0xff);
      }
      else if (payloadLength == 127)
      {
        uint8_t int64Size[sizeof(uint64_t)];
        bytesRead = m_transport->ReadFromSocket(int64Size, sizeof(int64Size), context);
        if (bytesRead != sizeof(int64Size))
        {
          throw std::runtime_error("Could not read short size from socket.");
        }
        payloadLength = 0;
        payloadLength |= (static_cast<uint64_t>(int64Size[0]) << 56) & 0xff00000000000000;
        payloadLength |= (static_cast<uint64_t>(int64Size[1]) << 48) & 0x00ff000000000000;
        payloadLength |= (static_cast<uint64_t>(int64Size[2]) << 40) & 0x0000ff0000000000;
        payloadLength |= (static_cast<uint64_t>(int64Size[3]) << 32) & 0x000000ff00000000;
        payloadLength |= (static_cast<uint64_t>(int64Size[4]) << 24) & 0x00000000ff000000;
        payloadLength |= (static_cast<uint64_t>(int64Size[5]) << 16) & 0x0000000000ff0000;
        payloadLength |= (static_cast<uint64_t>(int64Size[6]) << 8) & 0x000000000000ff00;
        payloadLength |= (static_cast<uint64_t>(int64Size[7])) & 0x00000000000000ff;
      }
      else if (payloadLength >= 126)
      {
        throw std::logic_error("Unexpected payload length.");
      }
      std::array<uint8_t, 4> maskKey{};
      if (isMasked)
      {
        bytesRead = m_transport->ReadFromSocket(maskKey.data(), maskKey.size(), context);
        if (bytesRead != sizeof(maskKey))
        {
          throw std::runtime_error("Could not read short size from socket.");
        }
      }

      // Now read the entire buffer from the socket.
      std::vector<uint8_t> readBuffer(payloadLength);
      bytesRead = m_transport->ReadFromSocket(readBuffer.data(), readBuffer.size(), context);

      // If the buffer was masked, unmask the buffer contents.
      if (isMasked)
      {
        int index = 0;
        std::transform(
            readBuffer.begin(),
            readBuffer.end(),
            readBuffer.begin(),
            [&maskKey, &index](uint8_t val) {
              val ^= maskKey[index % 4];
              index += 1;
              return val;
            });
      }
      // At this point, readBuffer contains the actual payload from the service.
      {
        switch (opcode)
        {
          case SocketOpcode::BinaryFrame:
            return std::make_shared<WebSocketBinaryFrame>(
                isFinal, readBuffer.data(), readBuffer.size());
          case SocketOpcode::TextFrame: {
            return std::make_shared<WebSocketTextFrame>(
                isFinal, readBuffer.data(), readBuffer.size());
          }
          case SocketOpcode::Close: {

            if (readBuffer.size() < 2)
            {
              throw std::runtime_error("Close response buffer is too short.");
            }
            uint16_t errorCode = 0;
            errorCode |= (readBuffer[0] << 8) & 0xff00;
            errorCode |= (readBuffer[1] & 0x00ff);
            return std::make_shared<WebSocketPeerCloseFrame>(
                errorCode,
                readBuffer.data() + sizeof(uint16_t),
                readBuffer.size() - sizeof(uint16_t));
          }
          case SocketOpcode::Ping:
          case SocketOpcode::Pong:
            __debugbreak();
            break;
          case SocketOpcode::Continuation:
            return std::make_shared<WebSocketContinuationFrame>(
                isFinal, readBuffer.data(), readBuffer.size());
          default:
            throw std::runtime_error("Unknown opcode received.");
        }
      }
    }

    return std::shared_ptr<WebSocketResult>();
    context;
  }

  std::array<uint8_t, 16> WebSocketImplementation::GenerateRandomKey()
  {
    std::random_device randomEngine;

    std::array<uint8_t, 16> rv;
    std::generate(begin(rv), end(rv), std::ref(randomEngine));
    return rv;
  }

  // Verify the Sec-WebSocket-Accept header as defined in RFC 6455 Section 1.3, which defines the
  // opening handshake used for establishing the WebSocket connection.
  std::string acceptHeaderGuid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  void WebSocketImplementation::VerifySocketAccept(
      std::string const& encodedKey,
      std::string const& acceptHeader)
  {
    std::string concatenatedKey(encodedKey);
    concatenatedKey += acceptHeaderGuid;
    Azure::Core::Cryptography::_internal::Sha1Hash sha1hash;

    sha1hash.Append(
        reinterpret_cast<const uint8_t*>(concatenatedKey.data()), concatenatedKey.size());
    auto keyHash = sha1hash.Final();
    std::string encodedHash = Azure::Core::Convert::Base64Encode(keyHash);
    if (encodedHash != acceptHeader)
    {
      throw std::runtime_error(
          "Hash returned by WebSocket server does not match expected hash. Aborting");
    }
  }

}}}}} // namespace Azure::Core::Http::WebSockets::_detail