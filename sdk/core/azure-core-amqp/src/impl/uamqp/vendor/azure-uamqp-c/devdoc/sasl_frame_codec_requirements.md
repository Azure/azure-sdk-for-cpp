# sasl_frame_codec requirements
 
## Overview

`sasl_frame_codec` is module that encodes/decodes SASL frames per the AMQP ISO.

## Exposed API

```C
#define SASL_MECHANISMS         (uint64_t)0x40
#define SASL_INIT               (uint64_t)0x41
#define SASL_CHALLENGE          (uint64_t)0x42
#define SASL_RESPONSE           (uint64_t)0x43
#define SASL_OUTCOME            (uint64_t)0x44

typedef struct SASL_FRAME_CODEC_INSTANCE_TAG* SASL_FRAME_CODEC_HANDLE;
typedef void(*ON_SASL_FRAME_RECEIVED)(void* context, AMQP_VALUE sasl_frame_value);
typedef void(*ON_SASL_FRAME_CODEC_ERROR)(void* context);

MOCKABLE_FUNCTION(, SASL_FRAME_CODEC_HANDLE, sasl_frame_codec_create, FRAME_CODEC_HANDLE, frame_codec, ON_SASL_FRAME_RECEIVED, on_sasl_frame_received, ON_SASL_FRAME_CODEC_ERROR, on_sasl_frame_codec_error, void*, callback_context);
MOCKABLE_FUNCTION(, void, sasl_frame_codec_destroy, SASL_FRAME_CODEC_HANDLE, sasl_frame_codec);
MOCKABLE_FUNCTION(, int, sasl_frame_codec_encode_frame, SASL_FRAME_CODEC_HANDLE, sasl_frame_codec, AMQP_VALUE, sasl_frame_value, ON_BYTES_ENCODED, on_bytes_encoded, void*, callback_context);
```

### sasl_frame_codec_create

```C
MOCKABLE_FUNCTION(, SASL_FRAME_CODEC_HANDLE, sasl_frame_codec_create, FRAME_CODEC_HANDLE, frame_codec, ON_SASL_FRAME_RECEIVED, on_sasl_frame_received, ON_SASL_FRAME_CODEC_ERROR, on_sasl_frame_codec_error, void*, callback_context);
```

**SRS_SASL_FRAME_CODEC_01_018: [**sasl_frame_codec_create shall create an instance of an sasl_frame_codec and return a non-NULL handle to it.**]** 
**SRS_SASL_FRAME_CODEC_01_019: [**If any of the arguments frame_codec, on_sasl_frame_received or on_sasl_frame_codec_error is NULL, sasl_frame_codec_create shall return NULL.**]** 
**SRS_SASL_FRAME_CODEC_01_020: [**sasl_frame_codec_create shall subscribe for SASL frames with the given frame_codec.**]** 
**SRS_SASL_FRAME_CODEC_01_021: [**If subscribing for SASL frames fails, sasl_frame_codec_create shall fail and return NULL.**]** 
**SRS_SASL_FRAME_CODEC_01_022: [**sasl_frame_codec_create shall create a decoder to be used for decoding SASL values.**]** 
**SRS_SASL_FRAME_CODEC_01_023: [**If creating the decoder fails, sasl_frame_codec_create shall fail and return NULL.**]** 
**SRS_SASL_FRAME_CODEC_01_024: [**If allocating memory for the new sasl_frame_codec fails, then sasl_frame_codec_create shall fail and return NULL.**]** 

### sasl_frame_codec_destroy

```C
MOCKABLE_FUNCTION(, void, sasl_frame_codec_destroy, SASL_FRAME_CODEC_HANDLE, sasl_frame_codec);
```

**SRS_SASL_FRAME_CODEC_01_025: [**sasl_frame_codec_destroy shall free all resources associated with the sasl_frame_codec instance.**]** 
**SRS_SASL_FRAME_CODEC_01_026: [**If sasl_frame_codec is NULL, sasl_frame_codec_destroy shall do nothing.**]** 
**SRS_SASL_FRAME_CODEC_01_027: [**sasl_frame_codec_destroy shall unsubscribe from receiving SASL frames from the frame_codec that was passed to sasl_frame_codec_create.**]** 
**SRS_SASL_FRAME_CODEC_01_028: [**The decoder created in sasl_frame_codec_create shall be destroyed by sasl_frame_codec_destroy.**]** 

### sasl_frame_codec_encode_frame

```C
MOCKABLE_FUNCTION(, int, sasl_frame_codec_encode_frame, SASL_FRAME_CODEC_HANDLE, sasl_frame_codec, AMQP_VALUE, sasl_frame_value, ON_BYTES_ENCODED, on_bytes_encoded, void*, callback_context);
```

**SRS_SASL_FRAME_CODEC_01_029: [**sasl_frame_codec_encode_frame shall encode the frame header and sasl_frame_value AMQP value in a SASL frame and on success it shall return 0.**]** 
**SRS_SASL_FRAME_CODEC_01_030: [**If sasl_frame_codec or sasl_frame_value is NULL, sasl_frame_codec_encode_frame shall fail and return a non-zero value.**]** 
**SRS_SASL_FRAME_CODEC_01_031: [**sasl_frame_codec_encode_frame shall encode the frame header and its contents by using frame_codec_encode_frame.**]** 
**SRS_SASL_FRAME_CODEC_01_032: [**The payload frame size shall be computed based on the encoded size of the sasl_frame_value and its fields.**]** 
**SRS_SASL_FRAME_CODEC_01_033: [**The encoded size of the sasl_frame_value and its fields shall be obtained by calling amqpvalue_get_encoded_size.**]** 
**SRS_SASL_FRAME_CODEC_01_034: [**If any error occurs during encoding, sasl_frame_codec_encode_frame shall fail and return a non-zero value.**]** 
**SRS_SASL_FRAME_CODEC_01_035: [**Encoding of the sasl_frame_value and its fields shall be done by calling amqpvalue_encode.**]**

### Receive frames

**SRS_SASL_FRAME_CODEC_01_039: [**sasl_frame_codec shall decode the sasl-frame value as a described type.**]** 
**SRS_SASL_FRAME_CODEC_01_040: [**Decoding the sasl-frame type shall be done by feeding the bytes to the decoder create in sasl_frame_codec_create.**]** 
**SRS_SASL_FRAME_CODEC_01_041: [**Once the sasl frame is decoded, the callback on_sasl_frame_received shall be called.**]** 
**SRS_SASL_FRAME_CODEC_01_042: [**The decoded sasl-frame value and the context passed in sasl_frame_codec_create shall be passed to on_sasl_frame_received.**]** 
**SRS_SASL_FRAME_CODEC_01_046: [**If any error occurs while decoding a frame, the decoder shall switch to an error state where decoding shall not be possible anymore.**]** 
**SRS_SASL_FRAME_CODEC_01_049: [**If any error occurs while decoding a frame, the decoder shall call the on_sasl_frame_codec_error and pass to it the callback_context, both of those being the ones given to sasl_frame_codec_create.**]** 

## ISO section (receive)

5.3.1 SASL Frames

SASL performatives are framed as per Part 2: section 2.3. **SRS_SASL_FRAME_CODEC_01_001: [**A SASL frame has a type code of 0x01.**]** **SRS_SASL_FRAME_CODEC_01_006: [**Bytes 6 and 7 of the header are ignored.**]** Implementations SHOULD set these to 0x00. **SRS_SASL_FRAME_CODEC_01_007: [**The extended header is ignored.**]** Implementations SHOULD therefore set DOFF to 0x02.

...

Figure 5.5: SASL Frame

**SRS_SASL_FRAME_CODEC_01_008: [**The maximum size of a SASL frame is defined by MIN-MAX-FRAME-SIZE.**]** There is no mechanism within the SASL negotiation to negotiate a different size. **SRS_SASL_FRAME_CODEC_01_009: [**The frame body of a SASL frame MUST contain exactly one AMQP type, whose type encoding MUST have provides="sasl-frame".**]** **SRS_SASL_FRAME_CODEC_01_010: [**Receipt of an empty frame is an irrecoverable error.**]** 

## ISO section (send)

5.3.1 SASL Frames

SASL performatives are framed as per Part 2: section 2.3. **SRS_SASL_FRAME_CODEC_01_011: [**A SASL frame has a type code of 0x01.**]** **SRS_SASL_FRAME_CODEC_01_012: [**Bytes 6 and 7 of the header are ignored.**]** **SRS_SASL_FRAME_CODEC_01_013: [**Implementations SHOULD set these to 0x00.**]** **SRS_SASL_FRAME_CODEC_01_014: [**The extended header is ignored.**]** **SRS_SASL_FRAME_CODEC_01_015: [**Implementations SHOULD therefore set DOFF to 0x02.**]** 

...

Figure 5.5: SASL Frame

**SRS_SASL_FRAME_CODEC_01_016: [**The maximum size of a SASL frame is defined by MIN-MAX-FRAME-SIZE.**]** There is no mechanism within the SASL negotiation to negotiate a different size.  **SRS_SASL_FRAME_CODEC_01_047: [**The frame body of a SASL frame MUST contain exactly one AMQP type, whose type encoding MUST have provides="sasl-frame".**]** Receipt of an empty frame is an irrecoverable error. 
