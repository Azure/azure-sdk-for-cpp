// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include "websocketsimpl.hpp"
#include "azure/core/base64.hpp"
#include "azure/core/http/policies/policy.hpp"
#include "azure/core/internal/cryptography/sha_hash.hpp"
// SUPPORT_NATIVE_TRANSPORT indicates if WinHTTP should be compiled with native transport support
// or not.
// Note that this is primarily required to improve the code coverage numbers in the CI pipeline.
#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
#include "azure/core/http/websockets/win_http_websockets_transport.hpp"
#define SUPPORT_NATIVE_TRANSPORT 1
#elif defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
#include "azure/core/http/websockets/curl_websockets_transport.hpp"
#define SUPPORT_NATIVE_TRANSPORT 0
#endif
#include "azure/core/internal/diagnostics/log.hpp"
#include <algorithm>
#include <array>
#include <iomanip>
#include <mutex>
#include <random>
#include <shared_mutex>
#include <sstream>

namespace Azure { namespace Core { namespace Http { namespace WebSockets { namespace _detail {
  using namespace Azure::Core::Diagnostics::_internal;
  using namespace Azure::Core::Diagnostics;
  using namespace std::chrono_literals;

  namespace {
    std::string HexEncode(std::vector<uint8_t> const& data, size_t length)
    {
      std::stringstream ss;
      for (size_t i = 0; i < std::min(data.size(), length); i++)
      {
        ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(data[i]);
      }
      return ss.str();
    }
  } // namespace

  WebSocketImplementation::WebSocketImplementation(
      Azure::Core::Url const& remoteUrl,
      WebSocketOptions const& options)
      : m_remoteUrl(remoteUrl), m_options(options), m_pingThread(this, m_options.PingInterval)
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
    auto winHttpTransport
        = std::make_shared<Azure::Core::Http::WebSockets::WinHttpWebSocketTransport>(
            transportOptions);
    m_transport = std::static_pointer_cast<WebSocketTransport>(winHttpTransport);
    m_options.Transport.Transport = std::static_pointer_cast<HttpTransport>(winHttpTransport);
#elif defined(BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
    CurlTransportOptions transportOptions;
    transportOptions.HttpKeepAlive = false;
    auto curlWebSockets
        = std::make_shared<Azure::Core::Http::WebSockets::CurlWebSocketTransport>(transportOptions);

    m_transport = std::static_pointer_cast<WebSocketTransport>(curlWebSockets);
    m_options.Transport.Transport = std::static_pointer_cast<HttpTransport>(curlWebSockets);
#endif

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perCallPolicies{};
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies{};
    // If the caller has told us a service name, add the telemetry policy to the pipeline to add
    // a user agent header to the request.
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
    if (!m_transport->HasNativeWebsocketSupport())
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
    if (!m_transport->HasNativeWebsocketSupport())
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
      m_initialBodyStream = response->ExtractBodyStream();
      m_pingThread.Start(m_transport);
    }

    // Remember the protocol that the client chose.
    auto chosenProtocol = responseHeaders.find("Sec-WebSocket-Protocol");
    if (chosenProtocol != responseHeaders.end())
    {
      m_chosenProtocol = chosenProtocol->second;
    }

    m_state = SocketState::Open;
  }
  bool WebSocketImplementation::HasNativeWebSocketSupport()
  {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_stateOwner = std::this_thread::get_id();
    if (m_state != SocketState::Open)
    {
      throw std::runtime_error("Socket is not open.");
    }
    return m_transport->HasNativeWebsocketSupport();
  }

  std::string const& WebSocketImplementation::GetChosenProtocol()
  {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_stateOwner = std::this_thread::get_id();
    if (m_state != SocketState::Open)
    {
      throw std::runtime_error("Socket is not open.");
    }
    return m_chosenProtocol;
  }

  void WebSocketImplementation::AddHeader(std::string const& header, std::string const& headerValue)
  {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_stateOwner = std::this_thread::get_id();
    if (m_state != SocketState::Closed && m_state != SocketState::Invalid)
    {
      throw std::runtime_error("AddHeader can only be called on closed sockets.");
    }
    m_headers.emplace(std::make_pair(header, headerValue));
  }

  void WebSocketImplementation::Close(
      uint16_t closeStatus,
      std::string const& closeReason,
      Azure::Core::Context const& context)
  {
    std::unique_lock<std::mutex> lock(m_stateMutex);
    m_stateOwner = std::this_thread::get_id();

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
#if SUPPORT_NATIVE_TRANSPORT
    if (m_transport->HasNativeWebsocketSupport())
    {
      m_transport->NativeCloseSocket(closeStatus, closeReason.c_str(), context);
    }
    else
#endif
    {
      // Send a going away message to the server.
      std::vector<uint8_t> closePayload;
      closePayload.push_back(closeStatus >> 8);
      closePayload.push_back(closeStatus & 0xff);
      closePayload.insert(closePayload.end(), closeReason.begin(), closeReason.end());
      std::vector<uint8_t> closeFrame = EncodeFrame(SocketOpcode::Close, true, closePayload);
      SendTransportBuffer(closeFrame, context);

      // Unlock the state mutex before waiting for the close response to be received.
      lock.unlock();
      // To ensure that we process the responses in a "timely" fashion, limit the close
      // reception to 20 seconds if we don't already have a timeout.
      Azure::Core::Context closeContext = context;
      auto cancelTimepoint = closeContext.GetDeadline();
      if (cancelTimepoint == Azure::DateTime::max())
      {
        closeContext = closeContext.WithDeadline(std::chrono::system_clock::now() + 20s);
      }
      // Drain the incoming series of frames from the server.
      // Note that there might be in-flight frames that were sent from the other end of the
      // WebSocket that we don't care about any more (since we're closing the WebSocket). So
      // drain those frames.
      auto closeResponse = ReceiveFrame(context);
      while (closeResponse->FrameType != WebSocketFrameType::PeerClosedReceived)
      {
        m_receiveStatistics.FramesDroppedByClose++;
        Log::Write(
            Logger::Level::Warning,
            "Received unexpected frame during close. Opcode: "
                + std::to_string(static_cast<uint8_t>(closeResponse->FrameType)));
        closeResponse = ReceiveFrame(closeContext);
      }

      // Re-acquire the state lock once we've received the close lock.
      lock.lock();
    }
    // Close the socket - after this point, the m_transport is invalid.
    m_pingThread.Shutdown();
    m_transport->NativeClose();
    m_state = SocketState::Closed;
  }

  void WebSocketImplementation::SendFrame(
      std::string const& textFrame,
      bool isFinalFrame,
      Azure::Core::Context const& context)
  {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_stateOwner = std::this_thread::get_id();
    if (m_state != SocketState::Open)
    {
      throw std::runtime_error("Socket is not open.");
    }
    std::vector<uint8_t> utf8text(textFrame.begin(), textFrame.end());
    m_receiveStatistics.TextFramesSent++;
#if SUPPORT_NATIVE_TRANSPORT
    if (m_transport->HasNativeWebsocketSupport())
    {
      m_transport->NativeSendFrame(
          (isFinalFrame ? WebSocketTransport::NativeWebSocketFrameType::FrameTypeText
                        : WebSocketTransport::NativeWebSocketFrameType::FrameTypeTextFragment),
          utf8text,
          context);
    }
    else
#endif
    {
      std::vector<uint8_t> sendFrame = EncodeFrame(SocketOpcode::TextFrame, isFinalFrame, utf8text);
      SendTransportBuffer(sendFrame, context);
    }
  }

  void WebSocketImplementation::SendFrame(
      std::vector<uint8_t> const& binaryFrame,
      bool isFinalFrame,
      Azure::Core::Context const& context)
  {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_stateOwner = std::this_thread::get_id();

    if (m_state != SocketState::Open)
    {
      throw std::runtime_error("Socket is not open.");
    }
    m_receiveStatistics.BinaryFramesSent++;
#if SUPPORT_NATIVE_TRANSPORT
    if (m_transport->HasNativeWebsocketSupport())
    {
      m_transport->NativeSendFrame(
          (isFinalFrame ? WebSocketTransport::NativeWebSocketFrameType::FrameTypeBinary
                        : WebSocketTransport::NativeWebSocketFrameType::FrameTypeBinaryFragment),
          binaryFrame,
          context);
    }
    else
#endif
    {
      //      Log::Write(Logger::Level::Verbose, "Send Binary Frame " + HexEncode(binaryFrame, 16));
      std::vector<uint8_t> sendFrame
          = EncodeFrame(SocketOpcode::BinaryFrame, isFinalFrame, binaryFrame);

      SendTransportBuffer(sendFrame, context);
    }
  }

  std::shared_ptr<WebSocketFrame> WebSocketImplementation::ReceiveFrame(
      Azure::Core::Context const& context)
  {
    std::unique_lock<std::mutex> lock(m_stateMutex);
    m_stateOwner = std::this_thread::get_id();

    if (m_state != SocketState::Open && m_state != SocketState::Closing)
    {
      throw std::runtime_error("Socket is not open.");
    }

    // Unlock the state lock to allow other threads to run. If we don't, we might end up in in a
    // situation where the server won't respond to the this client because all the client threads
    // are blocked on the state lock.
    lock.unlock();

    std::shared_ptr<WebSocketInternalFrame> frame;
    // Loop until we receive an returnable incoming frame.
    // If the incoming frame is returnable, we return the value from the frame.
    while (true)
    {
      frame = ReceiveTransportFrame(context);
      switch (frame->Opcode)
      {
          // When we receive a "ping" frame, we want to send a Pong frame back to the server.
        case SocketOpcode::Ping:
          Log::Write(
              Logger::Level::Verbose, "Received Ping frame: " + HexEncode(frame->Payload, 16));
          SendPong(frame->Payload, context);
          break;
          // We want to ignore all incoming "Pong" frames.
        case SocketOpcode::Pong:
          Log::Write(
              Logger::Level::Verbose, "Received Pong frame: " + HexEncode(frame->Payload, 16));
          break;

        case SocketOpcode::BinaryFrame:
          m_currentMessageType = SocketMessageType::Binary;
          return std::shared_ptr<WebSocketFrame>(new WebSocketBinaryFrame(
              frame->IsFinalFrame, frame->Payload.data(), frame->Payload.size()));

        case SocketOpcode::TextFrame:
          m_currentMessageType = SocketMessageType::Text;
          return std::shared_ptr<WebSocketFrame>(new WebSocketTextFrame(
              frame->IsFinalFrame, frame->Payload.data(), frame->Payload.size()));

        case SocketOpcode::Close: {
          if (frame->Payload.size() < 2)
          {
            throw std::runtime_error("Close response buffer is too short.");
          }
          uint16_t errorCode = 0;
          errorCode |= (frame->Payload[0] << 8) & 0xff00;
          errorCode |= (frame->Payload[1] & 0x00ff);

          // We received a close frame, mark the socket as closed. Make sure we
          // reacquire the state lock before setting the state to closed.
          lock.lock();
          m_state = SocketState::Closed;

          return std::shared_ptr<WebSocketFrame>(new WebSocketPeerCloseFrame(
              errorCode, std::string(frame->Payload.begin() + 2, frame->Payload.end())));
        }

          // Continuation frames need to be treated somewhat specially.
          // We depend on the fact that the protocol requires that a Continuation frame
          // only be sent if it is part of a multi-frame message whose previous frame was a Text or
          // Binary frame.
        case SocketOpcode::Continuation:
          if (m_currentMessageType == SocketMessageType::Text)
          {
            if (frame->IsFinalFrame)
            {
              m_currentMessageType = SocketMessageType::Unknown;
            }
            return std::shared_ptr<WebSocketFrame>(new WebSocketTextFrame(
                frame->IsFinalFrame, frame->Payload.data(), frame->Payload.size()));
          }
          else if (m_currentMessageType == SocketMessageType::Binary)
          {
            if (frame->IsFinalFrame)
            {
              m_currentMessageType = SocketMessageType::Unknown;
            }
            return std::shared_ptr<WebSocketFrame>(new WebSocketBinaryFrame(
                frame->IsFinalFrame, frame->Payload.data(), frame->Payload.size()));
          }
          else
          {
            m_receiveStatistics.FramesDroppedByProtocolError++;
            throw std::runtime_error("Unknown message type and received continuation opcode");
          }
        default:
          throw std::runtime_error("Unknown frame type received.");
      }
      context.ThrowIfCancelled();
    }
  }

  std::shared_ptr<WebSocketImplementation::WebSocketInternalFrame>
  WebSocketImplementation::ReceiveTransportFrame(Azure::Core::Context const& context)
  {
#if SUPPORT_NATIVE_TRANSPORT
    if (m_transport->HasNativeWebsocketSupport())
    {
      auto payload = m_transport->NativeReceiveFrame(context);
      m_receiveStatistics.FramesReceived++;
      switch (payload.FrameType)
      {
        case WebSocketTransport::NativeWebSocketFrameType::FrameTypeBinary:
          m_receiveStatistics.BinaryFramesReceived++;
          return std::make_shared<WebSocketInternalFrame>(
              SocketOpcode::BinaryFrame, true, payload.FrameData);
        case WebSocketTransport::NativeWebSocketFrameType::FrameTypeBinaryFragment:
          m_receiveStatistics.BinaryFramesReceived++;
          return std::make_shared<WebSocketInternalFrame>(
              SocketOpcode::BinaryFrame, false, payload.FrameData);
        case WebSocketTransport::NativeWebSocketFrameType::FrameTypeText:
          m_receiveStatistics.TextFramesReceived++;
          return std::make_shared<WebSocketInternalFrame>(
              SocketOpcode::TextFrame, true, payload.FrameData);
        case WebSocketTransport::NativeWebSocketFrameType::FrameTypeTextFragment:
          m_receiveStatistics.TextFramesReceived++;
          return std::make_shared<WebSocketInternalFrame>(
              SocketOpcode::TextFrame, false, payload.FrameData);
        case WebSocketTransport::NativeWebSocketFrameType::FrameTypeClosed: {
          m_receiveStatistics.CloseFramesReceived++;
          auto closeResult = m_transport->NativeGetCloseSocketInformation(context);
          std::vector<uint8_t> closePayload;
          closePayload.push_back(closeResult.CloseReason >> 8);
          closePayload.push_back(closeResult.CloseReason& 0xff);
          closePayload.insert(
              closePayload.end(), closeResult.CloseReasonDescription.begin(), closeResult.CloseReasonDescription.end());
          return std::make_shared<WebSocketInternalFrame>(SocketOpcode::Close, true, closePayload);
        }
        default:
          throw std::runtime_error("Unexpected frame type received.");
      }
    }
    else
#endif
    {
      SocketOpcode opcode;

      bool isFinal = false;
      std::vector<uint8_t> frameData = DecodeFrame(opcode, isFinal, context);
      // At this point, frameData contains the actual payload from the service.
      auto frame = std::make_shared<WebSocketInternalFrame>(opcode, isFinal, frameData);

      // Handle statistics for the incoming frame.
      m_receiveStatistics.FramesReceived++;
      switch (frame->Opcode)
      {
        case SocketOpcode::Ping: {
          m_receiveStatistics.PingFramesReceived++;
          break;
        }
        case SocketOpcode::Pong: {
          m_receiveStatistics.PongFramesReceived++;
          break;
        }
        case SocketOpcode::TextFrame: {
          m_receiveStatistics.TextFramesReceived++;
          break;
        }
        case SocketOpcode::BinaryFrame: {
          m_receiveStatistics.BinaryFramesReceived++;
          break;
        }
        case SocketOpcode::Close: {
          m_receiveStatistics.CloseFramesReceived++;
          break;
        }
        case SocketOpcode::Continuation: {
          m_receiveStatistics.ContinuationFramesReceived++;
          break;
        }
        default: {
          m_receiveStatistics.UnknownFramesReceived++;
          break;
        }
      }
      return frame;
    }
  }

  WebSocketStatistics WebSocketImplementation::GetStatistics() const
  {
    WebSocketStatistics returnValue{};
    returnValue.FramesSent = m_receiveStatistics.FramesSent.load();
    returnValue.FramesReceived = m_receiveStatistics.FramesReceived.load();
    returnValue.BinaryFramesReceived = m_receiveStatistics.BinaryFramesReceived.load();
    returnValue.TextFramesReceived = m_receiveStatistics.TextFramesReceived.load();
    returnValue.BinaryFramesSent = m_receiveStatistics.BinaryFramesSent.load();
    returnValue.TextFramesSent = m_receiveStatistics.TextFramesSent.load();
    returnValue.PingFramesReceived = m_receiveStatistics.PingFramesReceived.load();
    returnValue.PongFramesReceived = m_receiveStatistics.PongFramesReceived.load();
    returnValue.PingFramesSent = m_receiveStatistics.PingFramesSent.load();
    returnValue.PongFramesSent = m_receiveStatistics.PongFramesSent.load();

    returnValue.BytesSent = m_receiveStatistics.BytesSent.load();
    returnValue.BytesReceived = m_receiveStatistics.BytesReceived.load();
    returnValue.FramesDropped = m_receiveStatistics.FramesDropped.load();
    returnValue.FramesDroppedByClose = m_receiveStatistics.FramesDroppedByClose.load();
    returnValue.FramesDroppedByPayloadSizeLimit
        = m_receiveStatistics.FramesDroppedByPayloadSizeLimit.load();
    returnValue.FramesDroppedByProtocolError
        = m_receiveStatistics.FramesDroppedByProtocolError.load();
    returnValue.TransportReadBytes = m_receiveStatistics.TransportReadBytes.load();
    returnValue.TransportReads = m_receiveStatistics.TransportReads.load();
    return returnValue;
  }

  std::vector<uint8_t> WebSocketImplementation::EncodeFrame(
      SocketOpcode opcode,
      bool isFinal,
      std::vector<uint8_t> const& payload)
  {
    std::vector<uint8_t> encodedFrame;
    // Add opcode+fin.
    encodedFrame.push_back(static_cast<uint8_t>(opcode) | (isFinal ? 0x80 : 0));
    uint8_t maskAndLength = 0;
    maskAndLength |= 0x80;

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

    return encodedFrame;
  }
  std::vector<uint8_t> WebSocketImplementation::DecodeFrame(
      SocketOpcode& opcode,
      bool& isFinal,
      Azure::Core::Context const& context)
  {
    // Ensure single threaded access to receive this frame.
    std::unique_lock<std::mutex> lock(m_transportMutex);
    if (IsTransportEof())
    {
      throw std::runtime_error("Frame buffer is too small.");
    }
    uint8_t payloadByte = ReadTransportByte(context);
    opcode = static_cast<SocketOpcode>(payloadByte & 0x7f);
    isFinal = (payloadByte & 0x80) != 0;
    payloadByte = ReadTransportByte(context);
    if (payloadByte & 0x80)
    {
      throw std::runtime_error("Server sent a frame with a reserved bit set.");
    }
    int64_t payloadLength = payloadByte & 0x7f;
    if (payloadLength <= 125)
    {
      payloadByte += 1;
    }
    else if (payloadLength == 126)
    {
      payloadLength = ReadTransportShort(context);
    }
    else if (payloadLength == 127)
    {
      payloadLength = ReadTransportInt64(context);
    }
    else
    {
      throw std::logic_error("Unexpected payload length.");
    }

    return ReadTransportBytes(static_cast<size_t>(payloadLength), context);
  }

  uint8_t WebSocketImplementation::ReadTransportByte(Azure::Core::Context const& context)
  {
    if (m_bufferPos >= m_bufferLen)
    {
      // Start by reading data from our initial body stream.
      m_bufferLen = m_initialBodyStream->ReadToCount(m_buffer, m_bufferSize, context);
      if (m_bufferLen == 0)
      {
        // If we run out of the initial stream, we need to read from the transport.
        m_bufferLen = m_transport->ReadFromSocket(m_buffer, m_bufferSize, context);
        m_receiveStatistics.TransportReads++;
        m_receiveStatistics.TransportReadBytes += static_cast<uint32_t>(m_bufferLen);
        //        Log::Write(
        //            Logger::Level::Verbose,
        //            "Read #" + std::to_string(m_receiveStatistics.TransportReads.load())
        //                + "from transport: " + std::to_string(m_bufferLen));
      }
      else
      {
        Azure::Core::Diagnostics::_internal::Log::Write(
            Azure::Core::Diagnostics::Logger::Level::Informational,
            "Read data from initial stream");
      }
      m_bufferPos = 0;
      if (m_bufferLen == 0)
      {
        m_eof = true;
        return 0;
      }
    }

    m_receiveStatistics.BytesReceived++;
    return m_buffer[m_bufferPos++];
  }
  uint16_t WebSocketImplementation::ReadTransportShort(Azure::Core::Context const& context)
  {
    uint16_t result = ReadTransportByte(context);
    result <<= 8;
    result |= ReadTransportByte(context);
    return result;
  }
  uint64_t WebSocketImplementation::ReadTransportInt64(Azure::Core::Context const& context)
  {
    uint64_t result = 0;

    result |= (static_cast<uint64_t>(ReadTransportByte(context)) << 56 & 0xff00000000000000);
    result |= (static_cast<uint64_t>(ReadTransportByte(context)) << 48 & 0x00ff000000000000);
    result |= (static_cast<uint64_t>(ReadTransportByte(context)) << 40 & 0x0000ff0000000000);
    result |= (static_cast<uint64_t>(ReadTransportByte(context)) << 32 & 0x000000ff00000000);
    result |= (static_cast<uint64_t>(ReadTransportByte(context)) << 24 & 0x00000000ff000000);
    result |= (static_cast<uint64_t>(ReadTransportByte(context)) << 16 & 0x0000000000ff0000);
    result |= (static_cast<uint64_t>(ReadTransportByte(context)) << 8 & 0x000000000000ff00);
    result |= static_cast<uint64_t>(ReadTransportByte(context));
    return result;
  }
  std::vector<uint8_t> WebSocketImplementation::ReadTransportBytes(
      size_t readLength,
      Azure::Core::Context const& context)
  {
    std::vector<uint8_t> result;
    size_t index = 0;
    while (index < readLength)
    {
      uint8_t byte = ReadTransportByte(context);
      result.push_back(byte);
      index += 1;
    }
    return result;
  }

  void WebSocketImplementation::SendTransportBuffer(
      std::vector<uint8_t> const& sendFrame,
      Azure::Core::Context const& context)
  {
    std::unique_lock<std::mutex> transportLock(m_transportMutex);
    m_receiveStatistics.BytesSent += static_cast<uint32_t>(sendFrame.size());
    m_receiveStatistics.FramesSent += 1;
    //    Log::Write(
    //        Logger::Level::Verbose,
    //        "Send #" + std::to_string(m_receiveStatistics.FramesSent.load()) + "to
    //        transport:"
    //            + std::to_string(sendFrame.size()) + "Data: " + HexEncode(sendFrame, 0x10));
    m_transport->SendBuffer(sendFrame.data(), sendFrame.size(), context);
  }

  // Verify the Sec-WebSocket-Accept header as defined in RFC 6455 Section 1.3, which defines
  // the opening handshake used for establishing the WebSocket connection.
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

  WebSocketImplementation::PingThread::PingThread(
      WebSocketImplementation* socketImplementation,
      std::chrono::duration<int64_t> pingInterval)
      : m_webSocketImplementation(socketImplementation), m_pingInterval(pingInterval)
  {
  }
  void WebSocketImplementation::PingThread::Start(std::shared_ptr<WebSocketTransport> transport)
  {
    m_stop = false;
    // Spin up a thread to receive data from the transport.
    if (!transport->HasNativeWebsocketSupport())
    {
      std::unique_lock<std::mutex> lock(m_pingThreadStarted);
      m_pingThread = std::thread{&PingThread::PingThreadLoop, this};
      m_pingThreadReady.wait(lock);
    }
  }

  WebSocketImplementation::PingThread::~PingThread()
  {
    // Ensure that the receive thread is stopped.
    Shutdown();
  }
  void WebSocketImplementation::PingThread::Shutdown()
  {
    if (m_pingThread.joinable())
    {
      std::unique_lock<std::mutex> lock(m_stopMutex);
      m_stop = true;
      lock.unlock();
      m_pingThreadStopped.notify_all();

      m_pingThread.join();
    }
  }

  void WebSocketImplementation::PingThread::PingThreadLoop()
  {
    Log::Write(Logger::Level::Verbose, "Start Ping Thread Loop.");
    {
      std::unique_lock<std::mutex> lock(m_pingThreadStarted);
      m_pingThreadReady.notify_all();
    }
    while (true)
    {
      std::unique_lock<std::mutex> lock(m_stopMutex);
      if (this->m_pingThreadStopped.wait_for(lock, m_pingInterval) == std::cv_status::timeout)
      {
        Log::Write(Logger::Level::Verbose, "Send Ping to peer.");

        // The receiveContext timed out, this means we timed out our "ping" timeout.
        // Send a "Ping" request to the remote node.
        auto pingData = GenerateRandomBytes(4);
        SendPing(pingData, Azure::Core::Context{});
      }
      if (m_stop)
      {
        Log::Write(Logger::Level::Verbose, "Exiting ping thread");
        return;
      }
    }
  }

  bool WebSocketImplementation::PingThread::SendPing(
      std::vector<uint8_t> const& pingData,
      Azure::Core::Context const& context)
  {
    std::vector<uint8_t> pingFrame = EncodeFrame(SocketOpcode::Ping, true, pingData);
    m_webSocketImplementation->m_receiveStatistics.PingFramesSent++;
    m_webSocketImplementation->SendTransportBuffer(pingFrame, context);
    return true;
  }

  void WebSocketImplementation::SendPong(
      std::vector<uint8_t> const& pongData,
      Azure::Core::Context const& context)
  {
    std::vector<uint8_t> pongFrame = EncodeFrame(SocketOpcode::Pong, true, pongData);

    m_receiveStatistics.PongFramesSent++;
    SendTransportBuffer(pongFrame, context);
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