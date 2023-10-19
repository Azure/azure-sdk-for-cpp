tlsio_cyclonessl_socket
=============

## Overview

tlsio_cyclonessl_socket provides an implementation for creating/destroying BSD sockets to be used by the CycloneSSL TLS library.

## References

[CycloneSSL official page](http://www.oryx-embedded.com/cyclone_ssl.html)

## Exposed API

```c
MOCKABLE_FUNCTION(, int, tlsio_cyclonessl_socket_create, const char*, hostname, int, port, TlsSocket* new_socket);
MOCKABLE_FUNCTION(, void, tlsio_cyclonessl_socket_destroy, TlsSocket, socket);
```

###  tlsio_cyclonessl_socket_create

```c
int tlsio_cyclonessl_socket_create(const char* hostname, int port, TlsSocket* new_socket);
```

**SRS_TLSIO_CYCLONESSL_SOCKET_BSD_01_001: [** tlsio_cyclonessl_socket_create shall create a new socket to be used by CycloneSSL. **]**

**SRS_TLSIO_CYCLONESSL_SOCKET_BSD_01_008: [** On success tlsio_cyclonessl_socket_create shall return 0 and fill in the socket handle in the new_socket out argument. **]**

**SRS_TLSIO_CYCLONESSL_SOCKET_BSD_01_002: [** If hostname or new_socket is NULL, then tlsio_cyclonessl_socket_create shall fail and it shall return a non-zero value. **]**

**SRS_TLSIO_CYCLONESSL_SOCKET_BSD_01_003: [** tlsio_cyclonessl_socket_create shall call socket to create a TCP socket. **]**

**SRS_TLSIO_CYCLONESSL_SOCKET_BSD_01_004: [** tlsio_cyclonessl_socket_create shall call getaddrinfo to obtain a hint ADDRINFO. **]**

**SRS_TLSIO_CYCLONESSL_SOCKET_BSD_01_006: [** tlsio_cyclonessl_socket_create shall call connect and pass the constructed address in order to connect the socket. **]**

**SRS_TLSIO_CYCLONESSL_SOCKET_BSD_01_007: [** If any of the socket calls fails, then tlsio_cyclonessl_socket_create shall fail and return a non-zero value. **]**

###  tlsio_cyclonessl_socket_destroy

```c
void tlsio_cyclonessl_socket_destroy(TlsSocket socket)
```

**SRS_TLSIO_CYCLONESSL_SOCKET_BSD_01_009: [** tlsio_cyclonessl_socket_destroy shall close the socket passed as argument by calling the function close. **]**

**SRS_TLSIO_CYCLONESSL_SOCKET_BSD_01_010: [** If socket is INVALID_SOCKET (-1), tlsio_cyclonessl_socket_destroy shall do nothing. **]** 