# uws_frame_encoder requirements

## Overview

uws_frame_encoder is module that implements the WebSocket frame encoding rules.

## References

RFC6455 - The WebSocket Protocol.

## Exposed API

```c
#define RESERVED_1  0x04
#define RESERVED_2  0x02
#define RESERVED_3  0x01

#define WS_FRAME_TYPE_VALUES \
    WS_CONTINUATION_FRAME = 0x00, \
    WS_TEXT_FRAME = 0x01, \
    WS_BINARY_FRAME = 0x02, \
    WS_RESERVED_NON_CONTROL_FRAME_3 = 0x03, \
    WS_RESERVED_NON_CONTROL_FRAME_4 = 0x04, \
    WS_RESERVED_NON_CONTROL_FRAME_5 = 0x05, \
    WS_RESERVED_NON_CONTROL_FRAME_6 = 0x06, \
    WS_RESERVED_NON_CONTROL_FRAME_7 = 0x07, \
    WS_CLOSE_FRAME = 0x08, \
    WS_PING_FRAME = 0x09, \
    WS_PONG_FRAME = 0x0A, \
    WS_RESERVED_CONTROL_FRAME_B = 0x0B, \
    WS_RESERVED_CONTROL_FRAME_C = 0x0C, \
    WS_RESERVED_CONTROL_FRAME_D = 0x0D, \
    WS_RESERVED_CONTROL_FRAME_E = 0x0E, \
    WS_RESERVED_CONTROL_FRAME_F = 0x0F

MU_DEFINE_ENUM(WS_FRAME_TYPE, WS_FRAME_TYPE_VALUES);

extern int uws_frame_encoder_encode(BUFFER_HANDLE encode_buffer, WS_FRAME_TYPE opcode, const unsigned char* payload, size_t length, bool is_masked, bool is_final, unsigned char reserved);
```

###  uws_create

```c
extern int uws_frame_encoder_encode(BUFFER_HANDLE encode_buffer, WS_FRAME_TYPE opcode, const unsigned char* payload, size_t length, bool is_masked, bool is_final, unsigned char reserved);
```

**SRS_UWS_FRAME_ENCODER_01_001: [** `uws_frame_encoder_encode` shall encode the information given in `opcode`, `payload`, `length`, `is_masked`, `is_final` and `reserved` according to the RFC6455 into a new buffer. **]**

**SRS_UWS_FRAME_ENCODER_01_044: [** On success `uws_frame_encoder_encode` shall return a non-NULL handle to the result buffer. **]**

**SRS_UWS_FRAME_ENCODER_01_054: [** If `length` is greater than 0 and payload is NULL, then `uws_frame_encoder_encode` shall fail and return NULL. **]**

**SRS_UWS_FRAME_ENCODER_01_048: [** The newly created buffer shall be created by calling `BUFFER_new`. **]**

**SRS_UWS_FRAME_ENCODER_01_049: [** If `BUFFER_new` fails then `uws_frame_encoder_encode` shall fail and return NULL. **]**

**SRS_UWS_FRAME_ENCODER_01_046: [** The result buffer shall be resized accordingly using `BUFFER_enlarge`. **]**

**SRS_UWS_FRAME_ENCODER_01_047: [** If `BUFFER_enlarge` fails then `uws_frame_encoder_encode` shall fail and return NULL. **]**

**SRS_UWS_FRAME_ENCODER_01_050: [** The allocated memory shall be accessed by calling `BUFFER_u_char`. **]**

**SRS_UWS_FRAME_ENCODER_01_051: [** If `BUFFER_u_char` fails then `uws_frame_encoder_encode` shall fail and return a NULL. **]**

**SRS_UWS_FRAME_ENCODER_01_052: [** If `reserved` has any bits set except the lowest 3 then `uws_frame_encoder_encode` shall fail and return NULL. **]**

**SRS_UWS_FRAME_ENCODER_01_053: [** In order to obtain a 32 bit value for masking, `gb_rand` shall be used 4 times (for each byte). **]**

###  RFC6455 relevant parts

5.  Data Framing

5.2.  Base Framing Protocol

   This wire format for the data transfer part is described by the ABNF [RFC5234] given in detail in this section.
   (Note that, unlike in other sections of this document, the ABNF in this section is operating on groups of bits.
   The length of each group of bits is indicated in a comment.
   When encoded on the wire, the most significant bit is the leftmost in the ABNF).
   A high-level overview of the framing is given in the following figure.
   In a case of conflict between the figure below and the ABNF specified later in this section, the figure is authoritative.

      0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-------+-+-------------+-------------------------------+
     |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
     |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
     |N|V|V|V|       |S|             |   (if payload len==126/127)   |
     | |1|2|3|       |K|             |                               |
     +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
     |     Extended payload length continued, if payload len == 127  |
     + - - - - - - - - - - - - - - - +-------------------------------+
     |                               |Masking-key, if MASK set to 1  |
     +-------------------------------+-------------------------------+
     | Masking-key (continued)       |          Payload Data         |
     +-------------------------------- - - - - - - - - - - - - - - - +
     :                     Payload Data continued ...                :
     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
     |                     Payload Data continued ...                |
     +---------------------------------------------------------------+

   FIN:  1 bit

      **SRS_UWS_FRAME_ENCODER_01_002: [** Indicates that this is the final fragment in a message. **]**
      **SRS_UWS_FRAME_ENCODER_01_003: [** The first fragment MAY also be the final fragment. **]**

   RSV1, RSV2, RSV3:  1 bit each

      **SRS_UWS_FRAME_ENCODER_01_004: [** MUST be 0 unless an extension is negotiated that defines meanings for non-zero values. **]**
      If a nonzero value is received and none of the negotiated extensions defines the meaning of such a nonzero value, the receiving endpoint MUST _Fail the WebSocket Connection_.

   Opcode:  4 bits

      Defines the interpretation of the "Payload data".
      **SRS_UWS_FRAME_ENCODER_01_006: [** If an unknown opcode is received, the receiving endpoint MUST _Fail the WebSocket Connection_. **]**
      The following values are defined.

      **SRS_UWS_FRAME_ENCODER_01_007: [** *  %x0 denotes a continuation frame **]**

      **SRS_UWS_FRAME_ENCODER_01_008: [** *  %x1 denotes a text frame **]**

      **SRS_UWS_FRAME_ENCODER_01_009: [** *  %x2 denotes a binary frame **]**

      **SRS_UWS_FRAME_ENCODER_01_010: [** *  %x3-7 are reserved for further non-control frames **]**

      **SRS_UWS_FRAME_ENCODER_01_011: [** *  %x8 denotes a connection close **]**

      **SRS_UWS_FRAME_ENCODER_01_012: [** *  %x9 denotes a ping **]**

      **SRS_UWS_FRAME_ENCODER_01_013: [** *  %xA denotes a pong **]**

      **SRS_UWS_FRAME_ENCODER_01_014: [** *  %xB-F are reserved for further control frames **]**

   Mask:  1 bit

      **SRS_UWS_FRAME_ENCODER_01_015: [** Defines whether the "Payload data" is masked. **]**
      **SRS_UWS_FRAME_ENCODER_01_016: [** If set to 1, a masking key is present in masking-key, and this is used to unmask the "Payload data" as per Section 5.3. **]**
      All frames sent from client to server have this bit set to 1.

   Payload length:  7 bits, 7+16 bits, or 7+64 bits

      **SRS_UWS_FRAME_ENCODER_01_018: [** The length of the "Payload data", in bytes: **]** **SRS_UWS_FRAME_ENCODER_01_043: [** if 0-125, that is the payload length. **]**
      **SRS_UWS_FRAME_ENCODER_01_019: [** If 126, the following 2 bytes interpreted as a 16-bit unsigned integer are the payload length. **]**
      **SRS_UWS_FRAME_ENCODER_01_020: [** If 127, the following 8 bytes interpreted as a 64-bit unsigned integer (the most significant bit MUST be 0) are the payload length. **]**
      **SRS_UWS_FRAME_ENCODER_01_021: [** Multibyte length quantities are expressed in network byte order. **]**
      **SRS_UWS_FRAME_ENCODER_01_022: [** Note that in all cases, the minimal number of bytes MUST be used to encode the length, for example, the length of a 124-byte-long string can't be encoded as the sequence 126, 0, 124. **]**
      **SRS_UWS_FRAME_ENCODER_01_023: [** The payload length is the length of the "Extension data" + the length of the "Application data". **]**
      The length of the "Extension data" may be zero, in which case the payload length is the length of the "Application data".

   Masking-key:  0 or 4 bytes

      All frames sent from the client to the server are masked by a 32-bit value that is contained within the frame.
      **SRS_UWS_FRAME_ENCODER_01_026: [** This field is present if the mask bit is set to 1 and is absent if the mask bit is set to 0. **]**
      See Section 5.3 for further information on client-to-server masking.

   Payload data:  (x+y) bytes

      The "Payload data" is defined as "Extension data" concatenated with "Application data".

   Extension data:  x bytes

      The "Extension data" is 0 bytes unless an extension has been negotiated.
      Any extension MUST specify the length of the "Extension data", or how that length may be calculated, and how the extension use MUST be negotiated during the opening handshake.
      If present, the "Extension data" is included in the total payload length.

   Application data:  y bytes

      Arbitrary "Application data", taking up the remainder of the frame after any "Extension data".
      The length of the "Application data" is equal to the payload length minus the length of the "Extension data".

   The base framing protocol is formally defined by the following ABNF [RFC5234].
   It is important to note that the representation of this data is binary, not ASCII characters.
   As such, a field with a length of 1 bit that takes values %x0 / %x1 is represented as a single bit whose value is 0 or 1, not a full byte (octet) that stands for the characters "0" or "1" in the ASCII encoding.
   A field with a length of 4 bits with values between %x0-F again is represented by 4 bits, again NOT by an ASCII character or full byte (octet) with these values.
   [RFC5234] does not specify a character encoding: "Rules resolve into a string of terminal values, sometimes called characters. In ABNF, a character is merely a non-negative integer. In certain contexts, a specific mapping (encoding) of values into a character set (such as ASCII) will be specified."
   Here, the specified encoding is a binary encoding where each terminal value is encoded in the specified number of bits, which varies for each field.

    ws-frame                = frame-fin           ; 1 bit in length
                              frame-rsv1          ; 1 bit in length
                              frame-rsv2          ; 1 bit in length
                              frame-rsv3          ; 1 bit in length
                              frame-opcode        ; 4 bits in length
                              frame-masked        ; 1 bit in length
                              frame-payload-length   ; either 7, 7+16,
                                                     ; or 7+64 bits in
                                                     ; length
                              [ frame-masking-key ]  ; 32 bits in length
                              frame-payload-data     ; n*8 bits in
                                                     ; length, where
                                                     ; n >= 0

    frame-fin               = %x0 ; more frames of this message follow
                            / %x1 ; final frame of this message
                                  ; 1 bit in length

    frame-rsv1              = %x0 / %x1
                              ; 1 bit in length, MUST be 0 unless
                              ; negotiated otherwise

    frame-rsv2              = %x0 / %x1
                              ; 1 bit in length, MUST be 0 unless
                              ; negotiated otherwise

    frame-rsv3              = %x0 / %x1
                              ; 1 bit in length, MUST be 0 unless
                              ; negotiated otherwise

    frame-opcode            = frame-opcode-non-control /
                              frame-opcode-control /
                              frame-opcode-cont

    frame-opcode-cont       = %x0 ; frame continuation

    frame-opcode-non-control= %x1 ; text frame
                            / %x2 ; binary frame
                            / %x3-7
                            ; 4 bits in length,
                            ; reserved for further non-control frames

    frame-opcode-control    = %x8 ; connection close
                            / %x9 ; ping
                            / %xA ; pong
                            / %xB-F ; reserved for further control
                                    ; frames
                                    ; 4 bits in length

    frame-masked            = %x0
                            ; frame is not masked, no frame-masking-key
                            / %x1
                            ; frame is masked, frame-masking-key present
                            ; 1 bit in length

    frame-payload-length    = ( %x00-7D )
                            / ( %x7E frame-payload-length-16 )
                            / ( %x7F frame-payload-length-63 )
                            ; 7, 7+16, or 7+64 bits in length,
                            ; respectively

    frame-payload-length-16 = %x0000-FFFF ; 16 bits in length

    frame-payload-length-63 = %x0000000000000000-7FFFFFFFFFFFFFFF
                            ; 64 bits in length

    frame-masking-key       = 4( %x00-FF )
                              ; present only if frame-masked is 1
                              ; 32 bits in length

    frame-payload-data      = (frame-masked-extension-data
                               frame-masked-application-data)
                            ; when frame-masked is 1
                              / (frame-unmasked-extension-data
                                frame-unmasked-application-data)
                            ; when frame-masked is 0

    frame-masked-extension-data     = *( %x00-FF )
                            ; reserved for future extensibility
                            ; n*8 bits in length, where n >= 0

    frame-masked-application-data   = *( %x00-FF )
                            ; n*8 bits in length, where n >= 0

    frame-unmasked-extension-data   = *( %x00-FF )
                            ; reserved for future extensibility
                            ; n*8 bits in length, where n >= 0

    frame-unmasked-application-data = *( %x00-FF )
                            ; n*8 bits in length, where n >= 0

5.3.  Client-to-Server Masking

   **SRS_UWS_FRAME_ENCODER_01_033: [** A masked frame MUST have the field frame-masked set to 1, as defined in Section 5.2. **]**

   **SRS_UWS_FRAME_ENCODER_01_034: [** The masking key is contained completely within the frame, as defined in Section 5.2 as frame-masking-key. **]**
   
   **SRS_UWS_FRAME_ENCODER_01_035: [** It is used to mask the "Payload data" defined in the same section as frame-payload-data, which includes "Extension data" and "Application data". **]**

   **SRS_UWS_FRAME_ENCODER_01_036: [** The masking key is a 32-bit value chosen at random by the client. **]**

   **SRS_UWS_FRAME_ENCODER_01_037: [** When preparing a masked frame, the client MUST pick a fresh masking key from the set of allowed 32-bit values. **]**

   **SRS_UWS_FRAME_ENCODER_01_038: [** The masking key needs to be unpredictable; thus, the masking key MUST be derived from a strong source of entropy, and the masking key for a given frame MUST NOT make it simple for a server/proxy to predict the masking key for a subsequent frame. **]**

   The unpredictability of the masking key is essential to prevent authors of malicious applications from selecting the bytes that appear on the wire.
   RFC 4086 [RFC4086] discusses what entails a suitable source of entropy for security-sensitive applications.

   The masking does not affect the length of the "Payload data".

   **SRS_UWS_FRAME_ENCODER_01_039: [** To convert masked data into unmasked data, or vice versa, the following algorithm is applied. **]**

   **SRS_UWS_FRAME_ENCODER_01_040: [** The same algorithm applies regardless of the direction of the translation, e.g., the same steps are applied to mask the data as to unmask the data. **]**

   **SRS_UWS_FRAME_ENCODER_01_041: [** Octet i of the transformed data ("transformed-octet-i") is the XOR of octet i of the original data ("original-octet-i") with octet at index i modulo 4 of the masking key ("masking-key-octet-j"): **]**

     j                   = i MOD 4
     transformed-octet-i = original-octet-i XOR masking-key-octet-j

   **SRS_UWS_FRAME_ENCODER_01_042: [** The payload length, indicated in the framing as frame-payload-length, does NOT include the length of the masking key. **]**
   It is the length of the "Payload data", e.g., the number of bytes following the masking key.
