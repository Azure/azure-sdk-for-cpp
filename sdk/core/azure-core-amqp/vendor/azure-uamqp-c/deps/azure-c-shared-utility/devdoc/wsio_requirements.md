# wsio requirements

## Overview

`wsio` is module that implements a concrete IO that implements the WebSockets protocol by using the uws library.

## References

RFC6455 - The WebSocket Protocol.

## Exposed API

```c
typedef struct WSIO_CONFIG_TAG
{
    const char* hostname;
    int port;
    const char* resource_name;
    const char* protocol;
    bool use_ssl;
} WSIO_CONFIG;

extern const IO_INTERFACE_DESCRIPTION* wsio_get_interface_description(void);
```

### wsio_create

```c
CONCRETE_IO_HANDLE wsio_create(void* io_create_parameters);
```

`wsio_create` is the implementation provided via `wsio_get_interface_description` for the `concrete_io_create` member.

**SRS_WSIO_01_001: [** `wsio_create` shall create an instance of wsio and return a non-NULL handle to it. **]**

**SRS_WSIO_01_065: [** If the argument `io_create_parameters` is NULL then `wsio_create` shall return NULL. **]**

**SRS_WSIO_01_066: [** `io_create_parameters` shall be used as a `WSIO_CONFIG*` . **]**

**SRS_WSIO_01_067: [** If any of the members `hostname`, `resource_name` or `protocol` is NULL in `WSIO_CONFIG` then `wsio_create` shall return NULL. **]**

**SRS_WSIO_01_068: [** If allocating memory for the new wsio instance fails then `wsio_create` shall return NULL. **]**

**SRS_WSIO_01_070: [** The underlying uws instance shall be created by calling `uws_client_create_with_io`. **]**

**SRS_WSIO_01_071: [** The arguments for `uws_client_create_with_io` shall be: **]**

**SRS_WSIO_01_185: [** - `underlying_io_interface` shall be set to the `underlying_io_interface` field in the `io_create_parameters` passed to `wsio_create`. **]**

**SRS_WSIO_01_186: [** - `underlying_io_parameters` shall be set to the `underlying_io_parameters` field in the `io_create_parameters` passed to `wsio_create`. **]**

**SRS_WSIO_01_072: [** - `hostname` set to the `hostname` field in the `io_create_parameters` passed to `wsio_create`. **]**

**SRS_WSIO_01_130: [** - `port` set to the `port` field in the `io_create_parameters` passed to `wsio_create`. **]**

**SRS_WSIO_01_128: [** - `resource_name` set to the `resource_name` field in the `io_create_parameters` passed to `wsio_create`. **]**

**SRS_WSIO_01_129: [** - `protocols` shall be filled with only one structure, that shall have the `protocol` set to the value of the `protocol` field in the `io_create_parameters` passed to `wsio_create`. **]**

**SRS_WSIO_01_075: [** If `uws_client_create_with_io` fails, then `wsio_create` shall fail and return NULL. **]**

**SRS_WSIO_01_076: [** `wsio_create` shall create a pending send IO list that is to be used to queue send packets by calling `singlylinkedlist_create`. **]**

**SRS_WSIO_01_077: [** If `singlylinkedlist_create` fails then `wsio_create` shall fail and return NULL. **]**

### wsio_destroy

```c
void wsio_destroy(CONCRETE_IO_HANDLE ws_io);
```

`wsio_destroy` is the implementation provided via `wsio_get_interface_description` for the `concrete_io_destroy` member.

**SRS_WSIO_01_078: [** `wsio_destroy` shall free all resources associated with the wsio instance. **]**

**SRS_WSIO_01_079: [** If `ws_io` is NULL, `wsio_destroy` shall do nothing.  **]**

**SRS_WSIO_01_080: [** `wsio_destroy` shall destroy the uws instance created in `wsio_create` by calling `uws_client_destroy`. **]**

**SRS_WSIO_01_081: [** `wsio_destroy` shall free the list used to track the pending send IOs by calling `singlylinkedlist_destroy`. **]**

### wsio_open

```c
int wsio_open(CONCRETE_IO_HANDLE ws_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context);
```

`wsio_open` is the implementation provided via `wsio_get_interface_description` for the `concrete_io_open` member.

**SRS_WSIO_01_082: [** `wsio_open` shall open the underlying uws instance by calling `uws_client_open_async` and providing the uws handle created in `wsio_create` as argument. **]**

**SRS_WSIO_01_083: [** On success, `wsio_open` shall return 0. **]**

**SRS_WSIO_01_132: [** If any of the arguments `ws_io`, `on_io_open_complete`, `on_bytes_received`, `on_io_error` is NULL, `wsio_open` shall fail and return a non-zero value. **]**

**SRS_WSIO_01_165: [** `wsio_open` when CLOSING shall fail and return a non-zero value. **]**

**SRS_WSIO_01_084: [** If opening the underlying uws instance fails then `wsio_open` shall fail and return a non-zero value. **]**

**SRS_WSIO_01_131: [** `wsio_open` when already OPEN or OPENING shall fail and return a non-zero value. **]**

### wsio_close

```c
int wsio_close(CONCRETE_IO_HANDLE ws_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
```

`wsio_close` is the implementation provided via `wsio_get_interface_description` for the `concrete_io_close` member.

**SRS_WSIO_01_085: [** `wsio_close` shall close the websockets IO if an open action is either pending or has completed successfully (if the IO is open).  **]**

**SRS_WSIO_01_133: [** On success `wsio_close` shall return 0. **]**

**SRS_WSIO_01_086: [** if `ws_io` is NULL, `wsio_close` shall return a non-zero value.  **]**

**SRS_WSIO_01_087: [** `wsio_close` shall call `uws_client_close_async` while passing as argument the IO handle created in `wsio_create`.  **]**

**SRS_WSIO_01_164: [** When `uws_client_close_async` fails, `wsio_close` shall call the `on_io_close_complete` callback and continue. **]**

**SRS_WSIO_01_088: [** `wsio_close` when no open action has been issued shall fail and return a non-zero value. **]**

**SRS_WSIO_01_089: [** `wsio_close` after a `wsio_close` shall fail and return a non-zero value.  **]**

**SRS_WSIO_01_090: [** The argument `on_io_close_complete` shall be optional, if NULL is passed by the caller then no close complete callback shall be triggered.  **]**

**SRS_WSIO_01_091: [** `wsio_close` shall obtain all the pending IO items by repetitively querying for the head of the pending IO list and freeing that head item. **]**

**SRS_WSIO_01_092: [** Obtaining the head of the pending IO list shall be done by calling `singlylinkedlist_get_head_item`. **]**

**SRS_WSIO_01_093: [** For each pending item the send complete callback shall be called with `IO_SEND_CANCELLED`.**\]**

###  wsio_send

```c
int wsio_send(CONCRETE_IO_HANDLE ws_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
```

`wsio_send` is the implementation provided via `wsio_get_interface_description` for the `concrete_io_send` member.

**SRS_WSIO_01_095: [** `wsio_send` shall call `uws_client_send_frame_async`, passing the `buffer` and `size` arguments as they are: **]**

**SRS_WSIO_01_098: [** On success, `wsio_send` shall return 0. **]**

**SRS_WSIO_01_100: [** If any of the arguments `ws_io` or `buffer` are NULL, `wsio_send` shall fail and return a non-zero value. **]**

**SRS_WSIO_01_099: [** If the wsio is not OPEN (open has not been called or is still in progress) then `wsio_send` shall fail and return a non-zero value. **]**

**SRS_WSIO_01_102: [** An entry shall be queued in the singly linked list by calling `singlylinkedlist_add`. **]**

**SRS_WSIO_01_103: [** The entry shall contain the `on_send_complete` callback and its context. **]**

**SRS_WSIO_01_096: [** The frame type used shall be `WS_FRAME_TYPE_BINARY`. **]**

**SRS_WSIO_01_097: [** The `is_final` argument shall be set to true. **]**

**SRS_WSIO_01_101: [** If `size` is zero then `wsio_send` shall fail and return a non-zero value. **]**

**SRS_WSIO_01_134: [** If allocating memory for the pending IO data fails, `wsio_send` shall fail and return a non-zero value. **]**

**SRS_WSIO_01_104: [** If `singlylinkedlist_add` fails, `wsio_send` shall fail and return a non-zero value. **]**

**SRS_WSIO_01_105: [** The argument `on_send_complete` shall be optional, if NULL is passed by the caller then no send complete callback shall be triggered. **]**

###  wsio_dowork

```c
void wsio_dowork(CONCRETE_IO_HANDLE ws_io);
```

`wsio_dowork` is the implementation provided via `wsio_get_interface_description` for the `concrete_io_dowork` member.

**SRS_WSIO_01_106: [** `wsio_dowork` shall call `uws_client_dowork` with the uws handle created in `wsio_create`. **]**

**SRS_WSIO_01_107: [** If the `ws_io` argument is NULL, `wsio_dowork` shall do nothing. **]**

**SRS_WSIO_01_108: [** If the IO is not yet open, `wsio_dowork` shall do nothing. **]**

###  wsio_setoption

```c
int wsio_setoption(CONCRETE_IO_HANDLE ws_io, const char* option_name, const void* value);
```

`wsio_setoption` is the implementation provided via `wsio_get_interface_description` for the `concrete_io_setoption` member.

**SRS_WSIO_01_109: [** If any of the arguments `ws_io` or `option_name` is NULL `wsio_setoption` shall return a non-zero value. **]**

**SRS_WSIO_01_183: [** If the option name is `WSIOOptions` then `wsio_setoption` shall call `OptionHandler_FeedOptions` and pass to it the underlying IO handle and the `value` argument. **]**

**SRS_WSIO_01_184: [** If `OptionHandler_FeedOptions` fails, `wsio_setoption` shall fail and return a non-zero value. **]**

**SRS_WSIO_01_156: [** Otherwise all options shall be passed as they are to uws by calling `uws_client_set_option`. **]**

**SRS_WSIO_01_158: [** On success, `wsio_setoption` shall return 0. **]**

**SRS_WSIO_01_157: [** If `uws_client_set_option` fails, `wsio_setoption` shall fail and return a non-zero value. **]**

###  wsio_retrieveoptions

```c
OPTIONHANDLER_HANDLE wsio_retrieveoptions(CONCRETE_IO_HANDLE handle)
```

`wsio_retrieveoptions` is the implementation provided via `wsio_get_interface_description` for the `concrete_io_retrieveoptions` member.

**SRS_WSIO_01_118: [** If parameter `handle` is `NULL` then `wsio_retrieveoptions` shall fail and return NULL. **]**

**SRS_WSIO_01_119: [** `wsio_retrieveoptions` shall call `OptionHandler_Create` to produce an `OPTIONHANDLER_HANDLE` and on success return the new `OPTIONHANDLER_HANDLE` handle. **]**

**SRS_WSIO_01_120: [** If `OptionHandler_Create` fails then `wsio_retrieveoptions` shall fail and return NULL.  **]**

**SRS_WSIO_01_178: [** `uws_client_retrieve_options` shall add to the option handler one option, whose name shall be `uWSCLientOptions` and the value shall be queried by calling `uws_client_retrieve_options`. **]**

**SRS_WSIO_01_179: [** When calling `uws_client_retrieve_options` the uws client handle shall be passed to it. **]**

**SRS_WSIO_01_180: [** If `uws_client_retrieve_options` fails, `uws_client_retrieve_options` shall fail and return NULL. **]**

**SRS_WSIO_01_181: [** Adding the option shall be done by calling `OptionHandler_AddOption`. **]**

**SRS_WSIO_01_182: [** If `OptionHandler_AddOption` fails, `uws_client_retrieve_options` shall fail and return NULL. **]**

###  wsio_clone_option

`wsio_clone_option` is the implementation provided to the option handler instance created as part of `wsio_retrieve_options`.

```c
void* wsio_clone_option(const char* name, const void* value)
```

**SRS_WSIO_01_171: [** `wsio_clone_option` called with `name` being `WSIOOptions` shall return the same value. **]**

**SRS_WSIO_01_173: [** `wsio_clone_option` called with any other option name than `WSIOOptions` shall return NULL. **]**

**SRS_WSIO_01_174: [** If `wsio_clone_option` is called with NULL `name` or `value` it shall return NULL. **]**

###  wsio_destroy_option

`wsio_destroy_option` is the implementation provided to the option handler instance created as part of `wsio_retrieve_options`.

```c
void wsio_destroy_option(const char* name, const void* value)
```

**SRS_WSIO_01_175: [** `wsio_destroy_option` called with the option `name` being `WSIOOptions` shall destroy the value by calling `OptionHandler_Destroy`. **]**

**SRS_WSIO_01_176: [** If `wsio_destroy_option` is called with any other `name` it shall do nothing. **]**

**SRS_WSIO_01_177: [** If `wsio_destroy_option` is called with NULL `name` or `value` it shall do nothing. **]**

###  wsio_get_interface_description

```c
const IO_INTERFACE_DESCRIPTION* wsio_get_interface_description(void);
```

**SRS_WSIO_01_064: [** wsio_get_interface_description shall return a pointer to an IO_INTERFACE_DESCRIPTION structure that contains pointers to the functions: wsio_retrieveoptions, wsio_create, wsio_destroy, wsio_open, wsio_close, wsio_send and wsio_dowork. **]** 

###  on_underlying_ws_error

**SRS_WSIO_01_121: [** When `on_underlying_ws_error` is called while the IO is OPEN the wsio instance shall be set to ERROR and an error shall be indicated via the `on_io_error` callback passed to `wsio_open`. **]**

**SRS_WSIO_01_123: [** When calling `on_io_error`, the `on_io_error_context` argument given in `wsio_open` shall be passed to the callback `on_io_error`. **]**

**SRS_WSIO_01_122: [** When `on_underlying_ws_error` is called while the IO is OPENING, the `on_io_open_complete` callback passed to `wsio_open` shall be called with `IO_OPEN_ERROR`. **]**

**SRS_WSIO_01_135: [** When `on_underlying_ws_error` is called with a NULL context, it shall do nothing. **]**

###  on_underlying_ws_frame_received

**SRS_WSIO_01_124: [** When `on_underlying_ws_frame_received` is called the bytes in the frame shall be indicated by calling the `on_bytes_received` callback passed to `wsio_open`. **]**

**SRS_WSIO_01_125: [** When calling `on_bytes_received`, the `on_bytes_received_context` argument given in `wsio_open` shall be passed to the callback `on_bytes_received`. **]**

**SRS_WSIO_01_126: [** If `on_underlying_ws_frame_received` is called while the IO is in any state other than OPEN, it shall do nothing. **]**

**SRS_WSIO_01_150: [** If `on_underlying_ws_frame_received` is called with NULL context it shall do nothing. **]**

**SRS_WSIO_01_151: [** If the WebSocket frame type is not binary then an error shall be indicated by calling the `on_io_error` callback passed to `wsio_open`. **]**

**SRS_WSIO_01_153: [** When `on_underlying_ws_frame_received` is called with zero `size`, no bytes shall be indicated up as received. **]**

**SRS_WSIO_01_154: [** When `on_underlying_ws_frame_received` is called with a positive `size` and a NULL `buffer`, an error shall be indicated by calling the `on_io_error` callback passed to `wsio_open`. **]**

**SRS_WSIO_01_152: [** When calling `on_io_error`, the `on_io_error_context` argument given in `wsio_open` shall be passed to the callback `on_io_error`. **]**

###  on_underlying_ws_open_complete

**SRS_WSIO_01_136: [** When `on_underlying_ws_open_complete` is called with `WS_OPEN_OK` while the IO is opening, the callback `on_io_open_complete` shall be called with `IO_OPEN_OK`. **]**

**SRS_WSIO_01_149: [** When `on_underlying_ws_open_complete` is called with `WS_OPEN_CANCELLED` while the IO is opening, the callback `on_io_open_complete` shall be called with `IO_OPEN_CANCELLED`. **]**

**SRS_WSIO_01_137: [** When `on_underlying_ws_open_complete` is called with any other error code while the IO is opening, the callback `on_io_open_complete` shall be called with `IO_OPEN_ERROR`. **]**

**SRS_WSIO_01_138: [** When `on_underlying_ws_open_complete` is called with a NULL context, it shall do nothing. **]**

**SRS_WSIO_01_139: [** When `on_underlying_ws_open_complete` is called while in OPEN state it shall indicate an error by calling the `on_io_error` callback passed to `wsio_open` and switch to the ERROR state. **]**

**SRS_WSIO_01_141: [** When `on_underlying_ws_open_complete` is called while in the ERROR state it shall indicate an error by calling the `on_io_error` callback passed to `wsio_open`. **]**

**SRS_WSIO_01_140: [** When calling `on_io_error`, the `on_io_error_context` argument given in `wsio_open` shall be passed to the callback `on_io_error`. **]**

**SRS_WSIO_01_142: [** When `on_underlying_ws_open_complete` is called while in the CLOSING state an error shall be indicated by calling the `on_io_error` callback passed to `wsio_open`. **]**

###  on_underlying_ws_send_frame_complete

**SRS_WSIO_01_143: [** When `on_underlying_ws_send_frame_complete` is called after sending a WebSocket frame, the pending IO shall be removed from the list. **]**

**SRS_WSIO_01_145: [** Removing it from the list shall be done by calling `singlylinkedlist_remove`. **]**

**SRS_WSIO_01_144: [** Also the pending IO data shall be freed. **]**

**SRS_WSIO_01_146: [** When `on_underlying_ws_send_frame_complete` is called with `WS_SEND_OK`, the callback `on_send_complete` shall be called with `IO_SEND_OK`. **]**

**SRS_WSIO_01_147: [** When `on_underlying_ws_send_frame_complete` is called with `WS_SEND_CANCELLED`, the callback `on_send_complete` shall be called with `IO_SEND_CANCELLED`. **]**

**SRS_WSIO_01_148: [** When `on_underlying_ws_send_frame_complete` is called with any other error code, the callback `on_send_complete` shall be called with `IO_SEND_ERROR`. **]**

**SRS_WSIO_01_155: [** When `on_underlying_ws_send_frame_complete` is called with a NULL context it shall do nothing. **]**

###  on_underlying_ws_close_complete

**SRS_WSIO_01_159: [** When `on_underlying_ws_close_complete` while the IO is closing (after `wsio_close`), the close shall be indicated up by calling the `on_io_close_complete` callback passed to `wsio_close`. **]**

**SRS_WSIO_01_163: [** When `on_io_close_complete` is called, the context passed to `wsio_close` shall be passed as argument to `on_io_close_complete`. **]**

**SRS_WSIO_01_160: [** If NULL was passed to `wsio_close` no callback shall be called. **]**

**SRS_WSIO_01_161: [** If the context passed to `on_underlying_ws_close_complete` is NULL, `on_underlying_ws_close_complete` shall do nothing. **]**

###  on_underlying_ws_peer_closed

**SRS_WSIO_01_166: [** When `on_underlying_ws_peer_closed` and the state of the IO is OPEN an error shall be indicated by calling the `on_io_error` callback passed to `wsio_open`. **]**

**SRS_WSIO_01_169: [** When `on_underlying_ws_peer_closed` and the state of the IO is CLOSING an error shall be indicated by calling the `on_io_error` callback passed to `wsio_open`. **]**

**SRS_WSIO_01_170: [** When `on_underlying_ws_peer_closed` and the state of the IO is OPENING an error shall be indicated by calling the `on_io_open_complete` callback passed to `wsio_open` with the error code `IO_OPEN_ERROR`. **]**

**SRS_WSIO_01_168: [** The `close_code`, `extra_data` and `extra_data_length` arguments shall be ignored. **]**

**SRS_WSIO_01_167: [** If `on_underlying_ws_peer_closed` is called with a NULL context it shall do nothing. **]**

**SRS_WSIO_07_001: [** When `on_underlying_ws_peer_closed` and the state of the IO is NOT_OPEN an error will be raised and the io_state will remain as NOT_OPEN **]**
