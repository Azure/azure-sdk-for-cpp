tlsio_cyclonessl
=============

## Overview

tlsio_cyclonessl implements a tls adapter for the CycloneSSL TLS library.  
This adapter only works in blocking mode.

## References

[CycloneSSL official page](http://www.oryx-embedded.com/cyclone_ssl.html)

[TLS Protocol (generic information)](https://en.wikipedia.org/wiki/Transport_Layer_Security)

## Exposed API

```c
MOCKABLE_FUNCTION(, const IO_INTERFACE_DESCRIPTION*, tlsio_cyclonessl_get_interface_description);
```


###  tlsio_cyclonessl_create

tlsio_cyclonessl_create is the implementation provided via tlsio_cyclonessl_get_interface_description for the concrete_io_create member.

```c
CONCRETE_IO_HANDLE tlsio_cyclonessl_create(void* io_create_parameters);
```

**SRS_TLSIO_CYCLONESSL_01_001: [** tlsio_cyclonessl_create shall create a new instance of the tlsio for Cyclone SSL. **]**

**SRS_TLSIO_CYCLONESSL_01_002: [** If io_create_parameters is NULL, tlsio_cyclonessl_create shall fail and return NULL. **]**

**SRS_TLSIO_CYCLONESSL_01_003: [** io_create_parameters shall be used as a TLSIO_CONFIG\*. **]**

**SRS_TLSIO_CYCLONESSL_01_004: [** If the hostname member is NULL, then tlsio_cyclonessl_create shall fail and return NULL. **]**

**SRS_TLSIO_CYCLONESSL_01_005: [** tlsio_cyclonessl_create shall copy the hostname and port values for later use when the open of the underlying socket is needed. **]**

**SRS_TLSIO_CYCLONESSL_01_006: [** hostname shall be copied by calling mallocAndStrcpy_s. **]**

**SRS_TLSIO_CYCLONESSL_01_076: [** If allocating memory for the TLS IO instance fails then tlsio_cyclonessl_create shall fail and return NULL. **]**

**SRS_TLSIO_CYCLONESSL_01_007: [** If mallocAndStrcpy_s fails then tlsio_cyclonessl_create shall fail and return NULL. **]**

**SRS_TLSIO_CYCLONESSL_01_008: [** tlsio_cyclonessl_create shall initialize the yarrow context by calling yarrowInit. **]**

**SRS_TLSIO_CYCLONESSL_01_009: [** If yarrowInit fails then tlsio_cyclonessl_create shall return NULL. **]**

**SRS_TLSIO_CYCLONESSL_01_010: [** The yarrow context shall be seeded with 32 bytes of randomly chosen data by calling yarrowSeed. **]**

**SRS_TLSIO_CYCLONESSL_01_011: [** If yarrowSeed fails then tlsio_cyclonessl_create shall return NULL. **]**

**SRS_TLSIO_CYCLONESSL_01_012: [** tlsio_cyclonessl_create shall create a TLS context by calling tlsInit. **]**

**SRS_TLSIO_CYCLONESSL_01_013: [** If tlsInit fails then tlsio_cyclonessl_create shall return NULL. **]**

**SRS_TLSIO_CYCLONESSL_01_014: [** The TLS context shall be setup to operate as a client by calling tlsSetConnectionEnd with TLS_CONNECTION_END_CLIENT. **]**

**SRS_TLSIO_CYCLONESSL_01_015: [** If tlsSetConnectionEnd fails then tlsio_cyclonessl_create shall return NULL. **]**

**SRS_TLSIO_CYCLONESSL_01_016: [** The pseudo random number generator to be used shall be set by calling tlsSetPrng with the yarrow instance as argument. **]**

**SRS_TLSIO_CYCLONESSL_01_017: [** If tlsSetPrng fails then tlsio_cyclonessl_create shall return NULL. **]**

**SRS_TLSIO_CYCLONESSL_01_018: [** When tlsio_cyclonessl_create fails, all allocated resources up to that point shall be freed. **]**

###  tlsio_cyclonessl_destroy

tlsio_cyclonessl_destroy is the implementation provided via tlsio_cyclonessl_get_interface_description for the concrete_io_destroy member.

```c
void tlsio_cyclonessl_destroy(CONCRETE_IO_HANDLE tls_io);
```

**SRS_TLSIO_CYCLONESSL_01_019: [** tlsio_cyclonessl_destroy shall free the tlsio CycloneSSL instance. **]**

**SRS_TLSIO_CYCLONESSL_01_020: [** If tls_io is NULL, tlsio_cyclonessl_destroy shall do nothing. **]**

**SRS_TLSIO_CYCLONESSL_01_021: [** tlsio_cyclonessl_destroy shall deinitialize the yarrow context by calling yarrowRelease. **]**

**SRS_TLSIO_CYCLONESSL_01_022: [** The TLS context shall be freed by calling tlsFree. **]**

**SRS_TLSIO_CYCLONESSL_01_077: [** All options cached via tlsio_cyclonessl_set_option shall also be freed. **]**

**SRS_TLSIO_CYCLONESSL_01_081: [** tlsio_cyclonessl_destroy should close the IO if it was open before freeing all the resources. **]**

###  tlsio_cyclonessl_open

tlsio_cyclonessl_open is the implementation provided via tlsio_cyclonessl_get_interface_description for the concrete_io_open member.

```c
int tlsio_cyclonessl_open(CONCRETE_IO_HANDLE tls_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
```

**SRS_TLSIO_CYCLONESSL_01_023: [** tlsio_cyclonessl_open shall open the TLS io and on success it shall return 0. **]**

**SRS_TLSIO_CYCLONESSL_01_024: [** If any of the arguments tls_io, on_io_open_complete, on_bytes_received or on_io_error are NULL then tlsio_cyclonessl_open shall return a non-zero value. **]**

**SRS_TLSIO_CYCLONESSL_01_025: [** tlsio_cyclonessl_open shall create a socket by calling tlsio_cyclonessl_socket_create, while passing to it the hostname and port that were saved in the tlsio_cyclonessl_create. **]**

**SRS_TLSIO_CYCLONESSL_01_026: [** If tlsio_cyclonessl_socket_create fails, then tlsio_cyclonessl_open shall return a non-zero value. **]**

**SRS_TLSIO_CYCLONESSL_01_027: [** The socket created by tlsio_cyclonessl_socket_create shall be assigned to the TLS context by calling tlsSetSocket. **]**

**SRS_TLSIO_CYCLONESSL_01_028: [** If tlsSetSocket fails then tlsio_cyclonessl_open shall return a non-zero value. **]**

**SRS_TLSIO_CYCLONESSL_01_078: [** If certificates have been set by using tlsio_cyclonessl_set_option then a call to tlsSetTrustedCaList shall be made to pass the certificates to CycloneSSL. **]**

**SRS_TLSIO_CYCLONESSL_01_079: [** If tlsSetTrustedCaList fails then tlsio_cyclonessl_open shall return a non-zero value. **]**

**SRS_TLSIO_CYCLONESSL_01_031: [** tlsio_cyclonessl_open shall start the TLS handshake by calling tlsConnect. **]**

**SRS_TLSIO_CYCLONESSL_01_032: [** If tlsConnect fails then tlsio_cyclonessl_open shall return a non-zero value. **]**

**SRS_TLSIO_CYCLONESSL_01_033: [** If tlsConnect succeeds, the callback on_io_open_complete shall be called, while passing on_io_open_complete_context and IO_OPEN_OK as arguments. **]**

**SRS_TLSIO_CYCLONESSL_01_034: [** If tlsio_cyclonessl_open is called while the IO is open, tlsio_cyclonessl_open shall fail and return a non-zero value without performing any work to open the IO. **]**

###  tlsio_cyclonessl_close

tlsio_cyclonessl_close is the implementation provided via tlsio_cyclonessl_get_interface_description for the concrete_io_close member.

```c
int tlsio_cyclonessl_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* on_io_close_complete_context)
```

**SRS_TLSIO_CYCLONESSL_01_035: [** tlsio_cyclonessl_close shall close the TLS IO and on success it shall return 0. **]**

**SRS_TLSIO_CYCLONESSL_01_036: [** If the argument tls_io is NULL, tlsio_cyclonessl_close shall fail and return a non-zero value. **]**

**SRS_TLSIO_CYCLONESSL_01_037: [** tlsio_cyclonessl_close shall close the TLS connection by calling tlsShutdown. **]**

**SRS_TLSIO_CYCLONESSL_01_038: [** If tlsShutdown fails, tlsio_cyclonessl_close shall fail and return a non-zero value. **]**

**SRS_TLSIO_CYCLONESSL_01_039: [** tlsio_cyclonessl_destroy shall destroy the underlying socket by calling tlsio_cyclonessl_socket_destroy. **]**

**SRS_TLSIO_CYCLONESSL_01_040: [** On success, on_io_close_complete shall be called while passing as argument on_io_close_complete_context. **]**

**SRS_TLSIO_CYCLONESSL_01_041: [** If tlsio_cyclonessl_close is called when not open, tlsio_cyclonessl_close shall fail and return a non-zero value. **]** 

###  tlsio_cyclonessl_send

tlsio_cyclonessl_send is the implementation provided via tlsio_cyclonessl_get_interface_description for the concrete_io_send member.

```c
int tlsio_cyclonessl_send(CONCRETE_IO_HANDLE tls_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* on_send_complete_context)
```

**SRS_TLSIO_CYCLONESSL_01_042: [** tlsio_cyclonessl_send shall send the `size` bytes pointed to by `buffer` and on success it shall return 0. **]**

**SRS_TLSIO_CYCLONESSL_01_043: [** If any of the arguments tls_io or buffer is NULL, tlsio_cyclonessl_send shall fail and return a non-zero value. **]**

**SRS_TLSIO_CYCLONESSL_01_044: [** If size is 0, tlsio_cyclonessl_send shall fail and return a non-zero value. **]**

**SRS_TLSIO_CYCLONESSL_01_045: [** on_send_complete shall be allowed to be NULL. **]**

**SRS_TLSIO_CYCLONESSL_01_046: [** On success, if a non-NULL value was passed for on_send_complete, then on_send_complete shall be called while passing to it the on_send_complete_context value. **]**

**SRS_TLSIO_CYCLONESSL_01_047: [** tlsio_cyclonessl_send shall send the bytes by calling tlsWrite and passing buffer and size as arguments. 0 shall be passed for the flags argument. **]**

**SRS_TLSIO_CYCLONESSL_01_048: [** If tlsio_cyclonessl_send is called when the IO is not open, tlsio_cyclonessl_send shall fail and return a non-zero value. **]**

**SRS_TLSIO_CYCLONESSL_01_049: [** If the IO is in an error state (an error was reported through the on_io_error callback), tlsio_cyclonessl_send shall fail and return a non-zero value. **]**  

###  tlsio_cyclonessl_dowork

tlsio_cyclonessl_dowork is the implementation provided via tlsio_cyclonessl_get_interface_description for the concrete_io_dowork member.

```c
void tlsio_cyclonessl_dowork(CONCRETE_IO_HANDLE tls_io)
```

**SRS_TLSIO_CYCLONESSL_01_050: [** tlsio_cyclonessl_dowork shall check if any bytes are available to be read from the CycloneSSL library and indicate those bytes as received. **]**

**SRS_TLSIO_CYCLONESSL_01_051: [** If the tls_io argument is NULL, tlsio_cyclonessl_dowork shall do nothing. **]**

**SRS_TLSIO_CYCLONESSL_01_052: [** If the IO is not open (no open has been called or the IO has been closed) then tlsio_cyclonessl_dowork shall do nothing. **]**

**SRS_TLSIO_CYCLONESSL_01_053: [** If the IO is open, tlsio_cyclonessl_dowork shall attempt to read 64 bytes from the TLS library by calling tlsRead. **]**

**SRS_TLSIO_CYCLONESSL_01_080: [** If any bytes are read from CycloneSSL they should be indicated via the on_bytes_received callback passed to tlsio_cyclonessl_open. **]**

**SRS_TLSIO_CYCLONESSL_01_054: [** The flags argument for tlsRead shall be 0. **]**

**SRS_TLSIO_CYCLONESSL_01_055: [** If tlsRead fails, the error shall be indicated by calling the on_io_error callback passed in tlsio_cyclonessl_open, while passing the on_io_error_context to the callback. **]**

**SRS_TLSIO_CYCLONESSL_01_056: [** Also the IO shall be considered in error and any subsequent calls to tlsio_cyclonessl_send shall fail. **]**

###  tlsio_cyclonessl_setoption

tlsio_cyclonessl_setoption is the implementation provided via tlsio_cyclonessl_get_interface_description for the concrete_io_setoption member.

```c
int tlsio_cyclonessl_setoption(CONCRETE_IO_HANDLE tls_io, const char* optionName, const void* value)
```

**SRS_TLSIO_CYCLONESSL_01_057: [** If any of the arguments tls_io or option_name is NULL tlsio_cyclonessl_setoption shall return a non-zero value. **]**

**SRS_TLSIO_CYCLONESSL_01_058: [** If the option_name argument indicates an option that is not handled by tlsio_cyclonessl, then tlsio_cyclonessl_setoption shall return a non-zero value. **]**

**SRS_TLSIO_CYCLONESSL_01_059: [** If the option was handled by tlsio_cyclonessl, then tlsio_cyclonessl_setoption shall return 0. **]**

Options that shall be handled by tlsio_cyclonessl:

**SRS_TLSIO_CYCLONESSL_01_060: [** - "TrustedCerts" - a char\* that shall be saved by tlsio_cyclonessl as it shall be given to the underlying CycloneSSL TLS context when the IO is open. **]**

**SRS_TLSIO_CYCLONESSL_01_061: [** If copying the char\* passed in value fails then tlsio_cyclonessl_setoption shall return a non-zero value. **]**

**SRS_TLSIO_CYCLONESSL_01_062: [** If a previous TrustedCerts option was saved, then the previous value shall be freed. **]**

**SRS_TLSIO_CYCLONESSL_01_063: [** A NULL value shall be allowed for TrustedCerts, in which case the previously stored TrustedCerts option value shall be cleared. **]**

###  tlsio_cyclonessl_retrieve_options

tlsio_cyclonessl_retrieve_options is the implementation provided via tlsio_cyclonessl_get_interface_description for the concrete_io_retrieveoptions member.

```c
static OPTIONHANDLER_HANDLE tlsio_cyclonessl_retrieve_options(CONCRETE_IO_HANDLE handle)
```

tlsio_cyclonessl_retrieveoptions produces an OPTIONHANDLER_HANDLE. 

**SRS_TLSIO_CYCLONESSL_01_064: [** If parameter handle is `NULL` then `tlsio_cyclonessl_retrieve_options` shall fail and return NULL. **]**

**SRS_TLSIO_CYCLONESSL_01_065: [** `tlsio_cyclonessl_retrieve_options` shall produce an OPTIONHANDLER_HANDLE. **]**

**SRS_TLSIO_CYCLONESSL_01_066: [** `tlsio_cyclonessl_retrieve_options` shall add to it the options: **]**

**SRS_TLSIO_CYCLONESSL_01_067: [**  - TrustedCerts **]**

**SRS_TLSIO_CYCLONESSL_01_068: [** If producing the OPTIONHANDLER_HANDLE fails then tlsio_cyclonessl_retrieve_options shall fail and return NULL. **]** 

###  tlsio_cyclonessl_get_interface_description

```c
extern const IO_INTERFACE_DESCRIPTION* tlsio_cyclonessl_get_interface_description(void);
```

**SRS_TLSIO_CYCLONESSL_01_069: [** tlsio_cyclonessl_get_interface_description shall return a pointer to an IO_INTERFACE_DESCRIPTION structure that contains pointers to the functions: tlsio_cyclonessl_retrieve_options, tlsio_cyclonessl_create, tlsio_cyclonessl_destroy, tlsio_cyclonessl_open, tlsio_cyclonessl_close, tlsio_cyclonessl_send and tlsio_cyclonessl_dowork.  **]**

###  tlsio_cyclonessl_clone_option

tlsio_cyclonessl_clone_option is the implementation provided to the option handler instance created as part of tlsio_cyclonessl_retrieve_options.

```c
void* tlsio_cyclonessl_clone_option(const char* name, const void* value)
```

**SRS_TLSIO_CYCLONESSL_01_070: [** If the name or value arguments are NULL, tlsio_cyclonessl_clone_option shall return NULL. **]**

**SRS_TLSIO_CYCLONESSL_01_071: [** tlsio_cyclonessl_clone_option shall clone the option named `TrustedCerts` by calling mallocAndStrcpy_s. **]** **SRS_TLSIO_CYCLONESSL_01_072: [** On success it shall return a non-NULL pointer to the cloned option. **]**

**SRS_TLSIO_CYCLONESSL_01_073: [** If mallocAndStrcpy_s for `TrustedCerts` fails, tlsio_cyclonessl_clone_option shall return NULL. **]**

###  tlsio_cyclonessl_destroy_option

tlsio_cyclonessl_destroy_option is the implementation provided to the option handler instance created as part of tlsio_cyclonessl_retrieve_options.

```c
void tlsio_cyclonessl_destroy_option(const char* name, const void* value)
```

**SRS_TLSIO_CYCLONESSL_01_074: [** If any of the arguments is NULL, tlsio_cyclonessl_destroy_option shall do nothing. **]**

**SRS_TLSIO_CYCLONESSL_01_075: [** If the option name is `TrustedCerts`, tlsio_cyclonessl_destroy_option shall free the char\* option indicated by value. **]**
