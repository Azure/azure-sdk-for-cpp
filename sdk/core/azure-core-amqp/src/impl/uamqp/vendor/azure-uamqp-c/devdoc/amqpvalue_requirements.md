# amqpvalue requirements

## Overview

`amqpvalue` is module that encapsulates the typesystem of AMQP. It allows, creating AMQP type values from native C types, destroying them and converting the types to native C types.

## Exposed API

```C
    typedef struct AMQP_VALUE_DATA_TAG* AMQP_VALUE;
    typedef unsigned char uuid[16];
    typedef int64_t timestamp;

    typedef struct amqp_binary_TAG
    {
        const void* bytes;
        uint32_t length;
    } amqp_binary;

    /* type handling */
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_null);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_boolean, bool, bool_value);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_boolean, AMQP_VALUE, value, bool*, bool_value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_ubyte, unsigned char, ubyte_value);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_ubyte, AMQP_VALUE, value, unsigned char*, ubyte_value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_ushort, uint16_t, ushort_value);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_ushort, AMQP_VALUE, value, uint16_t*, ushort_value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_uint, uint32_t, uint_value);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_uint, AMQP_VALUE, value, uint32_t*, uint_value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_ulong, uint64_t, ulong_value);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_ulong, AMQP_VALUE, value, uint64_t*, ulong_value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_byte, char, byte_value);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_byte, AMQP_VALUE, value, char*, byte_value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_short, int16_t, short_value);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_short, AMQP_VALUE, value, int16_t*, short_value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_int, int32_t, int_value);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_int, AMQP_VALUE, value, int32_t*, int_value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_long, int64_t, long_value);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_long, AMQP_VALUE, value, int64_t*, long_value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_float, float, float_value);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_float, AMQP_VALUE, value, float*, float_value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_double, double, double_value);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_double, AMQP_VALUE, value, double*, double_value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_char, uint32_t, char_value);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_char, AMQP_VALUE, value, uint32_t*, char_value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_timestamp, int64_t, timestamp_value);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_timestamp, AMQP_VALUE, value, int64_t*, timestamp_value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_uuid, uuid, uuid_value);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_uuid, AMQP_VALUE, value, uuid*, uuid_value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_binary, amqp_binary, binary_value);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_binary, AMQP_VALUE, value, amqp_binary*, binary_value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_string, const char*, string_value);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_string, AMQP_VALUE, value, const char**, string_value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_symbol, const char*, symbol_value);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_symbol, AMQP_VALUE, value, const char**, symbol_value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_list);
    MOCKABLE_FUNCTION(, int, amqpvalue_set_list_item_count, AMQP_VALUE, list, uint32_t, count);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_list_item_count, AMQP_VALUE, list, uint32_t*, count);
    MOCKABLE_FUNCTION(, int, amqpvalue_set_list_item, AMQP_VALUE, list, uint32_t, index, AMQP_VALUE, list_item_value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_get_list_item, AMQP_VALUE, list, size_t, index);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_map);
    MOCKABLE_FUNCTION(, int, amqpvalue_set_map_value, AMQP_VALUE, map, AMQP_VALUE, key, AMQP_VALUE, value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_get_map_value, AMQP_VALUE, map, AMQP_VALUE, key);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_map_pair_count, AMQP_VALUE, map, uint32_t*, pair_count);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_map_key_value_pair, AMQP_VALUE, map, uint32_t, index, AMQP_VALUE*, key, AMQP_VALUE*, value);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_map, AMQP_VALUE, from_value, AMQP_VALUE*, map);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_array);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_array_item_count, AMQP_VALUE, value, uint32_t*, count);
    MOCKABLE_FUNCTION(, int, amqpvalue_add_array_item, AMQP_VALUE, value, AMQP_VALUE, array_item_value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_get_array_item, AMQP_VALUE, value, uint32_t, index);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_array, AMQP_VALUE, value, AMQP_VALUE*, array_value);
    MOCKABLE_FUNCTION(, AMQP_TYPE, amqpvalue_get_type, AMQP_VALUE, value);

    MOCKABLE_FUNCTION(, void, amqpvalue_destroy, AMQP_VALUE, value);

    MOCKABLE_FUNCTION(, bool, amqpvalue_are_equal, AMQP_VALUE, value1, AMQP_VALUE, value2);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_clone, AMQP_VALUE, value);

    /* encoding */
    typedef int (*AMQPVALUE_ENCODER_OUTPUT)(void* context, const unsigned char* bytes, size_t length);

    MOCKABLE_FUNCTION(, int, amqpvalue_encode, AMQP_VALUE, value, AMQPVALUE_ENCODER_OUTPUT, encoder_output, void*, context);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_encoded_size, AMQP_VALUE, value, size_t*, encoded_size);

    /* decoding */
    typedef struct AMQPVALUE_DECODER_HANDLE_DATA_TAG* AMQPVALUE_DECODER_HANDLE;
    typedef void(*ON_VALUE_DECODED)(void* context, AMQP_VALUE decoded_value);

    MOCKABLE_FUNCTION(, AMQPVALUE_DECODER_HANDLE, amqpvalue_decoder_create, ON_VALUE_DECODED, on_value_decoded, void*, callback_context);
    MOCKABLE_FUNCTION(, void, amqpvalue_decoder_destroy, AMQPVALUE_DECODER_HANDLE, handle);
    MOCKABLE_FUNCTION(, int, amqpvalue_decode_bytes, AMQPVALUE_DECODER_HANDLE, handle, const unsigned char*, buffer, size_t, size);

    /* misc for now, not spec'd */
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_get_inplace_descriptor, AMQP_VALUE, value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_get_inplace_described_value, AMQP_VALUE, value);

    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_composite, AMQP_VALUE, descriptor, uint32_t, list_size);
    MOCKABLE_FUNCTION(, int, amqpvalue_set_composite_item, AMQP_VALUE, value, uint32_t, index, AMQP_VALUE, item_value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_get_composite_item, AMQP_VALUE, value, size_t, index);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_described, AMQP_VALUE, descriptor, AMQP_VALUE, value);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_composite_with_ulong_descriptor, uint64_t, descriptor);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_get_list_item_in_place, AMQP_VALUE, value, size_t, index);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_get_composite_item_in_place, AMQP_VALUE, value, size_t, index);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_composite_item_count, AMQP_VALUE, value, uint32_t*, item_count);
```

### amqpvalue_create_null

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_null);
```

**SRS_AMQPVALUE_01_001: [**amqpvalue_create_null shall return a handle to an AMQP_VALUE that stores a null value.**]**
**SRS_AMQPVALUE_01_002: [**If allocating the AMQP_VALUE fails then amqpvalue_create_null shall return NULL.**]** 

### amqpvalue_create_boolean

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_boolean, bool, bool_value);
```

**SRS_AMQPVALUE_01_006: [**amqpvalue_create_boolean shall return a handle to an AMQP_VALUE that stores a boolean value.**]**
**SRS_AMQPVALUE_01_007: [**If allocating the AMQP_VALUE fails then amqpvalue_create_boolean shall return NULL.**]** 

### amqpvalue_get_boolean

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_boolean, AMQP_VALUE, value, bool*, bool_value);
```

**SRS_AMQPVALUE_01_008: [**amqpvalue_get_boolean shall fill in the bool_value argument the Boolean value stored by the AMQP value indicated by the value argument.**]**
**SRS_AMQPVALUE_01_010: [**On success amqpvalue_get_boolean shall return 0.**]**
**SRS_AMQPVALUE_01_009: [**If any of the arguments is NULL then amqpvalue_get_boolean shall return a non-zero value.**]**
**SRS_AMQPVALUE_01_011: [**If the type of the value is not Boolean, then amqpvalue_get_boolean shall return a non-zero value.**]**

### amqpvalue_create_ubyte

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_ubyte, unsigned char, ubyte_value);
```

**SRS_AMQPVALUE_01_032: [**amqpvalue_create_ubyte shall return a handle to an AMQP_VALUE that stores a unsigned char value.**]**
**SRS_AMQPVALUE_01_033: [**If allocating the AMQP_VALUE fails then amqpvalue_create_ubyte shall return NULL.**]** 

### amqpvalue_get_ubyte

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_ubyte, AMQP_VALUE, value, unsigned char*, ubyte_value);
```

**SRS_AMQPVALUE_01_034: [**amqpvalue_get_ubyte shall fill in the ubyte_value argument the unsigned char value stored by the AMQP value indicated by the value argument.**]**
**SRS_AMQPVALUE_01_035: [**On success amqpvalue_get_ubyte shall return 0.**]**
**SRS_AMQPVALUE_01_036: [**If any of the arguments is NULL then amqpvalue_get_ubyte shall return a non-zero value.**]**
**SRS_AMQPVALUE_01_037: [**If the type of the value is not ubyte (was not created with amqpvalue_create_ubyte), then amqpvalue_get_ubyte shall return a non-zero value.**]** 

### amqpvalue_create_ushort

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_ushort, uint16_t, ushort_value);
```

**SRS_AMQPVALUE_01_038: [**amqpvalue_create_ushort shall return a handle to an AMQP_VALUE that stores an uint16_t value.**]**
**SRS_AMQPVALUE_01_039: [**If allocating the AMQP_VALUE fails then amqpvalue_create_ushort shall return NULL.**]** 

### amqpvalue_get_ushort

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_ushort, AMQP_VALUE, value, uint16_t*, ushort_value);
```

**SRS_AMQPVALUE_01_040: [**amqpvalue_get_ushort shall fill in the ushort_value argument the uint16_t value stored by the AMQP value indicated by the value argument.**]**
**SRS_AMQPVALUE_01_041: [**On success amqpvalue_get_ushort shall return 0.**]**
**SRS_AMQPVALUE_01_042: [**If any of the arguments is NULL then amqpvalue_get_ushort shall return a non-zero value.**]**
**SRS_AMQPVALUE_01_043: [**If the type of the value is not ushort (was not created with amqpvalue_create_ushort), then amqpvalue_get_ushort shall return a non-zero value.**]** 

### amqpvalue_create_uint

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_uint, uint32_t, uint_value);
```

**SRS_AMQPVALUE_01_044: [**amqpvalue_create_uint shall return a handle to an AMQP_VALUE that stores an uint32_t value.**]**
**SRS_AMQPVALUE_01_045: [**If allocating the AMQP_VALUE fails then amqpvalue_create_uint shall return NULL.**]** 

### amqpvalue_get_uint

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_uint, AMQP_VALUE, value, uint32_t*, uint_value);
```

**SRS_AMQPVALUE_01_046: [**amqpvalue_get_uint shall fill in the uint_value argument the uint32_t value stored by the AMQP value indicated by the value argument.**]**
**SRS_AMQPVALUE_01_047: [**On success amqpvalue_get_uint shall return 0.**]**
**SRS_AMQPVALUE_01_079: [**If any of the arguments is NULL then amqpvalue_get_uint shall return a non-zero value.**]**
**SRS_AMQPVALUE_01_048: [**If the type of the value is not uint (was not created with amqpvalue_create_uint), then amqpvalue_get_uint shall return a non-zero value.**]** 

### amqpvalue_create_ulong

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_ulong, uint64_t, ulong_value);
```

**SRS_AMQPVALUE_01_049: [**amqpvalue_create_ulong shall return a handle to an AMQP_VALUE that stores an uint64_t value.**]**
**SRS_AMQPVALUE_01_050: [**If allocating the AMQP_VALUE fails then amqpvalue_create_ulong shall return NULL.**]** 

### amqpvalue_get_ulong

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_ulong, AMQP_VALUE, value, uint64_t*, ulong_value);
```

**SRS_AMQPVALUE_01_051: [**amqpvalue_get_ulong shall fill in the ulong_value argument the ulong64_t value stored by the AMQP value indicated by the value argument.**]**
**SRS_AMQPVALUE_01_052: [**On success amqpvalue_get_ulong shall return 0.**]**
**SRS_AMQPVALUE_01_053: [**If any of the arguments is NULL then amqpvalue_get_ulong shall return a non-zero value.**]**
**SRS_AMQPVALUE_01_054: [**If the type of the value is not ulong (was not created with amqpvalue_create_ulong), then amqpvalue_get_ulong shall return a non-zero value.**]** 

### amqpvalue_create_byte

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_byte, char, byte_value);
```

**SRS_AMQPVALUE_01_055: [**amqpvalue_create_byte shall return a handle to an AMQP_VALUE that stores a char value.**]**
**SRS_AMQPVALUE_01_056: [**If allocating the AMQP_VALUE fails then amqpvalue_create_byte shall return NULL.**]** 

### amqpvalue_get_byte

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_byte, AMQP_VALUE, value, char*, byte_value);
```

**SRS_AMQPVALUE_01_057: [**amqpvalue_get_byte shall fill in the byte_value argument the char value stored by the AMQP value indicated by the value argument.**]**
**SRS_AMQPVALUE_01_058: [**On success amqpvalue_get_byte shall return 0.**]**
**SRS_AMQPVALUE_01_059: [**If any of the arguments is NULL then amqpvalue_get_byte shall return a non-zero value.**]**
**SRS_AMQPVALUE_01_060: [**If the type of the value is not byte (was not created with amqpvalue_create_byte), then amqpvalue_get_byte shall return a non-zero value.**]** 

### amqpvalue_create_short

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_short, int16_t, short_value);
```

**SRS_AMQPVALUE_01_061: [**amqpvalue_create_short shall return a handle to an AMQP_VALUE that stores an int16_t value.**]**
**SRS_AMQPVALUE_01_062: [**If allocating the AMQP_VALUE fails then amqpvalue_create_short shall return NULL.**]** 

### amqpvalue_get_short

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_short, AMQP_VALUE, value, int16_t*, short_value);
```

**SRS_AMQPVALUE_01_063: [**amqpvalue_get_short shall fill in the short_value argument the int16_t value stored by the AMQP value indicated by the value argument.**]**
**SRS_AMQPVALUE_01_064: [**On success amqpvalue_get_short shall return 0.**]**
**SRS_AMQPVALUE_01_065: [**If any of the arguments is NULL then amqpvalue_get_short shall return a non-zero value.**]**
**SRS_AMQPVALUE_01_066: [**If the type of the value is not short (was not created with amqpvalue_create_short), then amqpvalue_get_short shall return a non-zero value.**]** 

### amqpvalue_create_int

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_int, int32_t, int_value);
```

**SRS_AMQPVALUE_01_067: [**amqpvalue_create_int shall return a handle to an AMQP_VALUE that stores an int32_t value.**]**
**SRS_AMQPVALUE_01_068: [**If allocating the AMQP_VALUE fails then amqpvalue_create_int shall return NULL.**]** 

### amqpvalue_get_int

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_int, AMQP_VALUE, value, int32_t*, int_value);
```

**SRS_AMQPVALUE_01_069: [**amqpvalue_get_int shall fill in the int_value argument the int32_t value stored by the AMQP value indicated by the value argument.**]**
**SRS_AMQPVALUE_01_070: [**On success amqpvalue_get_int shall return 0.**]**
**SRS_AMQPVALUE_01_071: [**If any of the arguments is NULL then amqpvalue_get_int shall return a non-zero value.**]**
**SRS_AMQPVALUE_01_072: [**If the type of the value is not int (was not created with amqpvalue_create_int), then amqpvalue_get_int shall return a non-zero value.**]** 

### amqpvalue_create_long

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_long, int64_t, long_value);
```

**SRS_AMQPVALUE_01_073: [**amqpvalue_create_long shall return a handle to an AMQP_VALUE that stores an int64_t value.**]**
**SRS_AMQPVALUE_01_074: [**If allocating the AMQP_VALUE fails then amqpvalue_create_long shall return NULL.**]** 


### amqpvalue_get_long

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_long, AMQP_VALUE, value, int64_t*, long_value);
```

**SRS_AMQPVALUE_01_075: [**amqpvalue_get_long shall fill in the long_value argument the int64_t value stored by the AMQP value indicated by the value argument.**]**
**SRS_AMQPVALUE_01_076: [**On success amqpvalue_get_long shall return 0.**]**
**SRS_AMQPVALUE_01_077: [**If any of the arguments is NULL then amqpvalue_get_long shall return a non-zero value.**]**
**SRS_AMQPVALUE_01_078: [**If the type of the value is not long (was not created with amqpvalue_create_long), then amqpvalue_get_long shall return a non-zero value.**]**

### amqpvalue_create_float

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_float, float, float_value);
```

**SRS_AMQPVALUE_01_080: [**amqpvalue_create_float shall return a handle to an AMQP_VALUE that stores a float value.**]**
**SRS_AMQPVALUE_01_081: [**If allocating the AMQP_VALUE fails then amqpvalue_create_float shall return NULL.**]** 

### amqpvalue_get_float

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_float, AMQP_VALUE, value, float*, float_value);
```

**SRS_AMQPVALUE_01_082: [**amqpvalue_get_float shall fill in the float_value argument the float value stored by the AMQP value indicated by the value argument.**]**
**SRS_AMQPVALUE_01_083: [**On success amqpvalue_get_float shall return 0.**]**
**SRS_AMQPVALUE_01_084: [**If any of the arguments is NULL then amqpvalue_get_float shall return a non-zero value.**]**
**SRS_AMQPVALUE_01_085: [**If the type of the value is not float (was not created with amqpvalue_create_float), then amqpvalue_get_float shall return a non-zero value.**]**

### amqpvalue_create_double

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_double, double, double_value);
```

**SRS_AMQPVALUE_01_086: [**amqpvalue_create_double shall return a handle to an AMQP_VALUE that stores a double value.**]**
**SRS_AMQPVALUE_01_087: [**If allocating the AMQP_VALUE fails then amqpvalue_create_double shall return NULL.**]**

### amqpvalue_get_double

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_double, AMQP_VALUE, value, double*, double_value);
```

**SRS_AMQPVALUE_01_088: [**amqpvalue_get_double shall fill in the double_value argument the double value stored by the AMQP value indicated by the value argument.**]**
**SRS_AMQPVALUE_01_089: [**On success amqpvalue_get_double shall return 0.**]**
**SRS_AMQPVALUE_01_090: [**If any of the arguments is NULL then amqpvalue_get_double shall return a non-zero value.**]**
**SRS_AMQPVALUE_01_091: [**If the type of the value is not double (was not created with amqpvalue_create_double), then amqpvalue_get_double shall return a non-zero value.**]** 

### amqpvalue_create_char

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_char, uint32_t, char_value);
```

**SRS_AMQPVALUE_01_092: [**amqpvalue_create_char shall return a handle to an AMQP_VALUE that stores a single UTF-32 character value.**]**
**SRS_AMQPVALUE_01_093: [**If allocating the AMQP_VALUE fails then amqpvalue_create_char shall return NULL.**]**
**SRS_AMQPVALUE_01_098: [**If the code point value is outside of the allowed range [0, 0x10FFFF**]** then amqpvalue_create_char shall return NULL.] 

### amqpvalue_get_char

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_char, AMQP_VALUE, value, uint32_t*, char_value);
```

**SRS_AMQPVALUE_01_094: [**amqpvalue_get_char shall fill in the char_value argument the UTF32 char value stored by the AMQP value indicated by the value argument.**]**
**SRS_AMQPVALUE_01_095: [**On success amqpvalue_get_char shall return 0.**]**
**SRS_AMQPVALUE_01_096: [**If any of the arguments is NULL then amqpvalue_get_char shall return a non-zero value.**]**
**SRS_AMQPVALUE_01_097: [**If the type of the value is not char (was not created with amqpvalue_create_char), then amqpvalue_get_char shall return a non-zero value.**]** 

### amqpvalue_create_timestamp

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_timestamp, int64_t, timestamp_value);
```

**SRS_AMQPVALUE_01_107: [**amqpvalue_create_timestamp shall return a handle to an AMQP_VALUE that stores an uint64_t value that represents a millisecond precision Unix time.**]**
**SRS_AMQPVALUE_01_108: [**If allocating the AMQP_VALUE fails then amqpvalue_create_timestamp shall return NULL.**]** 

### amqpvalue_get_timestamp

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_timestamp, AMQP_VALUE, value, int64_t*, timestamp_value);
```

**SRS_AMQPVALUE_01_109: [**amqpvalue_get_timestamp shall fill in the timestamp_value argument the timestamp value stored by the AMQP value indicated by the value argument.**]**
**SRS_AMQPVALUE_01_110: [**On success amqpvalue_get_timestamp shall return 0.**]**
**SRS_AMQPVALUE_01_111: [**If any of the arguments is NULL then amqpvalue_get_timestamp shall return a non-zero value.**]**
**SRS_AMQPVALUE_01_112: [**If the type of the value is not timestamp (was not created with amqpvalue_create_timestamp), then amqpvalue_get_timestamp shall return a non-zero value.**]** 

### amqpvalue_create_uuid

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_uuid, uuid, uuid_value);
```

**SRS_AMQPVALUE_01_113: [**amqpvalue_create_uuid shall return a handle to an AMQP_VALUE that stores an amqp_uuid value that represents a unique identifier per RFC-4122 section 4.1.2.**]**
**SRS_AMQPVALUE_01_114: [**If allocating the AMQP_VALUE fails then amqpvalue_create_uuid shall return NULL.**]** 

### amqpvalue_get_uuid

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_uuid, AMQP_VALUE, value, uuid*, uuid_value);
```

**SRS_AMQPVALUE_01_115: [**amqpvalue_get_uuid shall fill in the uuid_value argument the uuid value stored by the AMQP value indicated by the value argument.**]**
**SRS_AMQPVALUE_01_116: [**On success amqpvalue_get_uuid shall return 0.**]**
**SRS_AMQPVALUE_01_117: [**If any of the arguments is NULL then amqpvalue_get_uuid shall return a non-zero value.**]**
**SRS_AMQPVALUE_01_118: [**If the type of the value is not uuid (was not created with amqpvalue_create_uuid), then amqpvalue_get_uuid shall return a non-zero value.**]** 

### amqpvalue_create_binary

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_binary, amqp_binary, binary_value);
```

**SRS_AMQPVALUE_01_127: [**amqpvalue_create_binary shall return a handle to an AMQP_VALUE that stores a sequence of bytes.**]**
**SRS_AMQPVALUE_01_128: [**If allocating the AMQP_VALUE fails then amqpvalue_create_binary shall return NULL.**]**
**SRS_AMQPVALUE_01_129: [**If value.data is NULL and value.length is positive then amqpvalue_create_binary shall return NULL.**]**
**SRS_AMQPVALUE_01_130: [**If any other error occurs, amqpvalue_create_binary shall return NULL.**]** 

### amqpvalue_get_binary

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_binary, AMQP_VALUE, value, amqp_binary*, binary_value);
```

**SRS_AMQPVALUE_01_131: [**amqpvalue_get_binary shall yield a pointer to the sequence of bytes held by the AMQP_VALUE in binary_value.data and fill in the binary_value.length argument the number of bytes held in the binary value.**]**
**SRS_AMQPVALUE_01_132: [**If any of the arguments is NULL then amqpvalue_get_binary shall return NULL.**]**
**SRS_AMQPVALUE_01_133: [**If the type of the value is not binary (was not created with amqpvalue_create_binary), then amqpvalue_get_binary shall return NULL.**]**

### amqpvalue_create_string

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_string, const char*, string_value);
```

**SRS_AMQPVALUE_01_135: [**amqpvalue_create_string shall return a handle to an AMQP_VALUE that stores a sequence of Unicode characters.**]**
**SRS_AMQPVALUE_01_136: [**If allocating the AMQP_VALUE fails then amqpvalue_create_string shall return NULL.**]**
**SRS_AMQPVALUE_01_137: [**If any other error occurs, amqpvalue_create_string shall return NULL.**]** 

### amqpvalue_get_string

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_string, AMQP_VALUE, value, const char**, string_value);
```

**SRS_AMQPVALUE_01_138: [**amqpvalue_get_string shall yield a pointer to the sequence of bytes held by the AMQP_VALUE in string_value.**]**
**SRS_AMQPVALUE_01_141: [**On success, amqpvalue_get_string shall return 0.**]**
**SRS_AMQPVALUE_01_139: [**If any of the arguments is NULL then amqpvalue_get_string shall return a non-zero value.**]**
**SRS_AMQPVALUE_01_140: [**If the type of the value is not string (was not created with amqpvalue_create_string), then amqpvalue_get_string shall return a non-zero value.**]** 

### amqpvalue_create_symbol

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_symbol, const char*, symbol_value);
```

**SRS_AMQPVALUE_01_142: [**amqpvalue_create_symbol shall return a handle to an AMQP_VALUE that stores a symbol (ASCII string) value.**]**
**SRS_AMQPVALUE_01_400: [**If value is NULL, amqpvalue_create_symbol shall fail and return NULL.**]**
**SRS_AMQPVALUE_01_143: [**If allocating the AMQP_VALUE fails then amqpvalue_create_symbol shall return NULL.**]**
**SRS_AMQPVALUE_01_401: [** If the string pointed to by value is longer than 2^32-1 then amqpvalue_create_symbol shall return NULL. **]**

### amqpvalue_get_symbol

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_symbol, AMQP_VALUE, value, const char**, symbol_value);
```

**SRS_AMQPVALUE_01_145: [**amqpvalue_get_symbol shall fill in the symbol_value the symbol value string held by the AMQP_VALUE.**]**
**SRS_AMQPVALUE_01_146: [**On success, amqpvalue_get_symbol shall return 0.**]**
**SRS_AMQPVALUE_01_147: [**If any of the arguments is NULL then amqpvalue_get_symbol shall return a non-zero value.**]**
**SRS_AMQPVALUE_01_148: [**If the type of the value is not symbol (was not created with amqpvalue_create_symbol), then amqpvalue_get_symbol shall return a non-zero value.**]** 

### amqpvalue_create_list

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_list);
```

**SRS_AMQPVALUE_01_149: [**amqpvalue_create_list shall return a handle to an AMQP_VALUE that stores a list.**]**
**SRS_AMQPVALUE_01_150: [**If allocating the AMQP_VALUE fails then amqpvalue_create_list shall return NULL.**]**
**SRS_AMQPVALUE_01_151: [**The list shall have an initial size of zero.**]** 

### amqpvalue_set_list_item_count

```C
MOCKABLE_FUNCTION(, int, amqpvalue_set_list_item_count, AMQP_VALUE, list, uint32_t, count);
```

**SRS_AMQPVALUE_01_152: [**amqpvalue_set_list_item_count shall resize an AMQP list.**]**
**SRS_AMQPVALUE_01_153: [**On success amqpvalue_set_list_item_count shall return 0.**]**
**SRS_AMQPVALUE_01_155: [**If the value argument is NULL, amqpvalue_set_list_item_count shall return a non-zero value.**]**
**SRS_AMQPVALUE_01_154: [**If allocating memory for the list according to the new size fails, then amqpvalue_set_list_item_count shall return a non-zero value, while preserving the existing list contents.**]**
**SRS_AMQPVALUE_01_156: [**If the value is not of type list, then amqpvalue_set_list_item_count shall return a non-zero value.**]**
**SRS_AMQPVALUE_01_161: [**When the list is shrunk, the extra items shall be freed by using amqp_value_destroy.**]**
**SRS_AMQPVALUE_01_162: [**When a list is grown a null AMQP_VALUE shall be inserted as new list items to fill the list up to the new size.**]** 

### amqpvalue_get_list_item_count

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_list_item_count, AMQP_VALUE, list, uint32_t*, count);
```

**SRS_AMQPVALUE_01_157: [**amqpvalue_get_list_item_count shall fill in the size argument the number of items held by the AMQP list.**]**
**SRS_AMQPVALUE_01_158: [**On success amqpvalue_get_list_item_count shall return 0.**]**
**SRS_AMQPVALUE_01_159: [**If any of the arguments are NULL, amqpvalue_get_list_item_count shall return a non-zero value.**]**
**SRS_AMQPVALUE_01_160: [**If the AMQP_VALUE is not a list then amqpvalue_get_list_item_count shall return a non-zero value.**]** 

### amqpvalue_set_list_item

```C
MOCKABLE_FUNCTION(, int, amqpvalue_set_list_item, AMQP_VALUE, list, uint32_t, index, AMQP_VALUE, list_item_value);
```

**SRS_AMQPVALUE_01_163: [**amqpvalue_set_list_item shall replace the item at the 0 based index-th position in the list identified by the value argument with the AMQP_VALUE specified by list_item_value.**]**
**SRS_AMQPVALUE_01_164: [**On success amqpvalue_set_list_item shall return 0.**]**
**SRS_AMQPVALUE_01_165: [**If value or list_item_value is NULL, amqpvalue_set_list_item shall fail and return a non-zero value.**]**
**SRS_AMQPVALUE_01_166: [**If index is greater than the current list item count, the list shall be grown to accommodate the new item.**]**
**SRS_AMQPVALUE_01_172: [**If growing the list fails, then amqpvalue_set_list_item shall fail and return a non-zero value.**]**
**SRS_AMQPVALUE_01_167: [**Any previous value stored at the position index in the list shall be freed by using amqpvalue_destroy.**]**
**SRS_AMQPVALUE_01_168: [**The item stored at the index-th position in the list shall be a clone of list_item_value.**]**
**SRS_AMQPVALUE_01_169: [**If cloning the item fails, amqpvalue_set_list_item shall fail and return a non-zero value.**]**
**SRS_AMQPVALUE_01_170: [**When amqpvalue_set_list_item fails due to not being able to clone the item or grow the list, the list shall not be altered.**]**
**SRS_AMQPVALUE_01_171: [**If the list_item_value_would result in a list with an encoding that would exceed the ISO limits, amqpvalue_set_list_item shall fail and return a non-zero value.**]** 

### amqpvalue_get_list_item

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_get_list_item, AMQP_VALUE, list, size_t, index);
```

**SRS_AMQPVALUE_01_173: [**amqpvalue_get_list_item shall return a copy of the AMQP_VALUE stored at the 0 based position index in the list identified by value.**]**
**SRS_AMQPVALUE_01_174: [**If the value argument is NULL, amqpvalue_get_list_item shall fail and return NULL.**]**
**SRS_AMQPVALUE_01_175: [**If index is greater or equal to the number of items in the list then amqpvalue_get_list_item shall fail and return NULL.**]**
**SRS_AMQPVALUE_01_176: [**If cloning the item at position index fails, then amqpvalue_get_list_item shall fail and return NULL.**]**
**SRS_AMQPVALUE_01_177: [**If value is not a list then amqpvalue_get_list_item shall fail and return NULL.**]** 

### amqpvalue_create_map

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_map);
```

**SRS_AMQPVALUE_01_178: [**amqpvalue_create_map shall create an AMQP value that holds a map and return a handle to it.**]**
**SRS_AMQPVALUE_01_179: [**If allocating memory for the map fails, then amqpvalue_create_map shall return NULL.**]**
**SRS_AMQPVALUE_01_180: [**The number of key/value pairs in the newly created map shall be zero.**]** 

### amqpvalue_set_map_value

```C
MOCKABLE_FUNCTION(, int, amqpvalue_set_map_value, AMQP_VALUE, map, AMQP_VALUE, key, AMQP_VALUE, value);
```

**SRS_AMQPVALUE_01_181: [**amqpvalue_set_map_value shall set the value in the map identified by the map argument for a key/value pair identified by the key argument.**]**
**SRS_AMQPVALUE_01_182: [**On success amqpvalue_set_map_value shall return 0.**]**
**SRS_AMQPVALUE_01_183: [**If any of the arguments are NULL, amqpvalue_set_map_value shall fail and return a non-zero value.**]**
**SRS_AMQPVALUE_01_184: [**If the key already exists in the map, its value shall be replaced with the value provided by the value argument.**]**
**SRS_AMQPVALUE_01_185: [**When storing the key or value, their contents shall be cloned.**]**
**SRS_AMQPVALUE_01_186: [**If allocating memory to hold a new key/value pair fails, amqpvalue_set_map_value shall fail and return a non-zero value.**]**
**SRS_AMQPVALUE_01_187: [**If cloning the key fails, amqpvalue_set_map_value shall fail and return a non-zero value.**]**
**SRS_AMQPVALUE_01_188: [**If cloning the value fails, amqpvalue_set_map_value shall fail and return a non-zero value.**]**
**SRS_AMQPVALUE_01_196: [**If the map argument is not an AMQP value created with the amqpvalue_create_map function than amqpvalue_set_map_value shall fail and return a non-zero value.**]** 

### amqpvalue_get_map_value

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_get_map_value, AMQP_VALUE, map, AMQP_VALUE, key);
```

**SRS_AMQPVALUE_01_189: [**amqpvalue_get_map_value shall return the value whose key is identified by the key argument.**]**
**SRS_AMQPVALUE_01_192: [**The returned value shall be a clone of the actual value stored in the map.**]**
**SRS_AMQPVALUE_01_190: [**If any argument is NULL, amqpvalue_get_map_value shall return NULL.**]**
**SRS_AMQPVALUE_01_191: [**If the key cannot be found, amqpvalue_get_map_value shall return NULL.**]**
**SRS_AMQPVALUE_01_197: [**If the map argument is not an AMQP value created with the amqpvalue_create_map function than amqpvalue_get_map_value shall return NULL.**]** 

### amqpvalue_get_map_pair_count

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_map_pair_count, AMQP_VALUE, map, uint32_t*, pair_count);
```

**SRS_AMQPVALUE_01_193: [**amqpvalue_get_map_pair_count shall fill in the number of key/value pairs in the map in the pair_count argument.**]** 
**SRS_AMQPVALUE_01_194: [**On success amqpvalue_get_map_pair_count shall return 0.**]**
**SRS_AMQPVALUE_01_195: [**If any of the arguments is NULL, amqpvalue_get_map_pair_count shall fail and return a non-zero value.**]**
**SRS_AMQPVALUE_01_198: [**If the map argument is not an AMQP value created with the amqpvalue_create_map function then amqpvalue_get_map_pair_count shall fail and return a non-zero value.**]** 

### amqpvalue_get_map_key_value_pair

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_map_key_value_pair, AMQP_VALUE, map, uint32_t, index, AMQP_VALUE*, key, AMQP_VALUE*, value);
```

**SRS_AMQPVALUE_01_199: [**amqpvalue_get_map_key_value_pair shall fill in the key and value arguments copies of the key/value pair on the 0 based position index in a map.**]**
**SRS_AMQPVALUE_01_200: [**On success amqpvalue_get_map_key_value_pair shall return 0.**]**
**SRS_AMQPVALUE_01_201: [**If any of the map, key or value arguments is NULL, amqpvalue_get_map_key_value_pair shall fail and return a non-zero value.**]**
**SRS_AMQPVALUE_01_202: [**If cloning the key fails, amqpvalue_get_map_key_value_pair shall fail and return a non-zero value.**]**
**SRS_AMQPVALUE_01_203: [**If cloning the value fails, amqpvalue_get_map_key_value_pair shall fail and return a non-zero value.**]**
**SRS_AMQPVALUE_01_204: [**If the index argument is greater or equal to the number of key/value pairs in the map then amqpvalue_get_map_key_value_pair shall fail and return a non-zero value.**]**
**SRS_AMQPVALUE_01_205: [**If the map argument is not an AMQP value created with the amqpvalue_create_map function then amqpvalue_get_map_key_value_pair shall fail and return a non-zero value.**]** 

### amqpvalue_get_map

```c
MOCKABLE_FUNCTION(, int, amqpvalue_get_map, AMQP_VALUE, from_value, AMQP_VALUE*, map);
```

TBD

### amqpvalue_create_array

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_array);
```

**SRS_AMQPVALUE_01_404: [** `amqpvalue_create_array` shall return a handle to an AMQP_VALUE that stores an array. **]**
**SRS_AMQPVALUE_01_405: [** If allocating memory for the array fails, then `amqpvalue_create_array` shall return NULL. **]**
**SRS_AMQPVALUE_01_406: [** The array shall have an initial size of zero. **]**

### amqpvalue_add_array_item

```C
MOCKABLE_FUNCTION(, int, amqpvalue_add_array_item, AMQP_VALUE, value, AMQP_VALUE, array_item_value);
```

**SRS_AMQPVALUE_01_407: [** `amqpvalue_add_array_item` shall add the AMQP_VALUE specified by `array_item_value` at the 0 based n-th position in the array. **]**
**SRS_AMQPVALUE_01_408: [** On success `amqpvalue_add_array_item` shall return 0. **]**
**SRS_AMQPVALUE_01_424: [** If growing the array fails, then `amqpvalue_add_array_item` shall fail and return a non-zero value. **]**
**SRS_AMQPVALUE_01_409: [** If `value` or `array_item_value` is NULL, amqpvalue_add_array_item shall fail and return a non-zero value. **]**
**SRS_AMQPVALUE_01_410: [** The item stored at the n-th position in the array shall be a clone of `array_item_value`. **]**
**SRS_AMQPVALUE_01_412: [** If cloning the item fails, `amqpvalue_add_array_item` shall fail and return a non-zero value. **]**
**SRS_AMQPVALUE_01_423: [** When `amqpvalue_add_array_item` fails due to not being able to clone the item or grow the array, the array shall not be altered. **]**
**SRS_AMQPVALUE_01_413: [** If the `value` argument is not an AMQP array created with the `amqpvalue_create_array` function than `amqpvalue_add_array_item` shall fail and return a non-zero value. **]**
**SRS_AMQPVALUE_01_425: [** If the type of `array_item_value` does not match that of items already in the array then `amqpvalue_add_array_item` shall fail and return a non-zero value. **]**

### amqpvalue_get_array_item

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_get_array_item, AMQP_VALUE, value, uint32_t, index);
```

**SRS_AMQPVALUE_01_414: [** `amqpvalue_get_array_item` shall return a copy of the AMQP_VALUE stored at the 0 based position `index` in the array identified by `value`. **]**
**SRS_AMQPVALUE_01_416: [** If the `value` argument is NULL, `amqpvalue_get_array_item` shall fail and return NULL. **]**
**SRS_AMQPVALUE_01_417: [** If `index` is greater or equal to the number of items in the array then `amqpvalue_get_array_item` shall fail and return NULL. **]**
**SRS_AMQPVALUE_01_418: [** If value is not an array then `amqpvalue_get_array_item` shall fail and return NULL. **]**
**SRS_AMQPVALUE_01_426: [** If cloning the item at position `index` fails, then `amqpvalue_get_array_item` shall fail and return NULL. **]**

### amqpvalue_get_array_item_count

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_array_item_count, AMQP_VALUE, value, uint32_t*, count);
```

**SRS_AMQPVALUE_01_419: [** `amqpvalue_get_array_item_count` shall return in `count` the number of items in the array. **]**
**SRS_AMQPVALUE_01_420: [** On success `amqpvalue_get_array_item_count` shall return 0. **]**
**SRS_AMQPVALUE_01_421: [** If any of the arguments is NULL, `amqpvalue_get_array_item_count` shall fail and return a non-zero value. **]**
**SRS_AMQPVALUE_01_422: [** If the array argument is not an AMQP value created with the `amqpvalue_create_array` function then `amqpvalue_get_array_item_count` shall fail and return a non-zero value. **]**

### amqpvalue_get_array

```c
MOCKABLE_FUNCTION(, int, amqpvalue_get_array, AMQP_VALUE, value, AMQP_VALUE*, array_value);
```

TBD

### amqpvalue_destroy

```C
MOCKABLE_FUNCTION(, void, amqpvalue_destroy, AMQP_VALUE, value);
```

**SRS_AMQPVALUE_01_314: [**amqpvalue_destroy shall free all resources allocated by any of the amqpvalue_create_xxx functions or amqpvalue_clone.**]**
**SRS_AMQPVALUE_01_315: [**If the value argument is NULL, amqpvalue_destroy shall do nothing.**]** 

### amqpvalue_are_equal

```C
MOCKABLE_FUNCTION(, bool, amqpvalue_are_equal, AMQP_VALUE, value1, AMQP_VALUE, value2);
```

**SRS_AMQPVALUE_01_206: [**amqpvalue_are_equal shall return true if the contents of value1 and value2 are equal.**]**
**SRS_AMQPVALUE_01_207: [**If value1 and value2 are NULL, amqpvalue_are_equal shall return true.**]**
**SRS_AMQPVALUE_01_208: [**If one of the arguments is NULL and the other is not, amqpvalue_are_equal shall return false.**]**
**SRS_AMQPVALUE_01_209: [**If the types for value1 and value2 are different amqpvalue_are_equal shall return false.**]**

For each type the contents shall be compared according to the types defined in the ISO:
**SRS_AMQPVALUE_01_210: [**- null: always equal.**]** 
**SRS_AMQPVALUE_01_211: [**- boolean: compare the bool content.**]** 
**SRS_AMQPVALUE_01_212: [**- ubyte: compare the unsigned char content.**]** 
**SRS_AMQPVALUE_01_213: [**- ushort: compare the uint16_t content.**]** 
**SRS_AMQPVALUE_01_214: [**- uint: compare the uint32_t content.**]** 
**SRS_AMQPVALUE_01_215: [**- ulong: compare the uint64_t content.**]** 
**SRS_AMQPVALUE_01_216: [**- byte: compare the char content.**]** 
**SRS_AMQPVALUE_01_217: [**- short: compare the int16_t content.**]** 
**SRS_AMQPVALUE_01_218: [**- int: compare the int32_t content.**]** 
**SRS_AMQPVALUE_01_219: [**- long: compare the int64_t content.**]** 
**SRS_AMQPVALUE_01_224: [**- float: compare the float content.**]** 
**SRS_AMQPVALUE_01_225: [**- double: compare the double content.**]** 
**SRS_AMQPVALUE_01_260: [**- decimal32: TBD.**]** 
**SRS_AMQPVALUE_01_261: [**- decimal64: TBD.**]** 
**SRS_AMQPVALUE_01_262: [**- decimal128: TBD.**]** 
**SRS_AMQPVALUE_01_226: [**- char: compare the UNICODE character.**]** 
**SRS_AMQPVALUE_01_227: [**- timestamp: compare the underlying 64 bit integer.**]** 
**SRS_AMQPVALUE_01_228: [**- uuid: compare all uuid bytes.**]** 
**SRS_AMQPVALUE_01_229: [**- binary: compare all binary bytes.**]** 
**SRS_AMQPVALUE_01_230: [**- string: compare all string characters.**]** 
**SRS_AMQPVALUE_01_263: [**- symbol: compare all symbol characters.**]** 
**SRS_AMQPVALUE_01_231: [**- list: compare list item count and each element.**]** **SRS_AMQPVALUE_01_232: [**Nesting shall be considered in comparison.**]** 
**SRS_AMQPVALUE_01_427: [**- array: compare array item count and each element. **]** **SRS_AMQPVALUE_01_428: [** Nesting shall be considered in comparison. **]**
**SRS_AMQPVALUE_01_233: [**- map: compare map pair count and each key/value pair.**]** **SRS_AMQPVALUE_01_234: [**Nesting shall be considered in comparison.**]** 

### amqpvalue_clone

```C
MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_clone, AMQP_VALUE, value);
```

**SRS_AMQPVALUE_01_235: [**amqpvalue_clone shall clone the value passed as argument and return a new non-NULL handle to the cloned AMQP value.**]**
**SRS_AMQPVALUE_01_402: [** If `value` is NULL, `amqpvalue_clone` shall return NULL. **]**
**SRS_AMQPVALUE_01_403: [** Cloning should be done by reference counting. **]**

All ISO types shall be supported:
-	**SRS_AMQPVALUE_01_237: [**null**]** 
-	**SRS_AMQPVALUE_01_238: [**boolean**]** 
-	**SRS_AMQPVALUE_01_239: [**ubyte**]** 
-	**SRS_AMQPVALUE_01_240: [**ushort**]** 
-	**SRS_AMQPVALUE_01_241: [**uint**]** 
-	**SRS_AMQPVALUE_01_242: [**ulong**]** 
-	**SRS_AMQPVALUE_01_243: [**byte**]** 
-	**SRS_AMQPVALUE_01_244: [**short**]** 
-	**SRS_AMQPVALUE_01_245: [**int**]** 
-	**SRS_AMQPVALUE_01_246: [**long**]** 
-	**SRS_AMQPVALUE_01_247: [**float**]** 
-	**SRS_AMQPVALUE_01_248: [**double**]** 
-	**SRS_AMQPVALUE_01_249: [**decimal32**]** 
-	**SRS_AMQPVALUE_01_250: [**decimal64**]** 
-	**SRS_AMQPVALUE_01_251: [**decimal128**]** 
-	**SRS_AMQPVALUE_01_252: [**char**]** 
-	**SRS_AMQPVALUE_01_253: [**timestamp**]** 
-	**SRS_AMQPVALUE_01_254: [**uuid**]** 
-	**SRS_AMQPVALUE_01_255: [**binary**]** 
-	**SRS_AMQPVALUE_01_256: [**string**]** 
-	**SRS_AMQPVALUE_01_257: [**symbol**]** 
-	**SRS_AMQPVALUE_01_258: [**list**]** 
-	**SRS_AMQPVALUE_01_259: [**map**]** 

### amqpvalue_encode

```C
MOCKABLE_FUNCTION(, int, amqpvalue_encode, AMQP_VALUE, value, AMQPVALUE_ENCODER_OUTPUT, encoder_output, void*, context);
```

**SRS_AMQPVALUE_01_265: [**amqpvalue_encode shall encode the value per the ISO.**]**
**SRS_AMQPVALUE_01_266: [**On success amqpvalue_encode shall return 0.**]**
**SRS_AMQPVALUE_01_267: [**amqpvalue_encode shall pass the encoded bytes to the encoder_output function.**]**
**SRS_AMQPVALUE_01_268: [**On each call to the encoder_output function, amqpvalue_encode shall also pass the context argument.**]**
**SRS_AMQPVALUE_01_269: [**If value or encoder_output are NULL, amqpvalue_encode shall fail and return a non-zero value.**]**
**SRS_AMQPVALUE_01_274: [**When the encoder output function fails, amqpvalue_encode shall fail and return a non-zero value.**]**
**SRS_AMQPVALUE_01_271: [**If encoding fails due to any error not specifically mentioned here, it shall return a non-zero value.**]** 

### amqpvalue_get_encoded_size

```C
MOCKABLE_FUNCTION(, int, amqpvalue_get_encoded_size, AMQP_VALUE, value, size_t*, encoded_size);
```

**SRS_AMQPVALUE_01_308: [**amqpvalue_get_encoded_size shall fill in the encoded_size argument the number of bytes required to encode the given AMQP value.**]**
**SRS_AMQPVALUE_01_309: [**If any argument is NULL, amqpvalue_get_encoded_size shall return a non-zero value.**]** 

### amqpvalue_decoder_create

```C
MOCKABLE_FUNCTION(, AMQPVALUE_DECODER_HANDLE, amqpvalue_decoder_create, ON_VALUE_DECODED, on_value_decoded, void*, callback_context);
```

**SRS_AMQPVALUE_01_311: [**amqpvalue_decoder_create shall create a new amqp value decoder and return a non-NULL handle to it.**]**
**SRS_AMQPVALUE_01_312: [**If the on_value_decoded argument is NULL, amqpvalue_decoder_create shall return NULL.**]**
**SRS_AMQPVALUE_01_313: [**If creating the decoder fails, amqpvalue_decoder_create shall return NULL.**]** 

### amqpvalue_decoder_destroy

```C
MOCKABLE_FUNCTION(, void, amqpvalue_decoder_destroy, AMQPVALUE_DECODER_HANDLE, handle);
```

**SRS_AMQPVALUE_01_316: [**amqpvalue_decoder_destroy shall free all resources associated with the amqpvalue_decoder.**]**
**SRS_AMQPVALUE_01_317: [**If handle is NULL, amqpvalue_decoder_destroy shall do nothing.**]** 

### amqpvalue_decode_bytes

```C
MOCKABLE_FUNCTION(, int, amqpvalue_decode_bytes, AMQPVALUE_DECODER_HANDLE, handle, const unsigned char*, buffer, size_t, size);
```

**SRS_AMQPVALUE_01_318: [**amqpvalue_decode_bytes shall decode size bytes that are passed in the buffer argument.**]**
**SRS_AMQPVALUE_01_319: [**On success, amqpvalue_decode_bytes shall return 0.**]**
**SRS_AMQPVALUE_01_320: [**If handle or buffer are NULL, amqpvalue_decode_bytes shall return a non-zero value.**]**
**SRS_AMQPVALUE_01_321: [**If size is 0, amqpvalue_decode_bytes shall return a non-zero value.**]**
**SRS_AMQPVALUE_01_322: [**amqpvalue_decode_bytes shall process the bytes byte by byte, as a stream.**]**
**SRS_AMQPVALUE_01_323: [**When enough bytes have been processed for a valid amqp value, the on_value_decoded passed in amqpvalue_decoder_create shall be called.**]**
**SRS_AMQPVALUE_01_324: [**The decoded amqp value shall be passed to on_value_decoded.**]**
**SRS_AMQPVALUE_01_325: [**Also the context stored in amqpvalue_decoder_create shall be passed to the on_value_decoded callback.**]**
**SRS_AMQPVALUE_01_326: [**If any allocation failure occurs during decoding, amqpvalue_decode_bytes shall fail and return a non-zero value.**]**
**SRS_AMQPVALUE_01_327: [**If not enough bytes have accumulated to decode a value, the on_value_decoded shall not be called.**]** 

### Encoding ISO section

Primitive Type Definitions

**SRS_AMQPVALUE_01_003: [**1.6.1 null Indicates an empty value.**]** 

\<type name="null" class="primitive">

**SRS_AMQPVALUE_01_264: [**\<encoding code="0x40" category="fixed" width="0" label="the null value"/>**]** 

\</type>

**SRS_AMQPVALUE_01_004: [**1.6.2 boolean Represents a true or false value.**]** 

\<type name="boolean" class="primitive">

**SRS_AMQPVALUE_01_270: [**\<encoding code="0x56" category="fixed" width="1" label="boolean with the octet 0x00 being false and octet 0x01 being true"/>**]** 

**SRS_AMQPVALUE_01_272: [**\<encoding name="true" code="0x41" category="fixed" width="0" label="the boolean value true"/>**]** 

**SRS_AMQPVALUE_01_273: [**\<encoding name="false" code="0x42" category="fixed" width="0" label="the boolean value false"/>**]** 

</type>

**SRS_AMQPVALUE_01_005: [**1.6.3 ubyte Integer in the range 0 to 28 - 1 inclusive.**]** 

\<type name="ubyte" class="primitive">

**SRS_AMQPVALUE_01_275: [**\<encoding code="0x50" category="fixed" width="1" label="8-bit unsigned integer"/>**]** 

</type>

**SRS_AMQPVALUE_01_012: [**1.6.4 ushort Integer in the range 0 to 216 - 1 inclusive.**]** 

\<type name="ushort" class="primitive">

**SRS_AMQPVALUE_01_276: [**\<encoding code="0x60" category="fixed" width="2" label="16-bit unsigned integer in network byte order"/>**]** 

</type>

**SRS_AMQPVALUE_01_013: [**1.6.5 uint Integer in the range 0 to 232 - 1 inclusive.**]** 

\<type name="uint" class="primitive">

**SRS_AMQPVALUE_01_277: [**\<encoding code="0x70" category="fixed" width="4" label="32-bit unsigned integer in network byte order"/>**]** 

**SRS_AMQPVALUE_01_278: [**\<encoding name="smalluint" code="0x52" category="fixed" width="1" label="unsigned integer value in the range 0 to 255 inclusive"/>**]** 

**SRS_AMQPVALUE_01_279: [**\<encoding name="uint0" code="0x43" category="fixed" width="0" label="the uint value 0"/>**]** 

</type>

**SRS_AMQPVALUE_01_014: [**1.6.6 ulong Integer in the range 0 to 264 - 1 inclusive.**]** 

\<type name="ulong" class="primitive">

**SRS_AMQPVALUE_01_280: [**\<encoding code="0x80" category="fixed" width="8" label="64-bit unsigned integer in network byte order"/>**]** 

**SRS_AMQPVALUE_01_281: [**\<encoding name="smallulong" code="0x53" category="fixed" width="1" label="unsigned long value in the range 0 to 255 inclusive"/>**]** 

**SRS_AMQPVALUE_01_282: [**\<encoding name="ulong0" code="0x44" category="fixed" width="0" label="the ulong value 0"/>**]** 

</type>

**SRS_AMQPVALUE_01_015: [**1.6.7 byte Integer in the range -(27) to 27 - 1 inclusive.**]** 

\<type name="byte" class="primitive">

**SRS_AMQPVALUE_01_283: [**\<encoding code="0x51" category="fixed" width="1" label="8-bit two's-complement integer"/>**]** 

</type>

**SRS_AMQPVALUE_01_016: [**1.6.8 short Integer in the range -(215) to 215 - 1 inclusive.**]** 

\<type name="short" class="primitive">

**SRS_AMQPVALUE_01_284: [**\<encoding code="0x61" category="fixed" width="2" label="16-bit two's-complement integer in network byte order"/>**]** 

</type>

**SRS_AMQPVALUE_01_017: [**1.6.9 int Integer in the range -(231) to 231 - 1 inclusive.**]** 

\<type name="int" class="primitive">

**SRS_AMQPVALUE_01_285: [**\<encoding code="0x71" category="fixed" width="4" label="32-bit two's-complement integer in network byte order"/>**]** 

**SRS_AMQPVALUE_01_286: [**\<encoding name="smallint" code="0x54" category="fixed" width="1" label="8-bit two's-complement integer"/>**]** 

</type>

**SRS_AMQPVALUE_01_018: [**1.6.10 long Integer in the range -(263) to 263 - 1 inclusive.**]** 

\<type name="long" class="primitive">

**SRS_AMQPVALUE_01_287: [**\<encoding code="0x81" category="fixed" width="8" label="64-bit two's-complement integer in network byte order"/>**]** 

**SRS_AMQPVALUE_01_288: [**\<encoding name="smalllong" code="0x55" category="fixed" width="1" label="8-bit two's-complement integer"/>**]** 

</type>

**SRS_AMQPVALUE_01_019: [**1.6.11 float 32-bit floating point number (IEEE 754-2008 binary32).**]** 

\<type name="float" class="primitive">

**SRS_AMQPVALUE_01_289: [**\<encoding name="ieee-754" code="0x72" category="fixed" width="4" label="IEEE 754-2008 binary32"/>**]** 

</type>

**SRS_AMQPVALUE_01_106: [**A 32-bit floating point number (IEEE 754-2008 binary32 [IEEE754**]**).] 

**SRS_AMQPVALUE_01_020: [**1.6.12 double 64-bit floating point number (IEEE 754-2008 binary64).**]** 

\<type name="double" class="primitive">

**SRS_AMQPVALUE_01_290: [**\<encoding name="ieee-754" code="0x82" category="fixed" width="8" label="IEEE 754-2008 binary64"/>**]** 

</type>

**SRS_AMQPVALUE_01_105: [**A 64-bit floating point number (IEEE 754-2008 binary64 [IEEE754**]**).] 

**SRS_AMQPVALUE_01_021: [**1.6.13 decimal32 32-bit decimal number (IEEE 754-2008 decimal32).**]** 

\<type name="decimal32" class="primitive">

**SRS_AMQPVALUE_01_291: [**\<encoding name="ieee-754" code="0x74" category="fixed" width="4" label="IEEE 754-2008 decimal32 using the Binary Integer Decimal encoding"/>**]** 

</type>

**SRS_AMQPVALUE_01_104: [**A 32-bit decimal number (IEEE 754-2008 decimal32 [IEEE754**]**).] 

**SRS_AMQPVALUE_01_022: [**1.6.14 decimal64 64-bit decimal number (IEEE 754-2008 decimal64).**]** 

\<type name="decimal64" class="primitive">

**SRS_AMQPVALUE_01_292: [**\<encoding name="ieee-754" code="0x84" category="fixed" width="8" label="IEEE 754-2008 decimal64 using the Binary Integer Decimal encoding"/>**]** 

</type>

**SRS_AMQPVALUE_01_103: [**A 64-bit decimal number (IEEE 754-2008 decimal64 [IEEE754**]**).] 

**SRS_AMQPVALUE_01_023: [**1.6.15 decimal128 128-bit decimal number (IEEE 754-2008 decimal128).**]** 

\<type name="decimal128" class="primitive">

**SRS_AMQPVALUE_01_293: [**\<encoding name="ieee-754" code="0x94" category="fixed" width="16" label="IEEE 754-2008 decimal128 using the Binary Integer Decimal encoding"/>**]** 

</type>

**SRS_AMQPVALUE_01_102: [**A 128-bit decimal number (IEEE 754-2008 decimal128 [IEEE754**]**).] 

**SRS_AMQPVALUE_01_024: [**1.6.16 char A single Unicode character.**]** 

\<type name="char" class="primitive">

**SRS_AMQPVALUE_01_294: [**\<encoding name="utf32" code="0x73" category="fixed" width="4" label="a UTF-32BE encoded Unicode character"/>**]** 

</type>

**SRS_AMQPVALUE_01_101: [**A UTF-32BE encoded Unicode character [UNICODE6**]**.] 

**SRS_AMQPVALUE_01_025: [**1.6.17 timestamp An absolute point in time.**]** 

\<type name="timestamp" class="primitive">

**SRS_AMQPVALUE_01_295: [**\<encoding name="ms64" code="0x83" category="fixed" width="8" label="64-bit two's-complement integer representing milliseconds since the unix epoch"/>**]** 

</type>

**SRS_AMQPVALUE_01_099: [**Represents an approximate point in time using the Unix time t [IEEE1003**]** encoding of UTC, but with a precision of milliseconds.]
For example, 1311704463521 represents the moment 2011-07-26T18:21:03.521Z.

**SRS_AMQPVALUE_01_026: [**1.6.18 uuid A universally unique identifier as defined by RFC-4122 section 4.1.2 .**]** 

\<type name="uuid" class="primitive">

**SRS_AMQPVALUE_01_296: [**\<encoding code="0x98" category="fixed" width="16" label="UUID as defined in section 4.1.2 of RFC-4122"/>**]** 

</type>

**SRS_AMQPVALUE_01_100: [**UUID is defined in section 4.1.2 of RFC-4122 [RFC4122**]**.] 

**SRS_AMQPVALUE_01_027: [**1.6.19 binary A sequence of octets.**]** 

\<type name="binary" class="primitive">

**SRS_AMQPVALUE_01_297: [**\<encoding name="vbin8" code="0xa0" category="variable" width="1" label="up to 2^8 - 1 octets of binary data"/>**]** 

**SRS_AMQPVALUE_01_298: [**\<encoding name="vbin32" code="0xb0" category="variable" width="4" label="up to 2^32 - 1 octets of binary data"/>**]** 

</type>

**SRS_AMQPVALUE_01_028: [**1.6.20 string A sequence of Unicode characters.**]** 

\<type name="string" class="primitive">

**SRS_AMQPVALUE_01_299: [**\<encoding name="str8-utf8" code="0xa1" category="variable" width="1" label="up to 2^8 - 1 octets worth of UTF-8 Unicode (with no byte order mark)"/>**]** 

**SRS_AMQPVALUE_01_300: [**\<encoding name="str32-utf8" code="0xb1" category="variable" width="4" label="up to 2^32 - 1 octets worth of UTF-8 Unicode (with no byte order mark)"/>**]** 

</type>

**SRS_AMQPVALUE_01_119: [**A string represents a sequence of Unicode characters as defined by the Unicode V6.0.0 standard [UNICODE6**]**.] 

**SRS_AMQPVALUE_01_029: [**1.6.21 symbol Symbolic values from a constrained domain.**]** 

\<type name="symbol" class="primitive">

**SRS_AMQPVALUE_01_301: [**\<encoding name="sym8" code="0xa3" category="variable" width="1" label="up to 2^8 - 1 seven bit ASCII characters representing a symbolic value"/>**]** 

**SRS_AMQPVALUE_01_302: [**\<encoding name="sym32" code="0xb3" category="variable" width="4" label="up to 2^32 - 1 seven bit ASCII characters representing a symbolic value"/>**]** 

</type>

Symbols are values from a constrained domain. Although the set of possible domains is open-ended, typically the both number and size of symbols in use for any given application will be small, e.g. small enough that it is reasonable to cache all the distinct values. **SRS_AMQPVALUE_01_122: [**Symbols are encoded as ASCII characters [ASCII**]**.] 

**SRS_AMQPVALUE_01_030: [**1.6.22 list A sequence of polymorphic values.**]**

\<type name="list" class="primitive">

**SRS_AMQPVALUE_01_303: [**\<encoding name="list0" code="0x45" category="fixed" width="0" label="the empty list (i.e. the list with no elements)"/>**]** 

**SRS_AMQPVALUE_01_304: [**\<encoding name="list8" code="0xc0" category="compound" width="1" label="up to 2^8 - 1 list elements with total size less than 2^8 octets"/>**]** 

**SRS_AMQPVALUE_01_305: [**\<encoding name="list32" code="0xd0" category="compound" width="4" label="up to 2^32 - 1 list elements with total size less than 2^32 octets"/>**]** 

</type>

**SRS_AMQPVALUE_01_031: [**1.6.23 map A polymorphic mapping from distinct keys to values.**]** 

\<type name="map" class="primitive">

**SRS_AMQPVALUE_01_306: [**\<encoding name="map8" code="0xc1" category="compound" width="1" label="up to 2^8 - 1 octets of encoded map data"/>**]** 

**SRS_AMQPVALUE_01_307: [**\<encoding name="map32" code="0xd1" category="compound" width="4" label="up to 2^32 - 1 octets of encoded map data"/>**]** 

</type>

**SRS_AMQPVALUE_01_123: [**A map is encoded as a compound value where the constituent elements form alternating key value pairs.**]** 

...

Figure 1.20: Layout of Map Encoding
**SRS_AMQPVALUE_01_124: [**Map encodings MUST contain an even number of items (i.e. an equal number of keys and values).**]**
**SRS_AMQPVALUE_01_125: [**A map in which there exist two identical key values is invalid.**]** **SRS_AMQPVALUE_01_126: [**Unless known to be otherwise, maps MUST be considered to be ordered, that is, the order of the key-value pairs is semantically important and two maps which are different only in the order in which their key-value pairs are encoded are not equal.**]** 

**SRS_AMQPVALUE_01_397: [**1.6.24 array A sequence of values of a single type.**]** 

\<type name="array" class="primitive">

**SRS_AMQPVALUE_01_398: [**\<encoding name="array8" code="0xe0" category="array" width="1" label="up to 2^8 - 1 array elements with total size less than 2^8 octets"/>**]** 

**SRS_AMQPVALUE_01_399: [**\<encoding name="array32" code="0xf0" category="array" width="4" label="up to 2^32 - 1 array elements with total size less than 2^32 octets"/>**]** 

</type>

### Decoding ISO Section

Primitive Type Definitions

**SRS_AMQPVALUE_01_328: [**1.6.1 null Indicates an empty value.**]** 

\<type name="null" class="primitive">

**SRS_AMQPVALUE_01_329: [**\<encoding code="0x40" category="fixed" width="0" label="the null value"/>**]** 

</type>

**SRS_AMQPVALUE_01_330: [**1.6.2 boolean Represents a true or false value.**]**  

\<type name="boolean" class="primitive">

**SRS_AMQPVALUE_01_331: [**\<encoding code="0x56" category="fixed" width="1" label="boolean with the octet 0x00 being false and octet 0x01 being true"/>**]** 

**SRS_AMQPVALUE_01_332: [**\<encoding name="true" code="0x41" category="fixed" width="0" label="the boolean value true"/>**]** 

**SRS_AMQPVALUE_01_333: [**\<encoding name="false" code="0x42" category="fixed" width="0" label="the boolean value false"/>**]** 

</type>

**SRS_AMQPVALUE_01_334: [**1.6.3 ubyte Integer in the range 0 to 28 - 1 inclusive.**]**  

\<type name="ubyte" class="primitive">

**SRS_AMQPVALUE_01_335: [**\<encoding code="0x50" category="fixed" width="1" label="8-bit unsigned integer"/>**]** 

</type>

**SRS_AMQPVALUE_01_336: [**1.6.4 ushort Integer in the range 0 to 216 - 1 inclusive.**]**  

\<type name="ushort" class="primitive">

**SRS_AMQPVALUE_01_337: [**\<encoding code="0x60" category="fixed" width="2" label="16-bit unsigned integer in network byte order"/>**]** 

</type>

**SRS_AMQPVALUE_01_338: [**1.6.5 uint Integer in the range 0 to 232 - 1 inclusive.**]**  	

\<type name="uint" class="primitive">

**SRS_AMQPVALUE_01_339: [**\<encoding code="0x70" category="fixed" width="4" label="32-bit unsigned integer in network byte order"/>**]** 

**SRS_AMQPVALUE_01_340: [**\<encoding name="smalluint" code="0x52" category="fixed" width="1" label="unsigned integer value in the range 0 to 255 inclusive"/>**]** 

**SRS_AMQPVALUE_01_341: [**\<encoding name="uint0" code="0x43" category="fixed" width="0" label="the uint value 0"/>**]** 

</type>

**SRS_AMQPVALUE_01_342: [**1.6.6 ulong Integer in the range 0 to 264 - 1 inclusive.**]**  

\<type name="ulong" class="primitive">

**SRS_AMQPVALUE_01_343: [**\<encoding code="0x80" category="fixed" width="8" label="64-bit unsigned integer in network byte order"/>**]** 

**SRS_AMQPVALUE_01_344: [**\<encoding name="smallulong" code="0x53" category="fixed" width="1" label="unsigned long value in the range 0 to 255 inclusive"/>**]** 

**SRS_AMQPVALUE_01_345: [**\<encoding name="ulong0" code="0x44" category="fixed" width="0" label="the ulong value 0"/>**]** 

</type>

**SRS_AMQPVALUE_01_346: [**1.6.7 byte Integer in the range -(27) to 27 - 1 inclusive.**]**  

\<type name="byte" class="primitive">

**SRS_AMQPVALUE_01_347: [**\<encoding code="0x51" category="fixed" width="1" label="8-bit two's-complement integer"/>**]** 

</type>

**SRS_AMQPVALUE_01_348: [**1.6.8 short Integer in the range -(215) to 215 - 1 inclusive.**]**  

\<type name="short" class="primitive">

**SRS_AMQPVALUE_01_349: [**\<encoding code="0x61" category="fixed" width="2" label="16-bit two's-complement integer in network byte order"/>**]** 

</type>

**SRS_AMQPVALUE_01_350: [**1.6.9 int Integer in the range -(231) to 231 - 1 inclusive.**]**  

\<type name="int" class="primitive">

**SRS_AMQPVALUE_01_351: [**\<encoding code="0x71" category="fixed" width="4" label="32-bit two's-complement integer in network byte order"/>**]** 

**SRS_AMQPVALUE_01_352: [**\<encoding name="smallint" code="0x54" category="fixed" width="1" label="8-bit two's-complement integer"/>**]** 

</type>

**SRS_AMQPVALUE_01_353: [**1.6.10 long Integer in the range -(263) to 263 - 1 inclusive.**]**  

\<type name="long" class="primitive">

**SRS_AMQPVALUE_01_354: [**\<encoding code="0x81" category="fixed" width="8" label="64-bit two's-complement integer in network byte order"/>**]** 

**SRS_AMQPVALUE_01_355: [**\<encoding name="smalllong" code="0x55" category="fixed" width="1" label="8-bit two's-complement integer"/>**]** 

</type>

**SRS_AMQPVALUE_01_356: [**1.6.11 float 32-bit floating point number (IEEE 754-2008 binary32).**]**  

\<type name="float" class="primitive">

**SRS_AMQPVALUE_01_357: [**\<encoding name="ieee-754" code="0x72" category="fixed" width="4" label="IEEE 754-2008 binary32"/>**]** 

</type>

A 32-bit floating point number (IEEE 754-2008 binary32 [IEEE754]). 

**SRS_AMQPVALUE_01_358: [**1.6.12 double 64-bit floating point number (IEEE 754-2008 binary64).**]**  

\<type name="double" class="primitive">

**SRS_AMQPVALUE_01_359: [**\<encoding name="ieee-754" code="0x82" category="fixed" width="8" label="IEEE 754-2008 binary64"/>**]** 

</type>

A 64-bit floating point number (IEEE 754-2008 binary64 [IEEE754]). 

**SRS_AMQPVALUE_01_360: [**1.6.13 decimal32 32-bit decimal number (IEEE 754-2008 decimal32).**]**  

\<type name="decimal32" class="primitive">

**SRS_AMQPVALUE_01_361: [**\<encoding name="ieee-754" code="0x74" category="fixed" width="4" label="IEEE 754-2008 decimal32 using the Binary Integer Decimal encoding"/>**]** 

</type>

A 32-bit decimal number (IEEE 754-2008 decimal32 [IEEE754]). 

**SRS_AMQPVALUE_01_362: [**1.6.14 decimal64 64-bit decimal number (IEEE 754-2008 decimal64).**]**  

\<type name="decimal64" class="primitive">

**SRS_AMQPVALUE_01_363: [**\<encoding name="ieee-754" code="0x84" category="fixed" width="8" label="IEEE 754-2008 decimal64 using the Binary Integer Decimal encoding"/>**]** 

</type>

A 64-bit decimal number (IEEE 754-2008 decimal64 [IEEE754]). 

**SRS_AMQPVALUE_01_364: [**1.6.15 decimal128 128-bit decimal number (IEEE 754-2008 decimal128).**]**  

\<type name="decimal128" class="primitive">

**SRS_AMQPVALUE_01_365: [**\<encoding name="ieee-754" code="0x94" category="fixed" width="16" label="IEEE 754-2008 decimal128 using the Binary Integer Decimal encoding"/>**]** 

</type>

A 128-bit decimal number (IEEE 754-2008 decimal128 [IEEE754]). 

**SRS_AMQPVALUE_01_366: [**1.6.16 char A single Unicode character.**]**  

\<type name="char" class="primitive">

**SRS_AMQPVALUE_01_367: [**\<encoding name="utf32" code="0x73" category="fixed" width="4" label="a UTF-32BE encoded Unicode character"/>**]** 

</type>

A UTF-32BE encoded Unicode character [UNICODE6]. 

**SRS_AMQPVALUE_01_368: [**1.6.17 timestamp An absolute point in time.**]**  

\<type name="timestamp" class="primitive">

**SRS_AMQPVALUE_01_369: [**\<encoding name="ms64" code="0x83" category="fixed" width="8" label="64-bit two's-complement integer representing milliseconds since the unix epoch"/>**]** 

</type>

Represents an approximate point in time using the Unix time t [IEEE1003] encoding of UTC, but with a precision
of milliseconds. For example, 1311704463521 represents the moment 2011-07-26T18:21:03.521Z.

**SRS_AMQPVALUE_01_370: [**1.6.18 uuid A universally unique identifier as defined by RFC-4122 section 4.1.2 .**]**  

\<type name="uuid" class="primitive">

**SRS_AMQPVALUE_01_371: [**\<encoding code="0x98" category="fixed" width="16" label="UUID as defined in section 4.1.2 of RFC-4122"/>**]** 

</type>

UUID is defined in section 4.1.2 of RFC-4122 [RFC4122]. 

**SRS_AMQPVALUE_01_372: [**1.6.19 binary A sequence of octets.**]**  

\<type name="binary" class="primitive">

**SRS_AMQPVALUE_01_373: [**\<encoding name="vbin8" code="0xa0" category="variable" width="1" label="up to 2^8 - 1 octets of binary data"/>**]** 

**SRS_AMQPVALUE_01_374: [**\<encoding name="vbin32" code="0xb0" category="variable" width="4" label="up to 2^32 - 1 octets of binary data"/>**]** 

</type>

**SRS_AMQPVALUE_01_375: [**1.6.20 string A sequence of Unicode characters.**]**  

\<type name="string" class="primitive">

**SRS_AMQPVALUE_01_376: [**\<encoding name="str8-utf8" code="0xa1" category="variable" width="1" label="up to 2^8 - 1 octets worth of UTF-8 Unicode (with no byte order mark)"/>**]** 

**SRS_AMQPVALUE_01_377: [**\<encoding name="str32-utf8" code="0xb1" category="variable" width="4" label="up to 2^32 - 1 octets worth of UTF-8 Unicode (with no byte order mark)"/>**]** 

</type>

A string represents a sequence of Unicode characters as defined by the Unicode V6.0.0 standard [UNICODE6]. 

**SRS_AMQPVALUE_01_378: [**1.6.21 symbol Symbolic values from a constrained domain.**]**  

\<type name="symbol" class="primitive">

**SRS_AMQPVALUE_01_379: [**\<encoding name="sym8" code="0xa3" category="variable" width="1" label="up to 2^8 - 1 seven bit ASCII characters representing a symbolic value"/>**]** 

**SRS_AMQPVALUE_01_380: [**\<encoding name="sym32" code="0xb3" category="variable" width="4" label="up to 2^32 - 1 seven bit ASCII characters representing a symbolic value"/>**]** 

</type>

Symbols are values from a constrained domain. Although the set of possible domains is open-ended, typically the both number and size of symbols in use for any given application will be small, e.g. small enough that it is reasonable to cache all the distinct values. **SRS_AMQPVALUE_01_382: [**Symbols are encoded as ASCII characters [ASCII**]**.] 

**SRS_AMQPVALUE_01_383: [**1.6.22 list A sequence of polymorphic values.**]**  

\<type name="list" class="primitive">

**SRS_AMQPVALUE_01_384: [**\<encoding name="list0" code="0x45" category="fixed" width="0" label="the empty list (i.e. the list with no elements)"/>**]** 

**SRS_AMQPVALUE_01_385: [**\<encoding name="list8" code="0xc0" category="compound" width="1" label="up to 2^8 - 1 list elements with total size less than 2^8 octets"/>**]** 

**SRS_AMQPVALUE_01_386: [**\<encoding name="list32" code="0xd0" category="compound" width="4" label="up to 2^32 - 1 list elements with total size less than 2^32 octets"/>**]** 

</type>

**SRS_AMQPVALUE_01_387: [**1.6.23 map A polymorphic mapping from distinct keys to values.**]**  

\<type name="map" class="primitive">

**SRS_AMQPVALUE_01_388: [**\<encoding name="map8" code="0xc1" category="compound" width="1" label="up to 2^8 - 1 octets of encoded map data"/>**]** 

**SRS_AMQPVALUE_01_389: [**\<encoding name="map32" code="0xd1" category="compound" width="4" label="up to 2^32 - 1 octets of encoded map data"/>**]** 

</type>

**SRS_AMQPVALUE_01_390: [**A map is encoded as a compound value where the constituent elements form alternating key value pairs.**]** 

...

Figure 1.20: Layout of Map Encoding

**SRS_AMQPVALUE_01_391: [**Map encodings MUST contain an even number of items (i.e. an equal number of keys and values).**]** **SRS_AMQPVALUE_01_392: [**A map in which there exist two identical key values is invalid.**]** **SRS_AMQPVALUE_01_393: [**Unless known to be otherwise, maps MUST be considered to be ordered, that is, the order of the key-value pairs is semantically important and two maps which are different only in the order in which their key-value pairs are encoded are not equal.**]** 

**SRS_AMQPVALUE_01_394: [**1.6.24 array A sequence of values of a single type.**]** 

\<type name="array" class="primitive">

**SRS_AMQPVALUE_01_395: [**\<encoding name="array8" code="0xe0" category="array" width="1" label="up to 2^8 - 1 array elements with total size less than 2^8 octets"/>**]** 

**SRS_AMQPVALUE_01_396: [**\<encoding name="array32" code="0xf0" category="array" width="4" label="up to 2^32 - 1 array elements with total size less than 2^32 octets"/>**]** 

</type>
