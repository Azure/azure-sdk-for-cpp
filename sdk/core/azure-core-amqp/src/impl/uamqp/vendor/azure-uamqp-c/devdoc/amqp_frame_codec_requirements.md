# amqp_frame_codec requirements

## Overview

`amqp_frame_codec` is module that encodes/decodes AMQP frames per the AMQP ISO.

## Exposed API

```C
#define AMQP_OPEN               (uint64_t)0x10
#define AMQP_BEGIN              (uint64_t)0x11
#define AMQP_ATTACH             (uint64_t)0x12
#define AMQP_FLOW               (uint64_t)0x13
#define AMQP_TRANSFER           (uint64_t)0x14
#define AMQP_DISPOSITION        (uint64_t)0x15
#define AMQP_DETACH             (uint64_t)0x16
#define AMQP_END                (uint64_t)0x17
#define AMQP_CLOSE              (uint64_t)0x18

typedef struct AMQP_FRAME_CODEC_INSTANCE_TAG* AMQP_FRAME_CODEC_HANDLE;
typedef void(*AMQP_EMPTY_FRAME_RECEIVED_CALLBACK)(void* context, uint16_t channel);
typedef void(*AMQP_FRAME_RECEIVED_CALLBACK)(void* context, uint16_t channel, AMQP_VALUE performative, const unsigned char* payload_bytes, uint32_t frame_payload_size);
typedef void(*AMQP_FRAME_CODEC_ERROR_CALLBACK)(void* context);

MOCKABLE_FUNCTION(, AMQP_FRAME_CODEC_HANDLE, amqp_frame_codec_create, FRAME_CODEC_HANDLE, frame_codec, AMQP_FRAME_RECEIVED_CALLBACK, frame_received_callback, AMQP_EMPTY_FRAME_RECEIVED_CALLBACK, empty_frame_received_callback, AMQP_FRAME_CODEC_ERROR_CALLBACK, amqp_frame_codec_error_callback, void*, callback_context);
MOCKABLE_FUNCTION(, void, amqp_frame_codec_destroy, AMQP_FRAME_CODEC_HANDLE, amqp_frame_codec);
MOCKABLE_FUNCTION(, int, amqp_frame_codec_encode_frame, AMQP_FRAME_CODEC_HANDLE, amqp_frame_codec, uint16_t, channel, AMQP_VALUE, performative, const PAYLOAD*, payloads, size_t, payload_count, ON_BYTES_ENCODED, on_bytes_encoded, void*, callback_context);
MOCKABLE_FUNCTION(, int, amqp_frame_codec_encode_empty_frame, AMQP_FRAME_CODEC_HANDLE, amqp_frame_codec, uint16_t, channel, ON_BYTES_ENCODED, on_bytes_encoded, void*, callback_context);
```

### amqp_frame_codec_create
```C
MOCKABLE_FUNCTION(, AMQP_FRAME_CODEC_HANDLE, amqp_frame_codec_create, FRAME_CODEC_HANDLE, frame_codec, AMQP_FRAME_RECEIVED_CALLBACK, frame_received_callback, AMQP_EMPTY_FRAME_RECEIVED_CALLBACK, empty_frame_received_callback, AMQP_FRAME_CODEC_ERROR_CALLBACK, amqp_frame_codec_error_callback, void*, callback_context);
```

**SRS_AMQP_FRAME_CODEC_01_011: [**amqp_frame_codec_create shall create an instance of an amqp_frame_codec and return a non-NULL handle to it.**]** 
**SRS_AMQP_FRAME_CODEC_01_012: [**If any of the arguments frame_codec, frame_received_callback, amqp_frame_codec_error_callback or empty_frame_received_callback is NULL, amqp_frame_codec_create shall return NULL.**]** 
**SRS_AMQP_FRAME_CODEC_01_013: [**amqp_frame_codec_create shall subscribe for AMQP frames with the given frame_codec.**]** 
**SRS_AMQP_FRAME_CODEC_01_014: [**If subscribing for AMQP frames fails, amqp_frame_codec_create shall fail and return NULL.**]** 
**SRS_AMQP_FRAME_CODEC_01_018: [**amqp_frame_codec_create shall create a decoder to be used for decoding AMQP values.**]** 
**SRS_AMQP_FRAME_CODEC_01_019: [**If creating the decoder fails, amqp_frame_codec_create shall fail and return NULL.**]** 
**SRS_AMQP_FRAME_CODEC_01_020: [**If allocating memory for the new amqp_frame_codec fails, then amqp_frame_codec_create shall fail and return NULL.**]** 

### amqp_frame_codec_destroy

```C
MOCKABLE_FUNCTION(, void, amqp_frame_codec_destroy, AMQP_FRAME_CODEC_HANDLE, amqp_frame_codec);
```

**SRS_AMQP_FRAME_CODEC_01_015: [**amqp_frame_codec_destroy shall free all resources associated with the amqp_frame_codec instance.**]** 
**SRS_AMQP_FRAME_CODEC_01_016: [**If amqp_frame_codec is NULL, amqp_frame_codec_destroy shall do nothing.**]** 
**SRS_AMQP_FRAME_CODEC_01_017: [**amqp_frame_codec_destroy shall unsubscribe from receiving AMQP frames from the frame_codec that was passed to amqp_frame_codec_create.**]** 
**SRS_AMQP_FRAME_CODEC_01_021: [**The decoder created in amqp_frame_codec_create shall be destroyed by amqp_frame_codec_destroy.**]** 

### amqp_frame_codec_encode_frame

```C
MOCKABLE_FUNCTION(, int, amqp_frame_codec_encode_frame, AMQP_FRAME_CODEC_HANDLE, amqp_frame_codec, uint16_t, channel, AMQP_VALUE, performative, const PAYLOAD*, payloads, size_t, payload_count, ON_BYTES_ENCODED, on_bytes_encoded, void*, callback_context);
```

**SRS_AMQP_FRAME_CODEC_01_022: [**amqp_frame_codec_encode_frame shall encode the frame header and AMQP performative in an AMQP frame and on success it shall return 0.**]** 
**SRS_AMQP_FRAME_CODEC_01_024: [**If frame_codec, performative or on_bytes_encoded is NULL, amqp_frame_codec_encode_frame shall fail and return a non-zero value.**]** 
**SRS_AMQP_FRAME_CODEC_01_025: [**amqp_frame_codec_encode_frame shall encode the frame header by using frame_codec_encode_frame.**]** 
**SRS_AMQP_FRAME_CODEC_01_026: [**The payload frame size shall be computed based on the encoded size of the performative and its fields plus the sum of the payload sizes passed via the payloads argument.**]** 
**SRS_AMQP_FRAME_CODEC_01_027: [**The encoded size of the performative and its fields shall be obtained by calling amqpvalue_get_encoded_size.**]** 
**SRS_AMQP_FRAME_CODEC_01_029: [**If any error occurs during encoding, amqp_frame_codec_encode_frame shall fail and return a non-zero value.**]** 
**SRS_AMQP_FRAME_CODEC_01_030: [**Encoding of the AMQP performative and its fields shall be done by calling amqpvalue_encode.**]** 
**SRS_AMQP_FRAME_CODEC_01_028: [**The encode result for the performative shall be placed in a PAYLOAD structure.**]** 
**SRS_AMQP_FRAME_CODEC_01_070: [**The payloads argument for frame_codec_encode_frame shall be made of the payload for the encoded performative and the payloads passed to amqp_frame_codec_encode_frame.**]** 

### amqp_frame_codec_encode_empty_frame

```C
MOCKABLE_FUNCTION(, int, amqp_frame_codec_encode_empty_frame, AMQP_FRAME_CODEC_HANDLE, amqp_frame_codec, uint16_t, channel, ON_BYTES_ENCODED, on_bytes_encoded, void*, callback_context);
```

**SRS_AMQP_FRAME_CODEC_01_042: [**amqp_frame_codec_encode_empty_frame shall encode a frame with no payload.**]** 
**SRS_AMQP_FRAME_CODEC_01_043: [**On success, amqp_frame_codec_encode_empty_frame shall return 0.**]** 
**SRS_AMQP_FRAME_CODEC_01_044: [**amqp_frame_codec_encode_empty_frame shall use frame_codec_encode_frame to encode the frame.**]** 
**SRS_AMQP_FRAME_CODEC_01_045: [**If amqp_frame_codec is NULL, amqp_frame_codec_encode_empty_frame shall fail and return a non-zero value.**]** 
**SRS_AMQP_FRAME_CODEC_01_046: [**If encoding fails in any way, amqp_frame_codec_encode_empty_frame shall fail and return a non-zero value.**]** 

### Receive frames

**SRS_AMQP_FRAME_CODEC_01_048: [**When a frame header is received from frame_codec and the frame payload size is 0, empty_frame_received_callback shall be invoked, while passing the channel number as argument.**]** 
**SRS_AMQP_FRAME_CODEC_01_049: [**If not enough type specific bytes are received to decode the channel number, the decoding shall stop with an error.**]** 
**SRS_AMQP_FRAME_CODEC_01_050: [**All subsequent decoding shall fail and no AMQP frames shall be indicated from that point on to the consumers of amqp_frame_codec.**]** 
**SRS_AMQP_FRAME_CODEC_01_051: [**If the frame payload is greater than 0, amqp_frame_codec shall decode the performative as a described AMQP type.**]** 
**SRS_AMQP_FRAME_CODEC_01_052: [**Decoding the performative shall be done by feeding the bytes to the decoder create in amqp_frame_codec_create.**]** 
**SRS_AMQP_FRAME_CODEC_01_067: [**When the performative is decoded, the rest of the frame_bytes shall not be given to the AMQP decoder, but they shall be buffered so that later they are given to the frame_received callback.**]** 
**SRS_AMQP_FRAME_CODEC_01_054: [**Once the performative is decoded and all frame payload bytes are received, the callback frame_received_callback shall be called.**]** 
**SRS_AMQP_FRAME_CODEC_01_055: [**The decoded channel and performative shall be passed to frame_received_callback.**]** 
**SRS_AMQP_FRAME_CODEC_01_056: [**The AMQP frame payload size passed to frame_received_callback shall be computed from the frame payload size received from frame_codec and substracting the performative size.**]** 
**SRS_AMQP_FRAME_CODEC_01_068: [**A pointer to all the payload bytes shall also be passed to frame_received_callback.**]** 
**SRS_AMQP_FRAME_CODEC_01_060: [**If any error occurs while decoding a frame, the decoder shall switch to an error state where decoding shall not be possible anymore.**]** 
**SRS_AMQP_FRAME_CODEC_01_069: [**If any error occurs while decoding a frame, the decoder shall indicate the error by calling the amqp_frame_codec_error_callback  and passing to it the callback context argument that was given in amqp_frame_codec_create.**]** 

### ISO section (receive)

2.3.2 AMQP Frames

**SRS_AMQP_FRAME_CODEC_01_001: [**Bytes 6 and 7 of an AMQP frame contain the channel number **]** (see section 2.1). 
**SRS_AMQP_FRAME_CODEC_01_002: [**The frame body is defined as a performative followed by an opaque payload.**]** 
**SRS_AMQP_FRAME_CODEC_01_003: [**The performative MUST be one of those defined in section 2.7 and is encoded as a described type in the AMQP type system.**]** 
**SRS_AMQP_FRAME_CODEC_01_004: [**The remaining bytes in the frame body form the payload for that frame.**]** The presence and format of the payload is defined by the semantics of the given performative.

...

Figure 2.16: AMQP Frame Layout

**SRS_AMQP_FRAME_CODEC_01_007: [**An AMQP frame with no body MAY be used to generate artificial traffic as needed to satisfy any negotiated idle timeout interval **]** (see subsection 2.4.5).

### ISO section (send)

2.3.2 AMQP Frames

**SRS_AMQP_FRAME_CODEC_01_005: [**Bytes 6 and 7 of an AMQP frame contain the channel number **]** (see section 2.1). 
**SRS_AMQP_FRAME_CODEC_01_006: [**The frame body is defined as a performative followed by an opaque payload.**]** 
**SRS_AMQP_FRAME_CODEC_01_008: [**The performative MUST be one of those defined in section 2.7 and is encoded as a described type in the AMQP type system.**]** 
**SRS_AMQP_FRAME_CODEC_01_009: [**The remaining bytes in the frame body form the payload for that frame.**]** The presence and format of the payload is defined by the semantics of the given performative.

...

Figure 2.16: AMQP Frame Layout

**SRS_AMQP_FRAME_CODEC_01_010: [**An AMQP frame with no body MAY be used to generate artificial traffic as needed to satisfy any negotiated idle timeout interval **]** (see subsection 2.4.5).
