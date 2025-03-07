# message requirements
 
## Overview

`message` is module that stores AMQP messages per the Messaging layer in the AMQP ISO.

## Exposed API

```C
	typedef enum MESSAGE_BODY_TYPE_TAG
	{
		MESSAGE_BODY_TYPE_NONE,
		MESSAGE_BODY_TYPE_DATA,
		MESSAGE_BODY_TYPE_SEQUENCE,
		MESSAGE_BODY_TYPE_VALUE
	} MESSAGE_BODY_TYPE;

	typedef struct MESSAGE_INSTANCE_TAG* MESSAGE_HANDLE;
	typedef struct BINARY_DATA_TAG
	{
		const unsigned char* bytes;
		size_t length;
	} BINARY_DATA;

	MOCKABLE_FUNCTION(, MESSAGE_HANDLE, message_create);
	MOCKABLE_FUNCTION(, MESSAGE_HANDLE, message_clone, MESSAGE_HANDLE, source_message);
	MOCKABLE_FUNCTION(, void, message_destroy, MESSAGE_HANDLE, message);
	MOCKABLE_FUNCTION(, int, message_set_header, MESSAGE_HANDLE, message, HEADER_HANDLE, message_header);
	MOCKABLE_FUNCTION(, int, message_get_header, MESSAGE_HANDLE, message, HEADER_HANDLE*, message_header);
	MOCKABLE_FUNCTION(, int, message_set_delivery_annotations, MESSAGE_HANDLE, message, delivery_annotations, annotations);
	MOCKABLE_FUNCTION(, int, message_get_delivery_annotations, MESSAGE_HANDLE, message, delivery_annotations*, annotations);
	MOCKABLE_FUNCTION(, int, message_set_message_annotations, MESSAGE_HANDLE, message, message_annotations, annotations);
	MOCKABLE_FUNCTION(, int, message_get_message_annotations, MESSAGE_HANDLE, message, message_annotations*, annotations);
	MOCKABLE_FUNCTION(, int, message_set_properties, MESSAGE_HANDLE, message, PROPERTIES_HANDLE, properties);
	MOCKABLE_FUNCTION(, int, message_get_properties, MESSAGE_HANDLE, message, PROPERTIES_HANDLE*, properties);
	MOCKABLE_FUNCTION(, int, message_set_application_properties, MESSAGE_HANDLE, message, AMQP_VALUE, application_properties);
	MOCKABLE_FUNCTION(, int, message_get_application_properties, MESSAGE_HANDLE, message, AMQP_VALUE*, application_properties);
	MOCKABLE_FUNCTION(, int, message_set_footer, MESSAGE_HANDLE, message, annotations, footer);
	MOCKABLE_FUNCTION(, int, message_get_footer, MESSAGE_HANDLE, message, annotations*, footer);
	MOCKABLE_FUNCTION(, int, message_add_body_amqp_data, MESSAGE_HANDLE, message, BINARY_DATA, amqp_data);
	MOCKABLE_FUNCTION(, int, message_get_body_amqp_data_in_place, MESSAGE_HANDLE, message, size_t, index, BINARY_DATA*, amqp_data);
	MOCKABLE_FUNCTION(, int, message_get_body_amqp_data_count, MESSAGE_HANDLE, message, size_t*, count);
	MOCKABLE_FUNCTION(, int, message_set_body_amqp_value, MESSAGE_HANDLE, message, AMQP_VALUE, body_amqp_value);
	MOCKABLE_FUNCTION(, int, message_get_body_amqp_value_in_place, MESSAGE_HANDLE, message, AMQP_VALUE*, body_amqp_value);
	MOCKABLE_FUNCTION(, int, message_add_body_amqp_sequence, MESSAGE_HANDLE, message, AMQP_VALUE, sequence);
	MOCKABLE_FUNCTION(, int, message_get_body_amqp_sequence_in_place, MESSAGE_HANDLE, message, size_t, index, AMQP_VALUE*, sequence);
	MOCKABLE_FUNCTION(, int, message_get_body_amqp_sequence_count, MESSAGE_HANDLE, message, size_t*, count);
	MOCKABLE_FUNCTION(, int, message_get_body_type, MESSAGE_HANDLE, message, MESSAGE_BODY_TYPE*, body_type);
	MOCKABLE_FUNCTION(, int, message_set_message_format, MESSAGE_HANDLE, message, uint32_t, message_format);
    MOCKABLE_FUNCTION(, int, message_get_message_format, MESSAGE_HANDLE, message, uint32_t*, message_format);
```

### message_create

```C
MOCKABLE_FUNCTION(, MESSAGE_HANDLE, message_create);
```

**SRS_MESSAGE_01_001: [** `message_create` shall create a new AMQP message instance and on success it shall return a non-NULL handle for the newly created message instance. **]**
**SRS_MESSAGE_01_002: [** If allocating memory for the message fails, `message_create` shall fail and return NULL. **]**

### message_clone

```C
MOCKABLE_FUNCTION(, MESSAGE_HANDLE, message_clone, MESSAGE_HANDLE, source_message);
```

**SRS_MESSAGE_01_003: [** `message_clone` shall clone a message entirely and on success return a non-NULL handle to the cloned message. **]**
**SRS_MESSAGE_01_062: [** If `source_message` is NULL, `message_clone` shall fail and return NULL. **]**
**SRS_MESSAGE_01_004: [** If allocating memory for the new cloned message fails, `message_clone` shall fail and return NULL. **]**
**SRS_MESSAGE_01_005: [** If a header exists on the source message it shall be cloned by using `header_clone`. **]**
**SRS_MESSAGE_01_006: [** If delivery annotations exist on the source message they shall be cloned by using `annotations_clone`. **]**
**SRS_MESSAGE_01_007: [** If message annotations exist on the source message they shall be cloned by using `annotations_clone`. **]**
**SRS_MESSAGE_01_008: [** If message properties exist on the source message they shall be cloned by using `properties_clone`. **]**
**SRS_MESSAGE_01_009: [** If application properties exist on the source message they shall be cloned by using `amqpvalue_clone`. **]**
**SRS_MESSAGE_01_010: [** If a footer exists on the source message it shall be cloned by using `annotations_clone`. **]**
**SRS_MESSAGE_01_011: [** If an AMQP data has been set as message body on the source message it shall be cloned by allocating memory for the binary payload. **]**
**SRS_MESSAGE_01_159: [** If an AMQP value has been set as message body on the source message it shall be cloned by calling `amqpvalue_clone`. **]**
**SRS_MESSAGE_01_160: [** If AMQP sequences are set as AMQP body they shall be cloned by calling `amqpvalue_clone`. **]**
**SRS_MESSAGE_01_012: [** If any cloning operation for the members of the source message fails, then `message_clone` shall fail and return NULL. **]**

### message_destroy

```C
MOCKABLE_FUNCTION(, void, message_destroy, MESSAGE_HANDLE, message);
```

**SRS_MESSAGE_01_013: [** `message_destroy` shall free all resources allocated by the message instance identified by the `message` argument. **]**
**SRS_MESSAGE_01_014: [** If `message` is NULL, `message_destroy` shall do nothing. **]**
**SRS_MESSAGE_01_015: [** The message header shall be freed by calling `header_destroy`. **]**
**SRS_MESSAGE_01_016: [** The delivery annotations shall be freed by calling `annotations_destroy`. **]**
**SRS_MESSAGE_01_017: [** The message annotations shall be freed by calling `annotations_destroy`. **]**
**SRS_MESSAGE_01_018: [** The message properties shall be freed by calling `properties_destroy`. **]**
**SRS_MESSAGE_01_019: [** The application properties shall be freed by calling `amqpvalue_destroy`. **]**
**SRS_MESSAGE_01_020: [** The message footer shall be freed by calling `annotations_destroy`. **]**
**SRS_MESSAGE_01_021: [** If the message body is made of an AMQP value, the value shall be freed by calling `amqpvalue_destroy`. **]**
**SRS_MESSAGE_01_136: [** If the message body is made of several AMQP data items, they shall all be freed. **]**
**SRS_MESSAGE_01_136: [** If the message body is made of several AMQP sequences, they shall all be freed. **]**
**SRS_MESSAGE_01_137: [** Each sequence shall be freed by calling `amqpvalue_destroy`. **]**

### message_set_header

```C
MOCKABLE_FUNCTION(, int, message_set_header, MESSAGE_HANDLE, message, HEADER_HANDLE, message_header);
```

**SRS_MESSAGE_01_022: [** `message_set_header` shall copy the contents of `message_header` as the header for the message instance identified by message. **]**
**SRS_MESSAGE_01_023: [** On success it shall return 0. **]**
**SRS_MESSAGE_01_024: [** If `message` is NULL, `message_set_header` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_025: [** Cloning the header shall be done by calling `header_clone`. **]**
**SRS_MESSAGE_01_026: [** If `header_clone` fails, `message_set_header` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_138: [** If setting the header fails, the previous value shall be preserved. **]**
**SRS_MESSAGE_01_139: [** If `message_header` is NULL, the previously stored header associated with `message` shall be freed. **]**

### message_get_header

```C
MOCKABLE_FUNCTION(, int, message_get_header, MESSAGE_HANDLE, message, HEADER_HANDLE*, message_header);
```

**SRS_MESSAGE_01_027: [** `message_get_header` shall copy the contents of header for the message instance identified by `message` into the argument `message_header`. **]**
**SRS_MESSAGE_01_028: [** On success, `message_get_header` shall return 0.**]**
**SRS_MESSAGE_01_029: [** If `message` or `message_header` is NULL, `message_get_header` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_030: [** Cloning the header shall be done by calling `header_clone`. **]**
**SRS_MESSAGE_01_031: [** If `header_clone` fails, `message_get_header` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_143: [** If no header has been set, `message_get_header` shall set `message_header` to NULL. **]**

### message_set_delivery_annotations

```C
MOCKABLE_FUNCTION(, int, message_set_delivery_annotations, MESSAGE_HANDLE, message, delivery_annotations, annotations);
```

**SRS_MESSAGE_01_032: [** `message_set_delivery_annotations` shall copy the contents of `annotations` as the delivery annotations for the message instance identified by `message`. **]**
**SRS_MESSAGE_01_033: [** On success it shall return 0. **]**
**SRS_MESSAGE_01_034: [** If `message` is NULL, `message_set_delivery_annotations` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_035: [** Cloning the delivery annotations shall be done by calling `annotations_clone`. **]**
**SRS_MESSAGE_01_036: [** If `annotations_clone` fails, `message_set_delivery_annotations` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_140: [** If setting the delivery annotations fails, the previous value shall be preserved. **]**
**SRS_MESSAGE_01_141: [** If `annotations` is NULL, the previously stored delivery annotations associated with `message` shall be freed. **]**

### message_get_delivery_annotations

```C
MOCKABLE_FUNCTION(, int, message_get_delivery_annotations, MESSAGE_HANDLE, message, delivery_annotations*, annotations);
```

**SRS_MESSAGE_01_037: [** `message_get_delivery_annotations` shall copy the contents of delivery annotations for the message instance identified by `message` into the argument `annotations`. **]**
**SRS_MESSAGE_01_038: [** On success, `message_get_delivery_annotations` shall return 0. **]**
**SRS_MESSAGE_01_039: [** If `message` or `annotations` is NULL, `message_get_delivery_annotations` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_040: [** Cloning the delivery annotations shall be done by calling `annotations_clone`. **]**
**SRS_MESSAGE_01_041: [** If `annotations_clone` fails, `message_get_delivery_annotations` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_142: [** If no delivery annotations have been set, `message_get_delivery_annotations` shall set `annotations` to NULL. **]**

### message_set_message_annotations

```C
MOCKABLE_FUNCTION(, int, message_set_message_annotations, MESSAGE_HANDLE, message, message_annotations, annotations);
```

**SRS_MESSAGE_01_042: [** `message_set_message_annotations` shall copy the contents of `annotations` as the message annotations for the message instance identified by `message`. **]**
**SRS_MESSAGE_01_043: [** On success it shall return 0. **]**
**SRS_MESSAGE_01_044: [** If `message` is NULL, `message_set_message_annotations` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_045: [** Cloning the message annotations shall be done by calling `annotations_clone`. **]**
**SRS_MESSAGE_01_046: [** If `annotations_clone` fails, `message_set_message_annotations` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_144: [** If setting the message annotations fails, the previous value shall be preserved. **]**
**SRS_MESSAGE_01_145: [** If `annotations` is NULL, the previously stored message annotations associated with `message` shall be freed. **]**

### message_get_message_annotations

```C
MOCKABLE_FUNCTION(, int, message_get_message_annotations, MESSAGE_HANDLE, message, message_annotations*, annotations);
```

**SRS_MESSAGE_01_047: [** `message_get_message_annotations` shall copy the contents of message annotations for the message instance identified by `message` into the argument `annotations`. **]**
**SRS_MESSAGE_01_048: [** On success, `message_get_message_annotations` shall return 0. **]**
**SRS_MESSAGE_01_049: [** If `message` or `annotations` is NULL, `message_get_message_annotations` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_050: [** Cloning the message annotations shall be done by calling `annotations_clone`. **]**
**SRS_MESSAGE_01_051: [** If `annotations_clone` fails, `message_get_message_annotations` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_146: [** If no message annotations have been set, `message_get_message_annotations` shall set `annotations` to NULL. **]**

### message_set_properties

```C
MOCKABLE_FUNCTION(, int, message_set_properties, MESSAGE_HANDLE, message, PROPERTIES_HANDLE, properties);
```

**SRS_MESSAGE_01_052: [** `message_set_properties` shall copy the contents of `properties` as the message properties for the message instance identified by `message`. **]**
**SRS_MESSAGE_01_053: [** On success it shall return 0. **]**
**SRS_MESSAGE_01_054: [** If `message` is NULL, `message_set_properties` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_055: [** Cloning the message properties shall be done by calling `properties_clone`. **]**
**SRS_MESSAGE_01_056: [** If `properties_clone` fails, `message_set_properties` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_063: [** If setting the message properties fails, the previous value shall be preserved. **]**
**SRS_MESSAGE_01_147: [** If `properties` is NULL, the previously stored message properties associated with `message` shall be freed. **]**

### message_get_properties

```C
MOCKABLE_FUNCTION(, int, message_get_properties, MESSAGE_HANDLE, message, PROPERTIES_HANDLE*, properties);
```

**SRS_MESSAGE_01_057: [** `message_get_properties` shall copy the contents of message properties for the message instance identified by `message` into the argument `properties`. **]**
**SRS_MESSAGE_01_058: [** On success, `message_get_properties` shall return 0. **]**
**SRS_MESSAGE_01_059: [** If `message` or `properties` is NULL, `message_get_properties` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_060: [** Cloning the message properties shall be done by calling `properties_clone`. **]**
**SRS_MESSAGE_01_061: [** If `properties_clone` fails, `message_get_properties` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_148: [** If no message properties have been set, `message_get_properties` shall set `properties` to NULL. **]**

### message_set_application_properties

```C
MOCKABLE_FUNCTION(, int, message_set_application_properties, MESSAGE_HANDLE, message, AMQP_VALUE, application_properties);
```

**SRS_MESSAGE_01_064: [** `message_set_application_properties` shall copy the contents of `application_properties` as the application properties for the message instance identified by `message`. **]**
**SRS_MESSAGE_01_065: [** On success it shall return 0. **]**
**SRS_MESSAGE_01_066: [** If `message` is NULL, `message_set_application_properties` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_067: [** Cloning the message properties shall be done by calling `application_properties_clone`. **]**
**SRS_MESSAGE_01_068: [** If `application_properties_clone` fails, `message_set_application_properties` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_069: [** If setting the application properties fails, the previous value shall be preserved. **]**
**SRS_MESSAGE_01_149: [** If `application_properties` is NULL, the previously stored application properties associated with `message` shall be freed. **]**

### message_get_application_properties

```C
MOCKABLE_FUNCTION(, int, message_get_application_properties, MESSAGE_HANDLE, message, AMQP_VALUE*, application_properties);
```

**SRS_MESSAGE_01_070: [** `message_get_application_properties` shall copy the contents of application message properties for the message instance identified by `message` into the argument `application_properties`. **]**
**SRS_MESSAGE_01_071: [** On success, `message_get_application_properties` shall return 0. **]**
**SRS_MESSAGE_01_072: [** If `message` or `application_properties` is NULL, `message_get_application_properties` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_073: [** Cloning the application properties shall be done by calling `application_properties_clone`. **]**
**SRS_MESSAGE_01_074: [** If `application_properties_clone` fails, `message_get_application_properties` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_150: [** If no application properties have been set, `message_get_application_properties` shall set `application_properties` to NULL. **]**

### message_set_footer

```C
MOCKABLE_FUNCTION(, int, message_set_footer, MESSAGE_HANDLE, message, annotations, footer);
```

**SRS_MESSAGE_01_075: [** `message_set_footer` shall copy the contents of `footer` as the footer contents for the message instance identified by `message`. **]**
**SRS_MESSAGE_01_076: [** On success it shall return 0. **]**
**SRS_MESSAGE_01_077: [** If `message` is NULL, `message_set_footer` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_078: [** Cloning the footer shall be done by calling `annotations_clone`. **]**
**SRS_MESSAGE_01_079: [** If `annotations_clone` fails, `message_set_footer` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_080: [** If setting the footer fails, the previous value shall be preserved. **]**
**SRS_MESSAGE_01_151: [** If `footer` is NULL, the previously stored footer associated with `message` shall be freed. **]**

### message_get_footer

```C
MOCKABLE_FUNCTION(, int, message_get_footer, MESSAGE_HANDLE, message, annotations*, footer);
```

**SRS_MESSAGE_01_081: [** `message_get_footer` shall copy the contents of footer for the message instance identified by `message` into the argument `footer`. **]**
**SRS_MESSAGE_01_082: [** On success, `message_get_footer` shall return 0. **]**
**SRS_MESSAGE_01_083: [** If `message` or `footer` is NULL, `message_get_footer` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_084: [** Cloning the footer shall be done by calling `annotations_clone`. **]**
**SRS_MESSAGE_01_085: [** If `annotations_clone` fails, `message_get_footer` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_152: [** If no footer has been set, `message_get_footer` shall set `footer` to NULL. **]**

### message_add_body_amqp_data

```C
MOCKABLE_FUNCTION(, int, message_add_body_amqp_data, MESSAGE_HANDLE, message, BINARY_DATA, amqp_data);
```

**SRS_MESSAGE_01_086: [** `message_add_body_amqp_data` shall add the contents of `amqp_data` to the list of AMQP data values for the body of the message identified by `message`. **]**
**SRS_MESSAGE_01_087: [** On success it shall return 0. **]**
**SRS_MESSAGE_01_088: [** If `message` is NULL, `message_add_body_amqp_data` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_089: [** If the `bytes` member of `amqp_data` is NULL and the `size` member is non-zero, `message_add_body_amqp_data` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_153: [** If allocating memory to store the added AMQP data fails, `message_add_body_amqp_data` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_090: [** If adding the body AMQP data fails, the previous body content shall be preserved. **]**
**SRS_MESSAGE_01_091: [** If the body was already set to an AMQP value or a list of AMQP sequences, `message_add_body_amqp_data` shall fail and return a non-zero value. **]**

### message_get_body_amqp_data_in_place

```C
MOCKABLE_FUNCTION(, int, message_get_body_amqp_data_in_place, MESSAGE_HANDLE, message, size_t, index, BINARY_DATA*, amqp_data);
```

**SRS_MESSAGE_01_092: [** `message_get_body_amqp_data_in_place` shall place the contents of the `index`th AMQP data for the message instance identified by `message` into the argument `amqp_data`, without copying the binary payload memory. **]**
**SRS_MESSAGE_01_093: [** On success, `message_get_body_amqp_data_in_place` shall return 0. **]**
**SRS_MESSAGE_01_094: [** If `message` or `amqp_data` is NULL, `message_get_body_amqp_data_in_place` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_095: [** If `index` indicates an AMQP data entry that is out of bounds, `message_get_body_amqp_data_in_place` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_096: [** If the body for `message` is not of type `MESSAGE_BODY_TYPE_DATA`, `message_get_body_amqp_data_in_place` shall fail and return a non-zero value. **]**

### message_get_body_amqp_data_count

```C
MOCKABLE_FUNCTION(, int, message_get_body_amqp_data_count, MESSAGE_HANDLE, message, size_t*, count);
```

**SRS_MESSAGE_01_097: [** `message_get_body_amqp_data_count` shall fill in `count` the number of AMQP data chunks that are stored by the message identified by `message`. **]**
**SRS_MESSAGE_01_098: [** On success, `message_get_body_amqp_data_count` shall return 0. **]**
**SRS_MESSAGE_01_099: [** If `message` or `count` is NULL, `message_get_body_amqp_data_count` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_100: [** If the body for `message` is not of type `MESSAGE_BODY_TYPE_DATA`, `message_get_body_amqp_data_count` shall fail and return a non-zero value. **]**

### message_set_body_amqp_value

```C
MOCKABLE_FUNCTION(, int, message_set_body_amqp_value, MESSAGE_HANDLE, message, AMQP_VALUE, body_amqp_value);
```

**SRS_MESSAGE_01_101: [** `message_set_body_amqp_value` shall set the contents of body as being the AMQP value indicate by `body_amqp_value`. **]**
**SRS_MESSAGE_01_102: [** On success it shall return 0. **]**
**SRS_MESSAGE_01_103: [** If `message` or `body_amqp_value` is NULL, `message_set_body_amqp_value` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_154: [** Cloning the amqp value shall be done by calling `amqpvalue_clone`. **]**
**SRS_MESSAGE_01_155: [** If `amqpvalue_clone` fails, `message_set_body_amqp_value` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_104: [** If setting the body AMQP value fails, the previous value shall be preserved. **]**
**SRS_MESSAGE_01_105: [** If the body was already set to an AMQP data list or a list of AMQP sequences, `message_set_body_amqp_value` shall fail and return a non-zero value. **]**

### message_get_body_amqp_value_in_place

```C
MOCKABLE_FUNCTION(, int, message_get_body_amqp_value_in_place, MESSAGE_HANDLE, message, AMQP_VALUE*, body_amqp_value);
```

**SRS_MESSAGE_01_106: [** `message_get_body_amqp_value_in_place` shall get the body AMQP value for the message instance identified by `message` in place (not cloning) into the `body_amqp_value` argument. **]**
**SRS_MESSAGE_01_107: [** On success, `message_get_body_amqp_value_in_place` shall return 0. **]**
**SRS_MESSAGE_01_108: [** If `message` or `body_amqp_value` is NULL, `message_get_body_amqp_value_in_place` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_109: [** If the body for `message` is not of type `MESSAGE_BODY_TYPE_VALUE`, `message_get_body_amqp_value_in_place` shall fail and return a non-zero value. **]**

### message_add_body_amqp_sequence

```C
MOCKABLE_FUNCTION(, int, message_add_body_amqp_sequence, MESSAGE_HANDLE, message, AMQP_VALUE, sequence);
```

**SRS_MESSAGE_01_110: [** `message_add_body_amqp_sequence` shall add the contents of `sequence` to the list of AMQP sequences for the body of the message identified by `message`. **]**
**SRS_MESSAGE_01_111: [** On success it shall return 0. **]**
**SRS_MESSAGE_01_156: [** The AMQP sequence shall be cloned by calling `amqpvalue_clone`. **]**
**SRS_MESSAGE_01_158: [** If allocating memory in order to store the sequence fails, `message_add_body_amqp_sequence` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_157: [** If `amqpvalue_clone` fails, `message_add_body_amqp_sequence` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_112: [** If `message` or `sequence` is NULL, `message_add_body_amqp_sequence` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_114: [** If adding the AMQP sequence fails, the previous value shall be preserved. **]**
**SRS_MESSAGE_01_115: [** If the body was already set to an AMQP data list or an AMQP value, `message_add_body_amqp_sequence` shall fail and return a non-zero value. **]**

### message_get_body_amqp_sequence_in_place

```C
MOCKABLE_FUNCTION(, int, message_get_body_amqp_sequence_in_place, MESSAGE_HANDLE, message, size_t, index, AMQP_VALUE*, sequence);
```

**SRS_MESSAGE_01_116: [** `message_get_body_amqp_sequence_in_place` shall return in `sequence` the content of the `index`th AMQP seuquence entry for the message instance identified by `message`. **]**
**SRS_MESSAGE_01_117: [** On success, `message_get_body_amqp_sequence_in_place` shall return 0. **]**
**SRS_MESSAGE_01_118: [** If `message` or `sequence` is NULL, `message_get_body_amqp_sequence_in_place` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_119: [** If `index` indicates an AMQP sequence entry that is out of bounds, `message_get_body_amqp_sequence_in_place` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_120: [** If the body for `message` is not of type `MESSAGE_BODY_TYPE_SEQUENCE`, `message_get_body_amqp_sequence_in_place` shall fail and return a non-zero value. **]**

### message_get_body_amqp_sequence_count

```C
MOCKABLE_FUNCTION(, int, message_get_body_amqp_sequence_count, MESSAGE_HANDLE, message, size_t*, count);
```

**SRS_MESSAGE_01_121: [** `message_get_body_amqp_sequence_count` shall fill in `count` the number of AMQP sequences that are stored by the message identified by `message`. **]**
**SRS_MESSAGE_01_122: [** On success, `message_get_body_amqp_sequence_count` shall return 0. **]**
**SRS_MESSAGE_01_123: [** If `message` or `count` is NULL, `message_get_body_amqp_sequence_count` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_124: [** If the body for `message` is not of type `MESSAGE_BODY_TYPE_SEQUENCE`, `message_get_body_amqp_sequence_count` shall fail and return a non-zero value. **]**

### message_get_body_type

```C
MOCKABLE_FUNCTION(, int, message_get_body_type, MESSAGE_HANDLE, message, MESSAGE_BODY_TYPE*, body_type);
```

**SRS_MESSAGE_01_125: [** `message_get_body_type` shall fill in `body_type` the AMQP message body type. **]**
**SRS_MESSAGE_01_126: [** On success, `message_get_body_type` shall return 0. **]**
**SRS_MESSAGE_01_127: [** If `message` or `body_type` is NULL, `message_get_body_type` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_128: [** If no body has been set on the message, `body_type` shall be `MESSAGE_BODY_TYPE_NONE`. **]**

### message_set_message_format

```C
MOCKABLE_FUNCTION(, int, message_set_message_format, MESSAGE_HANDLE, message, uint32_t, message_format);
```

**SRS_MESSAGE_01_129: [** `message_set_message_format` shall set the message format for the message identified by `message`. **]**
**SRS_MESSAGE_01_130: [** On success, `message_set_message_format` shall return 0. **]**
**SRS_MESSAGE_01_131: [** If `message` is NULL, `message_set_message_format` shall fail and return a non-zero value. **]**


### message_get_message_format

```C
MOCKABLE_FUNCTION(, int, message_get_message_format, MESSAGE_HANDLE, message, uint32_t*, message_format);
```

**SRS_MESSAGE_01_132: [** `message_get_message_format` shall get the message format for the message identified by `message` and return it in the `message_fomrat` argument. **]**
**SRS_MESSAGE_01_133: [** On success, `message_get_message_format` shall return 0. **]**
**SRS_MESSAGE_01_134: [** If `message` or `message_format` is NULL, `message_get_message_format` shall fail and return a non-zero value. **]**
**SRS_MESSAGE_01_135: [** By default a message on which `message_set_message_format` was not called shall have message format set to 0. **]**
