socket_async
=================

## Overview

**socket_async** abstracts a non-blocking TCP or UDP socket while hiding OS implementation details. The socket handling code in `socket_async.c` is common to many environments, and
a system of header includes and #defines will adapt the code for each particular environment. Validation of SOCKET_ASYNC_HANDLE parameters is deferred to the underlying socket layer.

It is anticipated that socket_async.c will work for all non-Windows environments, and a socket_async_win32.c will be needed for Windows.
## References

[socket_async.h](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/socket_async.h)  
[sys/socket.h, a typical linux socket implementation](http://pubs.opengroup.org/onlinepubs/7908799/xns/syssocket.h.html)

###   Exposed API

**SRS_SOCKET_ASYNC_30_001: [** The socket_async shall use the constants and types defined in `socket_async.h`.
```c
#define SOCKET_ASYNC_INVALID_SOCKET -1

    typedef struct
    {
      int keep_alive;     // < 0 for system defaults, 0 to disable, >= 0 to use supplied keep_alive, idle, interval, and count
      int keep_idle;      // seconds before first keepalive packet (ignored if keep_alive <= 0)
      int keep_interval;  // seconds between keepalive packets (ignored if keep_alive <= 0)
      int keep_count;     // number of times to try before declaring failure (ignored if keep_alive <= 0)
    } SOCKET_ASYNC_OPTIONS;
    typedef SOCKET_ASYNC_OPTIONS* SOCKET_ASYNC_OPTIONS_HANDLE;

    typedef int SOCKET_ASYNC_HANDLE;
```
 **]**

**SRS_SOCKET_ASYNC_30_002: [** The socket_async shall implement the methods defined in `socket_async.h`.
```c
SOCKET_ASYNC_HANDLE socket_async_create(uint32_t host_ipv4, uint16_t port, bool is_UDP, SOCKET_ASYNC_OPTIONS_HANDLE options);
int socket_async_is_create_complete(SOCKET_ASYNC_HANDLE sock, bool* is_complete);
int socket_async_send(SOCKET_ASYNC_HANDLE sock, void* buffer, size_t size, size_t* sent_count);
int socket_async_receive(SOCKET_ASYNC_HANDLE sock, void* buffer, size_t size, size_t* received_count);
void socket_async_destroy(SOCKET_ASYNC_HANDLE sock);
```
 **]**


###   socket_async_create
`socket_async_create` creates a socket and sets its configuration, including setting the socket to non-blocking. It then binds the socket to the supplied `host_ipv4` and `port`, and begins the process of connecting the socket to the bound address.
```c
SOCKET_ASYNC_HANDLE socket_async_create(uint32_t host_ipv4, uint16_t port, bool is_UDP, SOCKET_ASYNC_OPTIONS_HANDLE options);
```

**SRS_SOCKET_ASYNC_30_011: [** The `host_ipv4` parameter shall be the 32-bit IP V4 of the target server. **]**

**SRS_SOCKET_ASYNC_30_012: [** The `port` parameter shall be the port number for the target server. **]**

**SRS_SOCKET_ASYNC_30_013: [** The `is_UDP` parameter shall be `true` for a UDP connection, and `false` for TCP. **]**

**SRS_SOCKET_ASYNC_30_014: [** If the optional `options` parameter is non-NULL and `is_UDP` is false, and options->keep_alive is non-negative, `socket_async_create` shall set the socket options to the provided options values. **]**

**SRS_SOCKET_ASYNC_30_015: [** If the optional `options` parameter is non-NULL and `is_UDP` is false, and options->keep_alive is negative, `socket_async_create` not set the socket keep-alive options. **]**

**SRS_SOCKET_ASYNC_30_016: [** If `is_UDP` is true, the optional `options` parameter shall be ignored. **]**

**SRS_SOCKET_ASYNC_30_017: [** If the optional `options` parameter is NULL and `is_UDP` is false, `socket_async_create` shall disable TCP keep-alive. **]**

**SRS_SOCKET_ASYNC_30_018: [** On success, `socket_async_create` shall return the created and configured SOCKET_ASYNC_HANDLE. **]**

**SRS_SOCKET_ASYNC_30_019: [** The socket returned shall be non-blocking. **]**

**SRS_SOCKET_ASYNC_30_010: [** If socket option creation fails, `socket_async_create` shall log an error and return SOCKET_ASYNC_INVALID_SOCKET. **]**

**SRS_SOCKET_ASYNC_30_020: [** If socket option setting fails, `socket_async_create` shall log an error and return SOCKET_ASYNC_INVALID_SOCKET. **]**

**SRS_SOCKET_ASYNC_30_021: [** If socket binding fails, `socket_async_create` shall log an error and return SOCKET_ASYNC_INVALID_SOCKET. **]**

**SRS_SOCKET_ASYNC_30_022: [** If socket connection fails, `socket_async_create` shall log an error and return SOCKET_ASYNC_INVALID_SOCKET. **]**


###   socket_async_is_create_complete
`socket_async_is_create_complete` tests whether the socket has completed its initial TCP handshake. For UDP connections the `socket_async_is_create_complete` call is optional and always returns true.

If this method fails then `socket_async_destroy` must be called.

Validation of the `sock` parameter is deferred to the underlying socket call.

```c
int socket_async_is_create_complete(SOCKET_ASYNC_HANDLE sock, bool* is_complete);
```

**SRS_SOCKET_ASYNC_30_023: [** The `sock` parameter shall be the socket to check for initial handshake completion. **]**

**SRS_SOCKET_ASYNC_30_024: [** The `is_complete` parameter shall receive the completion state. **]**

**SRS_SOCKET_ASYNC_30_025: [** If the `is_complete` parameter is NULL, `socket_async_is_create_complete` shall log an error and return _FAILURE_. **]**

**SRS_SOCKET_ASYNC_30_026: [** On success, the `is_complete` value shall be set to the completion state and `socket_async_create` shall return 0. **]**

**SRS_SOCKET_ASYNC_30_027: [** On failure, the `is_complete` value shall be set to `false` and `socket_async_create` shall return _FAILURE_. **]**


###   socket_async_send
`socket_async_send` attempts to send `size` bytes from `buffer` to the underlying socket.

If this method fails then `socket_async_destroy` must be called. This includes the case of NULL `buffer` and `sent_count` parameters, which are assumed to be unrecoverable program bugs.

Validation of the `sock` parameter is deferred to the underlying socket call.

The function signature for `socket_async_send` is a pass-through for the underlying socket `send()` call, and the parameter types are identical for the two calls.

```c
int socket_async_send(SOCKET_ASYNC_HANDLE sock, void* buffer, size_t size, size_t* sent_count);
```

**SRS_SOCKET_ASYNC_30_030: [** The `sock` parameter shall be the socket to send the message to. **]**

**SRS_SOCKET_ASYNC_30_031: [** The `buffer` parameter shall contain the message to send to the target server. **]**

**SRS_SOCKET_ASYNC_30_032: [** The `size` parameter shall be the size of the message in bytes. **]**

**SRS_SOCKET_ASYNC_30_033: [** If the `buffer` parameter is NULL, `socket_async_send` shall log the error return _FAILURE_. **]**

**SRS_SOCKET_ASYNC_30_034: [** If the `sent_count` parameter is NULL, `socket_async_send` shall log the error return _FAILURE_. **]**

**SRS_SOCKET_ASYNC_30_073: [** If the `size` parameter is 0, `socket_async_send` shall set `sent_count` to 0 and return 0. **]**

**SRS_SOCKET_ASYNC_30_035: [** If the underlying socket accepts one or more bytes for transmission, `socket_async_send` shall return 0 and the `sent_count` parameter shall receive the number of bytes accepted for transmission. **]**

**SRS_SOCKET_ASYNC_30_036: [** If the underlying socket is unable to accept any bytes for transmission because its buffer is full, `socket_async_send` shall return 0 and the `sent_count` parameter shall receive the value 0. **]**

**SRS_SOCKET_ASYNC_30_037: [** If `socket_async_send` fails unexpectedly, `socket_async_send` shall log the error return _FAILURE_. **]**

###   socket_async_receive
`socket_async_receive` attempts to receive up to `size` bytes into `buffer`.

If this method fails then `socket_async_destroy` must be called. This includes the case of NULL `buffer` and `received_count` parameters, which are assumed to be unrecoverable program bugs.

Validation of the `sock` parameter is deferred to the underlying socket call.

The function signature for `socket_async_receive` is a pass-through for the underlying socket `send()` call, and the parameter types are identical for the two calls.

```c
int socket_async_receive(SOCKET_ASYNC_HANDLE sock, void* buffer, size_t size, size_t* received_count);
```

**SRS_SOCKET_ASYNC_30_050: [** The `sock` parameter shall be the socket to receive the message from. **]**

**SRS_SOCKET_ASYNC_30_051: [** The `size` parameter shall be the size of the `buffer` in bytes. **]**

**SRS_SOCKET_ASYNC_30_052: [** If the `buffer` parameter is NULL, `socket_async_receive` shall log the error and return _FAILURE_. **]**

**SRS_SOCKET_ASYNC_30_053: [** If the `received_count` parameter is NULL, `socket_async_receive` shall log the error and return _FAILURE_. **]**

**SRS_SOCKET_ASYNC_30_072: [** If the `size` parameter is 0, `socket_async_receive` shall log an error and return _FAILURE_. **]**

**SRS_SOCKET_ASYNC_30_054: [** On success, the underlying socket shall set one or more received bytes into  `buffer`, `socket_async_receive` shall return 0, and the `received_count` parameter shall receive the number of bytes received into `buffer`. **]**

**SRS_SOCKET_ASYNC_30_055: [** If the underlying socket has no received bytes available, `socket_async_receive` shall return 0 and the `received_count` parameter shall receive the value 0. **]**

**SRS_SOCKET_ASYNC_30_056: [** If the underlying socket fails unexpectedly, `socket_async_receive` shall log the error and return _FAILURE_. **]**


 ###   socket_async_destroy
 `socket_async_destroy` calls the underlying socket `close()` on the supplied socket. Parameter validation is deferred to the underlying call, so no validation is performed by `socket_async_destroy`.

 ```c
 void socket_async_destroy(SOCKET_ASYNC_HANDLE sock);
 ```

**SRS_SOCKET_ASYNC_30_070: [** The `sock` parameter shall be the SOCKET_ASYNC_HANDLE of the socket to be closed. **]**  

**SRS_SOCKET_ASYNC_30_071: [** `socket_async_destroy` shall call the underlying `close` method on the supplied socket. **]**  
