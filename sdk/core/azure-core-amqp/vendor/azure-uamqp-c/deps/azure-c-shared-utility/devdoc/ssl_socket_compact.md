ssl_socket_compact
=================

## Overview

**ssl_socket_compact** creates and destroys a non-blocking TCP socket suitable for use
with SSL implementations that require a TCP socket. The compact (for microcontrollers) implementation of OpenSSL
is one example. The socket handling code in `ssl_socket_compact.c` is common to many environments, and
preprocessor switches enable the proper headers for a particular environment.

Socket creation, configuration,binding, and connection are all performed
in the `SSL_Socket_Create` call defined in `ssl_socket.h`.

## References

[ssl_socket.h](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/ssl_socket.h)  
[sys/socket.h, a typical linux socket implementation](http://pubs.opengroup.org/onlinepubs/7908799/xns/syssocket.h.html)

###   Exposed API

**SRS_SSL_SOCKET_COMPACT_30_001: [** The ssl_socket_compact shall use the constants defined in `ssl_socket.h`.
```c
#define AZURE_SSL_SOCKET_SO_KEEPALIVE 1    /* enable keepalive */
#define AZURE_SSL_SOCKET_TCP_KEEPIDLE 20   /* seconds */
#define AZURE_SSL_SOCKET_TCP_KEEPINTVL 2   /* seconds */
#define AZURE_SSL_SOCKET_TCP_KEEPCNT 3     /* retry count */
```
 **]**

**SRS_SSL_SOCKET_COMPACT_30_001: [** The ssl_socket_compact shall implement the methods defined in `ssl_socket.h`.
```c
int SSL_Socket_Create(const char* hostname, int port);
void SSL_Socket_Close(int sock);
```
 **]**


###   SSL_Socket_Create
`SSL_Socket_Create` creates a socket and sets its configuration, including setting the socket to non-blocking. It then binds the socket to the supplied `hostname` and `port`, and finally connects the socket to the bound address.

If successful, it returns a non-negative integer to represent the socket's file descriptor. On failure, it returns -1.

```c
int SSL_Socket_Create(const char* hostname, int port);
```

**SRS_SSL_SOCKET_COMPACT_30_002: [** The `hostname` parameter shall be the fully-qualified domain name (FQDN) of the target server. Example: azure-iot-team.azure-devices.net **]**

**SRS_SSL_SOCKET_COMPACT_30_003: [** The `port` shall be the TCP port number for the target server. **]**

**SRS_SSL_SOCKET_COMPACT_30_004: [** The `SO_KEEPALIVE` option value of the returned socket shall be `AZURE_SSL_SOCKET_SO_KEEPALIVE`. **]**

**SRS_SSL_SOCKET_COMPACT_30_005: [** The `TCP_KEEPIDLE` option value of the returned socket shall be `AZURE_SSL_SOCKET_TCP_KEEPIDLE`. **]**

**SRS_SSL_SOCKET_COMPACT_30_006: [** The `TCP_KEEPINTVL` option value of the returned socket shall be `AZURE_SSL_SOCKET_TCP_KEEPINTVL`. **]**

**SRS_SSL_SOCKET_COMPACT_30_007: [** The `TCP_KEEPCNT` option value of the returned socket shall be `AZURE_SSL_SOCKET_TCP_KEEPCNT`. **]**

**SRS_SSL_SOCKET_COMPACT_30_008: [** The returned socket shall be set to `O_NONBLOCK`. **]**

**SRS_SSL_SOCKET_COMPACT_30_009: [** If the `hostname` cannot be resolved by DNS lookup, `SSL_Socket_Create` shall return -1. **]**

**SRS_SSL_SOCKET_COMPACT_30_010: [** If socket binding fails, `SSL_Socket_Create` shall return -1. **]**

**SRS_SSL_SOCKET_COMPACT_30_011: [** If socket connection fails, `SSL_Socket_Create` shall return -1. **]**


 ###   SSL_Socket_Close
 `SSL_Socket_Close` calls the underlying socket `close()` on the supplied socket. Parameter validation is deferred to the underlying call, so no validation is performed by `SSL_Socket_Close`.

 ```c
 void SSL_Socket_Close(int sock);
 ```

  **SRS_SSL_SOCKET_COMPACT_30_012: [** The `sock` parameter shall be the integer file descriptor of the socket to be closed. **]**  
  
 **SRS_SSL_SOCKET_COMPACT_30_013: [** `SSL_Socket_Close` shall call the underlying `close` method on the supplied socket. **]**  
