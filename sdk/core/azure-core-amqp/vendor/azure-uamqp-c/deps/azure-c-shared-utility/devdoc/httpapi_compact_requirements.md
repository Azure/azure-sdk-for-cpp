httpapi_compact
===============

## Overview

httpapi_compact implements a compact version of the HTTP (Hypertext Transfer Protocol).

## References

[http://](http://httpwg.org/)

## Exposed API

**SRS_HTTPAPI_COMPACT_21_001: [** The httpapi_compact shall implement the methods defined by the `httpapi.h`.
```c
/**
 * @brief	Global initialization for the HTTP API component.
 *
 *			Platform specific implementations are expected to initialize
 *			the underlying HTTP API stacks.
 *
 * @return	@c HTTPAPI_OK if initialization is successful or an error
 * 			code in case it fails.
 */
MOCKABLE_FUNCTION(, HTTPAPI_RESULT, HTTPAPI_Init);

/** @brief	Free resources allocated in ::HTTPAPI_Init. */
MOCKABLE_FUNCTION(, void, HTTPAPI_Deinit);

/**
 * @brief	Creates an HTTPS connection to the host specified by the @p
 * 			hostName parameter.
 *
 * @param	hostName	Name of the host.
 *
 *			This function returns a handle to the newly created connection.
 *			You can use the handle in subsequent calls to execute specific
 *			HTTP calls using ::HTTPAPI_ExecuteRequest.
 *
 * @return	A @c HTTP_HANDLE to the newly created connection or @c NULL in
 * 			case an error occurs.
 */
MOCKABLE_FUNCTION(, HTTP_HANDLE, HTTPAPI_CreateConnection, const char*, hostName);

/**
 * @brief	Closes a connection created with ::HTTPAPI_CreateConnection.
 *
 * @param	handle	The handle to the HTTP connection created via ::HTTPAPI_CreateConnection.
 *
 * 			All resources allocated by ::HTTPAPI_CreateConnection should be
 * 			freed in ::HTTPAPI_CloseConnection.
 */
MOCKABLE_FUNCTION(, void, HTTPAPI_CloseConnection, HTTP_HANDLE, handle);

/**
 * @brief	Sends the HTTP request to the host and handles the response for
 * 			the HTTP call.
 *
 * @param	handle				 	The handle to the HTTP connection created
 * 									via ::HTTPAPI_CreateConnection.
 * @param	requestType			 	Specifies which HTTP method is used (GET,
 * 									POST, DELETE, PUT, PATCH).
 * @param	relativePath		 	Specifies the relative path of the URL
 * 									excluding the host name.
 * @param	httpHeadersHandle	 	Specifies a set of HTTP headers (name-value
 * 									pairs) to be added to the
 * 									HTTP request. The @p httpHeadersHandle
 * 									handle can be created and setup with
 * 									the proper name-value pairs by using the
 * 									HTTPHeaders APIs available in @c
 * 									HTTPHeaders.h.
 * @param	content				 	Specifies a pointer to the request body.
 * 									This value is optional and can be @c NULL.
 * @param	contentLength		 	Specifies the request body size (this is
 * 									typically added into the HTTP headers as
 * 									the Content-Length header). This value is
 * 									optional and can be 0.
 * @param   statusCode   	        This is an out parameter, where
 * 									::HTTPAPI_ExecuteRequest returns the status
 * 									code from the HTTP response (200, 201, 400,
 * 									401, etc.)
 * @param	responseHeadersHandle	This is an HTTP headers handle to which
 * 									::HTTPAPI_ExecuteRequest shall add all the
 * 									HTTP response headers so that the caller of
 * 									::HTTPAPI_ExecuteRequest can inspect them.
 * 									You can manipulate @p responseHeadersHandle
 * 									by using the HTTPHeaders APIs available in
 * 									@c HTTPHeaders.h
 * @param	responseContent		 	This is a buffer that shall be filled by
 * 									::HTTPAPI_ExecuteRequest with the contents
 * 									of the HTTP response body. The buffer size
 * 									shall be increased by the
 * 									::HTTPAPI_ExecuteRequest implementation in
 * 									order to fit the response body.
 * 									::HTTPAPI_ExecuteRequest shall also handle
 * 									chunked transfer encoding for HTTP responses.
 * 									To manipulate the @p responseContent buffer,
 * 									use the APIs available in @c Strings.h.
 *
 * @return	@c HTTPAPI_OK if the API call is successful or an error
 * 			code in case it fails.
 */
MOCKABLE_FUNCTION(, HTTPAPI_RESULT, HTTPAPI_ExecuteRequest, HTTP_HANDLE, handle, HTTPAPI_REQUEST_TYPE, requestType, const char*, relativePath,
                                             HTTP_HEADERS_HANDLE, httpHeadersHandle, const unsigned char*, content,
                                             size_t, contentLength, unsigned int*, statusCode,
                                             HTTP_HEADERS_HANDLE, responseHeadersHandle, BUFFER_HANDLE, responseContent);

/**
 * @brief	Sets the option named @p optionName bearing the value
 * 			@p value for the HTTP_HANDLE @p handle.
 *
 * @param	handle	  	The handle to the HTTP connection created via
 * 						::HTTPAPI_CreateConnection.
 * @param	optionName	A @c NULL terminated string representing the name
 * 						of the option.
 * @param	value	  	A pointer to the value for the option.
 *
 * @return	@c HTTPAPI_OK if initialization is successful or an error
 * 			code in case it fails.
 */
MOCKABLE_FUNCTION(, HTTPAPI_RESULT, HTTPAPI_SetOption, HTTP_HANDLE, handle, const char*, optionName, const void*, value);

/**
 * @brief	Clones the option named @p optionName bearing the value @p value
 * 			into the pointer @p savedValue.
 *
 * @param	optionName	A @c NULL terminated string representing the name of
 * 						the option
 * @param	value	  	A pointer to the value of the option.
 * @param	savedValue	This pointer receives the copy of the value of the
 * 						option. The copy needs to be free-able.
 *
 * @return	@c HTTPAPI_OK if initialization is successful or an error
 * 			code in case it fails.
 */
MOCKABLE_FUNCTION(, HTTPAPI_RESULT, HTTPAPI_CloneOption, const char*, optionName, const void*, value, const void**, savedValue);
```
 **]**

**SRS_HTTPAPI_COMPACT_21_002: [** The httpapi_compact shall support the http requests:
```c
#define HTTPAPI_REQUEST_TYPE_VALUES \
    HTTPAPI_REQUEST_GET,            \
    HTTPAPI_REQUEST_POST,           \
    HTTPAPI_REQUEST_PUT,            \
    HTTPAPI_REQUEST_DELETE,         \
    HTTPAPI_REQUEST_PATCH           \

/** @brief Enumeration specifying the HTTP request verbs accepted by
 *	the HTTPAPI module.
 */
MU_DEFINE_ENUM(HTTPAPI_REQUEST_TYPE, HTTPAPI_REQUEST_TYPE_VALUES);
```
 **]**

**SRS_HTTPAPI_COMPACT_21_003: [** The httpapi_compact shall return error codes defined by HTTPAPI_RESULT:
```c
#define HTTPAPI_RESULT_VALUES                \
HTTPAPI_OK,                                  \
HTTPAPI_INVALID_ARG,                         \
HTTPAPI_ERROR,                               \
HTTPAPI_OPEN_REQUEST_FAILED,                 \
HTTPAPI_SET_OPTION_FAILED,                   \
HTTPAPI_SEND_REQUEST_FAILED,                 \
HTTPAPI_RECEIVE_RESPONSE_FAILED,             \
HTTPAPI_QUERY_HEADERS_FAILED,                \
HTTPAPI_QUERY_DATA_AVAILABLE_FAILED,         \
HTTPAPI_READ_DATA_FAILED,                    \
HTTPAPI_ALREADY_INIT,                        \
HTTPAPI_NOT_INIT,                            \
HTTPAPI_HTTP_HEADERS_FAILED,                 \
HTTPAPI_STRING_PROCESSING_ERROR,             \
HTTPAPI_ALLOC_FAILED,                        \
HTTPAPI_INIT_FAILED,                         \
HTTPAPI_INSUFFICIENT_RESPONSE_BUFFER,        \
HTTPAPI_SET_X509_FAILURE,                    \
HTTPAPI_SET_TIMEOUTS_FAILED                  \

/** @brief Enumeration specifying the possible return values for the APIs in
 *		   this module.
 */
MU_DEFINE_ENUM(HTTPAPI_RESULT, HTTPAPI_RESULT_VALUES);
```
 **]**


###   HTTPAPI_Init
```c
HTTPAPI_RESULT HTTPAPI_Init(void);
```

**SRS_HTTPAPI_COMPACT_21_004: [** The HTTPAPI_Init shall allocate all memory to control the http protocol. **]**

**SRS_HTTPAPI_COMPACT_21_006: [** If HTTPAPI_Init succeed allocating all the needed memory, it shall return HTTPAPI_OK. **]**

**SRS_HTTPAPI_COMPACT_21_007: [** If there is not enough memory to control the http protocol, the HTTPAPI_Init shall return HTTPAPI_ALLOC_FAILED. **]**  


###   HTTPAPI_Deinit
```c
void HTTPAPI_Deinit(void);
```

**SRS_HTTPAPI_COMPACT_21_009: [** The HTTPAPI_Init shall release all memory allocated by the httpapi_compact. **]**  


###   HTTPAPI_CreateConnection
```c
HTTP_HANDLE HTTPAPI_CreateConnection(const char* hostName);
```

**SRS_HTTPAPI_COMPACT_21_011: [** The HTTPAPI_CreateConnection shall create an http connection to the host specified by the hostName parameter. **]**

**SRS_HTTPAPI_COMPACT_21_012: [** The HTTPAPI_CreateConnection shall return a non-NULL handle on success. **]**

**SRS_HTTPAPI_COMPACT_21_013: [** If there is not enough memory to control the http connection, the HTTPAPI_CreateConnection shall return NULL as the handle. **]**

**SRS_HTTPAPI_COMPACT_21_014: [** If the hostName is NULL, the HTTPAPI_CreateConnection shall return NULL as the handle. **]**

**SRS_HTTPAPI_COMPACT_21_015: [** If the hostName is empty, the HTTPAPI_CreateConnection shall return NULL as the handle. **]**

**SRS_HTTPAPI_COMPACT_21_016: [** If the HTTPAPI_CreateConnection failed to create the connection, it shall return NULL as the handle. **]**  


###   HTTPAPI_CloseConnection
```c
void HTTPAPI_CloseConnection(HTTP_HANDLE handle);
```

**SRS_HTTPAPI_COMPACT_21_017: [** The HTTPAPI_CloseConnection shall close the connection previously created in HTTPAPI_ExecuteRequest. **]**

**SRS_HTTPAPI_COMPACT_21_076: [** After close the connection, The HTTPAPI_CloseConnection shall destroy the connection previously created in HTTPAPI_CreateConnection. **]**

**SRS_HTTPAPI_COMPACT_21_018: [** If there is a certificate associated to this connection, the HTTPAPI_CloseConnection shall free all allocated memory for the certificate. **]**

**SRS_HTTPAPI_COMPACT_06_001: [** If there is a x509 client certificate associated to this connection, the HTTAPI_CloseConnection shall free all allocated memory for the certificate. **]**

**SRS_HTTPAPI_COMPACT_06_002: [** If there is a x509 client private key associated to this connection, then HTTP_CloseConnection shall free all the allocated memory for the private key. **]**

**SRS_HTTPAPI_COMPACT_21_019: [** If there is no previous connection, the HTTPAPI_CloseConnection shall not do anything. **]**

**SRS_HTTPAPI_COMPACT_21_020: [** If the connection handle is NULL, the HTTPAPI_CloseConnection shall not do anything. **]**

**SRS_HTTPAPI_COMPACT_21_084: [** The HTTPAPI_CloseConnection shall wait, at least, 10 seconds for the SSL close process. **]**

**SRS_HTTPAPI_COMPACT_21_085: [** If the HTTPAPI_CloseConnection retries 10 seconds to close the connection without success, it shall destroy the connection anyway. **]**

**SRS_HTTPAPI_COMPACT_21_086: [** The HTTPAPI_CloseConnection shall wait, at least, 100 milliseconds between retries. **]**

**SRS_HTTPAPI_COMPACT_21_087: [** If the xio return anything different than 0, the HTTPAPI_CloseConnection shall destroy the connection anyway. **]**  

###   HTTPAPI_ExecuteRequest
```c
HTTPAPI_RESULT HTTPAPI_ExecuteRequest(HTTP_HANDLE handle, HTTPAPI_REQUEST_TYPE requestType, const char* relativePath,
    HTTP_HEADERS_HANDLE httpHeadersHandle, const unsigned char* content,
    size_t contentLength, unsigned int* statusCode,
    HTTP_HEADERS_HANDLE responseHeadersHandle, BUFFER_HANDLE responseContent);
```

**SRS_HTTPAPI_COMPACT_21_021: [** The HTTPAPI_ExecuteRequest shall execute the http communtication with the provided host, sending a request and reciving the response. **]**

**SRS_HTTPAPI_COMPACT_21_022: [** If a Certificate was provided, the HTTPAPI_ExecuteRequest shall set this option on the transport layer. **]**

**SRS_HTTPAPI_COMPACT_21_023: [** If the transport failed setting the Certificate, the HTTPAPI_ExecuteRequest shall not send any request and return HTTPAPI_SET_OPTION_FAILED. **]**

**SRS_HTTPAPI_COMPACT_06_003: [** If the x509 client certificate is provided, the HTTPAPI_ExecuteRequest shall set this option on the transport layer. **]**

**SRS_HTTPAPI_COMPACT_06_005: [** If the transport failed setting the client certificate, the HTTPAPI_ExecuteRequest shall not send any request and return HTTPAPI_SET_OPTION_FAILED. **]**

**SRS_HTTPAPI_COMPACT_06_004: [** If the x509 client certificate private key is provided, the HTTPAPI_ExecuteRequest shall set this optionon the transport layer. **]**

**SRS_HTTPAPI_COMPACT_06_006: [** If the transport failed setting the client certificate private key, the HTTPAPI_ExecuteRequest shall not send any request and return HTTPAPI_SET_OPTION_FAILED. **]**

**SRS_HTTPAPI_COMPACT_21_024: [** The HTTPAPI_ExecuteRequest shall open the transport connection with the host to send the request. **]**

**SRS_HTTPAPI_COMPACT_21_025: [** If the open process failed, the HTTPAPI_ExecuteRequest shall not send any request and return HTTPAPI_OPEN_REQUEST_FAILED. **]**

**SRS_HTTPAPI_COMPACT_21_026: [** If the open process succeed, the HTTPAPI_ExecuteRequest shall send the request message to the host. **]**

**SRS_HTTPAPI_COMPACT_21_027: [** If the HTTPAPI_ExecuteRequest cannot create a buffer to send the request, it shall not send any request and return HTTPAPI_STRING_PROCESSING_ERROR. **]**

**SRS_HTTPAPI_COMPACT_21_028: [** If the HTTPAPI_ExecuteRequest cannot send the request header, it shall return HTTPAPI_HTTP_HEADERS_FAILED. **]**

**SRS_HTTPAPI_COMPACT_21_029: [** If the HTTPAPI_ExecuteRequest cannot send the buffer with the request, it shall return HTTPAPI_SEND_REQUEST_FAILED. **]**

**SRS_HTTPAPI_COMPACT_21_030: [** At the end of the transmission, the HTTPAPI_ExecuteRequest shall receive the response from the host. **]**

**SRS_HTTPAPI_COMPACT_21_032: [** If the HTTPAPI_ExecuteRequest cannot read the message with the request result, it shall return HTTPAPI_READ_DATA_FAILED. **]**

**SRS_HTTPAPI_COMPACT_21_033: [** If the whole process succeed, the HTTPAPI_ExecuteRequest shall retur HTTPAPI_OK. **]**

**SRS_HTTPAPI_COMPACT_21_034: [** If there is no previous connection, the HTTPAPI_ExecuteRequest shall return HTTPAPI_INVALID_ARG. **]**

**SRS_HTTPAPI_COMPACT_21_035: [** The HTTPAPI_ExecuteRequest shall execute resquest for types `GET`, `POST`, `PUT`, `DELETE`, `PATCH`. **]**

**SRS_HTTPAPI_COMPACT_21_036: [** The request type shall be provided in the parameter requestType. **]**

**SRS_HTTPAPI_COMPACT_21_037: [** If the request type is unknown, the HTTPAPI_ExecuteRequest shall return HTTPAPI_INVALID_ARG. **]**

**SRS_HTTPAPI_COMPACT_21_038: [** The HTTPAPI_ExecuteRequest shall execute the resquest for the path in relativePath parameter. **]**

**SRS_HTTPAPI_COMPACT_21_039: [** If the relativePath is NULL or invalid, the HTTPAPI_ExecuteRequest shall return HTTPAPI_INVALID_ARG. **]**

**SRS_HTTPAPI_COMPACT_21_040: [** The request shall contain the http header provided in httpHeadersHandle parameter. **]**

**SRS_HTTPAPI_COMPACT_21_041: [** If the httpHeadersHandle is NULL or invalid, the HTTPAPI_ExecuteRequest shall return HTTPAPI_INVALID_ARG. **]**

**SRS_HTTPAPI_COMPACT_21_042: [** The request can contain the a content message, provided in content parameter. **]**

**SRS_HTTPAPI_COMPACT_21_043: [** If the content is NULL, the HTTPAPI_ExecuteRequest shall send the request without content. **]**

**SRS_HTTPAPI_COMPACT_21_044: [** If the content is not NULL, the number of bytes in the content shall be provided in contentLength parameter. **]**

**SRS_HTTPAPI_COMPACT_21_045: [** If the contentLength is lower than one, the HTTPAPI_ExecuteRequest shall send the request without content. **]**

**SRS_HTTPAPI_COMPACT_21_046: [** The HTTPAPI_ExecuteRequest shall return the http status reported by the host in the received response. **]**

**SRS_HTTPAPI_COMPACT_21_047: [** The HTTPAPI_ExecuteRequest shall report the status in the statusCode parameter. **]**

**SRS_HTTPAPI_COMPACT_21_048: [** If the statusCode is NULL, the HTTPAPI_ExecuteRequest shall report not report any status. **]**

**SRS_HTTPAPI_COMPACT_21_049: [** If responseHeadersHandle is provide, the HTTPAPI_ExecuteRequest shall prepare a Response Header usign the HTTPHeaders_AddHeaderNameValuePair. **]**

**SRS_HTTPAPI_COMPACT_21_050: [** If there is a content in the response, the HTTPAPI_ExecuteRequest shall copy it in the responseContent buffer. **]**

**SRS_HTTPAPI_COMPACT_21_051: [** If the responseContent is NULL, the HTTPAPI_ExecuteRequest shall ignore any content in the response. **]**

**SRS_HTTPAPI_COMPACT_21_052: [** If any memory allocation get fail, the HTTPAPI_ExecuteRequest shall return HTTPAPI_ALLOC_FAILED. **]**

**SRS_HTTPAPI_COMPACT_21_053: [** The HTTPAPI_ExecuteRequest shall produce a set of http header to send to the host. **]**

**SRS_HTTPAPI_COMPACT_21_054: [** If Http header maker cannot provide the number of headers, the HTTPAPI_ExecuteRequest shall return HTTPAPI_INVALID_ARG. **]**

**SRS_HTTPAPI_COMPACT_21_055: [** If the HTTPAPI_ExecuteRequest cannot parser the received message, it shall return HTTPAPI_RECEIVE_RESPONSE_FAILED. **]**

**SRS_HTTPAPI_COMPACT_21_073: [** The message received by the HTTPAPI_ExecuteRequest shall starts with a valid header. **]**

**SRS_HTTPAPI_COMPACT_21_074: [** After the header, the message received by the HTTPAPI_ExecuteRequest can contain addition information about the content. **]**

**SRS_HTTPAPI_COMPACT_21_075: [** The message received by the HTTPAPI_ExecuteRequest can contain a body with the message content. **]**

**SRS_HTTPAPI_COMPACT_21_077: [** The HTTPAPI_ExecuteRequest shall wait, at least, 10 seconds for the SSL open process. **]**

**SRS_HTTPAPI_COMPACT_21_078: [** If the HTTPAPI_ExecuteRequest cannot open the connection in 10 seconds, it shall fail and return HTTPAPI_OPEN_REQUEST_FAILED. **]**

**SRS_HTTPAPI_COMPACT_21_079: [** The HTTPAPI_ExecuteRequest shall wait, at least, 20 seconds to send a buffer using the SSL connection. **]**

**SRS_HTTPAPI_COMPACT_21_080: [** If the HTTPAPI_ExecuteRequest retries to send the message for 20 seconds without success, it shall fail and return HTTPAPI_SEND_REQUEST_FAILED. **]**

**SRS_HTTPAPI_COMPACT_21_081: [** The HTTPAPI_ExecuteRequest shall try to read the message with the response up to 20 seconds. **]**

**SRS_HTTPAPI_COMPACT_21_082: [** If the HTTPAPI_ExecuteRequest retries 20 seconds to receive the message without success, it shall fail and return HTTPAPI_READ_DATA_FAILED. **]**

**SRS_HTTPAPI_COMPACT_21_083: [** The HTTPAPI_ExecuteRequest shall wait, at least, 100 milliseconds between retries. **]**  

**SRS_HTTPAPI_COMPACT_42_088: [** The message received by the HTTPAPI_ExecuteRequest should not contain http body. **]**  


###   HTTPAPI_SetOption
```c
HTTPAPI_RESULT HTTPAPI_SetOption(HTTP_HANDLE handle, const char* optionName, const void* value);
```

**SRS_HTTPAPI_COMPACT_21_056: [** The HTTPAPI_SetOption shall change the HTTP options. **]**

**SRS_HTTPAPI_COMPACT_21_057: [** The HTTPAPI_SetOption shall receive a handle that identiry the HTTP connection. **]**

**SRS_HTTPAPI_COMPACT_21_058: [** The HTTPAPI_SetOption shall receive the option as a pair optionName/value. **]**

**SRS_HTTPAPI_COMPACT_21_059: [** If the handle is NULL, the HTTPAPI_SetOption shall return HTTPAPI_INVALID_ARG. **]**

**SRS_HTTPAPI_COMPACT_21_060: [** If the optionName is NULL, the HTTPAPI_SetOption shall return HTTPAPI_INVALID_ARG. **]**

**SRS_HTTPAPI_COMPACT_21_061: [** If the value is NULL, the HTTPAPI_SetOption shall return HTTPAPI_INVALID_ARG. **]**

**SRS_HTTPAPI_COMPACT_21_062: [** If any memory allocation get fail, the HTTPAPI_SetOption shall return HTTPAPI_ALLOC_FAILED. **]**

**SRS_HTTPAPI_COMPACT_21_063: [** If the HTTP do not support the optionName, the HTTPAPI_SetOption shall return HTTPAPI_INVALID_ARG. **]**

**SRS_HTTPAPI_COMPACT_21_064: [** If the HTTPAPI_SetOption get success setting the option, it shall return HTTPAPI_OK. **]**  


###   HTTPAPI_CloneOption
```c
HTTPAPI_RESULT HTTPAPI_CloneOption(const char* optionName, const void* value, const void** savedValue);
```

**SRS_HTTPAPI_COMPACT_21_065: [** The HTTPAPI_CloneOption shall provide the means to clone the HTTP option. **]**

**SRS_HTTPAPI_COMPACT_21_066: [** The HTTPAPI_CloneOption shall return a clone of the value identified by the optionName. **]**

**SRS_HTTPAPI_COMPACT_21_067: [** If the optionName is NULL, the HTTPAPI_CloneOption shall return HTTPAPI_INVALID_ARG. **]**

**SRS_HTTPAPI_COMPACT_21_068: [** If the value is NULL, the HTTPAPI_CloneOption shall return HTTPAPI_INVALID_ARG. **]**

**SRS_HTTPAPI_COMPACT_21_069: [** If the savedValue is NULL, the HTTPAPI_CloneOption shall return HTTPAPI_INVALID_ARG. **]**

**SRS_HTTPAPI_COMPACT_21_070: [** If any memory allocation get fail, the HTTPAPI_CloneOption shall return HTTPAPI_ALLOC_FAILED. **]**

**SRS_HTTPAPI_COMPACT_21_071: [** If the HTTP do not support the optionName, the HTTPAPI_CloneOption shall return HTTPAPI_INVALID_ARG. **]**

**SRS_HTTPAPI_COMPACT_21_072: [** If the HTTPAPI_CloneOption get success setting the option, it shall return HTTPAPI_OK. **]**  
