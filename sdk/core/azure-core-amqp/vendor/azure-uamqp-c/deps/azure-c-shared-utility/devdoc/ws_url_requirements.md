STRING TOKENIZER Requirements
================

## Overview
The WS_URL provides functionality parse Websocket URLs.


## Exposed API
```C
typedef struct WS_URL_TAG* WS_URL_HANDLE;

extern WS_URL_HANDLE ws_url_create(const char* url);
extern int ws_url_is_secure(WS_URL_HANDLE url, bool* is_secure);
extern int ws_url_get_host(WS_URL_HANDLE url, const char** host, size_t* length);
extern int ws_url_get_port(WS_URL_HANDLE url, size_t* port);
extern int ws_url_get_path(WS_URL_HANDLE url, const char** path, size_t* length);
extern int ws_url_get_query(WS_URL_HANDLE url, const char** query, size_t* length);
extern void ws_url_destroy(WS_URL_HANDLE url);
```


###  ws_url_create

```c
extern WS_URL_HANDLE ws_url_create(const char* url);
```

**SRS_WS_URL_09_001: [** If `url` is NULL the function shall fail and return NULL **]**

**SRS_WS_URL_09_002: [** Memory shall be allocated for an instance of WS_URL (aka `ws_url`) **]**

**SRS_WS_URL_09_003: [** If `ws_url` failed to be allocated, the function shall return NULL **]**

**SRS_WS_URL_09_024: [** `url` shall be copied into `ws_url->url` **]**

**SRS_WS_URL_09_025: [** If `url` fails to be copied, the function shall free `ws_url` and return NULL **]**


#### Parsing starts

**SRS_WS_URL_09_004: [** If `url` starts with "ws://" (`protocol`), `ws_url->is_secure` shall be set to false **]**

**SRS_WS_URL_09_005: [** If `url` starts with "wss://" (`protocol`), `ws_url->is_secure` shall be set to true **]**

**SRS_WS_URL_09_024: [** If `protocol` cannot be identified in `url`, the function shall fail and return NULL **]**

**SRS_WS_URL_09_006: [** The pointer to the token starting right after `protocol` (in the `url` string) shall be stored in `ws_url->host` **]**

**SRS_WS_URL_09_007: [** If `ws_url->host` ends up being NULL, the function shall fail and return NULL **]**

**SRS_WS_URL_09_008: [** The length from `ws_url->host` up to the first occurrence of either ":" (`port_delimiter`), "/" (`path_delimiter`), "?" (`query_delimiter`) or `\0` shall be stored in `ws_url->host_length` **]**

**SRS_WS_URL_09_009: [** If `ws_url->host_length` ends up being zero, the function shall fail and return NULL **]**

**SRS_WS_URL_09_010: [** If after `ws_url->host` the `port_delimiter` occurs (not preceeded by `path_delimiter` or `query_delimiter`) the number that follows shall be parsed and stored in `ws_url->port` **]**

**SRS_WS_URL_09_011: [** If the port number fails to be parsed, the function shall fail and return NULL **]**

**SRS_WS_URL_09_012: [** If after `ws_url->host` or the port number the `path_delimiter` occurs (not preceeded by `query_delimiter`) the following pointer address shall be stored in `ws_url->path` **]**

**SRS_WS_URL_09_013: [** If the path component is present and `ws_url->path` ends up being NULL, the function shall fail and return NULL **]**

**SRS_WS_URL_09_014: [** The length from `ws_url->path` up to the first occurrence of either `query_delimiter` or `\0` shall be stored in `ws_url->path_length` **]**

**SRS_WS_URL_09_015: [** If the path component is present and `ws_url->path_length` ends up being zero, the function shall fail and return NULL **]**

**SRS_WS_URL_09_016: [** Next if the `query_delimiter` occurs the following pointer address shall be stored in `ws_url->query` **]**

**SRS_WS_URL_09_017: [** If the query component is present and `ws_url->query` ends up being NULL, the function shall fail and return NULL **]**

**SRS_WS_URL_09_018: [** The length from `ws_url->query` up to `\0` shall be stored in `ws_url->query_length` **]**

**SRS_WS_URL_09_019: [** If the query component is present and `ws_url->query_length` ends up being zero, the function shall fail and return NULL **]**

**SRS_WS_URL_09_020: [** If any component cannot be parsed or is out of order, the function shall fail and return NULL **]**


#### Finally

**SRS_WS_URL_09_021: [** If any failure occurs, all memory allocated by the function shall be released before returning **]**


###  ws_url_destroy

```c
extern void ws_url_destroy(WS_URL_HANDLE url);
```

**SRS_WS_URL_09_022: [** If `url` is NULL, the function shall return without further action **]**

**SRS_WS_URL_09_023: [** Otherwise, the memory allocated for `url` shall released **]**


### ws_url_is_secure
```c
extern int ws_url_is_secure(WS_URL_HANDLE url, bool* is_secure);
```

**SRS_WS_URL_09_026: [** If `url` is NULL, the function shall return a non-zero value (failure) **]**

**SRS_WS_URL_09_027: [** Otherwize the function shall set `is_secure` as `url->is_secure` **]**

**SRS_WS_URL_09_028: [** If no errors occur function shall return zero (success) **]**


### ws_url_get_host
```c
extern int ws_url_get_host(WS_URL_HANDLE url, const char** host, size_t* length);
```

**SRS_WS_URL_09_029: [** If `url` or `host` or `length` are NULL, the function shall return a non-zero value (failure) **]**

**SRS_WS_URL_09_030: [** Otherwize the function shall set `host` to `url->host` and `length` to `url->host_length` **]**

**SRS_WS_URL_09_031: [** If no errors occur function shall return zero (success) **]**


### ws_url_get_path
```c
extern int ws_url_get_path(WS_URL_HANDLE url, const char** path, size_t* length);
```

**SRS_WS_URL_09_032: [** If `url` or `path` or `length` are NULL, the function shall return a non-zero value (failure) **]**

**SRS_WS_URL_09_033: [** Otherwize the function shall set `path` to `url->path` and `length` to `url->path_length` **]**

**SRS_WS_URL_09_034: [** If no errors occur function shall return zero (success) **]**


### ws_url_get_query
```c
extern int ws_url_get_query(WS_URL_HANDLE url, const char** query, size_t* length);
```

**SRS_WS_URL_09_035: [** If `url` or `query` or `length` are NULL, the function shall return a non-zero value (failure) **]**

**SRS_WS_URL_09_036: [** Otherwize the function shall set `query` to `url->query` and `length` to `url->query_length` **]**

**SRS_WS_URL_09_037: [** If no errors occur function shall return zero (success) **]**


### ws_url_get_port
```c
extern int ws_url_get_port(WS_URL_HANDLE url, size_t* port);
```

**SRS_WS_URL_09_038: [** If `url` or `port` are NULL, the function shall return a non-zero value (failure) **]**

**SRS_WS_URL_09_039: [** Otherwize the function shall set `port` as `url->port` **]**

**SRS_WS_URL_09_040: [** If no errors occur function shall return zero (success) **]**