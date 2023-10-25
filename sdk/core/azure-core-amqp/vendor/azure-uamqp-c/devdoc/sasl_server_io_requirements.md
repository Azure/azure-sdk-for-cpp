# sasl_server_io requirements
 
## Overview

`sasl_server_io` is module that implements a concrete IO, that implements the section 5.3 of the AMQP ISO, allowing the usage on the server side of different negotiable SASL mechanisms.
The module allows by its configuration passed at create time the selection of one of several SASL mechanisms.
The implementation of each SASL mechanism for the server side is not part of this module.
The SASL server IO module communicates with concrete SASL mechanisms implementations through the `sasl_server_mechanism` API.

## Exposed API

```C
typedef struct SASL_SERVER_IO_TAG
{
    SERVER_PROTOCOL_IO_CONFIG server_protocol_io;
    const SASL_SERVER_MECHANISM_HANDLE* sasl_server_mechanisms;
    size_t sasl_server_mechanism_count;
} SASL_SERVER_IO;

MOCKABLE_FUNCTION(, const IO_INTERFACE_DESCRIPTION*, sasl_server_io_get_interface_description);
```

### sasl_server_io_get_interface_description

```C
MOCKABLE_FUNCTION(, const IO_INTERFACE_DESCRIPTION*, sasl_server_io_get_interface_description);
```

**SRS_SASL_SERVER_IO_01_001: [** `sasl_server_io_get_interface_description` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains pointers to the functions: `sasl_server_io_create`, `sasl_server_io_destroy`, `sasl_server_io_open_async`, `sasl_server_io_close_async`, `sasl_server_io_send_async`, `sasl_server_io_setoptions` and `sasl_server_io_dowork`. **]**

### sasl_server_io_create

```C
CONCRETE_IO_HANDLE sasl_server_io_create(void* io_create_parameters);
```

`sasl_server_io_create` is the implementation provided via `sasl_server_io_get_interface_description` for the `concrete_io_create` member.

**SRS_SASL_SERVER_IO_01_002: [** `sasl_server_io_create` shall return on success a non-NULL handle to a new SASL server IO instance. **]**

**SRS_SASL_SERVER_IO_01_003: [** If `io_create_parameters` is NULL, `sasl_server_io_create` shall fail and return NULL. **]**

**SRS_SASL_SERVER_IO_01_096: [** If the member `sasl_server_mechanisms` is NULL and `sasl_server_mechanism_count` is non-zero `sasl_server_io_create` shall fail and return NULL. **]**

**SRS_SASL_SERVER_IO_01_097: [** If the member `sasl_server_mechanism_count` is 0, `sasl_server_mechanism_count` is non-zero `sasl_server_io_create` shall fail and return NULL. **]**

**SRS_SASL_SERVER_IO_01_004: [** `io_create_parameters` shall be used as a pointer to the `server_protocol_io` member of a `SASL_SERVER_IO` structure (thus giving access to the entire `SASL_SERVER_IO` structure). **]**

**SRS_SASL_SERVER_IO_01_005: [** The SASL server mechanism handles shall be copied for later use. **]**

**SRS_SASL_SERVER_IO_01_006: [** If allocating memory for the SASL server mechanism handles array fails, `sasl_server_io_create` shall fail and return NULL. **]**

**SRS_SASL_SERVER_IO_01_007: [** If memory cannot be allocated for the new instance, `sasl_server_io_create` shall fail and return NULL. **]**

**SRS_SASL_SERVER_IO_01_008: [** `sasl_server_io_create` shall create a frame codec to be used for encoding/decoding frames by calling `frame_codec_create`. **]**

**SRS_SASL_SERVER_IO_01_009: [** If `frame_codec_create` fails, then `sasl_server_io_create` shall fail and return NULL. **]**

**SRS_SASL_SERVER_IO_01_010: [** `sasl_server_io_create` shall create a `sasl_frame_codec` to be used for SASL frame encoding/decoding by calling `sasl_frame_codec_create` and passing the just created frame codec as argument. **]**

**SRS_SASL_SERVER_IO_01_011: [** If `sasl_frame_codec_create` fails, then `sasl_server_io_create` shall fail and return NULL. **]**

### sasl_server_io_destroy

```C
void sasl_server_io_destroy(CONCRETE_IO_HANDLE sasl_server_io);
```

`sasl_server_io_destroy` is the implementation provided via `sasl_server_io_get_interface_description` for the `concrete_io_destroy` member.

**SRS_SASL_SERVER_IO_01_012: [** `sasl_server_io_destroy` shall free all resources associated with the SASL server IO handle. **]**

**SRS_SASL_SERVER_IO_01_013: [** `sasl_server_io_destroy` shall destroy the SASL frame codec created in `sasl_server_io_create` by calling `sasl_frame_codec_destroy`. **]**

**SRS_SASL_SERVER_IO_01_014: [** `sasl_server_io_destroy` shall destroy the frame codec created in `sasl_server_io_create` by calling `frame_codec_destroy`. **]**

**SRS_SASL_SERVER_IO_01_015: [** If the argument `sasl_server_io` is NULL, `sasl_server_io_destroy` shall do nothing. **]**

### sasl_server_io_open_async

```C
int sasl_server_io_open_async(CONCRETE_IO_HANDLE sasl_server_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
```

`sasl_server_io_open_async` is the implementation provided via `sasl_server_io_get_interface_description` for the `concrete_io_open` member.

**SRS_SASL_SERVER_IO_01_016: [** `sasl_server_io_open_async` shall populate the `on_bytes_received` pointer passed as part of the create parameters with the `on_underlying_bytes_received` function pointer. **]**

**SRS_SASL_SERVER_IO_01_017: [** `sasl_server_io_open_async` shall populate the `on_bytes_received_context` pointer passed as part of the create parameters with the IO handle. **]**

**SRS_SASL_SERVER_IO_01_018: [** `sasl_server_io_open_async` shall kick off the SASL handshake by encoding and sending the SASL mechanisms frame. **]**

**SRS_SASL_SERVER_IO_01_019: [** The SASL mechanisms encoded in the SASL mechanisms frame shall be the ones passed via the create parameters. **]**

**SRS_SASL_SERVER_IO_01_020: [** The name of each SASL mechanism shall be obtained by calling `sasl_server_mechanism_get_mechanism_name`. **]**

**SRS_SASL_SERVER_IO_01_095: [** When populating the frame, the order of the mechanisms shall be the preserved. **]**

**SRS_SASL_SERVER_IO_01_021: [** If `sasl_server_mechanism_get_mechanism_name` fails, `sasl_server_io_open_async` shall fail and return non-zero value. **]**

**SRS_SASL_SERVER_IO_01_022: [** If creating the mechanisms AMQP values fails, `sasl_server_io_open_async` shall fail and return non-zero value. **]**

**SRS_SASL_SERVER_IO_01_023: [** The SASL mechanisms encode shall be done by calling `sasl_frame_codec_encode_frame`. **]**

**SRS_SASL_SERVER_IO_01_024: [** On success, `sasl_server_io_open_async` shall return 0. **]**

**SRS_SASL_SERVER_IO_01_025: [** If any of the `sasl_server_io`, `on_io_open_complete`, `on_bytes_received` or `on_io_error` arguments is NULL, `sasl_server_io_open_async` shall fail and return a non-zero value. **]**

**SRS_SASL_SERVER_IO_01_026: [** `on_io_open_complete_context`, `on_bytes_received_context` and `on_io_error_context` shall be allowed to be NULL. **]**

**SRS_SASL_SERVER_IO_01_027: [** If `sasl_frame_codec_encode_frame` fails, `sasl_server_io_open_async` shall fail and return non-zero value. **]**

**SRS_SASL_SERVER_IO_01_028: [** `sasl_server_io_open_async` when already OPENING shall fail and return a non-zero value. **]**

### sasl_server_io_close_async

```C
int sasl_server_io_close_async(CONCRETE_IO_HANDLE sasl_server_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* on_io_close_complete_context);
```

`sasl_server_io_close_async` is the implementation provided via `sasl_server_io_get_interface_description` for the `concrete_io_close` member.

**SRS_SASL_SERVER_IO_01_029: [** `sasl_server_io_close_async` shall close the underlying io handle passed in `sasl_server_io_create` by calling `xio_close`. **]**

**SRS_SASL_SERVER_IO_01_030: [** On success, `sasl_server_io_close_async` shall return 0. **]**

**SRS_SASL_SERVER_IO_01_031: [** `sasl_server_io_close_async` when the IO is not open shall fail and return a non-zero value. **]**

**SRS_SASL_SERVER_IO_01_032: [** If `sasl_server_io_close_async` is called when the IO is OPENING shall close the underlying IO and call the `on_io_open_complete` callback with `IO_OPEN_CANCELLED`. **]**

**SRS_SASL_SERVER_IO_01_033: [** If `sasl_server_io` is NULL, `sasl_server_io_close_async` shall fail and return a non-zero value. **]**

**SRS_SASL_SERVER_IO_01_034: [** If `xio_close` fails, then `sasl_server_io_close_async` shall return a non-zero value. **]**

**SRS_SASL_SERVER_IO_01_035: [** If `sasl_server_io_close_async` is closed while already CLOSING it shall fail and return a non-zero value. **]**

### sasl_server_io_send_async

```C
int sasl_server_io_send_async(CONCRETE_IO_HANDLE sasl_server_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
```

`sasl_server_io_send_async` is the implementation provided via `sasl_server_io_get_interface_description` for the `concrete_io_send` member.

**SRS_SASL_SERVER_IO_01_036: [** If the SASL server IO state is OPEN, `sasl_server_io_send_async` shall call `xio_send` on the underlying IO passed to `sasl_server_io_create`, while passing as arguments the `buffer`, `size`, `on_send_complete` and `callback_context` arguments. **]**

**SRS_SASL_SERVER_IO_01_037: [** On success, If `sasl_server_io_send_async` shall return 0. **]**

**SRS_SASL_SERVER_IO_01_038: [** If the `sasl_server_io` or `buffer` argument is NULL, `sasl_server_io_send_async` shall fail and return a non-zero value. **]**

**SRS_SASL_SERVER_IO_01_039: [** If `sasl_server_io_send_async` is called while the IO is not OPEN, `sasl_server_io_send_async` shall fail and return a non-zero value. **]**

**SRS_SASL_SERVER_IO_01_040: [** If `size` is 0, `sasl_server_io_send_async` shall fail and return a non-zero value. **]**

**SRS_SASL_SERVER_IO_01_041: [** If the call to `xio_send` fails, then `sasl_server_io_send_async` shall fail and return a non-zero value. **]**

### sasl_server_io_dowork

```C
void sasl_server_io_dowork(CONCRETE_IO_HANDLE sasl_server_io);
```

`sasl_server_io_dowork` is the implementation provided via `sasl_server_io_get_interface_description` for the `concrete_io_dowork` member.

**SRS_SASL_SERVER_IO_01_042: [** `sasl_server_io_dowork` shall call the `xio_dowork` on the underlying IO passed in `sasl_server_io_create`.**]**

**SRS_SASL_SERVER_IO_01_043: [** If the `sasl_server_io` argument is NULL, `sasl_server_io_dowork` shall do nothing. **]**

### sasl_server_io_setoption

```C
int sasl_server_io_setoption(CONCRETE_IO_HANDLE sasl_server_io, const char* option_name, const void* value);
```

`sasl_server_io_setoption` is the implementation provided via `sasl_server_io_get_interface_description` for the `concrete_io_setoption` member.

**SRS_SASL_SERVER_IO_01_044: [** `sasl_server_io_setoption` shall forward options to underlying io by calling `xio_setoption`. **]**

**SRS_SASL_SERVER_IO_01_045: [** On success `sasl_server_io_setoption` shall return 0. **]**

**SRS_SASL_SERVER_IO_01_046: [** If `sasl_server_io` or `option_name` is NULL, `sasl_server_io_setoption` shall fail and return a non-zero value. **]**

**SRS_SASL_SERVER_IO_01_047: [** If `xio_setoption` fails, `sasl_server_io_setoption` shall fail and return a non-zero value. **]**

### sasl_server_io_retrieve_options

```C
OPTIONHANDLER_HANDLE sasl_server_io_retrieve_options(CONCRETE_IO_HANDLE header_detect_io)
```

`sasl_server_io_retrieve_options` is the implementation provided via `header_detect_io_get_interface_description` for the `concrete_io_retrieveoptions` member.

**SRS_SASL_SERVER_IO_01_048: [** `sasl_server_io_retrieve_options` shall create a new `OPTIONHANDLER_HANDLE` by calling `OptionHandler_Create` and on success it shall return a non-NULL handle to the newly created option handler. **]**

**SRS_SASL_SERVER_IO_01_049: [** No options shall be added to the newly created option handler. **]**

**SRS_SASL_SERVER_IO_01_050: [** If `OptionHandler_Create` fails, `sasl_server_io_retrieve_options` shall return NULL. **]**

**SRS_SASL_SERVER_IO_01_051: [** If `header_detect_io` is NULL, `sasl_server_io_retrieve_options` shall return NULL. **]**

### on_underlying_bytes_received

```C
void on_underlying_io_bytes_received(void* context, const unsigned char* buffer, size_t size)
```

**SRS_SASL_SERVER_IO_01_052: [** `on_underlying_bytes_received` called with a NULL `context` shall do nothing. **]**

**SRS_SASL_SERVER_IO_01_053: [** When the `on_underlying_bytes_received` callback is called and the SASL server IO is OPEN, the bytes shall be indicated to the user of SASL server IO by calling the `on_bytes_received` that was passed in `sasl_server_io_open_async`. **]**

**SRS_SASL_SERVER_IO_01_054: [** If `buffer` is NULL or size is zero, nothing should be indicated as received and an error should be indicated by calling the `on_io_error` callback shall be triggered, while passing the `on_io_error_context` to it. **]**

**SRS_SASL_SERVER_IO_01_055: [** If bytes are received when the SASL server IO state is OPENING, the bytes shall be consumed by the SASL server IO to satisfy the SASL handshake by calling `frame_codec_receive_bytes` with each byte that was received. **]**

**SRS_SASL_SERVER_IO_01_056: [** If bytes are received when the SASL server IO state is in ERROR, SASL server IO shall do nothing. **]**

**SRS_SASL_SERVER_IO_01_057: [** Any bytes that are extra and were not consumed for the SASL shandshake shall be indicated to the user of SASL server IO by calling the `on_bytes_received` that was passed in `sasl_server_io_open_async`. **]**

### on_sasl_frame_received_callback

```C
void on_sasl_frame_received_callback(void* context, AMQP_VALUE sasl_frame);
```

**SRS_SASL_SERVER_IO_01_058: [** `on_sasl_frame_received_callback` with a NULL `context` shall do nothing. **]**

**SRS_SASL_SERVER_IO_01_059: [** If `on_sasl_frame_received_callback` is called when the IO is OPEN then the SASL server IO shall call the `on_io_open_complete` callback with `IO_OPEN_ERROR` and close the IO. **]**

**SRS_SASL_SERVER_IO_01_060: [** When a frame is indicated as received by the SASL frame codec it shall be processed as described in the ISO. **]**

**SRS_SASL_SERVER_IO_01_061: [** The calls to the concrete implementation of the SASL mechanism shall be done by calling `sasl_server_mechanism_handle_initial_response` and `sasl_server_mechanism_handle_response`. **]**

**SRS_SASL_SERVER_IO_01_062: [** When a frame needs to be sent as part of the SASL handshake frame exchange, the send shall be done by calling `sasl_frame_codec_encode_frame`. **]**

**SRS_SASL_SERVER_IO_01_063: [** If `sasl_frame_codec_encode_frame` fails, then the SASL server IO shall call the `on_io_open_complete` callback with `IO_OPEN_ERROR` and close the IO. **]**

**SRS_SASL_SERVER_IO_01_064: [** When the SASL handshake is complete, if the handshake is successful, the SASL server IO shall be considered OPEN and the `on_io_open_complete` callback shall be called with `IO_OPEN_OK`. **]**

**SRS_SASL_SERVER_IO_01_065: [** If any error is encountered when parsing the received frame, the SASL server IO state shall call the `on_io_open_complete` callback with `IO_OPEN_ERROR` and close the IO. **]**

**SRS_SASL_SERVER_IO_01_066: [** If the handshake fails (i.e. the outcome is an error) the SASL server IO state shall call the `on_io_open_complete` callback with `IO_OPEN_ERROR` and close the IO. **]**

### on_bytes_encoded

```C
void on_bytes_encoded(void* context, const unsigned char* bytes, size_t length, bool encode_complete)
```

**SRS_SASL_SERVER_IO_01_067: [** When SASL server IO is notified by the SASL frame codec of bytes that have been encoded via the `on_bytes_encoded` callback while the SASL server IO is OPENING, it shall send these bytes by calling `xio_send` on the underlying IO. **]**

**SRS_SASL_SERVER_IO_01_068: [** When `on_bytes_encoded` is called with a NULL `context` it shall do nothing. **]**

**SRS_SASL_SERVER_IO_01_069: [** When `on_bytes_encoded` is called with NULL bytes it shall call the `on_io_open_complete` callback with `IO_OPEN_ERROR` and close the IO.  **]**

**SRS_SASL_SERVER_IO_01_070: [** When `on_bytes_encoded` is called with 0 `size` it shall call the `on_io_open_complete` callback with `IO_OPEN_ERROR` and close the IO.  **]**

**SRS_SASL_SERVER_IO_01_071: [** If `xio_send` fails, the SASL server IO state shall call the `on_io_open_complete` callback with `IO_OPEN_ERROR` and close the IO. **]**

**SRS_SASL_SERVER_IO_01_072: [** If `on_bytes_encoded` is called when already OPEN, then the `on_io_error` callback shall be called while passing to it `on_io_error_context`. **]**

### on_frame_codec_error

```C
void on_frame_codec_error(void* context);
```

**SRS_SASL_SERVER_IO_01_073: [** When `on_frame_codec_error` is called while OPENING SASL server IO shall call the `on_io_open_complete` callback with `IO_OPEN_ERROR` and close the IO. **]**

**SRS_SASL_SERVER_IO_01_074: [** When `on_frame_codec_error` is called when the IO is OPEN then the `on_io_error` callback shall be called while passing to it `on_io_error_context`. **]**

**SRS_SASL_SERVER_IO_01_075: [** When `on_frame_codec_error` is called with NULL `context`, `on_frame_codec_error` shall do nothing. **]**

**SRS_SASL_SERVER_IO_01_076: [** When `on_frame_codec_error` is called after the IO had an error, `on_frame_codec_error` shall do nothing. **]**

### on_sasl_frame_codec_error

```C
void on_sasl_frame_codec_error(void* context);
```

**SRS_SASL_SERVER_IO_01_077: [** When `on_sasl_frame_codec_error` is called when OPENING the SASL server IO shall call the `on_io_open_complete` callback with `IO_OPEN_ERROR` and close the IO. **]**

**SRS_SASL_SERVER_IO_01_078: [** When `on_sasl_frame_codec_error` is called when the IO is OPEN then the `on_io_error` callback shall be called while passing to it `on_io_error_context`. **]**

**SRS_SASL_SERVER_IO_01_079: [** When `on_sasl_frame_codec_error` is called with NULL `context`, `on_sasl_frame_codec_error` shall do nothing. **]**

**SRS_SASL_SERVER_IO_01_080: [** When `on_sasl_frame_codec_error` is called after the IO had an error, `on_sasl_frame_codec_error` shall do nothing. **]**

## ISO section

To establish a SASL layer, each peer MUST start by sending a protocol header.
The protocol header consists of the upper case ASCII letters "AMQP" followed by a protocol id of three, followed by three unsigned bytes representing the major, minor, and revision of the specification version (currently 1 (SASL-MAJOR), 0 (SASLMINOR), 0 (SASL-REVISION)). In total this is an 8-octet sequence:

...

Figure 5.3: Protocol Header for SASL Security Layer

Other than using a protocol id of three, the exchange of SASL layer headers follows the same rules specified in the version negotiation section of the transport specification (See Part 2: section 2.2).
The following diagram illustrates the interaction involved in creating a SASL security layer:

...

Figure 5.4: Establishing a SASL Security Layer

SASL Negotiation

**SRS_SASL_SERVER_IO_01_081: [** The peer acting as the SASL server MUST announce supported authentication mechanisms using the sasl-mechanisms frame. **]**
**SRS_SASL_SERVER_IO_01_082: [** The partner MUST then choose one of the supported mechanisms and initiate a sasl exchange. **]**

SASL Client SASL Server

**SRS_SASL_SERVER_IO_01_083: [** <-- SASL-MECHANISMS **]**
**SRS_SASL_SERVER_IO_01_084: [** SASL-INIT --> **]**
...
**SRS_SASL_SERVER_IO_01_085: [** <-- SASL-CHALLENGE * **]**
**SRS_SASL_SERVER_IO_01_086: [** SASL-RESPONSE --> **]**
...
**SRS_SASL_SERVER_IO_01_087: [** <-- SASL-OUTCOME **]**

* Note that **SRS_SASL_SERVER_IO_01_088: [** the SASL challenge/response step can occur zero or more times depending on the details of the SASL mechanism chosen. **]**

Figure 5.6: SASL Exchange

**SRS_SASL_SERVER_IO_01_089: [** The peer playing the role of the SASL client and the peer playing the role of the SASL server MUST correspond to the TCP client and server respectively. **]**

5.3.3 Security Frame Bodies

5.3.3.1 SASL Mechanisms

**SRS_SASL_SERVER_IO_01_090: [** Advertise available sasl mechanisms. **]**

\<type name="sasl-mechanisms" class="composite" source="list" provides="sasl-frame">
\<descriptor name="amqp:sasl-mechanisms:list" code="0x00000000:0x00000040"/>
\<field name="sasl-server-mechanisms" type="symbol" multiple="true" mandatory="true"/>
\</type>

Advertises the available SASL mechanisms that can be used for authentication.

Field Details

**SRS_SASL_SERVER_IO_01_091: [** sasl-server-mechanisms supported sasl mechanisms **]**

**SRS_SASL_SERVER_IO_01_092: [** A list of the sasl security mechanisms supported by the sending peer. **]**
**SRS_SASL_SERVER_IO_01_093: [** It is invalid for this list to be null or empty. **]**
**SRS_SASL_SERVER_IO_01_094: [** If the sending peer does not require its partner to authenticate with it, then it SHOULD send a list of one element with its value as the SASL mechanism ANONYMOUS. **]**
The server mechanisms are ordered in decreasing level of preference.

5.3.3.2 SASL Init

Initiate sasl exchange.

\<type name="sasl-init" class="composite" source="list" provides="sasl-frame">

\<descriptor name="amqp:sasl-init:list" code="0x00000000:0x00000041"/>

\<field name="mechanism" type="symbol" mandatory="true"/>

\<field name="initial-response" type="binary"/>

\<field name="hostname" type="string"/>

\</type>

**SRS_SASL_SERVER_IO_01_098: [** Selects the sasl mechanism and provides the initial response if needed. **]**

Field Details

mechanism selected security mechanism
**SRS_SASL_SERVER_IO_01_099: [** The name of the SASL mechanism used for the SASL exchange. **]**
**SRS_SASL_SERVER_IO_01_100: [** If the selected mechanism is not supported by the receiving peer, it MUST close the connection with the authentication-failure close-code. **]** Each peer MUST authenticate using the highest-level security profile it can handle from the list provided by the partner.
initial-response security response data
**SRS_SASL_SERVER_IO_01_101: [** A block of opaque data passed to the security mechanism. **]** The contents of this data are defined by the SASL security mechanism.
hostname the name of the target host
The DNS name of the host (either fully qualified or relative) to which the sending peer is connecting.
**SRS_SASL_SERVER_IO_01_102: [** It is not mandatory to provide the hostname. **]** If no hostname is provided the receiving peer SHOULD select a default based on its own configuration.
This field can be used by AMQP proxies to determine the correct back-end service to connect the client to, and to determine the domain to validate the client's credentials against.
This field might already have been specified by the server name indication extension as described in RFC-4366 [RFC4366, if a TLS layer is used, in which case this field SHOULD either be null or contain the same value.] It is undefined what a different value to those already specified means.

5.3.3.3 SASL Challenge

**SRS_SASL_SERVER_IO_01_103: [** Security mechanism challenge. **]**

\<type name="sasl-challenge" class="composite" source="list" provides="sasl-frame">

\<descriptor name="amqp:sasl-challenge:list" code="0x00000000:0x00000042"/>

\<field name="challenge" type="binary" mandatory="true"/>

\</type>

Send the SASL challenge data as defined by the SASL specification.

Field Details

challenge security challenge data

**SRS_SASL_SERVER_IO_01_104: [** Challenge information, a block of opaque binary data passed to the security mechanism. **]**

5.3.3.4 SASL Response

Security mechanism response.

\<type name="sasl-response" class="composite" source="list" provides="sasl-frame">

\<descriptor name="amqp:sasl-response:list" code="0x00000000:0x00000043"/>

\<field name="response" type="binary" mandatory="true"/>

\</type>

**SRS_SASL_SERVER_IO_01_105: [** Send the SASL response data as defined by the SASL specification. **]**

Field Details

response security response data

**SRS_SASL_SERVER_IO_01_106: [** A block of opaque data passed to the security mechanism. **]** The contents of this data are defined by the SASL security mechanism.

5.3.3.5 SASL Outcome

**SRS_SASL_SERVER_IO_01_107: [** Indicates the outcome of the sasl dialog. **]**

\<type name="sasl-outcome" class="composite" source="list" provides="sasl-frame">

\<descriptor name="amqp:sasl-outcome:list" code="0x00000000:0x00000044"/>

\<field name="code" type="sasl-code" mandatory="true"/>

\<field name="additional-data" type="binary"/>

\</type>

**SRS_SASL_SERVER_IO_01_108: [** This frame indicates the outcome of the SASL dialog. **]**
**SRS_SASL_SERVER_IO_01_109: [** Upon successful completion of the SASL dialog the security layer has been established **]**, and the peers MUST exchange protocol headers to either start a nested security layer, or to establish the AMQP connection.

Field Details

code indicates the outcome of the sasl dialog

**SRS_SASL_SERVER_IO_01_110: [** A reply-code indicating the outcome of the SASL dialog. **]**

additional-data additional data as specified in RFC-4422

**SRS_SASL_SERVER_IO_01_111: [** The additional-data field carries additional data on successful authentication outcome as specified by the SASL specification [RFC4422.] **]**
**SRS_SASL_SERVER_IO_01_112: [** If the authentication is unsuccessful, this field is not set. **]**

5.3.3.6 SASL Code

**SRS_SASL_SERVER_IO_01_113: [** Codes to indicate the outcome of the sasl dialog. **]**

\<type name="sasl-code" class="restricted" source="ubyte">

\<choice name="ok" value="0"/>

\<choice name="auth" value="1"/>

\<choice name="sys" value="2"/>

\<choice name="sys-perm" value="3"/>

\<choice name="sys-temp" value="4"/>

\</type>

Valid Values

**SRS_SASL_SERVER_IO_01_114: [** 0 Connection authentication succeeded. **]**
**SRS_SASL_SERVER_IO_01_115: [** 1 Connection authentication failed due to an unspecified problem with the supplied credentials. **]**
**SRS_SASL_SERVER_IO_01_116: [** 2 Connection authentication failed due to a system error. **]**
**SRS_SASL_SERVER_IO_01_117: [** 3 Connection authentication failed due to a system error that is unlikely to be corrected without intervention. **]**
**SRS_SASL_SERVER_IO_01_118: [** 4 Connection authentication failed due to a transient system error. **]**

5.3.4 Constant Definitions

SASL-MAJOR 1 major protocol version.
SASL-MINOR 0 minor protocol version.
SASL-REVISION 0 protocol revision.
