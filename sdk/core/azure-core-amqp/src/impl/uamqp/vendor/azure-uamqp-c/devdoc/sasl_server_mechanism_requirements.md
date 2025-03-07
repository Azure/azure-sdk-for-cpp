# `sasl_server_mechanism` requirements

## Overview

`sasl_server_mechanism` is a module that implements a SASL mechanism interface for the server portion of the SASL standard, abstracting from its consumers functionality of handling initial and subsequent responses as part of the SASL negotiation.

## Exposed API

```C
    typedef struct SASL_SERVER_MECHANISM_INSTANCE_TAG* SASL_SERVER_MECHANISM_HANDLE;
    typedef void* CONCRETE_SASL_SERVER_MECHANISM_HANDLE;

    typedef struct SASL_SERVER_MECHANISM_BYTES_TAG
    {
        const void* bytes;
        uint32_t length;
    } SASL_SERVER_MECHANISM_BYTES;

    typedef CONCRETE_SASL_SERVER_MECHANISM_HANDLE(*SASL_SERVER_MECHANISM_CREATE)(void* config);
    typedef void(*SASL_SERVER_MECHANISM_DESTROY)(CONCRETE_SASL_SERVER_MECHANISM_HANDLE concrete_sasl_server_mechanism);
    typedef int(*SASL_SERVER_MECHANISM_HANDLE_INITIAL_RESPONSE)(CONCRETE_SASL_SERVER_MECHANISM_HANDLE concrete_sasl_server_mechanism, const SASL_SERVER_MECHANISM_BYTES* initial_response_bytes, const char* hostname, bool* send_challenge, SASL_SERVER_MECHANISM_BYTES* challenge_bytes);
    typedef int(*SASL_SERVER_MECHANISM_HANDLE_RESPONSE)(CONCRETE_SASL_SERVER_MECHANISM_HANDLE concrete_sasl_server_mechanism, const SASL_SERVER_MECHANISM_BYTES* response_bytes, bool* send_next_challenge, SASL_SERVER_MECHANISM_BYTES* next_challenge_bytes);
    typedef const char*(*SASL_SERVER_MECHANISM_GET_MECHANISM_NAME)(void);

    typedef struct SASL_SERVER_MECHANISM_INTERFACE_DESCRIPTION_TAG
    {
        SASL_SERVER_MECHANISM_CREATE create;
        SASL_SERVER_MECHANISM_DESTROY destroy;
        SASL_SERVER_MECHANISM_HANDLE_INITIAL_RESPONSE handle_initial_response;
        SASL_SERVER_MECHANISM_HANDLE_RESPONSE handle_response;
        SASL_SERVER_MECHANISM_GET_MECHANISM_NAME get_mechanism_name;
    } SASL_SERVER_MECHANISM_INTERFACE_DESCRIPTION;

    MOCKABLE_FUNCTION(, SASL_SERVER_MECHANISM_HANDLE, sasl_server_mechanism_create, const SASL_SERVER_MECHANISM_INTERFACE_DESCRIPTION*, sasl_server_mechanism_interface_description, void*, sasl_server_mechanism_create_parameters);
    MOCKABLE_FUNCTION(, void, sasl_server_mechanism_destroy, SASL_SERVER_MECHANISM_HANDLE, sasl_server_mechanism);
    MOCKABLE_FUNCTION(, int, sasl_server_mechanism_handle_initial_response, SASL_SERVER_MECHANISM_HANDLE, sasl_server_mechanism, const SASL_SERVER_MECHANISM_BYTES*, initial_response_bytes, const char*, hostname, bool*, send_challenge, SASL_SERVER_MECHANISM_BYTES*, challenge_bytes);
    MOCKABLE_FUNCTION(, int, sasl_server_mechanism_handle_response, SASL_SERVER_MECHANISM_HANDLE, sasl_server_mechanism, const SASL_SERVER_MECHANISM_BYTES*, response_bytes, bool*, send_next_challenge, SASL_SERVER_MECHANISM_BYTES*, next_challenge_bytes);
    MOCKABLE_FUNCTION(, const char*, sasl_server_mechanism_get_mechanism_name, SASL_SERVER_MECHANISM_HANDLE, sasl_server_mechanism);
```

### sasl_server_mechanism_create

```C
MOCKABLE_FUNCTION(, SASL_SERVER_MECHANISM_HANDLE, sasl_server_mechanism_create, const SASL_SERVER_MECHANISM_INTERFACE_DESCRIPTION*, sasl_server_mechanism_interface_description, void*, sasl_server_mechanism_create_parameters);
```

**SRS_SASL_SERVER_MECHANISM_01_001: [**`sasl_server_mechanism_create` shall return on success a non-NULL handle to a new SASL server mechanism interface.**]**

**SRS_SASL_SERVER_MECHANISM_01_002: [** In order to instantiate the concrete SASL server mechanism implementation the function `create` from the `sasl_server_mechanism_interface_description` shall be called, passing the `sasl_server_mechanism_create_parameters` to it.**]**

**SRS_SASL_SERVER_MECHANISM_01_003: [** If the underlying `create` call fails, `sasl_server_mechanism_create` shall return NULL. **]**

**SRS_SASL_SERVER_MECHANISM_01_004: [** If the argument `sasl_server_mechanism_interface_description` is NULL, `sasl_server_mechanism_create` shall return NULL.**]**

**SRS_SASL_SERVER_MECHANISM_01_005: [** If any `sasl_server_mechanism_interface_description` member is NULL, `sasl_server_mechanism_create` shall fail and return NULL.**]**

**SRS_SASL_SERVER_MECHANISM_01_006: [** If allocating the memory needed for the SASL server mechanism interface fails then `sasl_server_mechanism_create` shall fail and return NULL. **]**

### sasl_server_mechanism_destroy

```C
MOCKABLE_FUNCTION(, void, sasl_server_mechanism_destroy, SASL_SERVER_MECHANISM_HANDLE, sasl_server_mechanism);
```

**SRS_SASL_SERVER_MECHANISM_01_007: [** `sasl_server_mechanism_destroy` shall free all resources associated with the SASL mechanism handle. **]**

**SRS_SASL_SERVER_MECHANISM_01_008: [** `sasl_server_mechanism_destroy` shall also call the `destroy` function that is member of the `sasl_mechanism_interface_description` argument passed to `sasl_server_mechanism_create`, while passing as argument to `destroy` the result of the underlying concrete SASL mechanism handle. **]**

**SRS_SASL_SERVER_MECHANISM_01_009: [** If the argument `sasl_server_mechanism` is NULL, `sasl_server_mechanism_destroy` shall do nothing. **]**

### sasl_server_mechanism_handle_initial_response

```C
MOCKABLE_FUNCTION(, int, sasl_server_mechanism_handle_initial_response, SASL_SERVER_MECHANISM_HANDLE, sasl_server_mechanism, const SASL_SERVER_MECHANISM_BYTES*, initial_response_bytes, const char*, hostname, bool*, send_challenge, SASL_SERVER_MECHANISM_BYTES*, challenge_bytes);
```

**SRS_SASL_SERVER_MECHANISM_01_010: [** `sasl_server_mechanism_handle_initial_response` shall call the specific `handle_initial_response` function specified in `sasl_server_mechanism_create`, passing the `initial_response_bytes`, `hostname`, `send_challenge` and `challenge_bytes` arguments to it. **]**

**SRS_SASL_SERVER_MECHANISM_01_011: [** On success, `sasl_server_mechanism_handle_initial_response` shall return 0. **]**

**SRS_SASL_SERVER_MECHANISM_01_012: [** If the argument `sasl_server_mechanism` is NULL, `sasl_server_mechanism_handle_initial_response` shall fail and return a non-zero value. **]**

**SRS_SASL_SERVER_MECHANISM_01_013: [** If the underlying `handle_initial_response` fails, `sasl_server_mechanism_handle_initial_response` shall fail and return a non-zero value. **]**

### sasl_server_mechanism_handle_response

```C
MOCKABLE_FUNCTION(, int, sasl_server_mechanism_handle_response, SASL_SERVER_MECHANISM_HANDLE, sasl_server_mechanism, const SASL_SERVER_MECHANISM_BYTES*, response_bytes, bool*, send_next_challenge, SASL_SERVER_MECHANISM_BYTES*, next_challenge_bytes);
```

**SRS_SASL_SERVER_MECHANISM_01_014: [** `sasl_server_mechanism_handle_response` shall call the specific `handle_response` function specified in `sasl_server_mechanism_create`, passing the `response_bytes`, `send_next_challenge` and `next_challenge_bytes` arguments to it. **]**

**SRS_SASL_SERVER_MECHANISM_01_016: [** On success, `sasl_server_mechanism_handle_response` shall return 0. **]**

**SRS_SASL_SERVER_MECHANISM_01_017: [** If the argument `sasl_server_mechanism` is NULL, `sasl_server_mechanism_handle_response` shall fail and return a non-zero value. **]**

**SRS_SASL_SERVER_MECHANISM_01_018: [** If the underlying `handle_response` fails, `sasl_server_mechanism_handle_response` shall fail and return a non-zero value. **]**

### sasl_server_mechanism_get_mechanism_name

```C
MOCKABLE_FUNCTION(, const char*, sasl_server_mechanism_get_mechanism_name, SASL_SERVER_MECHANISM_HANDLE, sasl_server_mechanism);
```

**SRS_SASL_SERVER_MECHANISM_01_019: [** `sasl_server_mechanism_get_mechanism_name` shall call the specific `get_mechanism_name` function specified in `sasl_server_mechanism_create`. **]**

**SRS_SASL_SERVER_MECHANISM_01_020: [** On success, `sasl_server_mechanism_get_mechanism_name` shall return a pointer to a string with the mechanism name. **]**

**SRS_SASL_SERVER_MECHANISM_01_021: [** If the argument `sasl_server_mechanism` is NULL, `sasl_server_mechanism_get_mechanism_name` shall fail and return a non-zero value. **]**

**SRS_SASL_SERVER_MECHANISM_01_022: [** If the underlying `get_mechanism_name` fails, `sasl_server_mechanism_get_mechanism_name` shall return NULL. **]**
