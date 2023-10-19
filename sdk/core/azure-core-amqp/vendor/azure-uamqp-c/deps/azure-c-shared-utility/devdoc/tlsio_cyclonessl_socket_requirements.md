tlsio_cyclonessl_socket
=============

## Overview

tlsio_cyclonessl_socket provides an implementation for creating/destroying Cyclone TCP sockets to be used by the CycloneSSL TLS library.

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

**SRS_TLSIO_CYCLONESSL_SOCKET_01_001: [** tlsio_cyclonessl_socket_create shall create a new socket to be used by CycloneSSL. **]**

**SRS_TLSIO_CYCLONESSL_SOCKET_01_002: [** On success tlsio_cyclonessl_socket_create shall return 0 and fill in the socket handle in the new_socket out argument. **]**

**SRS_TLSIO_CYCLONESSL_SOCKET_01_003: [** If hostname or socket is NULL, then tlsio_cyclonessl_socket_create shall fail and it shall return a non-zero value. **]**

**SRS_TLSIO_CYCLONESSL_SOCKET_01_004: [** tlsio_cyclonessl_socket_create shall call socketOpen to create a TCP socket. **]**

**SRS_TLSIO_CYCLONESSL_SOCKET_01_005: [** tlsio_cyclonessl_socket_create shall call getHostByName to obtain an IpAddr structure filled with the address of the hostname. **]**

**SRS_TLSIO_CYCLONESSL_SOCKET_01_006: [** tlsio_cyclonessl_socket_create shall call socketConnect and pass the obtained address in order to connect the socket. **]**

**SRS_TLSIO_CYCLONESSL_SOCKET_01_007: [** If any of the socket calls fails, then tlsio_cyclonessl_socket_create shall fail and return a non-zero value. **]**

###  tlsio_cyclonessl_socket_destroy

```c
void tlsio_cyclonessl_socket_destroy(TlsSocket socket)
```

**SRS_TLSIO_CYCLONESSL_SOCKET_01_008: [** tlsio_cyclonessl_socket_destroy shall close the socket passed as argument by calling the function socketClose. **]**

**SRS_TLSIO_CYCLONESSL_SOCKET_01_009: [** If socket is NULL, tlsio_cyclonessl_socket_destroy shall do nothing. **]**