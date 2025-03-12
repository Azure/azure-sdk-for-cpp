# header_detect_io requirements
 
## Overview

`header_detect_io` is module that detects which AMQP header is being received and creates an appropriate IO for the detected header.

## Exposed API

```C

	typedef struct AMQP_HEADER_TAG
	{
		const unsigned char* header_bytes;
		size_t header_size;
	} AMQP_HEADER;

	typedef struct HEADER_DETECT_ENTRY_TAG
	{
		AMQP_HEADER header;
        const IO_INTERFACE_DESCRIPTION* io_interface_description;
	} HEADER_DETECT_ENTRY;

	typedef struct HEADER_DETECT_IO_CONFIG_TAG
	{
		XIO_HANDLE underlying_io;
		HEADER_DETECT_ENTRY* header_detect_entries;
		size_t header_detect_entry_count;
	} HEADER_DETECT_IO_CONFIG;
    
	MOCKABLE_FUNCTION(, const AMQP_HEADER, header_detect_io_get_amqp_header);
	MOCKABLE_FUNCTION(, const AMQP_HEADER, header_detect_io_get_sasl_amqp_header);
	MOCKABLE_FUNCTION(, const IO_INTERFACE_DESCRIPTION*, header_detect_io_get_interface_description);
```

### header_detect_io_create

```C
CONCRETE_IO_HANDLE header_detect_io_create(void* io_create_parameters);
```

`header_detect_io_create` is the implementation provided via `header_detect_io_get_interface_description` for the `concrete_io_create` member.

**SRS_HEADER_DETECT_IO_01_001: [** `header_detect_io_create` shall create a new header detect IO instance and on success it shall return a non-NULL handle to the newly created instance. **]**
**SRS_HEADER_DETECT_IO_01_002: [** If allocating memory for the header detect IO instance fails, `header_detect_io_create` shall fail and return NULL. **]**
**SRS_HEADER_DETECT_IO_01_003: [** If `io_create_parameters` is NULL, `header_detect_io_create` shall fail and return NULL. **]**
**SRS_HEADER_DETECT_IO_01_004: [** `io_create_parameters` shall be used as `HEADER_DETECT_IO_CONFIG*`. **]**
**SRS_HEADER_DETECT_IO_01_005: [** If the member `header_detect_entry_count` of `HEADER_DETECT_IO_CONFIG` is 0 then `header_detect_io_create` shall fail and return NULL. **]**
**SRS_HEADER_DETECT_IO_01_006: [** If the member `header_detect_entries` is NULL then `header_detect_io_create` shall fail and return NULL. **]**
**SRS_HEADER_DETECT_IO_01_007: [** If the member `underlying_io` is NULL then `header_detect_io_create` shall fail and return NULL. **]**
**SRS_HEADER_DETECT_IO_01_008: [** If the `header` member in the `header_detect_entries` is NULL then `header_detect_io_create` shall fail and return NULL. **]**
**SRS_HEADER_DETECT_IO_01_052: [** The `io` member in the in each of the `header_detect_entries` shall be allowed to be NULL. **]**
**SRS_HEADER_DETECT_IO_01_054: [** At least one entry in `header_detect_entries` shall have IO set to NULL, otherwise `header_detect_io_create` shall fail and return NULL. **]**
**SRS_HEADER_DETECT_IO_01_009: [** The `header_detect_entries` array shall be copied so that it can be later used when detecting which header was received. **]**
**SRS_HEADER_DETECT_IO_01_014: [** For each entry in `header_detect_entries` the `header` field shall also be copied. **]**
**SRS_HEADER_DETECT_IO_01_010: [** If allocating memory for the `header_detect_entries` or its constituents fails then `header_detect_io_create` shall fail and return NULL. **]**
**SRS_HEADER_DETECT_IO_01_060: [** `header_detect_io_create` shall create a singly linked list by calling `singlylinkedlist_create` where the chained detected IOs shall be stored. **]**
**SRS_HEADER_DETECT_IO_01_065: [** If `singlylinkedlist_create` fails then `header_detect_io_create` shall fail and return NULL. **]**

### header_detect_io_destroy

```C
void header_detect_io_destroy(CONCRETE_IO_HANDLE header_detect_io)
```

`header_detect_io_destroy` is the implementation provided via `header_detect_io_get_interface_description` for the `concrete_io_destroy` member.

**SRS_HEADER_DETECT_IO_01_011: [** `header_detect_io_destroy` shall free all resources associated with the `header_detect_io` handle. **]**
**SRS_HEADER_DETECT_IO_01_013: [** `header_detect_io_destroy` shall free the memory allocated for the `header_detect_entries`. **]**
**SRS_HEADER_DETECT_IO_01_012: [** If `header_detect_io` is NULL, `header_detect_io_destroy` shall do nothing. **]**
**SRS_HEADER_DETECT_IO_01_061: [** `header_detect_io_destroy` shall destroy the chained IO list by calling `singlylinkedlist_destroy`. **]**
**SRS_HEADER_DETECT_IO_01_062: [** If the IO is still open when `header_detect_io_destroy` is called, all actions normally executed when closing the IO shall also be executed. **]**

### header_detect_io_open_async

```C
int header_detect_io_open_async(CONCRETE_IO_HANDLE header_detect_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
```

`header_detect_io_open_async` is the implementation provided via `header_detect_io_get_interface_description` for the `concrete_io_open` member.

**SRS_HEADER_DETECT_IO_01_015: [** `header_detect_io_open_async` shall open the underlying IO by calling `xio_open` and passing to it: **]**
**SRS_HEADER_DETECT_IO_01_016: [** - `xio` shall be the `underlying_io` member of the `io_create_parameters` passed to `header_detect_io_create`. **]**
**SRS_HEADER_DETECT_IO_01_017: [** - `on_io_open_complete`, `on_io_open_complete_context`, `on_bytes_received`, `on_bytes_received_context`, `on_error` and `on_error_context` shall be set to implementation specific values of `header_detect_io`. **]**
**SRS_HEADER_DETECT_IO_01_018: [** On success `header_detect_io_open_async` shall return 0. **]**
**SRS_HEADER_DETECT_IO_01_021: [** If `header_detect_io`, `on_io_open_complete`, `on_bytes_received` or `on_io_error` is NULL, `header_detect_io_open_async` shall fail and return a non-zero value. **]**
**SRS_HEADER_DETECT_IO_01_019: [** If `xio_open` fails, `header_detect_io_open_async` shall fail and return a non-zero value. **]**
**SRS_HEADER_DETECT_IO_01_020: [** If the IO is already OPEN or OPENING then `header_detect_io_open_async` shall fail and return a non-zero value. **]**

### header_detect_io_close_async

```C
int header_detect_io_close_async(CONCRETE_IO_HANDLE header_detect_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context)
```

`header_detect_io_close_async` is the implementation provided via `header_detect_io_get_interface_description` for the `concrete_io_close` member.

**SRS_HEADER_DETECT_IO_01_022: [** `header_detect_io_close_async` shall close the underlying IO by calling `xio_close` and passing to it: **]**
**SRS_HEADER_DETECT_IO_01_023: [** - `xio` shall be the `underlying_io` member of the `io_create_parameters` passed to `header_detect_io_create`. **]**
**SRS_HEADER_DETECT_IO_01_024: [** - `on_io_close_complete` shall be set to implementation specific values of `header_detect_io`. **]**
**SRS_HEADER_DETECT_IO_01_025: [** On success `header_detect_io_close_async` shall return 0. **]**
**SRS_HEADER_DETECT_IO_01_092: [** If `xio_close` fails `header_detect_io_close_async` shall fail and return a non-zero value. **]**
**SRS_HEADER_DETECT_IO_01_026: [** If `header_detect_io` is NULL, `header_detect_io_close_async` shall fail and return a non-zero value. **]**
**SRS_HEADER_DETECT_IO_01_094: [** `on_io_close_complete` shall be allowed to be NULL, in which case no close complete callback shall be triggered. **]**
**SRS_HEADER_DETECT_IO_01_027: [** If the IO is not OPEN (open has not been called or close has been completely carried out) `header_detect_io_close_async` shall fail and return a non-zero value. **]**
**SRS_HEADER_DETECT_IO_01_028: [** If the IO is OPENING (`header_detect_io_open_async` has been called, but no header has been detected yet), `header_detect_io_close_async` shall close the underlying IO and call `on_io_open_complete` with `IO_OPEN_CANCELLED`. **]**
**SRS_HEADER_DETECT_IO_01_053: [** If the IO is CLOSING then `header_detect_io_close_async` shall fail and return a non-zero value. **]**
**SRS_HEADER_DETECT_IO_01_063: [** `header_detect_io_close_async` shall close the last detected IO that was created as a result of matching a header. **]**
**SRS_HEADER_DETECT_IO_01_070: [** If no detected IO was created then `header_detect_io_close_async` shall close the `underlying_io` passed in `header_detect_io_create`. **]**
**SRS_HEADER_DETECT_IO_01_064: [** `header_detect_io_close_async` shall also destroy all these chained IOs. **]**

### header_detect_io_send_async

```C
int header_detect_io_send_async(CONCRETE_IO_HANDLE header_detect_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
```

`header_detect_io_send_async` is the implementation provided via `header_detect_io_get_interface_description` for the `concrete_io_send` member.

**SRS_HEADER_DETECT_IO_01_071: [** If the header IO is open `header_detect_io_send_async` shall send the bytes to the last detected IO by calling `xio_send` that was created as result of matching a header. **]**
**SRS_HEADER_DETECT_IO_01_029: [** If no detected IO was created, `header_detect_io_send_async` shall send the bytes to the underlying IO passed via `header_detect_io_create`. **]**
**SRS_HEADER_DETECT_IO_01_030: [** The `buffer`, `size`, `on_send_complete` and `callback_context` shall be passed as is to `xio_send`. **]**
**SRS_HEADER_DETECT_IO_01_031: [** On success `header_detect_io_send_async` shall return 0. **]**
**SRS_HEADER_DETECT_IO_01_033: [** If `header_detect_io` or `buffer` is NULL, `header_detect_io_send_async` shall fail and return a non-zero value. **]**
**SRS_HEADER_DETECT_IO_01_055: [** `on_send_complete` and `callback_context` shall be allowed to be NULL. **]**
**SRS_HEADER_DETECT_IO_01_034: [** If `size` is 0, `header_detect_io_send_async` shall fail and return a non-zero value. **]**
**SRS_HEADER_DETECT_IO_01_032: [** If `xio_send` fails, `header_detect_io_send_async` shall fail and return a non-zero value. **]**
**SRS_HEADER_DETECT_IO_01_093: [** `header_detect_io_send_async` when the IO is not open shall fail and return a non-zero value. **]**

### header_detect_io_dowork

```C
void header_detect_io_dowork(CONCRETE_IO_HANDLE header_detect_io)
```

`header_detect_io_dowork` is the implementation provided via `header_detect_io_get_interface_description` for the `concrete_io_dowork` member.

**SRS_HEADER_DETECT_IO_01_035: [** `header_detect_io_dowork` shall schedule work for the underlying IO associated with `header_detect_io` by calling `xio_dowork` and passing as argument the `underlying_io` member of the `io_create_parameters` passed to `header_detect_io_create`. **]**
**SRS_HEADER_DETECT_IO_01_036: [** If `header_detect_io` is NULL, `header_detect_io_dowork` shall do nothing. **]**
**SRS_HEADER_DETECT_IO_01_037: [** No work shall be scheduled if `header_detect_io` is not OPEN or in ERROR (an error has been indicated to the user). **]**
**SRS_HEADER_DETECT_IO_01_056: [** `header_detect_io_dowork` shall call `xio_dowork` for all detected IOs created as a result of matching headers. **]**

### header_detect_io_set_option

```C
int header_detect_io_set_option(CONCRETE_IO_HANDLE header_detect_io, const char* option_name, const void* value)
```

`header_detect_io_set_option` is the implementation provided via `header_detect_io_get_interface_description` for the `concrete_io_setoption` member.

**SRS_HEADER_DETECT_IO_01_072: [** If any detected IO was created, `header_detect_io_set_option` shall pass any option to the last detected IO by calling `xio_setoption` and passing as IO handle the `underlying_io` member of the `io_create_parameters` passed to `header_detect_io_create`. **]**
**SRS_HEADER_DETECT_IO_01_042: [** If no detected IO was created `header_detect_io_set_option` shall pass any option to the underlying IO by calling `xio_setoption` and passing as IO handle the `underlying_io` member of the `io_create_parameters` passed to `header_detect_io_create`. **]**
**SRS_HEADER_DETECT_IO_01_043: [** On success, `header_detect_io_set_option` shall return 0. **]**
**SRS_HEADER_DETECT_IO_01_044: [** If `header_detect_io` or `option_name` is NULL, `header_detect_io_set_option` shall fail and return a non-zero value. **]**
**SRS_HEADER_DETECT_IO_01_045: [** If `xio_setoption` fails, `header_detect_io_set_option` shall fail and return a non-zero value. **]**

### header_detect_io_retrieve_options

```C
OPTIONHANDLER_HANDLE header_detect_io_retrieve_options(CONCRETE_IO_HANDLE header_detect_io)
```

`header_detect_io_retrieve_options` is the implementation provided via `header_detect_io_get_interface_description` for the `concrete_io_retrieveoptions` member.

**SRS_HEADER_DETECT_IO_01_038: [** `header_detect_io_retrieve_options` shall create a new `OPTIONHANDLER_HANDLE` by calling `OptionHandler_Create` and on success it shall return a non-NULL handle to the newly created option handler. **]**
**SRS_HEADER_DETECT_IO_01_039: [** No options shall be added to the newly created option handler. **]**
**SRS_HEADER_DETECT_IO_01_040: [** If `OptionHandler_Create` fails, `header_detect_io_retrieve_options` shall return NULL. **]**
**SRS_HEADER_DETECT_IO_01_041: [** If `header_detect_io` is NULL, `header_detect_io_retrieve_options` shall return NULL. **]**

### on_underlying_io_open_complete

```C
void on_underlying_io_open_complete(void* context, IO_OPEN_RESULT open_result)
```

**SRS_HEADER_DETECT_IO_01_046: [** When `on_underlying_io_open_complete` is called with `open_result` being `IO_OPEN_OK` while OPENING, the IO shall start monitoring received bytes in order to detect headers. **]**
**SRS_HEADER_DETECT_IO_01_047: [** When `on_underlying_io_open_complete` is called with `open_result` being `IO_OPEN_ERROR` while OPENING, the `on_io_open_complete` callback passed to `header_detect_io_open_async` shall be called with `IO_OPEN_ERROR`. **]**
**SRS_HEADER_DETECT_IO_01_048: [** If `context` is NULL, `on_underlying_io_open_complete` shall do nothing. **]**

### on_underlying_io_close_complete

**SRS_HEADER_DETECT_IO_01_095: [** When `on_underlying_io_open_complete` is called when the IO is closing, it shall destroy all the detected IOs that were created. **]**

### on_underlying_io_error

```C
void on_underlying_io_error(void* context)
```

**SRS_HEADER_DETECT_IO_01_058: [** If `context` is NULL, `on_underlying_io_error` shall do nothing. **]**
**SRS_HEADER_DETECT_IO_01_057: [** When `on_underlying_io_error` is called while OPENING, the IO shall indicate an error by calling `on_io_open_complete` with `IO_OPEN_ERROR` and it shall close the underlying IOs. **]**
**SRS_HEADER_DETECT_IO_01_059: [** When `on_underlying_io_error` is called while OPEN, the error should be indicated to the consumer by calling `on_io_error` and passing the `on_io_error_context` to it. **]**

### on_underlying_io_bytes_received

```C
void on_underlying_io_bytes_received(void* context, const unsigned char* buffer, size_t size)
```

**SRS_HEADER_DETECT_IO_01_050: [** If `context` is NULL, `on_underlying_io_bytes_received` shall do nothing. **]**
**SRS_HEADER_DETECT_IO_01_049: [** When `on_underlying_io_bytes_received` is called while opening the underlying IO (before the underlying open complete is received), an error shall be indicated by calling `on_io_open_complete` with `IO_OPEN_ERROR`. **]**
**SRS_HEADER_DETECT_IO_01_051: [** If `buffer` is NULL or `size` is 0 while the IO is OPEN an error shall be indicated by calling `on_io_error`. **]**
**SRS_HEADER_DETECT_IO_01_067: [** When `on_underlying_io_bytes_received` is called while waiting for header bytes (after the underlying IO was open), the bytes shall be matched against the entries provided in the configuration passed to `header_detect_io_create`. **]**
**SRS_HEADER_DETECT_IO_01_068: [** Header bytes shall be accepted in multiple `on_underlying_io_bytes_received` calls. **]**
**SRS_HEADER_DETECT_IO_01_066: [** If the bytes received since matching started do not match any of the headers in the `header_detect_entries` field, then the IO shall be considered not open and an open complete with `IO_OPEN_ERROR` shall be indicated. **]**
**SRS_HEADER_DETECT_IO_01_069: [** If a header match was detected on an entry with a non-NULL io handle, a new IO associated shall be created by calling `xio_create`. **]** This IO is referenced as the detected IO in subsequent requirements.
**SRS_HEADER_DETECT_IO_01_073: [** The interface description passed to `xio_create` shall be the interface description associated with the detected header. **]**
**SRS_HEADER_DETECT_IO_01_074: [** The IO create parameters shall be a `SERVER_PROTOCOL_IO_CONFIG` structure. **]**
**SRS_HEADER_DETECT_IO_01_075: [** The underlying IO in the `SERVER_PROTOCOL_IO_CONFIG` structure shall be set to the last detected IO that was created if any. **]**
**SRS_HEADER_DETECT_IO_01_076: [** If no detected IO was created then the underlying IO in the `SERVER_PROTOCOL_IO_CONFIG` structure shall be set to the `underlying_io` passed in the create arguments. **]**
**SRS_HEADER_DETECT_IO_01_077: [** If `xio_create` fails the header detect IO shall be closed and an error shall be indicated by calling `on_io_open_complete` with `IO_OPEN_ERROR`. **]**
**SRS_HEADER_DETECT_IO_01_078: [** The newly create IO shall be open by calling `xio_open`. **]**
**SRS_HEADER_DETECT_IO_01_079: [** The `on_io_open_complete` callback passed to `xio_open` shall be `on_underlying_io_open_complete`. **]**
**SRS_HEADER_DETECT_IO_01_080: [** The `on_bytes_received` callback passed to `xio_open` shall be `on_underlying_io_bytes_received`. **]**
**SRS_HEADER_DETECT_IO_01_081: [** The `on_io_error` callback passed to `xio_open` shall be `on_underlying_io_error`. **]**
**SRS_HEADER_DETECT_IO_01_082: [** If `xio_open` fails the header detect IO shall be closed and an error shall be indicated by calling `on_io_open_complete` with `IO_OPEN_ERROR`. **]**
**SRS_HEADER_DETECT_IO_01_083: [** The header detect IO shall wait for opening of the detected IO (signaled by the `on_underlying_io_open_complete`). **]**
**SRS_HEADER_DETECT_IO_01_085: [** If the received bytes were only partially consumed for header detection, all the remaining bytes shall be used as if they were indicated as received in a subsequent call to `on_underlying_io_bytes_received`. **]**
**SRS_HEADER_DETECT_IO_01_086: [** The newly created IO shall be added to the chain of IOs by calling `singlylinkedlist_add`. **]**
**SRS_HEADER_DETECT_IO_01_084: [** If `singlylinkedlist_add` fails the newly created IO shall be destroyed and an error shall be indicated by calling `on_io_open_complete` with `IO_OPEN_ERROR`. **]**
**SRS_HEADER_DETECT_IO_01_087: [** If `on_underlying_io_bytes_received` is called while waiting for the detected IO to complete its open, the bytes shall be given to the last created IO by calling its `on_bytes_received` callback that was filled into the `on_bytes_received` member of `SERVER_PROTOCOL_IO_CONFIG`. **]**
**SRS_HEADER_DETECT_IO_01_089: [** If `on_underlying_io_bytes_received` is called while header detect IO is OPEN the bytes shall be given to the user via the `on_bytes_received` callback that was the `on_bytes_received` callback passed to `header_detect_io_open_async`. **]**
**SRS_HEADER_DETECT_IO_01_090: [** If no detected IOs were created and `on_underlying_io_bytes_received` is called while header detect IO is OPEN, the `on_bytes_received` callback passed to `header_detect_io_open_async` shall be called to indicate the bytes as received. **]**

### on_underlying_io_bytes_received

```C
MOCKABLE_FUNCTION(, const AMQP_HEADER, header_detect_io_get_amqp_header);
```

**SRS_HEADER_DETECT_IO_01_091: [** `header_detect_io_get_amqp_header` shall return a structure that should point to a buffer that contains the bytes { 'A', 'M', 'Q', 'P', 0, 1, 0, 0 }. **]**

```C
MOCKABLE_FUNCTION(, const AMQP_HEADER, header_detect_io_get_sasl_amqp_header);
```

**SRS_HEADER_DETECT_IO_01_091: [** `header_detect_io_get_sasl_amqp_header` shall return a structure that should point to a buffer that contains the bytes { 'A', 'M', 'Q', 'P', 3, 1, 0, 0 }. **]**

### header_detect_io_get_interface_description

```c
MOCKABLE_FUNCTION(, const IO_INTERFACE_DESCRIPTION*, header_detect_io_get_interface_description);
```

**SRS_HEADER_DETECT_IO_01_096: [** `header_detect_io_get_interface_description` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains pointers to the functions: `header_detect_io_retrieve_options`, `header_detect_io_retrieve_create`, `header_detect_io_destroy`, `header_detect_io_open`, `header_detect_io_close`, `header_detect_io_send` and `header_detect_io_dowork`. **]**