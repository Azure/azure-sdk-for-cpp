# saslclientio requirements
 
## Overview

`saslclientio` is module that implements a SASL client concrete IO, per the section 5.3 of the AMQP ISO.

## Exposed API

```C
typedef struct SASLCLIENTIO_CONFIG_TAG
{
    XIO_HANDLE underlying_io;
    SASL_MECHANISM_HANDLE sasl_mechanism;
} SASLCLIENTIO_CONFIG;

MOCKABLE_FUNCTION(, const IO_INTERFACE_DESCRIPTION*, saslclientio_get_interface_description);
```

### saslclientio_create

```C
CONCRETE_IO_HANDLE saslclientio_create(void* io_create_parameters);
```

`saslclientio_create` is the implementation provided via `header_detect_io_get_interface_description` for the `concrete_io_create` member.

**SRS_SASLCLIENTIO_01_004: [**`saslclientio_create` shall return on success a non-NULL handle to a new SASL client IO instance.**]**

**SRS_SASLCLIENTIO_01_005: [**If `io_create_parameters` is NULL, `saslclientio_create` shall fail and return NULL.**]**

**SRS_SASLCLIENTIO_01_006: [**If memory cannot be allocated for the new instance, `saslclientio_create` shall fail and return NULL.**]**

**SRS_SASLCLIENTIO_01_089: [**`saslclientio_create` shall create a frame codec to be used for encoding/decoding frames by calling `frame_codec_create` and passing `on_frame_codec_error` and a context as arguments.**]**

**SRS_SASLCLIENTIO_01_090: [**If `frame_codec_create` fails, then `saslclientio_create` shall fail and return NULL.**]**

**SRS_SASLCLIENTIO_01_084: [**`saslclientio_create` shall create a SASL frame codec to be used for SASL frame encoding/decoding by calling `sasl_frame_codec_create` and passing the just created frame codec as argument.**]**

**SRS_SASLCLIENTIO_01_085: [**If `sasl_frame_codec_create` fails, then `saslclientio_create` shall fail and return NULL.**]**

**SRS_SASLCLIENTIO_01_092: [**If any of the `sasl_mechanism` or `underlying_io` members of the configuration structure are NULL, `saslclientio_create` shall fail and return NULL.**]**

### saslclientio_destroy

```C
void saslclientio_destroy(CONCRETE_IO_HANDLE sasl_client_io);
```

`saslclientio_destroy` is the implementation provided via `header_detect_io_get_interface_description` for the `concrete_io_destroy` member.

**SRS_SASLCLIENTIO_01_007: [**`saslclientio_destroy` shall free all resources associated with the SASL client IO handle.**]** 

**SRS_SASLCLIENTIO_01_086: [**`saslclientio_destroy` shall destroy the SASL frame codec created in `saslclientio_create` by calling `sasl_frame_codec_destroy`.**]** 

**SRS_SASLCLIENTIO_01_091: [**`saslclientio_destroy` shall destroy the frame codec created in `saslclientio_create` by calling `frame_codec_destroy`.**]** 

**SRS_SASLCLIENTIO_01_008: [**If the argument `sasl_client_io` is NULL, `saslclientio_destroy` shall do nothing.**]** 

### saslclientio_open_async

```C
int saslclientio_open_async(CONCRETE_IO_HANDLE sasl_client_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context);
```

`saslclientio_open_async` is the implementation provided via `header_detect_io_get_interface_description` for the `concrete_io_open` member.

**SRS_SASLCLIENTIO_01_009: [**`saslclientio_open` shall call `xio_open` on the `underlying_io` passed to `saslclientio_create`.**]** 

**SRS_SASLCLIENTIO_01_010: [**On success, `saslclientio_open_async` shall return 0.**]** 

**SRS_SASLCLIENTIO_01_011: [**If any of the `sasl_client_io`, `on_io_open_complete`, `on_bytes_received` or `on_io_error` arguments is NULL, `saslclientio_open` shall fail and return a non-zero value.**]** 

**SRS_SASLCLIENTIO_01_012: [**If the open of the `underlying_io` fails, `saslclientio_open_async` shall fail and return non-zero value.**]** 

**SRS_SASLCLIENTIO_01_013: [**`saslclientio_open_async` shall pass to `xio_open` the `on_underlying_io_open_complete` as `on_io_open_complete` argument, `on_underlying_io_bytes_received` as `on_bytes_received` argument and `on_underlying_io_error` as `on_io_error` argument.**]** 

### saslclientio_close_async

```C
int saslclientio_close_async(CONCRETE_IO_HANDLE sasl_client_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* on_io_close_complete_context);
```

`saslclientio_close_async` is the implementation provided via `header_detect_io_get_interface_description` for the `concrete_io_close` member.

**SRS_SASLCLIENTIO_01_015: [**`saslclientio_close_async` shall close the underlying io handle passed in `saslclientio_create` by calling `xio_close`.**]** 

**SRS_SASLCLIENTIO_01_016: [**On success, `saslclientio_close_async` shall return 0.**]** 

**SRS_SASLCLIENTIO_01_098: [**`saslclientio_close_async` shall only perform the close if the state is OPEN, OPENING or ERROR.**]** 

**SRS_SASLCLIENTIO_01_097: [**If `saslclientio_close_async` is called when the IO is in the `IO_STATE_NOT_OPEN` state, `saslclientio_close_async` shall fail and return a non zero value.**]** 

**SRS_SASLCLIENTIO_01_017: [**If `sasl_client_io` is NULL, `saslclientio_close_async` shall fail and return a non-zero value.**]** 

**SRS_SASLCLIENTIO_01_018: [**If `xio_close` fails, then `saslclientio_close_async` shall return a non-zero value.**]** 

### saslclientio_send_async

```C
int saslclientio_send_async(CONCRETE_IO_HANDLE sasl_client_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
```

`saslclientio_send_async` is the implementation provided via `header_detect_io_get_interface_description` for the `concrete_io_send` member.

**SRS_SASLCLIENTIO_01_019: [**If `saslclientio_send_async` is called while the SASL client IO state is not `IO_STATE_OPEN`, `saslclientio_send_async` shall fail and return a non-zero value.**]** 

**SRS_SASLCLIENTIO_01_020: [**If the SASL client IO state is `IO_STATE_OPEN`, `saslclientio_send_async` shall call `xio_send` on the `underlying_io` passed to `saslclientio_create`, while passing as arguments the `buffer`,`size`, `on_send_complete` and `callback_context`.**]** 

**SRS_SASLCLIENTIO_01_021: [**On success, `saslclientio_send_async` shall return 0.**]** 

**SRS_SASLCLIENTIO_01_022: [**If the `sasl_client_io` or `buffer` argument is NULL, `saslclientio_send_async` shall fail and return a non-zero value.**]** 

**SRS_SASLCLIENTIO_01_023: [**If `size` is 0, `saslclientio_send_async` shall fail and return a non-zero value.**]**

**SRS_SASLCLIENTIO_01_127: [** `on_send_complete` shall be allowed to be NULL. **]** 

**SRS_SASLCLIENTIO_01_024: [**If the call to `xio_send` fails, then `saslclientio_send_async` shall fail and return a non-zero value.**]** 

### saslclientio_dowork

```C
void saslclientio_dowork(CONCRETE_IO_HANDLE sasl_client_io);
```

`saslclientio_dowork` is the implementation provided via `header_detect_io_get_interface_description` for the `concrete_io_dowork` member.

**SRS_SASLCLIENTIO_01_025: [**`saslclientio_dowork` shall call the `xio_dowork` on the `underlying_io` passed in `saslclientio_create`.**]** 

**SRS_SASLCLIENTIO_01_099: [**If the state of the IO is `IO_NOT_OPEN`, `saslclientio_dowork` shall make no calls to the underlying IO.**]** 

**SRS_SASLCLIENTIO_01_026: [**If the `sasl_client_io` argument is NULL, `saslclientio_dowork` shall do nothing.**]** 

### saslclientio_get_interface_description

```C
MOCKABLE_FUNCTION(, const IO_INTERFACE_DESCRIPTION*, saslclientio_get_interface_description);
```

**SRS_SASLCLIENTIO_01_087: [**`saslclientio_get_interface_description` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains pointers to the functions: `saslclientio_create`, `saslclientio_destroy`, `saslclientio_open_async`, `saslclientio_close_async`, `saslclientio_send_async`, `saslclientio_setoption`, `saslclientio_retrieveoptions` and `saslclientio_dowork`.**]** 

### saslclientio_setoption

```C
int saslclientio_setoption(CONCRETE_IO_HANDLE sasl_client_io, const char* option_name, const void* value);
```

`saslclientio_setoption` is the implementation provided via `header_detect_io_get_interface_description` for the `concrete_io_setoption` member.

**SRS_SASLCLIENTIO_03_001: [**`saslclientio_setoption` shall forward all unhandled options to underlying io by calling `xio_setoption`.**]**

**SRS_SASLCLIENTIO_01_128: [** On success, `saslclientio_setoption` shall return 0. **]**

**SRS_SASLCLIENTIO_01_129: [** If `xio_setoption` fails, `saslclientio_setoption` shall fail and return a non-zero value. **]**

**SRS_SASLCLIENTIO_01_130: [** If `sasl_client_io` or `option_name` is NULL, `saslclientio_setoption`  shall fail and return a non-zero value. **]**

**SRS_SASLCLIENTIO_01_131: [** SASL client IO shall handle the following options: **]**

**SRS_SASLCLIENTIO_01_132: [** - logtrace - bool. **]**

### saslclientio_retrieveoption

```C
OPTIONHANDLER_HANDLE saslclientio_retrieveoptions(CONCRETE_IO_HANDLE sasl_client_io);
```

`saslclientio_retrieveoptions` is the implementation provided via `header_detect_io_get_interface_description` for the `concrete_io_retrieveoptions` member.

**SRS_SASLCLIENTIO_01_133: [** `saslclientio_retrieveoptions` shall create an option handler by calling `OptionHandler_Create`. **]**

**SRS_SASLCLIENTIO_01_134: [** `saslclientio_retrieveoptions` shall add the handled options to it: **]**

**SRS_SASLCLIENTIO_01_135: [** - logtrace - bool. **]**

**SRS_SASLCLIENTIO_01_136: [** If the `logtrace` option was not set it shall not be added to the option Handler. **]**

**SRS_SASLCLIENTIO_01_137: [** The options shall be added by calling `OptionHandler_AddOption`. **]**

**SRS_SASLCLIENTIO_01_138: [** If `OptionHandler_AddOption` or `OptionHandler_Create` fails then `saslclientio_retrieveoptions` shall fail and return NULL. **]**

**SRS_SASLCLIENTIO_01_139: [** When `saslclientio_retrieveoptions` is called with NULL `sasl_client_io` it shall fail and return NULL. **]**

### on_underlying_io_bytes_received

```C
void on_underlying_io_bytes_received(void* context, const unsigned char* buffer, size_t size)
```

**SRS_SASLCLIENTIO_01_027: [**When the `on_underlying_io_bytes_received` callback passed to the underlying IO is called and the SASL client IO state is `IO_STATE_OPEN`, the bytes shall be indicated to the user of SASL client IO by calling the `on_bytes_received` callback that was passed in `saslclientio_open`.**]** 

**SRS_SASLCLIENTIO_01_140: [** If the `context` argument is NULL, `on_underlying_io_bytes_received` shall do nothing. **]**

**SRS_SASLCLIENTIO_01_028: [**If `buffer` is NULL or `size` is zero, nothing should be indicated as received, the state shall be switched to ERROR and the `on_io_error` callback shall be triggered.**]** 

**SRS_SASLCLIENTIO_01_029: [**The `context` argument for `on_io_error` shall be set to the `on_io_error_context` passed in `saslclientio_open`.**]** 

**SRS_SASLCLIENTIO_01_030: [**If bytes are received when the SASL client IO state is `IO_STATE_OPENING`, the bytes shall be consumed by the SASL client IO to satisfy the SASL handshake.**]** 

**SRS_SASLCLIENTIO_01_031: [**If bytes are received when the SASL client IO state is `IO_STATE_ERROR`, SASL client IO shall do nothing.**]** 

### State handling

The following actions matrix shall be implemented when a new state change is received from the underlying IO:

|SASL state\new underlying IO state|`NOT_OPEN`	 |`OPENING`                                        |`OPEN`                                                    |`ERROR`                                                                   |
|----------------------------------|-------------|-------------------------------------------------|----------------------------------------------------------|--------------------------------------------------------------------------|
|`NOT_OPEN`                        |do nothing   |do nothing                                       |do nothing                                                |do nothing                                                                | 
|`OPENING`                         |not possible |do nothing                                       |**`SRS_SASLCLIENTIO_01_105`: [**start header exchange**]**|**SRS_SASLCLIENTIO_01_101: [**`on_open_complete` with `IO_OPEN_ERROR`**]**| 
|`OPEN`	                           |not possible |**`SRS_SASLCLIENTIO_01_110`: [**raise ERROR**]** |**`SRS_SASLCLIENTIO_01_106`: [**raise error**]**          |**SRS_SASLCLIENTIO_01_102: [**raise ERROR**]**                            |
|`ERROR`                           |not possible |**`SRS_SASLCLIENTIO_01_111`: [**do nothing**]**  |**`SRS_SASLCLIENTIO_01_107`: [**do nothing**]**           |**SRS_SASLCLIENTIO_01_103: [**do nothing**]**                             |

Starting the header exchange is done as follows:

**SRS_SASLCLIENTIO_01_078: [**SASL client IO shall start the header exchange by sending the SASL header.**]** 

**SRS_SASLCLIENTIO_01_095: [**Sending the header shall be done by using `xio_send`.**]** 

**SRS_SASLCLIENTIO_01_077: [**If sending the SASL header fails, the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.**]** 

**SRS_SASLCLIENTIO_01_116: [**If the underlying IO indicates another open while the after the header exchange has been started an error shall be indicated by calling `on_io_error`.**]** 

### on_sasl_frame_received_callback

```C
void on_sasl_frame_received_callback(void* context, AMQP_VALUE sasl_frame);
```

**SRS_SASLCLIENTIO_01_117: [**If `on_sasl_frame_received_callback` is called when the state of the IO is OPEN then the `on_io_error` callback shall be triggered.**]** 

**SRS_SASLCLIENTIO_01_118: [**If `on_sasl_frame_received_callback` is called in the OPENING state but the header exchange has not yet been completed, then the `on_io_error` callback shall be triggered.**]** 
When a frame is indicated as received by sasl_frame_codec it shall be processed as described in the ISO.

**SRS_SASLCLIENTIO_01_070: [**When a frame needs to be sent as part of the SASL handshake frame exchange, the send shall be done by calling `sasl_frame_codec_encode_frame`.**]** 

**SRS_SASLCLIENTIO_01_071: [**If `sasl_frame_codec_encode_frame` fails, then the `on_io_error` callback shall be triggered.**]** 

**SRS_SASLCLIENTIO_01_072: [**When the SASL handshake is complete, if the handshake is successful, the SASL client IO state shall be switched to `IO_STATE_OPEN` and the `on_io_open_complete` callback shall be called with `IO_OPEN_OK`.**]** 

**SRS_SASLCLIENTIO_01_119: [**If any error is encountered when parsing the received frame, the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.**]** 

**SRS_SASLCLIENTIO_01_073: [**If the handshake fails (i.e. the outcome is an error) the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.**]**

### on_bytes_encoded

```C
void on_bytes_encoded(void* context, const unsigned char* bytes, size_t length, bool encode_complete);
```

**SRS_SASLCLIENTIO_01_120: [**When SASL client IO is notified by `sasl_frame_codec` of bytes that have been encoded via the `on_bytes_encoded` callback and SASL client IO is in the state OPENING, SASL client IO shall send these bytes by using `xio_send`.**]** 

**SRS_SASLCLIENTIO_01_121: [**If `xio_send` fails, the `on_io_error` callback shall be triggered.**]** 

### on_frame_codec_error

```C
void on_frame_codec_error(void* context);
```

**SRS_SASLCLIENTIO_01_122: [**When `on_frame_codec_error` is called while in the OPENING state the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`.**]**

**SRS_SASLCLIENTIO_01_143: [** When `on_frame_codec_error` is called while in the OPEN state the `on_io_error` callback shall be triggered. **]** 

**SRS_SASLCLIENTIO_01_123: [**When `on_frame_codec_error` is called in the ERROR state nothing shall be done.**]** 

### on_sasl_frame_codec_error

```C
void on_sasl_frame_codec_error(void* context);
```

**SRS_SASLCLIENTIO_01_141: [** When `on_sasl_frame_codec_error` is called while in the OPENING state the `on_io_open_complete` callback shall be triggered with `IO_OPEN_ERROR`. **]**

**SRS_SASLCLIENTIO_01_144: [** When `on_sasl_frame_codec_error` is called while OPEN state the `on_io_error` callback shall be triggered. **]** 

**SRS_SASLCLIENTIO_01_142: [** When `on_sasl_frame_codec_error` is called in the ERROR state nothing shall be done. **]** 

## SASL negotiation

**SRS_SASLCLIENTIO_01_067: [**The SASL frame exchange shall be started as soon as the SASL header handshake is done.**]** 

**SRS_SASLCLIENTIO_01_068: [**During the SASL frame exchange that constitutes the handshake the received bytes from the underlying IO shall be fed to the frame codec instance created in `saslclientio_create` by calling `frame_codec_receive_bytes`.**]** 

**SRS_SASLCLIENTIO_01_088: [**If `frame_codec_receive_bytes` fails, the `on_io_error` callback shall be triggered.**]** 

## ISO section

**SRS_SASLCLIENTIO_01_001: [**To establish a SASL layer, each peer MUST start by sending a protocol header.**]** 
**SRS_SASLCLIENTIO_01_002: [**The protocol header consists of the upper case ASCII letters "AMQP" followed by a protocol id of three, followed by three unsigned bytes representing the major, minor, and revision of the specification version (currently 1 (SASL-MAJOR), 0 (SASLMINOR), 0 (SASL-REVISION)).**]** In total this is an 8-octet sequence:

...

Figure 5.3: Protocol Header for SASL Security Layer

**SRS_SASLCLIENTIO_01_003: [**Other than using a protocol id of three, the exchange of SASL layer headers follows the same rules specified in the version negotiation section of the transport specification (See Part 2: section 2.2).**]** 
The following diagram illustrates the interaction involved in creating a SASL security layer:

...

Figure 5.4: Establishing a SASL Security Layer

SASL Negotiation

**SRS_SASLCLIENTIO_01_032: [**The peer acting as the SASL server MUST announce supported authentication mechanisms using the sasl-mechanisms frame.**]** 
**SRS_SASLCLIENTIO_01_033: [**The partner MUST then choose one of the supported mechanisms and initiate a sasl exchange.**]** 

SASL Client SASL Server

**SRS_SASLCLIENTIO_01_034: [**<-- SASL-MECHANISMS**]** 
**SRS_SASLCLIENTIO_01_035: [**SASL-INIT -->**]** 
...
**SRS_SASLCLIENTIO_01_036: [**<-- SASL-CHALLENGE ***]** 
**SRS_SASLCLIENTIO_01_037: [**SASL-RESPONSE -->**]** 
...
**SRS_SASLCLIENTIO_01_038: [**<-- SASL-OUTCOME**]** 

* Note that **SRS_SASLCLIENTIO_01_039: [**the SASL challenge/response step can occur zero or more times depending on the details of the SASL mechanism chosen.**]** 

Figure 5.6: SASL Exchange

**SRS_SASLCLIENTIO_01_040: [**The peer playing the role of the SASL client and the peer playing the role of the SASL server MUST correspond to the TCP client and server respectively.**]** 

5.3.3 Security Frame Bodies

5.3.3.1 SASL Mechanisms

Advertise available sasl mechanisms.

\<type name="sasl-mechanisms" class="composite" source="list" provides="sasl-frame">
\<descriptor name="amqp:sasl-mechanisms:list" code="0x00000000:0x00000040"/>
\<field name="sasl-server-mechanisms" type="symbol" multiple="true" mandatory="true"/>
\</type>

Advertises the available SASL mechanisms that can be used for authentication.

Field Details

sasl-server-mechanisms supported sasl mechanisms

**SRS_SASLCLIENTIO_01_041: [**A list of the sasl security mechanisms supported by the sending peer.**]** **SRS_SASLCLIENTIO_01_042: [**It is invalid for this list to be null or empty.**]** If the sending peer does not require its partner to authenticate with it, then it SHOULD send a list of one element with its value as the SASL mechanism ANONYMOUS. **SRS_SASLCLIENTIO_01_043: [**The server mechanisms are ordered in decreasing level of preference.**]** 

5.3.3.2 SASL Init

Initiate sasl exchange.

\<type name="sasl-init" class="composite" source="list" provides="sasl-frame">

\<descriptor name="amqp:sasl-init:list" code="0x00000000:0x00000041"/>

\<field name="mechanism" type="symbol" mandatory="true"/>

\<field name="initial-response" type="binary"/>

\<field name="hostname" type="string"/>

\</type>

**SRS_SASLCLIENTIO_01_054: [**Selects the sasl mechanism and provides the initial response if needed.**]** 

Field Details

mechanism selected security mechanism

**SRS_SASLCLIENTIO_01_045: [**The name of the SASL mechanism used for the SASL exchange.**]** If the selected mechanism is not supported by the receiving peer, it MUST close the connection with the authentication-failure close-code. **SRS_SASLCLIENTIO_01_046: [**Each peer MUST authenticate using the highest-level security profile it can handle from the list provided by the partner.**]** 
initial-response security response data
**SRS_SASLCLIENTIO_01_047: [**A block of opaque data passed to the security mechanism.**]** **SRS_SASLCLIENTIO_01_048: [**The contents of this data are defined by the SASL security mechanism.**]** 
hostname the name of the target host
**SRS_SASLCLIENTIO_01_049: [**The DNS name of the host (either fully qualified or relative) to which the sending peer is connecting.**]** 
**SRS_SASLCLIENTIO_01_050: [**It is not mandatory to provide the hostname.**]** If no hostname is provided the receiving peer SHOULD select a default based on its own configuration.
This field can be used by AMQP proxies to determine the correct back-end service to connect the client to, and to determine the domain to validate the client's credentials against.
**SRS_SASLCLIENTIO_01_051: [**This field might already have been specified by the server name indication extension as described in RFC-4366 [RFC4366**]**, if a TLS layer is used, in which case this field SHOULD either be null or contain the same value.] It is undefined what a different value to those already specified means.

5.3.3.3 SASL Challenge

Security mechanism challenge.

\<type name="sasl-challenge" class="composite" source="list" provides="sasl-frame">

\<descriptor name="amqp:sasl-challenge:list" code="0x00000000:0x00000042"/>

\<field name="challenge" type="binary" mandatory="true"/>

\</type>

**SRS_SASLCLIENTIO_01_052: [**Send the SASL challenge data as defined by the SASL specification.**]** 

Field Details

challenge security challenge data

**SRS_SASLCLIENTIO_01_053: [**Challenge information, a block of opaque binary data passed to the security mechanism.**]** 

5.3.3.4 SASL Response

Security mechanism response.

\<type name="sasl-response" class="composite" source="list" provides="sasl-frame">

\<descriptor name="amqp:sasl-response:list" code="0x00000000:0x00000043"/>

\<field name="response" type="binary" mandatory="true"/>

\</type>

**SRS_SASLCLIENTIO_01_055: [**Send the SASL response data as defined by the SASL specification.**]** 

Field Details

response security response data

**SRS_SASLCLIENTIO_01_056: [**A block of opaque data passed to the security mechanism.**]** **SRS_SASLCLIENTIO_01_057: [**The contents of this data are defined by the SASL security mechanism.**]** 

5.3.3.5 SASL Outcome

Indicates the outcome of the sasl dialog.

\<type name="sasl-outcome" class="composite" source="list" provides="sasl-frame">

\<descriptor name="amqp:sasl-outcome:list" code="0x00000000:0x00000044"/>

\<field name="code" type="sasl-code" mandatory="true"/>

\<field name="additional-data" type="binary"/>

\</type>

**SRS_SASLCLIENTIO_01_058: [**This frame indicates the outcome of the SASL dialog.**]****SRS_SASLCLIENTIO_01_059: [**Upon successful completion of the SASL dialog the security layer has been established**]**, and the peers MUST exchange protocol headers to either start a nested security layer, or to establish the AMQP connection.

Field Details

code indicates the outcome of the sasl dialog

**SRS_SASLCLIENTIO_01_060: [**A reply-code indicating the outcome of the SASL dialog.**]** 

additional-data additional data as specified in RFC-4422

**SRS_SASLCLIENTIO_01_061: [**The additional-data field carries additional data on successful authentication outcome as specified by the SASL specification [RFC4422**]**.] If the authentication is unsuccessful, this field is not set.

5.3.3.6 SASL Code

Codes to indicate the outcome of the sasl dialog.

\<type name="sasl-code" class="restricted" source="ubyte">

\<choice name="ok" value="0"/>

\<choice name="auth" value="1"/>

\<choice name="sys" value="2"/>

\<choice name="sys-perm" value="3"/>

\<choice name="sys-temp" value="4"/>

\</type>

Valid Values

**SRS_SASLCLIENTIO_01_062: [**0 Connection authentication succeeded.**]** 
**SRS_SASLCLIENTIO_01_063: [**1 Connection authentication failed due to an unspecified problem with the supplied credentials.**]** 
**SRS_SASLCLIENTIO_01_064: [**2 Connection authentication failed due to a system error.**]** 
**SRS_SASLCLIENTIO_01_065: [**3 Connection authentication failed due to a system error that is unlikely to be corrected without intervention.**]** 
**SRS_SASLCLIENTIO_01_066: [**4 Connection authentication failed due to a transient system error.**]** 

5.3.4 Constant Definitions

**SRS_SASLCLIENTIO_01_124: [**SASL-MAJOR 1 major protocol version.**]** 
**SRS_SASLCLIENTIO_01_125: [**SASL-MINOR 0 minor protocol version.**]** 
**SRS_SASLCLIENTIO_01_126: [**SASL-REVISION 0 protocol revision.**]** 
