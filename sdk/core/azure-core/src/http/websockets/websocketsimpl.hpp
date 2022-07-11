// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include "azure/core/http/websockets/websockets.hpp"
#include "azure/core/http/websockets/websockets_transport.hpp"
#include "azure/core/internal/http/pipeline.hpp"
#include <array>
#include <random>
#include <shared_mutex>

// Implementation of WebSocket protocol.
namespace Azure { namespace Core { namespace Http { namespace WebSockets { namespace _detail {

  // Generator for random bytes. Used in WebSocketImplementation and tests.
  std::vector<uint8_t> GenerateRandomBytes(size_t vectorSize);

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

    std::shared_ptr<WebSocketResult> ReceiveFrame(
        Azure::Core::Context const& context,
        bool stateIsLocked = false);

    void AddHeader(std::string const& headerName, std::string const& headerValue);

    std::string const& GetChosenProtocol();
    bool IsOpen() { return m_state == SocketState::Open; }

  private:
    // WebSocket opcodes.
    enum class SocketOpcode : uint8_t
    {
      Continuation = 0x00,
      TextFrame = 0x01,
      BinaryFrame = 0x02,
      Close = 0x08,
      Ping = 0x09,
      Pong = 0x0a
    };

    /**
     * Indicates the type of the message currently being processed. Used when processing
     * Continuation Opcode frames.
     */
    enum class SocketMessageType : int
    {
      Unknown,
      Text,
      Binary,
    };

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
     * @brief Encode a websocket frame according to RFC 6455 section 5.2.
     *
     *    This wire format for the data transfer part is described by the ABNF
     *   [RFC5234] given in detail in this section.  (Note that, unlike in
     *   other sections of this document, the ABNF in this section is
     *   operating on groups of bits.  The length of each group of bits is
     *   indicated in a comment.  When encoded on the wire, the most
     *   significant bit is the leftmost in the ABNF).  A high-level overview
     *   of the framing is given in the following figure.  In a case of
     *   conflict between the figure below and the ABNF specified later in
     *   this section, the figure is authoritative.
     *
     *      0                   1                   2                   3
     *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     *     +-+-+-+-+-------+-+-------------+-------------------------------+
     *     |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
     *     |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
     *     |N|V|V|V|       |S|             |   (if payload len==126/127)   |
     *     | |1|2|3|       |K|             |                               |
     *     +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
     *     |     Extended payload length continued, if payload len == 127  |
     *     + - - - - - - - - - - - - - - - +-------------------------------+
     *     |                               |Masking-key, if MASK set to 1  |
     *     +-------------------------------+-------------------------------+
     *     | Masking-key (continued)       |          Payload Data         |
     *     +-------------------------------- - - - - - - - - - - - - - - - +
     *     :                     Payload Data continued ...                :
     *     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
     *     |                     Payload Data continued ...                |
     *     +---------------------------------------------------------------+
     *
     *   FIN:  1 bit
     *
     *      Indicates that this is the final fragment in a message.  The first
     *      fragment MAY also be the final fragment.
     *
     *   RSV1, RSV2, RSV3:  1 bit each
     *
     *      MUST be 0 unless an extension is negotiated that defines meanings
     *      for non-zero values.  If a nonzero value is received and none of
     *      the negotiated extensions defines the meaning of such a nonzero
     *      value, the receiving endpoint MUST _Fail the WebSocket
     *      Connection_.
     *
     *   Opcode:  4 bits
     *
     *      Defines the interpretation of the "Payload data".  If an unknown
     *      opcode is received, the receiving endpoint MUST _Fail the
     *      WebSocket Connection_.  The following values are defined.
     *
     *      *  %x0 denotes a continuation frame
     *
     *      *  %x1 denotes a text frame
     *
     *      *  %x2 denotes a binary frame
     *
     *      *  %x3-7 are reserved for further non-control frames
     *
     *      *  %x8 denotes a connection close
     *
     *      *  %x9 denotes a ping
     *
     *      *  %xA denotes a pong
     *
     *      *  %xB-F are reserved for further control frames
     *
     *   Mask:  1 bit
     *
     *      Defines whether the "Payload data" is masked.  If set to 1, a
     *      masking key is present in masking-key, and this is used to unmask
     *      the "Payload data" as per Section 5.3.  All frames sent from
     *      client to server have this bit set to 1.
     *
     *  Payload length:  7 bits, 7+16 bits, or 7+64 bits
     *
     *      The length of the "Payload data", in bytes: if 0-125, that is the
     *      payload length.  If 126, the following 2 bytes interpreted as a
     *      16-bit unsigned integer are the payload length.  If 127, the
     *      following 8 bytes interpreted as a 64-bit unsigned integer (the
     *      most significant bit MUST be 0) are the payload length.  Multibyte
     *      length quantities are expressed in network byte order.  Note that
     *      in all cases, the minimal number of bytes MUST be used to encode
     *      the length, for example, the length of a 124-byte-long string
     *      can't be encoded as the sequence 126, 0, 124.  The payload length
     *      is the length of the "Extension data" + the length of the
     *      "Application data".  The length of the "Extension data" may be
     *      zero, in which case the payload length is the length of the
     *      "Application data".
     *   Masking-key:  0 or 4 bytes
     *
     *      All frames sent from the client to the server are masked by a
     *      32-bit value that is contained within the frame.  This field is
     *      present if the mask bit is set to 1 and is absent if the mask bit
     *      is set to 0.  See Section 5.3 for further information on client-
     *      to-server masking.
     *
     *   Payload data:  (x+y) bytes
     *
     *      The "Payload data" is defined as "Extension data" concatenated
     *      with "Application data".
     *
     *   Extension data:  x bytes
     *
     *      The "Extension data" is 0 bytes unless an extension has been
     *      negotiated.  Any extension MUST specify the length of the
     *      "Extension data", or how that length may be calculated, and how
     *      the extension use MUST be negotiated during the opening handshake.
     *      If present, the "Extension data" is included in the total payload
     *      length.
     *
     *   Application data:  y bytes
     *
     *      Arbitrary "Application data", taking up the remainder of the frame
     *      after any "Extension data".  The length of the "Application data"
     *      is equal to the payload length minus the length of the "Extension
     *      data".
     */
    std::vector<uint8_t> EncodeFrame(
        SocketOpcode opcode,
        bool isFinal,
        std::vector<uint8_t> const& payload);

    /**
     * @brief Decode a frame received from the websocket server.
     *
     * @param streamReader Buffered stream reader to read the frame from.
     * @param opcode Opcode returned by the server.
     * @param isFinal True if this is the final message.
     * @returns A pointer to the start of the decoded data.
     */
    std::vector<uint8_t> DecodeFrame(
        BufferedStreamReader& streamReader,
        SocketOpcode& opcode,
        uint64_t& payloadLength,
        bool& isFinal,
        Azure::Core::Context const& context);

    SocketState m_state{SocketState::Invalid};

    std::vector<uint8_t> GenerateRandomKey() { return GenerateRandomBytes(16); };
    void VerifySocketAccept(std::string const& encodedKey, std::string const& acceptHeader);
    Azure::Core::Url m_remoteUrl;
    WebSocketOptions m_options;
    std::map<std::string, std::string> m_headers;
    std::string m_chosenProtocol;
    std::shared_ptr<Azure::Core::Http::WebSockets::WebSocketTransport> m_transport;
    BufferedStreamReader m_bufferedStreamReader;
    SocketMessageType m_currentMessageType{SocketMessageType::Unknown};

    std::mutex m_transportMutex;
    std::mutex m_stateMutex;
  };
}}}}} // namespace Azure::Core::Http::WebSockets::_detail
