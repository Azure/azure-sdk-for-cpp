// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/http/websockets/websockets.hpp"
#include <random>

namespace Azure { namespace Core { namespace Http { namespace WebSockets { namespace _detail {
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

  class WebSocketFrameEncoder {

  public:
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
        // Payloads greater than 65536 have their length are encoded as a 64 bit unsigned integer in
        // network byte order.
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
      // Calculate the masking key. This MUST be 4 bytes of high entropy random numbers used to mask
      // the input data.
      if (maskOutput)
      {
        // Start by generating the mask - 4 bytes of random data.
        std::random_device randomEngine;

        std::array<uint8_t, 4> rv;
        std::generate(begin(rv), end(rv), std::ref(randomEngine));
        // Append the mask to the payload.
        encodedFrame.insert(encodedFrame.end(), rv.begin(), rv.end());
        size_t index = 0;
        for (auto ch : payload)
        {
          encodedFrame.push_back(ch ^ rv[index % 4]);
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

    /**
     * @brief Decode a frame received from the websocket server.
     *
     * @param paylod Pointer to the payload returned by the service. Note that this may be shorter
     * than the full data in the response message.
     * @param opcode Opcode returned by the server.
     * @param isFinal True if this is the final message.
     * @param maskKey On Return, contains the contents of the mask key if the data is masked.
     * @returns A pointer to the start of the decoded data.
     */
    static uint8_t const* DecodeFrame(
        std::vector<uint8_t> const& payload,
        SocketOpcode& opcode,
        uint64_t& payloadLength,
        bool& isFinal,
        bool& isMasked,
        std::array<uint8_t, 4>& maskKey)
    {
      if (payload.empty() || payload.size() <= 2)
      {
        throw std::runtime_error("Frame buffer is too small.");
      }
      uint8_t const* payloadByte = payload.data();
      opcode = static_cast<SocketOpcode>(*payloadByte & 0x7f);
      isFinal = (*payloadByte & 0x80) != 0;
      payloadByte += 1;
      isMasked = false;
      if (*payloadByte & 0x80)
      {
        isMasked = true;
      }
      payloadLength = *payloadByte & 0x7f;
      if (payloadLength <= 125)
      {
        payloadByte += 1;
      }
      else if (payloadLength == 126)
      {
        if (payload.size() < 4)
        {
          throw std::runtime_error("Payload is too small");
        }
        payloadLength = 0;
        payloadLength |= (static_cast<uint64_t>(*payloadByte++) << 8) & 0xff;
        payloadLength |= (static_cast<uint64_t>(*payloadByte++) & 0xff);
      }
      else if (payloadLength == 127)
      {
        if (payload.size() < 10)
        {
          throw std::runtime_error("Payload is too small");
        }
        payloadLength = 0;
        payloadLength |= (static_cast<uint64_t>(*payloadByte++) << 56) & 0xff00000000000000;
        payloadLength |= (static_cast<uint64_t>(*payloadByte++) << 48) & 0x00ff000000000000;
        payloadLength |= (static_cast<uint64_t>(*payloadByte++) << 40) & 0x0000ff0000000000;
        payloadLength |= (static_cast<uint64_t>(*payloadByte++) << 32) & 0x000000ff00000000;
        payloadLength |= (static_cast<uint64_t>(*payloadByte++) << 24) & 0x00000000ff000000;
        payloadLength |= (static_cast<uint64_t>(*payloadByte++) << 16) & 0x0000000000ff0000;
        payloadLength |= (static_cast<uint64_t>(*payloadByte++) << 8) & 0x000000000000ff00;
        payloadLength |= (static_cast<uint64_t>(*payloadByte)) & 0x000000000000ff00;
      }
      else
      {
        throw std::logic_error("Unexpected payload length.");
      }

      if (isMasked)
      {
        maskKey = {*payloadByte++, *payloadByte++, *payloadByte++, *payloadByte++};
      }
      return payloadByte;
    }
  };

}}}}} // namespace Azure::Core::Http::WebSockets::_detail
