// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include "azure/core/http/websockets/websockets.hpp"
#include "azure/core/http/websockets/websockets_transport.hpp"
#include "azure/core/internal/diagnostics/log.hpp"
#include "azure/core/internal/http/pipeline.hpp"
#include <array>
#include <queue>
#include <random>
#include <shared_mutex>
#include <thread>

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
    WebSocketImplementation(
        Azure::Core::Url const& remoteUrl,
        _internal::WebSocketOptions const& options);

    void Open(Azure::Core::Context const& context);
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

    std::shared_ptr<_internal::WebSocketFrame> ReceiveFrame(Azure::Core::Context const& context);

    void AddHeader(std::string const& headerName, std::string const& headerValue);

    std::string const& GetChosenProtocol();
    bool IsOpen() { return m_state == SocketState::Open; }
    bool HasNativeWebSocketSupport();

    _internal::WebSocketStatistics GetStatistics() const;

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

    class WebSocketInternalFrame {
    public:
      SocketOpcode Opcode{};
      bool IsFinalFrame{false};
      std::vector<uint8_t> Payload;
      std::exception_ptr Exception;
      WebSocketInternalFrame(
          SocketOpcode opcode,
          bool isFinalFrame,
          std::vector<uint8_t> const& payload)
          : Opcode(opcode), IsFinalFrame(isFinalFrame), Payload(payload)
      {
      }
      WebSocketInternalFrame(std::exception_ptr exception) : Exception(exception) {}
    };

    struct ReceiveStatistics
    {
      std::atomic<uint32_t> FramesSent;
      std::atomic<uint32_t> FramesReceived;
      std::atomic<uint32_t> BytesSent;
      std::atomic<uint32_t> BytesReceived;
      std::atomic<uint32_t> PingFramesSent;
      std::atomic<uint32_t> PingFramesReceived;
      std::atomic<uint32_t> PongFramesSent;
      std::atomic<uint32_t> PongFramesReceived;
      std::atomic<uint32_t> TextFramesReceived;
      std::atomic<uint32_t> BinaryFramesReceived;
      std::atomic<uint32_t> ContinuationFramesReceived;
      std::atomic<uint32_t> CloseFramesReceived;
      std::atomic<uint32_t> UnknownFramesReceived;
      std::atomic<uint32_t> FramesDropped;
      std::atomic<uint32_t> FramesDroppedByPayloadSizeLimit;
      std::atomic<uint32_t> FramesDroppedByProtocolError;
      std::atomic<uint32_t> TransportReads;
      std::atomic<uint32_t> TransportReadBytes;
      std::atomic<uint32_t> BinaryFramesSent;
      std::atomic<uint32_t> TextFramesSent;
      std::atomic<uint32_t> FramesDroppedByClose;

      void Reset()
      {
        FramesSent = 0;
        BytesSent = 0;
        FramesReceived = 0;
        BytesReceived = 0;
        PingFramesReceived = 0;
        PingFramesSent = 0;
        PongFramesReceived = 0;
        PongFramesSent = 0;
        TextFramesReceived = 0;
        TextFramesSent = 0;
        BinaryFramesReceived = 0;
        BinaryFramesSent = 0;
        ContinuationFramesReceived = 0;
        CloseFramesReceived = 0;
        UnknownFramesReceived = 0;
        FramesDropped = 0;
        FramesDroppedByClose = 0;
        FramesDroppedByPayloadSizeLimit = 0;
        FramesDroppedByProtocolError = 0;
        TransportReads = 0;
        TransportReadBytes = 0;
      }
    };
    /**
     * @brief The PingThread handles sending Ping operations from the WebSocket server.
     *
     */
    class PingThread {
    public:
      /**
       * @brief Construct a new ReceiveQueue object.
       *
       * @param webSocketImplementation Parent object, used to send Ping threads.
       * @param pingInterval Interval to wait between sending pings.
       */
      PingThread(
          WebSocketImplementation* webSocketImplementation,
          std::chrono::duration<int64_t> pingInterval);
      /**
       * @brief Destroys a ReceiveQueue object. Blocks until the queue thread is completed.
       */
      ~PingThread();

      /**
       * @brief Start the receive queue. This will start a thread that will process incoming frames.
       *
       * @param transport The websocket transport to use for receiving frames.
       */
      void Start(std::shared_ptr<WebSocketTransport> transport);
      /**
       * @brief Stop the receive queue. This will stop the thread that processes incoming frames.
       */
      void Shutdown();

    private:
      /**
       * @brief The receive queue thread.
       */
      void PingThreadLoop();
      /**
       * @brief Send a "ping" frame to the other side of the WebSocket.
       *
       * @returns True if the ping was sent, false if the underlying transport didn't support "Ping"
       * operations.
       */
      bool SendPing(std::vector<uint8_t> const& pingData, Azure::Core::Context const& context);

      WebSocketImplementation* m_webSocketImplementation;
      std::chrono::duration<int64_t> m_pingInterval;
      std::thread m_pingThread;
      std::mutex m_pingThreadStarted;
      std::condition_variable m_pingThreadReady;

      std::mutex m_stopMutex;
      std::condition_variable m_pingThreadStopped;
      bool m_stop = false;
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
    static std::vector<uint8_t> EncodeFrame(
        SocketOpcode opcode,
        bool isFinal,
        std::vector<uint8_t> const& payload);

    SocketState m_state{SocketState::Invalid};

    std::vector<uint8_t> GenerateRandomKey() { return GenerateRandomBytes(16); };
    void VerifySocketAccept(std::string const& encodedKey, std::string const& acceptHeader);

    /*********
     * Buffered Read Support. Read data from the underlying transport into a buffer.
     */
    uint8_t ReadTransportByte(Azure::Core::Context const& context);
    uint16_t ReadTransportShort(Azure::Core::Context const& context);
    uint64_t ReadTransportInt64(Azure::Core::Context const& context);
    std::vector<uint8_t> ReadTransportBytes(size_t readLength, Azure::Core::Context const& context);
    bool IsTransportEof() const { return m_eof; }
    void SendPong(std::vector<uint8_t> const& pongData, Azure::Core::Context const& context);
    void SendTransportBuffer(
        std::vector<uint8_t> const& payload,
        Azure::Core::Context const& context);
    std::shared_ptr<WebSocketInternalFrame> ReceiveTransportFrame(
        Azure::Core::Context const& context);

    /**
     * @brief Decode a frame received from the websocket server.
     *
     * @returns A pointer to the start of the decoded data.
     */
    std::shared_ptr<WebSocketInternalFrame> DecodeFrame(Azure::Core::Context const& context);

    Azure::Core::Url m_remoteUrl;
    _internal::WebSocketOptions m_options;
    std::map<std::string, std::string> m_headers;
    std::string m_chosenProtocol;
    std::shared_ptr<Azure::Core::Http::WebSockets::WebSocketTransport> m_transport;
    PingThread m_pingThread;
    SocketMessageType m_currentMessageType{SocketMessageType::Unknown};
    std::mutex m_stateMutex;
    std::thread::id m_stateOwner;

    ReceiveStatistics m_receiveStatistics{};

    std::mutex m_transportMutex;

    std::unique_ptr<Azure::Core::IO::BodyStream> m_initialBodyStream;
    constexpr static size_t m_bufferSize = 1024;
    uint8_t m_buffer[m_bufferSize]{};
    size_t m_bufferPos = 0;
    size_t m_bufferLen = 0;
    bool m_eof = false;
  };
}}}}} // namespace Azure::Core::Http::WebSockets::_detail
