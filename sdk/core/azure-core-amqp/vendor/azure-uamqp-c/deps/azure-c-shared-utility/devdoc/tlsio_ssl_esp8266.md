tlsio_ssl_esp8266
=============

## Overview

tlsio_ssl_esp8266 implements a tls adapter for the ESP8266 RTOS platform. This adapter only works in blocking mode. 


## References

[TLS Protocol (RFC2246)](https://www.ietf.org/rfc/rfc2246.txt)

[TLS Protocol (generic information)](https://en.wikipedia.org/wiki/Transport_Layer_Security)

## Exposed API

**SRS_TLSIO_SSL_ESP8266_99_001: [** The tlsio_ssl_esp8266 shall implement and export all the Concrete functions in the VTable IO_INTERFACE_DESCRIPTION defined in the `xio.h`. 
```c
typedef OPTIONHANDLER_HANDLE (*IO_RETRIEVEOPTIONS)(CONCRETE_IO_HANDLE concrete_io);
typedef CONCRETE_IO_HANDLE(*IO_CREATE)(void* io_create_parameters);
typedef void(*IO_DESTROY)(CONCRETE_IO_HANDLE concrete_io);
typedef int(*IO_OPEN)(CONCRETE_IO_HANDLE concrete_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context);
typedef int(*IO_CLOSE)(CONCRETE_IO_HANDLE concrete_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
typedef int(*IO_SEND)(CONCRETE_IO_HANDLE concrete_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
typedef void(*IO_DOWORK)(CONCRETE_IO_HANDLE concrete_io);
typedef int(*IO_SETOPTION)(CONCRETE_IO_HANDLE concrete_io, const char* optionName, const void* value);


typedef struct IO_INTERFACE_DESCRIPTION_TAG
{
    IO_RETRIEVEOPTIONS concrete_io_retrieveoptions;
    IO_CREATE concrete_io_create;
    IO_DESTROY concrete_io_destroy;
    IO_OPEN concrete_io_open;
    IO_CLOSE concrete_io_close;
    IO_SEND concrete_io_send;
    IO_DOWORK concrete_io_dowork;
    IO_SETOPTION concrete_io_setoption;
} IO_INTERFACE_DESCRIPTION;
```
**]**

**SRS_TLSIO_SSL_ESP8266_99_002: [** The tlsio_ssl_esp8266 shall report the open operation status using the IO_OPEN_RESULT enumerator defined in the `xio.h`:
```c
typedef enum IO_OPEN_RESULT_TAG
{
    IO_OPEN_OK,
    IO_OPEN_ERROR,
    IO_OPEN_CANCELLED
} IO_OPEN_RESULT;
```
**]**

**SRS_TLSIO_SSL_ESP8266_99_003: [** The tlsio_ssl_esp8266 shall report the send operation status using the IO_SEND_RESULT enumerator defined in the `xio.h`:
```c
typedef enum IO_SEND_RESULT_TAG
{
    IO_SEND_OK,
    IO_SEND_ERROR,
    IO_SEND_CANCELLED
} IO_SEND_RESULT;
```
**]**

**SRS_TLSIO_SSL_ESP8266_99_004: [** The tlsio_ssl_esp8266 shall call the callbacks functions defined in the `xio.h`:
```c
typedef void(*ON_BYTES_RECEIVED)(void* context, const unsigned char* buffer, size_t size);
typedef void(*ON_SEND_COMPLETE)(void* context, IO_SEND_RESULT send_result);
typedef void(*ON_IO_OPEN_COMPLETE)(void* context, IO_OPEN_RESULT open_result);
typedef void(*ON_IO_CLOSE_COMPLETE)(void* context);
typedef void(*ON_IO_ERROR)(void* context);
```
**]**

**SRS_TLSIO_SSL_ESP8266_99_005: [** The tlsio_ssl_esp8266 shall received the connection information using the TLSIO_CONFIG structure defined in `tlsio.h`:
```c
typedef struct TLSIO_CONFIG_TAG
{
    const char* hostname;
    int port;
} TLSIO_CONFIG;
```
**]**


## Callbacks

**SRS_TLSIO_SSL_ESP8266_99_006: [** The tlsio_ssl_esp8266 shall return the status of all async operations using the callbacks. **]**

**SRS_TLSIO_SSL_ESP8266_99_007: [** If the callback function is set as NULL. The tlsio_ssl_esp8266 shall not call anything. **]**


###  tlsio_openssl_get_interface_description
```c
const IO_INTERFACE_DESCRIPTION* tlsio_openssl_get_interface_description(void);
```

**SRS_TLSIO_SSL_ESP8266_99_008: [** The tlsio_openssl_get_interface_description shall return the VTable IO_INTERFACE_DESCRIPTION. **]**


###  tlsio_openssl_create
Implementation of `IO_CREATE concrete_io_create`
```c
CONCRETE_IO_HANDLE tlsio_openssl_create(void* io_create_parameters);
```

**SRS_TLSIO_SSL_ESP8266_99_009: [** The tlsio_openssl_create shall create a new instance of the tlsio for esp8266. **]**

**SRS_TLSIO_SSL_ESP8266_99_010: [** The tlsio_openssl_create shall return a non-NULL handle on success. **]**

**SRS_TLSIO_SSL_ESP8266_99_011: [** The tlsio_openssl_create shall allocate memory to control the tlsio instance. **]**

**SRS_TLSIO_SSL_ESP8266_99_012: [** If there is not enough memory to create the tlsio, the tlsio_openssl_create shall return NULL as the handle. **]**

**SRS_TLSIO_SSL_ESP8266_99_013: [** The tlsio_openssl_create shall return NULL when io_create_parameters is NULL. **]**

**SRS_TLSIO_SSL_ESP8266_99_016: [** The tlsio_openssl_create shall initialize all callback pointers as NULL. **]**

**SRS_TLSIO_SSL_ESP8266_99_017: [** The tlsio_openssl_create shall receive the connection configuration (TLSIO_CONFIG). **]**

**SRS_TLSIO_SSL_ESP8266_99_020: [** If tlsio_openssl_create get success to create the tlsio instance, it shall set the tlsio state as TLSIO_STATE_NOT_OPEN. **]**


###  tlsio_openssl_destroy
Implementation of `IO_DESTROY concrete_io_destroy`
```c
void tlsio_openssl_destroy(CONCRETE_IO_HANDLE tls_io);
```

**SRS_TLSIO_SSL_ESP8266_99_021: [** The tlsio_openssl_destroy shall destroy a created instance of the tlsio for ESP8266 identified by the CONCRETE_IO_HANDLE. **]**

**SRS_TLSIO_SSL_ESP8266_99_022: [** The tlsio_openssl_destroy shall free all memory allocated for tlsio_instance. **]**

**SRS_TLSIO_SSL_ESP8266_99_024: [** If the tlsio_handle is NULL, the tlsio_openssl_destroy shall not do anything. **]**

**SRS_TLSIO_SSL_ESP8266_99_025: [** If the tlsio state is TLSIO_STATE_OPENING, TLSIO_STATE_OPEN, or TLSIO_STATE_CLOSING, the tlsio_openssl_destroy shall destroy the tlsio, but log an error. **]**


###  tlsio_openssl_open
Implementation of `IO_OPEN concrete_io_open`
```c
int tlsio_openssl_open(
    CONCRETE_IO_HANDLE tls_io, 
    ON_IO_OPEN_COMPLETE on_io_open_complete, 
    void* on_io_open_complete_context, 
    ON_BYTES_RECEIVED on_bytes_received, 
    void* on_bytes_received_context, 
    ON_IO_ERROR on_io_error, 
    void* on_io_error_context);
```

**SRS_TLSIO_SSL_ESP8266_99_018: [** The tlsio_openssl_open shall convert the provide hostName to an IP address. **]**

**SRS_TLSIO_SSL_ESP8266_99_019: [** If the WiFi cannot find the IP for the hostName, the tlsio_openssl_open shall return __LINE__. **]**

**SRS_TLSIO_SSL_ESP8266_99_026: [** The tlsio_openssl_open shall start the process to open the ssl connection with the host provided in the tlsio_openssl_create. **]**

**SRS_TLSIO_SSL_ESP8266_99_027: [** The tlsio_openssl_open shall set the tlsio to try to open the connection for MAX_RETRY times before assuming that connection failed. **]**

**SRS_TLSIO_SSL_ESP8266_99_028: [** The tlsio_openssl_open shall store the provided on_io_open_complete callback function address. **]**

**SRS_TLSIO_SSL_ESP8266_99_029: [** The tlsio_openssl_open shall store the provided on_io_open_complete_context handle. **]**

**SRS_TLSIO_SSL_ESP8266_99_030: [** The tlsio_openssl_open shall store the provided on_bytes_received callback function address. **]**

**SRS_TLSIO_SSL_ESP8266_99_031: [** The tlsio_openssl_open shall store the provided on_bytes_received_context handle. **]**

**SRS_TLSIO_SSL_ESP8266_99_032: [** The tlsio_openssl_open shall store the provided on_io_error callback function address. **]**

**SRS_TLSIO_SSL_ESP8266_99_033: [** The tlsio_openssl_open shall store the provided on_io_error_context handle. **]**

**SRS_TLSIO_SSL_ESP8266_99_034: [** If tlsio_openssl_open get success to open the ssl connection, it shall set the tlsio state as TLSIO_STATE_OPEN, and return 0. **]**

**SRS_TLSIO_SSL_ESP8266_99_035: [** If the tlsio state is not TLSIO_STATE_NOT_OPEN and not TLSIO_STATE_ERROR, then tlsio_openssl_open shall set the tlsio state as TLSIO_STATE_ERROR, and return _LINE_. **]**

**SRS_TLSIO_SSL_ESP8266_99_036: [** If the tls_io handle is NULL, the tlsio_openssl_open shall not do anything, and return _LINE_. **]**

**SRS_TLSIO_SSL_ESP8266_99_037: [** If the ssl client is not connected, the tlsio_openssl_open shall change the state to TLSIO_STATE_ERROR, log the error, and return _LINE_. **]**

**SRS_TLSIO_SSL_ESP8266_99_039: [** If the tlsio_openssl_open failed to open the tls connection, and the on_io_open_complete callback was provided, it shall call the on_io_open_complete with IO_OPEN_ERROR. **]**

**SRS_TLSIO_SSL_ESP8266_99_040: [** If the tlsio_openssl_open failed to open the tls connection, and the on_io_error callback was provided, it shall call the on_io_error. **]**

**SRS_TLSIO_SSL_ESP8266_99_041: [** If the tlsio_openssl_open get success to open the tls connection, and the on_io_open_complete callback was provided, it shall call the on_io_open_complete with IO_OPEN_OK. **]**

**SRS_TLSIO_SSL_ESP8266_99_042: [** If the tlsio_openssl_open retry SSL_connect to open more than MAX_RETRY times without success, it shall return __LINE__. **]**

**SRS_TLSIO_SSL_ESP8266_99_080: [** If socket failed, the tlsio_openssl_open shall return __LINE__. **]**

**SRS_TLSIO_SSL_ESP8266_99_081: [** If setsockopt failed, the tlsio_openssl_open shall return __LINE__. **]**

**SRS_TLSIO_SSL_ESP8266_99_082: [** If bind failed, the tlsio_openssl_open shall return __LINE__. **]**

**SRS_TLSIO_SSL_ESP8266_99_083: [** If connect and getsockopt failed, the tlsio_openssl_open shall return __LINE__. **]**

**SRS_TLSIO_SSL_ESP8266_99_084: [** If lwip_select failed, the tlsio_openssl_open shall return __LINE__. **]**

**SRS_TLSIO_SSL_ESP8266_99_085: [** If SSL_CTX_new failed, the tlsio_openssl_open shall return __LINE__. **]**

**SRS_TLSIO_SSL_ESP8266_99_087: [** If SSL_new failed, the tlsio_openssl_open shall return __LINE__. **]**

**SRS_TLSIO_SSL_ESP8266_99_088: [** If SSL_set_fd failed, the tlsio_openssl_open shall return __LINE__. **]**

**SRS_TLSIO_SSL_ESP8266_99_089: [** If SSL_connect failed, the tlsio_openssl_open shall return __LINE__. **]**

###  tlsio_openssl_close
Implementation of `IO_CLOSE concrete_io_close`
```c
int tlsio_openssl_close(CONCRETE_IO_HANDLE tls_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
```

**SRS_TLSIO_SSL_ESP8266_99_043: [** The tlsio_openssl_close shall start the process to close the ssl connection. **]**

**SRS_TLSIO_SSL_ESP8266_99_045: [** The tlsio_openssl_close shall store the provided on_io_close_complete callback function address. **]**

**SRS_TLSIO_SSL_ESP8266_99_046: [** The tlsio_openssl_close shall store the provided on_io_close_complete_context handle. **]**

**SRS_TLSIO_SSL_ESP8266_99_047: [** If tlsio_openssl_close get success to start the process to close the ssl connection, it shall set the tlsio state as TLSIO_STATE_CLOSING, and return 0. **]**

**SRS_TLSIO_SSL_ESP8266_99_048: [** If the tlsio state is TLSIO_STATE_NOT_OPEN, TLSIO_STATE_OPENING, or TLSIO_STATE_CLOSING, the tlsio_openssl_close shall set the tlsio state as TLSIO_STATE_ERROR, and return _LINE_. **]**

**SRS_TLSIO_SSL_ESP8266_99_049: [** If the tlsio_handle is NULL, the tlsio_openssl_close shall not do anything, and return _LINE_. **]**

**SRS_TLSIO_SSL_ESP8266_99_050: [** If tlsio_openssl_close successfully destroys the ssl connection, it shall set the tlsio state as TLSIO_STATE_NOT_OPEN, and return 0. **]**

**SRS_TLSIO_SSL_ESP8266_99_051: [** If tlsio_openssl_close successfully destroys the ssl connection, it shall call on_io_close_complete. **]**

**SRS_TLSIO_SSL_ESP8266_99_052: [** If tlsio_openssl_close fails to shutdown the ssl connection, it shall set the tlsio state as TLSIO_STATE_ERROR, and return _LINE_, and call on_io_error. **]**


###  tlsio_openssl_send
Implementation of `IO_SEND concrete_io_send`
```c
int tlsio_openssl_send(CONCRETE_IO_HANDLE tls_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
```

**SRS_TLSIO_SSL_ESP8266_99_053: [** The tlsio_openssl_send shall send all bytes in a buffer to the ssl connection. **]**

**SRS_TLSIO_SSL_ESP8266_99_054: [** The tlsio_openssl_send shall use the provided on_io_send_complete callback function address. **]**

**SRS_TLSIO_SSL_ESP8266_99_055: [** The tlsio_openssl_send shall use the provided on_io_send_complete_context handle. **]**

**SRS_TLSIO_SSL_ESP8266_99_056: [** The ssl will continue to send all data in the buffer until all bytes have been sent. **]**

**SRS_TLSIO_SSL_ESP8266_99_057: [** If the ssl was not able to send all the bytes in the buffer, the tlsio_openssl_send shall call the on_send_complete with IO_SEND_ERROR, and return _LINE_. **]**

**SRS_TLSIO_SSL_ESP8266_99_058: [** If the ssl finish to send all bytes in the buffer, then tlsio_openssl_send shall call the on_send_complete with IO_SEND_OK, and return 0 **]**

**SRS_TLSIO_SSL_ESP8266_99_059: [** If the tlsio state is not TLSIO_STATE_OPEN, the tlsio_openssl_send shall return _LINE_. **]**

**SRS_TLSIO_SSL_ESP8266_99_060: [** If the tls_io handle is NULL, the tlsio_openssl_send shall not do anything, and return _LINE_. **]**

**SRS_TLSIO_SSL_ESP8266_99_061: [** If the buffer is NULL, the tlsio_openssl_send shall not do anything, and return _LINE_. **]**

**SRS_TLSIO_SSL_ESP8266_99_062: [** If the size is 0, the tlsio_openssl_send shall not do anything, and return _LINE_. **]**


###  tlsio_openssl_dowork
Implementation of `IO_DOWORK concrete_io_dowork`
```c
void tlsio_openssl_dowork(CONCRETE_IO_HANDLE tls_io)
```

**SRS_TLSIO_SSL_ESP8266_99_063: [** The tlsio_openssl_dowork shall execute the async jobs for the tlsio. **]**

**SRS_TLSIO_SSL_ESP8266_99_069: [** If the tlsio state is TLSIO_STATE_OPEN, the tlsio_openssl_dowork shall read data from the ssl client. **]**

**SRS_TLSIO_SSL_ESP8266_99_070: [** If there are received data in the ssl client, the tlsio_openssl_dowork shall read this data and call the on_bytes_received with the pointer to the buffer with the data. **]**

**SRS_TLSIO_SSL_ESP8266_99_071: [** If there are no received data in the ssl client, the tlsio_openssl_dowork shall do nothing. **]**

**SRS_TLSIO_SSL_ESP8266_99_074: [** If the tlsio handle is NULL, the tlsio_openssl_dowork shall not do anything. **]**

**SRS_TLSIO_SSL_ESP8266_99_075: [** The tlsio_openssl_dowork shall create a buffer to store the data received from the ssl client. **]**

**SRS_TLSIO_SSL_ESP8266_99_076: [** The tlsio_openssl_dowork shall delete the buffer to store the data received from the ssl client. **]**


###  tlsio_openssl_setoption
Implementation of `IO_SETOPTION concrete_io_setoption`
```c
int tlsio_openssl_setoption(CONCRETE_IO_HANDLE tls_io, const char* optionName, const void* value)
```

**SRS_TLSIO_SSL_ESP8266_99_077: [** The tlsio_openssl_setoption shall not do anything, and return 0. **]**


###  tlsio_openssl_retrieveoptions
Implementation of `IO_RETRIEVEOPTIONS concrete_io_retrieveoptions`
```c
OPTIONHANDLER_HANDLE tlsio_openssl_retrieveoptions(CONCRETE_IO_HANDLE tlsio_handle)
```

**SRS_TLSIO_SSL_ESP8266_99_078: [** The tlsio_openssl_retrieveoptions shall not do anything, and return NULL. **]**


