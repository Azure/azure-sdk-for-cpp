# frame_codec requirements
 
## Overview

`frame_codec` is module that encodes/decodes frames (regardless of their type).

## Exposed API

```C
typedef struct PAYLOAD_TAG
{
    const unsigned char* bytes;
    size_t length;
} PAYLOAD;

#define FRAME_TYPE_AMQP    (uint8_t)0x00

#define FRAME_TYPE_SASL    (uint8_t)0x01

    typedef struct FRAME_CODEC_INSTANCE_TAG* FRAME_CODEC_HANDLE;
    typedef void(*ON_FRAME_RECEIVED)(void* context, const unsigned char* type_specific, uint32_t type_specific_size, const unsigned char* frame_body, uint32_t frame_body_size);
    typedef void(*ON_FRAME_CODEC_ERROR)(void* context);
    typedef void(*ON_BYTES_ENCODED)(void* context, const unsigned char* bytes, size_t length, bool encode_complete);

    MOCKABLE_FUNCTION(, FRAME_CODEC_HANDLE, frame_codec_create, ON_FRAME_CODEC_ERROR, on_frame_codec_error, void*, callback_context);
    MOCKABLE_FUNCTION(, void, frame_codec_destroy, FRAME_CODEC_HANDLE, frame_codec);
    MOCKABLE_FUNCTION(, int, frame_codec_set_max_frame_size, FRAME_CODEC_HANDLE, frame_codec, uint32_t, max_frame_size);
    MOCKABLE_FUNCTION(, int, frame_codec_subscribe, FRAME_CODEC_HANDLE, frame_codec, uint8_t, type, ON_FRAME_RECEIVED, on_frame_received, void*, callback_context);
    MOCKABLE_FUNCTION(, int, frame_codec_unsubscribe, FRAME_CODEC_HANDLE, frame_codec, uint8_t, type);
    MOCKABLE_FUNCTION(, int, frame_codec_receive_bytes, FRAME_CODEC_HANDLE, frame_codec, const unsigned char*, buffer, size_t, size);
    MOCKABLE_FUNCTION(, int, frame_codec_encode_frame, FRAME_CODEC_HANDLE, frame_codec, uint8_t, type, const PAYLOAD*, payloads, size_t, payload_count, const unsigned char*, type_specific_bytes, uint32_t, type_specific_size, ON_BYTES_ENCODED, on_bytes_encoded, void*, callback_context);
```

### frame_codec_create

```C
MOCKABLE_FUNCTION(, FRAME_CODEC_HANDLE, frame_codec_create, ON_FRAME_CODEC_ERROR, on_frame_codec_error, void*, callback_context);
```

**SRS_FRAME_CODEC_01_021: [**frame_codec_create shall create a new instance of frame_codec and return a non-NULL handle to it on success.**]** 
**SRS_FRAME_CODEC_01_020: [**If the on_frame_codec_error argument is NULL, frame_codec_create shall return NULL.**]** 
**SRS_FRAME_CODEC_01_022: [**If allocating memory for the frame_codec instance fails, frame_codec_create shall return NULL.**]** 
**SRS_FRAME_CODEC_01_082: [**The initial max_frame_size_shall be 512.**]** 
**SRS_FRAME_CODEC_01_104: [**The callback_context shall be allowed to be NULL.**]** 

### frame_codec_destroy

```C
MOCKABLE_FUNCTION(, void, frame_codec_destroy, FRAME_CODEC_HANDLE, frame_codec);
```

**SRS_FRAME_CODEC_01_023: [**frame_codec_destroy shall free all resources associated with a frame_codec instance.**]** 
**SRS_FRAME_CODEC_01_024: [**If frame_codec is NULL, frame_codec_destroy shall do nothing.**]** 

### frame_codec_set_max_frame_size

```C
MOCKABLE_FUNCTION(, int, frame_codec_set_max_frame_size, FRAME_CODEC_HANDLE, frame_codec, uint32_t, max_frame_size);
```

**SRS_FRAME_CODEC_01_075: [**frame_codec_set_max_frame_size shall set the maximum frame size for a frame_codec.**]** 
**SRS_FRAME_CODEC_01_076: [**On success, frame_codec_set_max_frame_size shall return 0.**]** 
**SRS_FRAME_CODEC_01_077: [**If frame_codec is NULL, frame_codec_set_max_frame_size shall return a non-zero value.**]** 
**SRS_FRAME_CODEC_01_078: [**If max_frame_size is invalid according to the AMQP standard, frame_codec_set_max_frame_size shall return a non-zero value.**]** 
**SRS_FRAME_CODEC_01_079: [**The new frame size shall take effect immediately, even for a frame that is being decoded at the time of the call.**]** 
**SRS_FRAME_CODEC_01_081: [**If a frame being decoded already has a size bigger than the max_frame_size argument then frame_codec_set_max_frame_size shall return a non-zero value and the previous frame size shall be kept.**]** 
**SRS_FRAME_CODEC_01_097: [**Setting a frame size on a frame_codec that had a decode error shall fail.**]** 

### frame_codec_subscribe

```C
MOCKABLE_FUNCTION(, int, frame_codec_subscribe, FRAME_CODEC_HANDLE, frame_codec, uint8_t, type, ON_FRAME_RECEIVED, on_frame_received, void*, callback_context);
```

**SRS_FRAME_CODEC_01_033: [**frame_codec_subscribe subscribes for a certain type of frame received by the frame_codec instance identified by frame_codec.**]** 
**SRS_FRAME_CODEC_01_087: [**On success, frame_codec_subscribe shall return zero.**]** 
**SRS_FRAME_CODEC_01_034: [**If any of the frame_codec or on_frame_received arguments is NULL, frame_codec_subscribe shall return a non-zero value.**]** 
**SRS_FRAME_CODEC_01_035: [**After successfully registering a callback for a certain frame type, when subsequently that frame type is received the callbacks shall be invoked, passing to it the received frame and the callback_context value.**]** 
**SRS_FRAME_CODEC_01_036: [**Only one callback pair shall be allowed to be registered for a given frame type.**]** 
**SRS_FRAME_CODEC_01_037: [**If any failure occurs while performing the subscribe operation, frame_codec_subscribe shall return a non-zero value.**]** 

### frame_codec_unsubscribe

```C
MOCKABLE_FUNCTION(, int, frame_codec_unsubscribe, FRAME_CODEC_HANDLE, frame_codec, uint8_t, type);
```

**SRS_FRAME_CODEC_01_038: [**frame_codec_unsubscribe removes a previous subscription for frames of type type and on success it shall return 0.**]** 
**SRS_FRAME_CODEC_01_039: [**If frame_codec is NULL, frame_codec_unsubscribe shall return a non-zero value.**]** 
**SRS_FRAME_CODEC_01_040: [**If no subscription for the type frame type exists, frame_codec_unsubscribe shall return a non-zero value.**]** 
**SRS_FRAME_CODEC_01_041: [**If any failure occurs while performing the unsubscribe operation, frame_codec_unsubscribe shall return a non-zero value.**]** 

### frame_codec_receive_bytes

```C
MOCKABLE_FUNCTION(, int, frame_codec_receive_bytes, FRAME_CODEC_HANDLE, frame_codec, const unsigned char*, buffer, size_t, size);
```

**SRS_FRAME_CODEC_01_025: [**frame_codec_receive_bytes decodes a sequence of bytes into frames and on success it shall return zero.**]** 
**SRS_FRAME_CODEC_01_026: [**If frame_codec or buffer are NULL, frame_codec_receive_bytes shall return a non-zero value.**]** 
**SRS_FRAME_CODEC_01_027: [**If size is zero, frame_codec_receive_bytes shall return a non-zero value.**]** 
**SRS_FRAME_CODEC_01_028: [**The sequence of bytes shall be decoded according to the AMQP ISO.**]** 
**SRS_FRAME_CODEC_01_029: [**The sequence of bytes does not have to be a complete frame, frame_codec shall be responsible for maintaining decoding state between frame_codec_receive_bytes calls.**]** 
**SRS_FRAME_CODEC_01_030: [**If a decoding error occurs, frame_codec_receive_bytes shall return a non-zero value.**]** 
**SRS_FRAME_CODEC_01_074: [**If a decoding error is detected, any subsequent calls on frame_codec_receive_bytes shall fail.**]** 
**SRS_FRAME_CODEC_01_031: [**When a complete frame is successfully decoded it shall be indicated to the upper layer by invoking the on_frame_received passed to frame_codec_subscribe.**]** 
**SRS_FRAME_CODEC_01_032: [**Besides passing the frame information, the callback_context value passed to frame_codec_subscribe shall be passed to the on_frame_received function.**]** 
**SRS_FRAME_CODEC_01_099: [**A pointer to the frame_body bytes shall also be passed to the on_frame_received.**]** 
**SRS_FRAME_CODEC_01_102: [**frame_codec_receive_bytes shall allocate memory to hold the frame_body bytes.**]** 
**SRS_FRAME_CODEC_01_101: [**If the memory for the frame_body bytes cannot be allocated, frame_codec_receive_bytes shall fail and return a non-zero value.**]** 
**SRS_FRAME_CODEC_01_100: [**If the frame body size is 0, the frame_body pointer passed to on_frame_received shall be NULL.**]** 
**SRS_FRAME_CODEC_01_096: [**If a frame bigger than the current max frame size is received, frame_codec_receive_bytes shall fail and return a non-zero value.**]** 
**SRS_FRAME_CODEC_01_103: [**Upon any decode error, if an error callback has been passed to frame_codec_create, then the error callback shall be called with the context argument being the frame_codec_error_callback_context argument passed to frame_codec_create.**]**

### frame_codec_encode_frame

```C
MOCKABLE_FUNCTION(, int, frame_codec_encode_frame, FRAME_CODEC_HANDLE, frame_codec, uint8_t, type, const PAYLOAD*, payloads, size_t, payload_count, const unsigned char*, type_specific_bytes, uint32_t, type_specific_size, ON_BYTES_ENCODED, on_bytes_encoded, void*, callback_context);
```

**SRS_FRAME_CODEC_01_042: [**frame_codec_encode_frame encodes the header, type specific bytes and frame payload of a frame that has frame_payload_size bytes.**]** 
**SRS_FRAME_CODEC_01_043: [**On success it shall return 0.**]** 
**SRS_FRAME_CODEC_01_110: [** If the `bytes` member of a payload entry is NULL, `frame_codec_encode_frame` shall fail and return a non-zero value. **]**
**SRS_FRAME_CODEC_01_111: [** If the `length` member of a payload entry is 0, `frame_codec_encode_frame` shall fail and return a non-zero value. **]**
**SRS_FRAME_CODEC_01_108: [** Memory shall be allocated to hold the entire frame. **]**
**SRS_FRAME_CODEC_01_109: [** If allocating memory fails, `frame_codec_encode_frame` shall fail and return a non-zero value. **]**
**SRS_FRAME_CODEC_01_044: [**If any of arguments `frame_codec` or `on_bytes_encoded` is NULL, `frame_codec_encode_frame` shall return a non-zero value.**]** 
**SRS_FRAME_CODEC_01_107: [**If the argument `payloads` is NULL and `payload_count` is non-zero, `frame_codec_encode_frame` shall return a non-zero value.**]** 
**SRS_FRAME_CODEC_01_090: [**If the type_specific_size - 2 does not divide by 4, frame_codec_encode_frame shall pad the type_specific bytes with zeroes so that type specific data is according to the AMQP ISO.**]** 
**SRS_FRAME_CODEC_01_092: [**If type_specific_size is too big to allow encoding the frame according to the AMQP ISO then frame_codec_encode_frame shall return a non-zero value.**]** 
**SRS_FRAME_CODEC_01_091: [**If the argument type_specific_size is greater than 0 and type_specific_bytes is NULL, frame_codec_encode_frame shall return a non-zero value.**]** 
**SRS_FRAME_CODEC_01_105: [**The frame_payload_size shall be computed by summing up the lengths of the payload segments identified by the payloads argument.**]** 
**SRS_FRAME_CODEC_01_106: [**All payloads shall be encoded in order as part of the frame.**]** 
**SRS_FRAME_CODEC_01_088: [**Encoded bytes shall be passed to the `on_bytes_encoded` callback in a single call, while setting the `encode complete` argument to true.**]** 
**SRS_FRAME_CODEC_01_095: [**If the frame_size needed for the frame is bigger than the maximum frame size, frame_codec_encode_frame shall fail and return a non-zero value.**]** 

## ISO section (receive)

2.3 Framing

**SRS_FRAME_CODEC_01_001: [**Frames are divided into three distinct areas: a fixed width frame header, a variable width extended header, and a variable width frame body.**]** 

...

Figure 2.14: Frame Layout

**SRS_FRAME_CODEC_01_002: [**frame header The frame header is a fixed size (8 byte) structure that precedes each frame.**]** 
**SRS_FRAME_CODEC_01_003: [**The frame header includes mandatory information necessary to parse the rest of the frame including size and type information.**]** 
**SRS_FRAME_CODEC_01_004: [**extended header The extended header is a variable width area preceding the frame body.**]** 
**SRS_FRAME_CODEC_01_005: [**This is an extension point defined for future expansion.**]** 
**SRS_FRAME_CODEC_01_006: [**The treatment of this area depends on the frame type.**]** 
**SRS_FRAME_CODEC_01_007: [**frame body The frame body is a variable width sequence of bytes the format of which depends on the frame type.**]** 

2.3.1 Frame Layout

The diagram below shows the details of the general frame layout for all frame types.

...

Figure 2.15: General Frame Layout

**SRS_FRAME_CODEC_01_008: [**SIZE Bytes 0-3 of the frame header contain the frame size.**]** 
**SRS_FRAME_CODEC_01_009: [**This is an unsigned 32-bit integer that MUST contain the total frame size of the frame header, extended header, and frame body.**]** 
**SRS_FRAME_CODEC_01_010: [**The frame is malformed if the size is less than the size of the frame header (8 bytes).**]** 
**SRS_FRAME_CODEC_01_011: [**DOFF Byte 4 of the frame header is the data offset.**]** 
**SRS_FRAME_CODEC_01_012: [**This gives the position of the body within the frame.**]** 
**SRS_FRAME_CODEC_01_013: [**The value of the data offset is an unsigned, 8-bit integer specifying a count of 4-byte words.**]** 
**SRS_FRAME_CODEC_01_014: [**Due to the mandatory 8-byte frame header, the frame is malformed if the value is less than 2.**]** 
**SRS_FRAME_CODEC_01_015: [**TYPE Byte 5 of the frame header is a type code.**]** 
**SRS_FRAME_CODEC_01_016: [**The type code indicates the format and purpose of the frame.**]** 
**SRS_FRAME_CODEC_01_017: [**The subsequent bytes in the frame header MAY be interpreted differently depending on the type of the frame.**]** 
**SRS_FRAME_CODEC_01_018: [**A type code of 0x00 indicates that the frame is an AMQP frame.**]** 
**SRS_FRAME_CODEC_01_019: [**A type code of 0x01 indicates that the frame is a SASL frame**]** , see Part 5: section 5.3.

## ISO section (send)

2.3 Framing

**SRS_FRAME_CODEC_01_055: [**Frames are divided into three distinct areas: a fixed width frame header, a variable width extended header, and a variable width frame body.**]** 

...

Figure 2.14: Frame Layout

**SRS_FRAME_CODEC_01_056: [**frame header The frame header is a fixed size (8 byte) structure that precedes each frame.**]** 
**SRS_FRAME_CODEC_01_057: [**The frame header includes mandatory information necessary to parse the rest of the frame including size and type information.**]** 
**SRS_FRAME_CODEC_01_058: [**extended header The extended header is a variable width area preceding the frame body.**]** 
**SRS_FRAME_CODEC_01_059: [**This is an extension point defined for future expansion.**]** 
**SRS_FRAME_CODEC_01_060: [**The treatment of this area depends on the frame type.**]** 
**SRS_FRAME_CODEC_01_061: [**frame body The frame body is a variable width sequence of bytes the format of which depends on the frame type.**]** 

2.3.1 Frame Layout

The diagram below shows the details of the general frame layout for all frame types.

...

Figure 2.15: General Frame Layout

**SRS_FRAME_CODEC_01_062: [**SIZE Bytes 0-3 of the frame header contain the frame size.**]** 
**SRS_FRAME_CODEC_01_063: [**This is an unsigned 32-bit integer that MUST contain the total frame size of the frame header, extended header, and frame body.**]** 
**SRS_FRAME_CODEC_01_064: [**The frame is malformed if the size is less than the size of the frame header (8 bytes).**]** 
**SRS_FRAME_CODEC_01_065: [**DOFF Byte 4 of the frame header is the data offset.**]**
**SRS_FRAME_CODEC_01_066: [**This gives the position of the body within the frame.**]** 
**SRS_FRAME_CODEC_01_067: [**The value of the data offset is an unsigned, 8-bit integer specifying a count of 4-byte words.**]** 
**SRS_FRAME_CODEC_01_068: [**Due to the mandatory 8-byte frame header, the frame is malformed if the value is less than 2.**]** 
**SRS_FRAME_CODEC_01_069: [**TYPE Byte 5 of the frame header is a type code.**]**
**SRS_FRAME_CODEC_01_070: [**The type code indicates the format and purpose of the frame.**]** 
**SRS_FRAME_CODEC_01_071: [**The subsequent bytes in the frame header MAY be interpreted differently depending on the type of the frame.**]** 
**SRS_FRAME_CODEC_01_072: [**A type code of 0x00 indicates that the frame is an AMQP frame.**]** 
**SRS_FRAME_CODEC_01_073: [**A type code of 0x01 indicates that the frame is a SASL frame**]** , see Part 5: section 5.3.
