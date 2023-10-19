http_proxy_io
=============

## Overview

http_proxy_io implements an IO that has minimal functionality allowing communication over an HTTP proxy.

## References

[RFC 2616](https://tools.ietf.org/html/rfc2616)
[RFC 2617](https://tools.ietf.org/html/rfc2617)
[RFC 2817](https://www.ietf.org/rfc/rfc2817.txt)

## Exposed API

```c
MOCKABLE_FUNCTION(, const IO_INTERFACE_DESCRIPTION*, http_proxy_io_get_interface_description);
```


###  http_proxy_io_create

`http_proxy_io_create` is the implementation provided via `http_proxy_io_get_interface_description` for the `concrete_io_create` member.

```c
CONCRETE_IO_HANDLE http_proxy_io_create(void* io_create_parameters);
```

**SRS_HTTP_PROXY_IO_01_001: [** `http_proxy_io_create` shall create a new instance of the HTTP proxy IO. **]**

**SRS_HTTP_PROXY_IO_01_002: [** If `io_create_parameters` is NULL, `http_proxy_io_create` shall fail and return NULL. **]**

**SRS_HTTP_PROXY_IO_01_003: [** `io_create_parameters` shall be used as an `HTTP_PROXY_IO_CONFIG*`. **]**

**SRS_HTTP_PROXY_IO_01_004: [** If the `hostname` or `proxy_hostname` member is NULL, then `http_proxy_io_create` shall fail and return NULL. **]**

**SRS_HTTP_PROXY_IO_01_005: [** `http_proxy_io_create` shall copy the `hostname`, `port`, `username` and `password` values for later use when the actual CONNECT is performed. **]**

**SRS_HTTP_PROXY_IO_01_006: [** `hostname` and `proxy_hostname`, `username` and `password` shall be copied by calling `mallocAndStrcpy_s`. **]**

**SRS_HTTP_PROXY_IO_01_094: [** `username` and `password` shall be optional. **]**

**SRS_HTTP_PROXY_IO_01_095: [** If one of the fields `username` and `password` is non-NULL, then the other has to be also non-NULL, otherwise `http_proxy_io_create` shall fail and return NULL. **]**

**SRS_HTTP_PROXY_IO_01_007: [** If `mallocAndStrcpy_s` fails then `http_proxy_io_create` shall fail and return NULL. **]**

**SRS_HTTP_PROXY_IO_01_009: [** `http_proxy_io_create` shall create a new socket IO by calling `xio_create` with the arguments: **]**

**SRS_HTTP_PROXY_IO_01_010: [** - `io_interface_description` shall be set to the result of `socketio_get_interface_description`. **]**

**SRS_HTTP_PROXY_IO_01_011: [** - `xio_create_parameters` shall be set to a `SOCKETIO_CONFIG*` where `hostname` is set to the `proxy_hostname` member of `io_create_parameters` and `port` is set to the `proxy_port` member of `io_create_parameters`. **]**

**SRS_HTTP_PROXY_IO_01_050: [** If `socketio_get_interface_description` fails, `http_proxy_io_create` shall fail and return NULL. **]**

**SRS_HTTP_PROXY_IO_01_012: [** If `xio_create` fails, `http_proxy_io_create` shall fail and return NULL. **]**

**SRS_HTTP_PROXY_IO_01_008: [** When `http_proxy_io_create` fails, all allocated resources up to that point shall be freed. **]**

###  http_proxy_io_destroy

`http_proxy_io_destroy` is the implementation provided via `http_proxy_io_get_interface_description` for the `concrete_io_destroy` member.

```c
void http_proxy_io_destroy(CONCRETE_IO_HANDLE http_proxy_io);
```

**SRS_HTTP_PROXY_IO_01_013: [** `http_proxy_io_destroy` shall free the HTTP proxy IO instance indicated by `http_proxy_io`. **]**

**SRS_HTTP_PROXY_IO_01_014: [** If `http_proxy_io` is NULL, `http_proxy_io_destroy` shall do nothing. **]**

**SRS_HTTP_PROXY_IO_01_015: [** `http_proxy_io_destroy` should close the IO if it was open before freeing all the resources. **]**

**SRS_HTTP_PROXY_IO_01_016: [** `http_proxy_io_destroy` shall destroy the underlying IO created in `http_proxy_io_create` by calling `xio_destroy`. **]**

###  http_proxy_io_open

`http_proxy_io_open` is the implementation provided via `http_proxy_io_get_interface_description` for the `concrete_io_open` member.

```c
int http_proxy_io_open(CONCRETE_IO_HANDLE http_proxy_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
```

**SRS_HTTP_PROXY_IO_01_017: [** `http_proxy_io_open` shall open the HTTP proxy IO and on success it shall return 0. **]**

**SRS_HTTP_PROXY_IO_01_018: [** If any of the arguments `http_proxy_io`, `on_io_open_complete`, `on_bytes_received` or `on_io_error` are NULL then `http_proxy_io_open` shall return a non-zero value. **]**

**SRS_HTTP_PROXY_IO_01_019: [** `http_proxy_io_open` shall open the underlying IO by calling `xio_open` on the underlying IO handle created in `http_proxy_io_create`, while passing to it the callbacks `on_underlying_io_open_complete`, `on_underlying_io_bytes_received` and `on_underlying_io_error`. **]**

**SRS_HTTP_PROXY_IO_01_020: [** If `xio_open` fails, then `http_proxy_io_open` shall return a non-zero value. **]**

**SRS_HTTP_PROXY_IO_01_021: [** If `http_proxy_io_open` is called while the IO was already open, `http_proxy_io_open` shall fail and return a non-zero value. **]**

**SRS_HTTP_PROXY_IO_01_051: [** The arguments `on_io_open_complete_context`, `on_bytes_received_context` and `on_io_error_context` shall be allowed to be NULL. **]**

###  http_proxy_io_close

`http_proxy_io_close` is the implementation provided via `http_proxy_io_get_interface_description` for the `concrete_io_close` member.

```c
int http_proxy_io_close(CONCRETE_IO_HANDLE http_proxy_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* on_io_close_complete_context)
```

**SRS_HTTP_PROXY_IO_01_022: [** `http_proxy_io_close` shall close the HTTP proxy IO and on success it shall return 0. **]**

**SRS_HTTP_PROXY_IO_01_023: [** If the argument `http_proxy_io` is NULL, `http_proxy_io_close` shall fail and return a non-zero value. **]**

**SRS_HTTP_PROXY_IO_01_024: [** `http_proxy_io_close` shall close the underlying IO by calling `xio_close` on the IO handle create in `http_proxy_io_create`, while passing to it the `on_underlying_io_close_complete` callback. **]**

**SRS_HTTP_PROXY_IO_01_025: [** If `xio_close` fails, `http_proxy_io_close` shall fail and return a non-zero value. **]**

**SRS_HTTP_PROXY_IO_01_026: [** The `on_io_close_complete` and `on_io_close_complete_context` arguments shall be saved for later use. **]**

**SRS_HTTP_PROXY_IO_01_027: [** If `http_proxy_io_close` is called when not open, `http_proxy_io_close` shall fail and return a non-zero value. **]**

**SRS_HTTP_PROXY_IO_01_028: [** `on_io_close_complete` shall be allowed to be NULL. **]**

**SRS_HTTP_PROXY_IO_01_052: [** `on_io_close_complete_context` shall be allowed to be NULL. **]**

**SRS_HTTP_PROXY_IO_01_053: [** `http_proxy_io_close` while OPENING shall trigger the `on_io_open_complete` callback with `IO_OPEN_CANCELLED`. **]**

**SRS_HTTP_PROXY_IO_01_054: [** `http_proxy_io_close` while OPENING shall fail and return a non-zero value. **]**

###  http_proxy_io_send

`http_proxy_io_send` is the implementation provided via `http_proxy_io_get_interface_description` for the `concrete_io_send` member.

```c
int http_proxy_io_send(CONCRETE_IO_HANDLE http_proxy_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* on_send_complete_context)
```

**SRS_HTTP_PROXY_IO_01_029: [** `http_proxy_io_send` shall send the `size` bytes pointed to by `buffer` and on success it shall return 0. **]**

**SRS_HTTP_PROXY_IO_01_030: [** If any of the arguments `http_proxy_io` or `buffer` is NULL, `http_proxy_io_send` shall fail and return a non-zero value. **]**

**SRS_HTTP_PROXY_IO_01_031: [** If `size` is 0, `http_proxy_io_send` shall fail and return a non-zero value. **]**

**SRS_HTTP_PROXY_IO_01_032: [** `on_send_complete` shall be allowed to be NULL. **]**

**SRS_HTTP_PROXY_IO_01_033: [** `http_proxy_io_send` shall send the bytes by calling `xio_send` on the underlying IO created in `http_proxy_io_create` and passing `buffer` and `size` as arguments. **]**

**SRS_HTTP_PROXY_IO_01_034: [** If `http_proxy_io_send` is called when the IO is not open, `http_proxy_io_send` shall fail and return a non-zero value. **]**

**SRS_HTTP_PROXY_IO_01_035: [** If the IO is in an error state (an error was reported through the `on_io_error` callback), `http_proxy_io_send` shall fail and return a non-zero value. **]**

**SRS_HTTP_PROXY_IO_01_055: [** If `xio_send` fails, `http_proxy_io_send` shall fail and return a non-zero value. **]**

###  http_proxy_io_dowork

`http_proxy_io_dowork` is the implementation provided via `http_proxy_io_get_interface_description` for the `concrete_io_dowork` member.

```c
void http_proxy_io_dowork(CONCRETE_IO_HANDLE http_proxy_io)
```

**SRS_HTTP_PROXY_IO_01_037: [** `http_proxy_io_dowork` shall call `xio_dowork` on the underlying IO created in `http_proxy_io_create`. **]**

**SRS_HTTP_PROXY_IO_01_038: [** If the `http_proxy_io` argument is NULL, `http_proxy_io_dowork` shall do nothing. **]**

**SRS_HTTP_PROXY_IO_01_039: [** If the IO is not open (no open has been called or the IO has been closed) then `http_proxy_io_dowork` shall do nothing. **]**

###  http_proxy_io_set_option

`http_proxy_io_set_option` is the implementation provided via `http_proxy_io_get_interface_description` for the `concrete_io_setoption` member.

```c
int http_proxy_io_set_option(CONCRETE_IO_HANDLE http_proxy_io, const char* optionName, const void* value)
```

**SRS_HTTP_PROXY_IO_01_040: [** If any of the arguments `http_proxy_io` or `option_name` is NULL, `http_proxy_io_set_option` shall return a non-zero value. **]**

**SRS_HTTP_PROXY_IO_01_042: [** If the option was handled by `http_proxy_io_set_option` or the underlying IO, then `http_proxy_io_set_option` shall return 0. **]**

**SRS_HTTP_PROXY_IO_01_043: [** If the `option_name` argument indicates an option that is not handled by `http_proxy_io_set_option`, then `xio_setoption` shall be called on the underlying IO created in `http_proxy_io_create`, passing the option name and value to it. **]**

**SRS_HTTP_PROXY_IO_01_044: [** if `xio_setoption` fails, `http_proxy_io_set_option` shall return a non-zero value. **]**

**SRS_HTTP_PROXY_IO_01_056: [** The `value` argument shall be allowed to be NULL. **]**

Options that shall be handled by HTTP proxy IO:

**SRS_HTTP_PROXY_IO_01_045: [** None. **]**

###  http_proxy_io_retrieve_options

`http_proxy_io_retrieve_options` is the implementation provided via `http_proxy_io_get_interface_description` for the `concrete_io_retrieveoptions` member.

```c
static OPTIONHANDLER_HANDLE http_proxy_io_retrieve_options(CONCRETE_IO_HANDLE http_proxy_io)
```

**SRS_HTTP_PROXY_IO_01_046: [** `http_proxy_io_retrieve_options` shall return an `OPTIONHANDLER_HANDLE` obtained by calling `xio_retrieveoptions` on the underlying IO created in `http_proxy_io_create`. **]**

**SRS_HTTP_PROXY_IO_01_047: [** If the parameter `http_proxy_io` is NULL then `http_proxy_io_retrieve_options` shall fail and return NULL. **]**

**SRS_HTTP_PROXY_IO_01_048: [** If `xio_retrieveoptions` fails, `http_proxy_io_retrieve_options` shall return NULL. **]**

###  http_proxy_io_get_interface_description

```c
extern const IO_INTERFACE_DESCRIPTION* http_proxy_io_get_interface_description(void);
```

**SRS_HTTP_PROXY_IO_01_049: [** `http_proxy_io_get_interface_description` shall return a pointer to an `IO_INTERFACE_DESCRIPTION` structure that contains pointers to the functions: `http_proxy_io_retrieve_options`, `http_proxy_io_retrieve_create`, `http_proxy_io_destroy`, `http_proxy_io_open`, `http_proxy_io_close`, `http_proxy_io_send` and `http_proxy_io_dowork`. **]**

###  on_underlying_io_open_complete

**SRS_HTTP_PROXY_IO_01_057: [** When `on_underlying_io_open_complete` is called, the `http_proxy_io` shall send the CONNECT request constructed per RFC 2817: **]**

**SRS_HTTP_PROXY_IO_01_078: [** When `on_underlying_io_open_complete` is called with `IO_OPEN_ERROR`, the `on_open_complete` callback shall be triggered with `IO_OPEN_ERROR`, passing also the `on_open_complete_context` argument as `context`. **]**

**SRS_HTTP_PROXY_IO_01_079: [** When `on_underlying_io_open_complete` is called with `IO_OPEN_CANCELLED`, the `on_open_complete` callback shall be triggered with `IO_OPEN_CANCELLED`, passing also the `on_open_complete_context` argument as `context`. **]**

**SRS_HTTP_PROXY_IO_01_059: [** - If `username` and `password` have been specified in the arguments passed to `http_proxy_io_create`, then the header `Proxy-Authorization` shall be added to the request. **]**

**SRS_HTTP_PROXY_IO_01_060: [** - The value of `Proxy-Authorization` shall be the constructed according to RFC 2617. **]**

**SRS_HTTP_PROXY_IO_01_061: [** Encoding to Base64 shall be done by calling `Azure_Base64_Encode_Bytes`. **]**

**SRS_HTTP_PROXY_IO_01_062: [** If any failure is encountered while constructing the request, the `on_open_complete` callback shall be triggered with `IO_OPEN_ERROR`, passing also the `on_open_complete_context` argument as `context`. **]**

**SRS_HTTP_PROXY_IO_01_063: [** The request shall be sent by calling `xio_send` and passing NULL as `on_send_complete` callback. **]**

**SRS_HTTP_PROXY_IO_01_064: [** If `xio_send` fails, the `on_open_complete` callback shall be triggered with `IO_OPEN_ERROR`, passing also the `on_open_complete_context` argument as `context`. **]**

**SRS_HTTP_PROXY_IO_01_076: [** When `on_underlying_io_open_complete` is called while waiting for the CONNECT reply, the `on_open_complete` callback shall be triggered with `IO_OPEN_ERROR`, passing also the `on_open_complete_context` argument as `context`. **]**

**SRS_HTTP_PROXY_IO_01_077: [** When `on_underlying_io_open_complete` is called in after OPEN has completed, the `on_io_error` callback shall be triggered passing the `on_io_error_context` argument as `context`. **]**

**SRS_HTTP_PROXY_IO_01_081: [** `on_underlying_io_open_complete` called with NULL context shall do nothing. **]**

###  on_underlying_io_bytes_received

**SRS_HTTP_PROXY_IO_01_065: [** When bytes are received and the response to the CONNECT request was not yet received, the bytes shall be accumulated until a double new-line is detected. **]**

**SRS_HTTP_PROXY_IO_01_066: [** When a double new-line is detected the response shall be parsed in order to extract the status code. **]**

**SRS_HTTP_PROXY_IO_01_067: [** If allocating memory for the buffered bytes fails, the `on_open_complete` callback shall be triggered with `IO_OPEN_ERROR`, passing also the `on_open_complete_context` argument as `context`. **]**

**SRS_HTTP_PROXY_IO_01_068: [** If parsing the CONNECT response fails, the `on_open_complete` callback shall be triggered with `IO_OPEN_ERROR`, passing also the `on_open_complete_context` argument as `context`. **]**

**SRS_HTTP_PROXY_IO_01_069: [** Any successful (2xx) response to a CONNECT request indicates that the proxy has established a connection to the requested host and port, and has switched to tunneling the current connection to that server connection. **]**

**SRS_HTTP_PROXY_IO_01_070: [** When a success status code is parsed, the `on_open_complete` callback shall be triggered with `IO_OPEN_OK`, passing also the `on_open_complete_context` argument as `context`. **]**

**SRS_HTTP_PROXY_IO_01_071: [** If the status code is not successful, the `on_open_complete` callback shall be triggered with `IO_OPEN_ERROR`, passing also the `on_open_complete_context` argument as `context`. **]**

**SRS_HTTP_PROXY_IO_01_072: [** Any bytes that are extra (not consumed by the CONNECT response), shall be indicated as received by calling the `on_bytes_received` callback and passing the `on_bytes_received_context` as context argument. **]**

**SRS_HTTP_PROXY_IO_01_073: [** Once a success status code was parsed, the IO shall be OPEN. **]**

**SRS_HTTP_PROXY_IO_01_074: [** If `on_underlying_io_bytes_received` is called while OPEN, all bytes shall be indicated as received by calling the `on_bytes_received` callback and passing the `on_bytes_received_context` as context argument. **]**

**SRS_HTTP_PROXY_IO_01_080: [** If `on_underlying_io_bytes_received` is called while the underlying IO is being opened, the `on_open_complete` callback shall be triggered with `IO_OPEN_ERROR`, passing also the `on_open_complete_context` argument as `context`. **]**

**SRS_HTTP_PROXY_IO_01_082: [** `on_underlying_io_bytes_received` called with NULL context shall do nothing. **]**

###  on_underlying_io_close_complete

**SRS_HTTP_PROXY_IO_01_083: [** `on_underlying_io_close_complete` while CLOSING shall call the `on_io_close_complete` callback, passing to it the `on_io_close_complete_context` as `context` argument. **]**

**SRS_HTTP_PROXY_IO_01_084: [** `on_underlying_io_close_complete` called with NULL context shall do nothing. **]**

**SRS_HTTP_PROXY_IO_01_086: [** If the `on_io_close_complete` callback passed to `http_proxy_io_close` was NULL, no callback shall be triggered. **]**

###  on_underlying_io_error

**SRS_HTTP_PROXY_IO_01_087: [** If the `on_underlying_io_error` callback is called while OPENING, the `on_open_complete` callback shall be triggered with `IO_OPEN_ERROR`, passing also the `on_open_complete_context` argument as `context`. **]**

**SRS_HTTP_PROXY_IO_01_088: [** `on_underlying_io_error` called with NULL context shall do nothing. **]**

**SRS_HTTP_PROXY_IO_01_089: [** If the `on_underlying_io_error` callback is called while the IO is OPEN, the `on_io_error` callback shall be called with the `on_io_error_context` argument as `context`. **]**

## RFC 2817 relevant part

5.2 Requesting a Tunnel with CONNECT

   A CONNECT method requests that a proxy establish a tunnel connection on its behalf.
   **SRS_HTTP_PROXY_IO_01_075: [** The Request-URI portion of the Request-Line is always an 'authority' as defined by URI Generic Syntax [2], which is to say the host name and port number destination of the requested connection separated by a colon: **]**

      CONNECT server.example.com:80 HTTP/1.1
      Host: server.example.com:80

   Other HTTP mechanisms can be used normally with the CONNECT method -- except end-to-end protocol Upgrade requests, of course, since the tunnel must be established first.

   For example, proxy authentication might be used to establish the authority to create a tunnel:

      CONNECT server.example.com:80 HTTP/1.1
      Host: server.example.com:80
      Proxy-Authorization: basic aGVsbG86d29ybGQ=

   Like any other pipelined HTTP/1.1 request, data to be tunneled may be sent immediately after the blank line.
   The usual caveats also apply: data may be discarded if the eventual response is negative, and the connection may be reset with no response if more than one TCP segment is outstanding.

5.3 Establishing a Tunnel with CONNECT

   **SRS_HTTP_PROXY_IO_01_090: [** Any successful (2xx) response to a CONNECT request indicates that the proxy has established a connection to the requested host and port, and has switched to tunneling the current connection to that server connection. **]**

   It may be the case that the proxy itself can only reach the requested origin server through another proxy.
   In this case, the first proxy SHOULD make a CONNECT request of that next proxy, requesting a tunnel to the authority.
   A proxy MUST NOT respond with any 2xx status code unless it has either a direct or tunnel connection established to the authority.

   An origin server which receives a CONNECT request for itself MAY respond with a 2xx status code to indicate that a connection is established.

   If at any point either one of the peers gets disconnected, any outstanding data that came from that peer will be passed to the other one, and after that also the other connection will be terminated by the proxy.
   If there is outstanding data to that peer undelivered, that data will be discarded.

## RFC 2617 relevant part

 Basic Authentication Scheme

   The "basic" authentication scheme is based on the model that the client must authenticate itself with a user-ID and a password for each realm.
   The realm value should be considered an opaque string which can only be compared for equality with other realms on that server.
   The server will service the request only if it can validate the user-ID and password for the protection space of the Request-URI.
   There are no optional authentication parameters.

   For Basic, the framework above is utilized as follows:

      challenge   = "Basic" realm
      credentials = "Basic" basic-credentials

   Upon receipt of an unauthorized request for a URI within the protection space, the origin server MAY respond with a challenge like the following:

      WWW-Authenticate: Basic realm="WallyWorld"

   where "WallyWorld" is the string assigned by the server to identify the protection space of the Request-URI.
   A proxy may respond with the same challenge using the Proxy-Authenticate header field.

   **SRS_HTTP_PROXY_IO_01_091: [** To receive authorization, the client sends the userid and password, separated by a single colon (":") character, within a base64 [7] encoded string in the credentials. **]**

      basic-credentials = base64-user-pass
      base64-user-pass  = <base64 [4] encoding of user-pass,
                       except not limited to 76 char/line>
      user-pass   = userid ":" password
      userid      = *<TEXT excluding ":">
[SuppressMessage("Microsoft.Security", "CS002:SecretInNextLine", Justification="Not a password")]
      password    = *TEXT

   **SRS_HTTP_PROXY_IO_01_093: [** Userids might be case sensitive. **]**

   If the user agent wishes to send the userid "Aladdin" and password "open sesame", it would use the following header field:

      Authorization: Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==

   A client SHOULD assume that all paths at or deeper than the depth of the last symbolic element in the path field of the Request-URI also are within the protection space specified by the Basic realm value of the current challenge.
   
   **SRS_HTTP_PROXY_IO_01_092: [** A client MAY preemptively send the corresponding Authorization header with requests for resources in that space without receipt of another challenge from the server. **]**
   Similarly, when a client sends a request to a proxy, it may reuse a userid and password in the Proxy-Authorization header field without receiving another challenge from the proxy server.
   See section 4 for security considerations associated with Basic authentication.
