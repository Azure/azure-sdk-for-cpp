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
#include <algorithm>
#include <array>
#include <mutex>
#include <random>
#include <shared_mutex>

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
    WinHttpTransportOptions transportOptions;
    // WinHTTP overrides the Connection: Upgrade header, so disable keep-alive.
    transportOptions.EnableWebSocketUpgrade = true;

    m_transport = std::make_shared<Azure::Core::Http::WebSockets::WinHttpWebSocketTransport>(
        transportOptions);
    m_options.Transport.Transport = m_transport;
#elif defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
    CurlTransportOptions transportOptions;
    transportOptions.HttpKeepAlive = false;
    m_transport
        = std::make_shared<Azure::Core::Http::WebSockets::CurlWebSocketTransport>(transportOptions);
    m_options.Transport.Transport = m_transport;
    m_bufferedStreamReader.SetTransport(m_transport);
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

    // Generate the random request key. Only used when the transport doesn't support websockets
    // natively.
    auto randomKey = GenerateRandomKey();
    auto encodedKey = Azure::Core::Convert::Base64Encode(randomKey);
    if (!m_transport->NativeWebsocketSupport())
    {
      // If the transport doesn't support WebSockets natively, set the standardized WebSocket
      // upgrade headers.
      openSocketRequest.SetHeader("Upgrade", "websocket");
      openSocketRequest.SetHeader("Connection", "upgrade");
      openSocketRequest.SetHeader("Sec-WebSocket-Version", "13");
      openSocketRequest.SetHeader("Sec-WebSocket-Key", encodedKey);
    }
    if (!m_options.Protocols.empty())
    {

      std::string protocols;
      for (auto const& protocol : m_options.Protocols)
      {
        protocols += protocol;
        protocols += ", ";
      }
      protocols = protocols.substr(0, protocols.size() - 2);
      openSocketRequest.SetHeader("Sec-WebSocket-Protocol", protocols);
    }
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
    if (!m_transport->NativeWebsocketSupport())
    {

      auto socketAccept(responseHeaders.find("Sec-WebSocket-Accept"));
      if (socketAccept == responseHeaders.end())
      {
        throw Azure::Core::Http::TransportException("Missing Sec-WebSocket-Accept header");
      }
      // Verify that the WebSocket server received *this* open request.
      else
      {
        VerifySocketAccept(encodedKey, socketAccept->second);
      }
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

  std::string const& WebSocketImplementation::GetChosenProtocol()
  {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    if (m_state != SocketState::Open)
    {
      throw std::runtime_error("Socket is not open.");
    }
    return m_chosenProtocol;
  }

  void WebSocketImplementation::AddHeader(std::string const& header, std::string const& headerValue)
  {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    if (m_state != SocketState::Closed && m_state != SocketState::Invalid)
    {
      throw std::runtime_error("AddHeader can only be called on closed sockets.");
    }
    m_headers.emplace(std::make_pair(header, headerValue));
  }

  void WebSocketImplementation::Close(Azure::Core::Context const& context)
  {
    std::lock_guard<std::mutex> lock(m_stateMutex);

    // If we're closing an already closed socket, we're done.
    if (m_state == SocketState::Closed)
    {
      return;
    }
    if (m_state != SocketState::Open)
    {
      throw std::runtime_error("Socket is not open.");
    }
    m_state = SocketState::Closing;
    if (m_transport->NativeWebsocketSupport())
    {
      m_transport->CloseSocket(
          static_cast<uint16_t>(WebSocketErrorCode::EndpointDisappearing), "", context);
    }
    else
    {
      // Send a going away message to the server.
      uint16_t closeReason = static_cast<uint16_t>(WebSocketErrorCode::EndpointDisappearing);
      std::vector<uint8_t> closePayload;
      closePayload.push_back(closeReason >> 8);
      closePayload.push_back(closeReason & 0xff);
      std::vector<uint8_t> closeFrame
          = EncodeFrame(SocketOpcode::Close, m_options.EnableMasking, true, closePayload);
      m_transport->SendBuffer(closeFrame.data(), closeFrame.size(), context);

      auto closeResponse = ReceiveFrame(context, true);
      if (closeResponse->ResultType != WebSocketResultType::PeerClosed)
      {
        throw std::runtime_error("Unexpected result type received during close().");
      }
    }
    // Close the socket - after this point, the m_transport is invalid.
    m_transport->Close();
    m_state = SocketState::Closed;
  }

  void WebSocketImplementation::Close(
      uint16_t closeStatus,
      std::string const& closeReason,
      Azure::Core::Context const& context)
  {
    std::lock_guard<std::mutex> lock(m_stateMutex);
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

      std::vector<uint8_t> closeFrame
          = EncodeFrame(SocketOpcode::Close, m_options.EnableMasking, true, closePayload);
      m_transport->SendBuffer(closeFrame.data(), closeFrame.size(), context);

      auto closeResponse = ReceiveFrame(context, true);
      if (closeResponse->ResultType != WebSocketResultType::PeerClosed)
      {
        throw std::runtime_error("Unexpected result type received during close().");
      }
    }
    // Close the socket - after this point, the m_transport is invalid.
    m_transport->Close();

    m_state = SocketState::Closed;
  }

  void WebSocketImplementation::SendFrame(
      std::string const& textFrame,
      bool isFinalFrame,
      Azure::Core::Context const& context)
  {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    if (m_state != SocketState::Open)
    {
      throw std::runtime_error("Socket is not open.");
    }
    std::vector<uint8_t> utf8text(textFrame.begin(), textFrame.end());
    if (m_transport->NativeWebsocketSupport())
    {
      m_transport->SendFrame(
          (isFinalFrame ? WebSocketTransport::WebSocketFrameType::FrameTypeText
                        : WebSocketTransport::WebSocketFrameType::FrameTypeTextFragment),
          utf8text,
          context);
    }
    else
    {
      std::vector<uint8_t> sendFrame
          = EncodeFrame(SocketOpcode::TextFrame, m_options.EnableMasking, isFinalFrame, utf8text);

      m_transport->SendBuffer(sendFrame.data(), sendFrame.size(), context);
    }
  }

  void WebSocketImplementation::SendFrame(
      std::vector<uint8_t> const& binaryFrame,
      bool isFinalFrame,
      Azure::Core::Context const& context)
  {
    std::lock_guard<std::mutex> lock(m_stateMutex);

    if (m_state != SocketState::Open)
    {
      throw std::runtime_error("Socket is not open.");
    }
    if (m_transport->NativeWebsocketSupport())
    {
      m_transport->SendFrame(
          (isFinalFrame ? WebSocketTransport::WebSocketFrameType::FrameTypeBinary
                        : WebSocketTransport::WebSocketFrameType::FrameTypeBinaryFragment),
          binaryFrame,
          context);
    }
    else
    {
      std::vector<uint8_t> sendFrame = EncodeFrame(
          SocketOpcode::BinaryFrame, m_options.EnableMasking, isFinalFrame, binaryFrame);

      std::unique_lock<std::mutex> transportLock(m_transportMutex);
      m_transport->SendBuffer(sendFrame.data(), sendFrame.size(), context);
    }
  }

  std::shared_ptr<WebSocketResult> WebSocketImplementation::ReceiveFrame(
      Azure::Core::Context const& context,
      bool stateIsLocked)
  {
    std::unique_lock<std::mutex> lock(m_stateMutex, std::defer_lock);

    if (!stateIsLocked)
    {
      lock.lock();
    }

    if (m_state != SocketState::Open && m_state != SocketState::Closing)
    {
      throw std::runtime_error("Socket is not open.");
    }
    if (m_transport->NativeWebsocketSupport())
    {
      WebSocketTransport::WebSocketFrameType frameType;
      std::vector<uint8_t> payload = m_transport->ReceiveFrame(frameType, context);
      switch (frameType)
      {
        case WebSocketTransport::WebSocketFrameType::FrameTypeBinary:
          return std::make_shared<WebSocketBinaryFrame>(true, payload.data(), payload.size());
        case WebSocketTransport::WebSocketFrameType::FrameTypeBinaryFragment:
          return std::make_shared<WebSocketBinaryFrame>(false, payload.data(), payload.size());
        case WebSocketTransport::WebSocketFrameType::FrameTypeText:
          return std::make_shared<WebSocketTextFrame>(true, payload.data(), payload.size());
        case WebSocketTransport::WebSocketFrameType::FrameTypeTextFragment:
          return std::make_shared<WebSocketTextFrame>(false, payload.data(), payload.size());
        case WebSocketTransport::WebSocketFrameType::FrameTypeClosed: {
          auto closeResult = m_transport->GetCloseSocketInformation(context);
          return std::make_shared<WebSocketPeerCloseFrame>(closeResult.first, closeResult.second);
        }
        default:
          throw std::runtime_error("Unexpected frame type received.");
      }
    }
    else
    {
      SocketOpcode opcode;

      bool isFinal = false;
      bool isMasked = false;
      uint64_t payloadLength;
      std::array<uint8_t, 4> maskKey{};
      std::vector<uint8_t> frameData = DecodeFrame(
          m_bufferedStreamReader, opcode, payloadLength, isFinal, isMasked, maskKey, context);

      if (isMasked)
      {
        int index = 0;
        std::transform(
            frameData.begin(), frameData.end(), frameData.begin(), [&maskKey, &index](uint8_t val) {
              val ^= maskKey[index % 4];
              index += 1;
              return val;
            });
      }
      // At this point, readBuffer contains the actual payload from the service.
      switch (opcode)
      {
        case SocketOpcode::BinaryFrame:
          m_currentMessageType = SocketMessageType::Binary;
          return std::make_shared<WebSocketBinaryFrame>(
              isFinal, frameData.data(), frameData.size());
        case SocketOpcode::TextFrame: {
          m_currentMessageType = SocketMessageType::Text;
          return std::make_shared<WebSocketTextFrame>(isFinal, frameData.data(), frameData.size());
        }
        case SocketOpcode::Close: {

          if (frameData.size() < 2)
          {
            throw std::runtime_error("Close response buffer is too short.");
          }
          uint16_t errorCode = 0;
          errorCode |= (frameData[0] << 8) & 0xff00;
          errorCode |= (frameData[1] & 0x00ff);

          // Update our state to be closed once we've received a closed frame. We only need to
          // do this if our state is not currently locked.
          if (!stateIsLocked)
          {
            lock.unlock();
            std::unique_lock<std::mutex> closeLock(m_stateMutex);
            m_state = SocketState::Closed;
          }
          return std::make_shared<WebSocketPeerCloseFrame>(
              errorCode, std::string(frameData.begin() + 2, frameData.end()));
        }
        case SocketOpcode::Ping:
        case SocketOpcode::Pong:
          throw std::runtime_error("Unexpected Ping/Pong opcode received.");
          break;
        case SocketOpcode::Continuation:
          if (m_currentMessageType == SocketMessageType::Text)
          {
            if (isFinal)
            {
              m_currentMessageType = SocketMessageType::Unknown;
            }
            return std::make_shared<WebSocketTextFrame>(
                isFinal, frameData.data(), frameData.size());
          }
          else if (m_currentMessageType == SocketMessageType::Binary)
          {
            if (isFinal)
            {
              m_currentMessageType = SocketMessageType::Unknown;
            }
            return std::make_shared<WebSocketBinaryFrame>(
                isFinal, frameData.data(), frameData.size());
          }
          else
          {
            throw std::runtime_error("Unknown message type and received continuation opcode");
          }
          break;
        default:
          throw std::runtime_error("Unknown opcode received.");
      }
    }
#if !defined(_MSC_VER)
    // gcc 5 doesn't seem to detect that this is dead code, so we'll just leave it here.
    return nullptr;
#endif
  }

  std::vector<uint8_t> WebSocketImplementation::EncodeFrame(
      SocketOpcode opcode,
      bool maskOutput,
      bool isFinal,
      std::vector<uint8_t> const& payload)
  {
    std::vector<uint8_t> encodedFrame;
    // Add opcode+fin.
    encodedFrame.push_back(static_cast<uint8_t>(opcode) | (isFinal ? 0x80 : 0));
    uint8_t maskAndLength = 0;
    if (maskOutput)
    {
      maskAndLength |= 0x80;
    }
    // Payloads smaller than 125 bytes are encoded directly in the maskAndLength field.
    uint64_t payloadSize = static_cast<uint64_t>(payload.size());
    if (payloadSize <= 125)
    {
      maskAndLength |= static_cast<uint8_t>(payload.size());
    }
    else if (payloadSize <= 65535)
    {
      // Payloads greater than 125 whose size can fit in a 16 bit integer bytes
      // are encoded as a 16 bit unsigned integer in network byte order.
      maskAndLength |= 126;
    }
    else
    {
      // Payloads greater than 65536 have their length are encoded as a 64 bit unsigned integer
      // in network byte order.
      maskAndLength |= 127;
    }
    encodedFrame.push_back(maskAndLength);
    // Encode a 16 bit length.
    if (payloadSize > 125 && payloadSize <= 65535)
    {
      encodedFrame.push_back(static_cast<uint16_t>(payload.size()) >> 8);
      encodedFrame.push_back(static_cast<uint16_t>(payload.size()) & 0xff);
    }
    // Encode a 64 bit length.
    else if (payloadSize >= 65536)
    {

      encodedFrame.push_back((payloadSize >> 56) & 0xff);
      encodedFrame.push_back((payloadSize >> 48) & 0xff);
      encodedFrame.push_back((payloadSize >> 40) & 0xff);
      encodedFrame.push_back((payloadSize >> 32) & 0xff);
      encodedFrame.push_back((payloadSize >> 24) & 0xff);
      encodedFrame.push_back((payloadSize >> 16) & 0xff);
      encodedFrame.push_back((payloadSize >> 8) & 0xff);
      encodedFrame.push_back(payloadSize & 0xff);
    }
    // Calculate the masking key. This MUST be 4 bytes of high entropy random numbers used to
    // mask the input data.
    if (maskOutput)
    {
      // Start by generating the mask - 4 bytes of random data.
      std::vector<uint8_t> mask = GenerateRandomBytes(4);

      // Append the mask to the payload.
      encodedFrame.insert(encodedFrame.end(), mask.begin(), mask.end());

      // And mask the payload before transmitting it.
      size_t index = 0;
      for (auto ch : payload)
      {
        encodedFrame.push_back(ch ^ mask[index % 4]);
        index += 1;
      }
    }
    else
    {
      // Since the payload is unmasked, simply append the payload to the encoded frame.
      encodedFrame.insert(encodedFrame.end(), payload.begin(), payload.end());
    }

    return encodedFrame;
  }

  std::vector<uint8_t> WebSocketImplementation::DecodeFrame(
      WebSocketImplementation::BufferedStreamReader& streamReader,
      SocketOpcode& opcode,
      uint64_t& payloadLength,
      bool& isFinal,
      bool& isMasked,
      std::array<uint8_t, 4>& maskKey,
      Azure::Core::Context const& context)
  {
    std::unique_lock<std::mutex> lock(m_transportMutex);
    if (streamReader.IsEof())
    {
      throw std::runtime_error("Frame buffer is too small.");
    }
    uint8_t payloadByte = streamReader.ReadByte(context);
    opcode = static_cast<SocketOpcode>(payloadByte & 0x7f);
    isFinal = (payloadByte & 0x80) != 0;
    payloadByte = streamReader.ReadByte(context);
    isMasked = false;
    if (payloadByte & 0x80)
    {
      isMasked = true;
    }
    payloadLength = payloadByte & 0x7f;
    if (payloadLength <= 125)
    {
      payloadByte += 1;
    }
    else if (payloadLength == 126)
    {
      payloadLength = streamReader.ReadShort(context);
    }
    else if (payloadLength == 127)
    {
      payloadLength = streamReader.ReadInt64(context);
    }
    else
    {
      throw std::logic_error("Unexpected payload length.");
    }

    if (isMasked)
    {
      maskKey[0] = streamReader.ReadByte(context);
      maskKey[1] = streamReader.ReadByte(context);
      maskKey[2] = streamReader.ReadByte(context);
      maskKey[3] = streamReader.ReadByte(context);
    };
    return streamReader.ReadBytes(static_cast<size_t>(payloadLength), context);
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

  // Generator for random bytes. Used in WebSocketImplementation and tests.
  std::vector<uint8_t> GenerateRandomBytes(size_t vectorSize)
  {
    std::random_device randomEngine;

    std::vector<uint8_t> rv(vectorSize);
    std::generate(begin(rv), end(rv), [&randomEngine]() mutable {
      return static_cast<uint8_t>(randomEngine() % UINT8_MAX);
    });
    return rv;
  }
}}}}} // namespace Azure::Core::Http::WebSockets::_detail