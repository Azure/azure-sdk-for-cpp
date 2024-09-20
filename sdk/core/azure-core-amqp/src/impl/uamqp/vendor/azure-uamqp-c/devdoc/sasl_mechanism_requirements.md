# `sasl_mechanism` requirements

## Overview

`sasl_mechanism` is a module that implements a SASL mechanism interface, abstracting from its consumers functionality of providing init bytes, handling the challenge and providing a response for it as part of the SASL negotiation.

## Exposed API

```C
    typedef struct SASL_MECHANISM_INSTANCE_TAG* SASL_MECHANISM_HANDLE;
    typedef void* CONCRETE_SASL_MECHANISM_HANDLE;

    typedef struct SASL_MECHANISM_BYTES_TAG
    {
        const void* bytes;
        uint32_t length;
    } SASL_MECHANISM_BYTES;

    typedef CONCRETE_SASL_MECHANISM_HANDLE(*SASL_MECHANISM_CREATE)(void* config);
    typedef void(*SASL_MECHANISM_DESTROY)(CONCRETE_SASL_MECHANISM_HANDLE concrete_sasl_mechanism);
    typedef int(*SASL_MECHANISM_GET_INIT_BYTES)(CONCRETE_SASL_MECHANISM_HANDLE concrete_sasl_mechanism, SASL_MECHANISM_BYTES* init_bytes);
    typedef const char*(*SASL_MECHANISM_GET_MECHANISM_NAME)(CONCRETE_SASL_MECHANISM_HANDLE concrete_sasl_mechanism);
    typedef int(*SASL_MECHANISM_CHALLENGE)(CONCRETE_SASL_MECHANISM_HANDLE concrete_sasl_mechanism, const SASL_MECHANISM_BYTES* challenge_bytes, SASL_MECHANISM_BYTES* response_bytes);

    typedef struct SASL_MECHANISM_INTERFACE_TAG
    {
        SASL_MECHANISM_CREATE concrete_sasl_mechanism_create;
        SASL_MECHANISM_DESTROY concrete_sasl_mechanism_destroy;
        SASL_MECHANISM_GET_INIT_BYTES concrete_sasl_mechanism_get_init_bytes;
        SASL_MECHANISM_GET_MECHANISM_NAME concrete_sasl_mechanism_get_mechanism_name;
        SASL_MECHANISM_CHALLENGE concrete_sasl_mechanism_challenge;
    } SASL_MECHANISM_INTERFACE_DESCRIPTION;

    MOCKABLE_FUNCTION(, SASL_MECHANISM_HANDLE, saslmechanism_create, const SASL_MECHANISM_INTERFACE_DESCRIPTION*, sasl_mechanism_interface_description, void*, sasl_mechanism_create_parameters);
    MOCKABLE_FUNCTION(, void, saslmechanism_destroy, SASL_MECHANISM_HANDLE, sasl_mechanism);
    MOCKABLE_FUNCTION(, int, saslmechanism_get_init_bytes, SASL_MECHANISM_HANDLE, sasl_mechanism, SASL_MECHANISM_BYTES*, init_bytes);
    MOCKABLE_FUNCTION(, const char*, saslmechanism_get_mechanism_name, SASL_MECHANISM_HANDLE, sasl_mechanism);
    MOCKABLE_FUNCTION(, int, saslmechanism_challenge, SASL_MECHANISM_HANDLE, sasl_mechanism, const SASL_MECHANISM_BYTES*, challenge_bytes, SASL_MECHANISM_BYTES*, response_bytes);
```

### saslmechanism_create

```C
MOCKABLE_FUNCTION(, SASL_MECHANISM_HANDLE, saslmechanism_create, const SASL_MECHANISM_INTERFACE_DESCRIPTION*, sasl_mechanism_interface_description, void*, sasl_mechanism_create_parameters);
```

**SRS_SASL_MECHANISM_01_001: [**`saslmechanism_create` shall return on success a non-NULL handle to a new SASL mechanism interface.**]**

**SRS_SASL_MECHANISM_01_002: [**In order to instantiate the concrete SASL mechanism implementation the function `concrete_sasl_mechanism_create` from the `sasl_mechanism_interface_description` shall be called, passing the `sasl_mechanism_create_parameters` to it.**]**

**SRS_SASL_MECHANISM_01_003: [**If the underlying `concrete_sasl_mechanism_create` call fails, `saslmechanism_create` shall return NULL.**]**

**SRS_SASL_MECHANISM_01_004: [**If the argument `sasl_mechanism_interface_description` is NULL, `saslmechanism_create` shall return NULL.**]**

**SRS_SASL_MECHANISM_01_005: [**If any `sasl_mechanism_interface_description` member is NULL, `saslmechanism_create` shall fail and return NULL.**]**

**SRS_SASL_MECHANISM_01_006: [**If allocating the memory needed for the SASL mechanism interface fails then `saslmechanism_create` shall fail and return NULL.**]** 

### saslmechanism_destroy

```C
MOCKABLE_FUNCTION(, void, saslmechanism_destroy, SASL_MECHANISM_HANDLE, sasl_mechanism);
```

**SRS_SASL_MECHANISM_01_007: [**`saslmechanism_destroy` shall free all resources associated with the SASL mechanism handle.**]**

**SRS_SASL_MECHANISM_01_008: [**`saslmechanism_destroy` shall also call the `concrete_sasl_mechanism_destroy` function that is member of the `sasl_mechanism_interface_description` argument passed to `saslmechanism_create`, while passing as argument to `concrete_sasl_mechanism_destroy` the result of the underlying concrete SASL mechanism handle.**]**

**SRS_SASL_MECHANISM_01_009: [**If the argument `sasl_mechanism` is NULL, `saslmechanism_destroy` shall do nothing.**]** 

### saslmechanism_get_init_bytes

```C
MOCKABLE_FUNCTION(, int, saslmechanism_get_init_bytes, SASL_MECHANISM_HANDLE, sasl_mechanism, SASL_MECHANISM_BYTES*, init_bytes);
```

**SRS_SASL_MECHANISM_01_010: [**`saslmechanism_get_init_bytes` shall call the specific `concrete_sasl_mechanism_get_init_bytes` function specified in `saslmechanism_create`, passing the `init_bytes` argument to it.**]**

**SRS_SASL_MECHANISM_01_011: [**On success, `saslmechanism_get_init_bytes` shall return 0.**]**

**SRS_SASL_MECHANISM_01_012: [**If the argument `sasl_mechanism` is NULL, `saslmechanism_get_init_bytes` shall fail and return a non-zero value.**]**

**SRS_SASL_MECHANISM_01_013: [**If the underlying `concrete_sasl_mechanism_get_init_bytes` fails, `saslmechanism_get_init_bytes` shall fail and return a non-zero value.**]** 

### saslmechanism_get_mechanism_name

```C
MOCKABLE_FUNCTION(, const char*, saslmechanism_get_mechanism_name, SASL_MECHANISM_HANDLE, sasl_mechanism);
```

**SRS_SASL_MECHANISM_01_014: [**`saslmechanism_get_mechanism_name` shall call the specific `concrete_sasl_mechanism_get_mechanism_name` function specified in `saslmechanism_create`.**]**

**SRS_SASL_MECHANISM_01_015: [**On success, `saslmechanism_get_mechanism_name` shall return a pointer to a string with the mechanism name.**]**

**SRS_SASL_MECHANISM_01_016: [**If the argument `sasl_mechanism` is NULL, `saslmechanism_get_mechanism_name` shall fail and return a non-zero value.**]**

**SRS_SASL_MECHANISM_01_017: [**If the underlying `concrete_sasl_mechanism_get_mechanism_name` fails, `saslmechanism_get_mechanism_name` shall return NULL.**]** 

### saslmechanism_challenge

```C
MOCKABLE_FUNCTION(, int, saslmechanism_challenge, SASL_MECHANISM_HANDLE, sasl_mechanism, const SASL_MECHANISM_BYTES*, challenge_bytes, SASL_MECHANISM_BYTES*, response_bytes);
```

**SRS_SASL_MECHANISM_01_018: [**`saslmechanism_challenge` shall call the specific `concrete_sasl_mechanism_challenge` function specified in `saslmechanism_create`, while passing the `challenge_bytes` and `response_bytes` arguments to it.**]**

**SRS_SASL_MECHANISM_01_019: [**On success, `saslmechanism_challenge` shall return 0.**]**

**SRS_SASL_MECHANISM_01_020: [**If the argument `sasl_mechanism` is NULL, `saslmechanism_challenge` shall fail and return a non-zero value.**]**

**SRS_SASL_MECHANISM_01_021: [**If the underlying `concrete_sasl_mechanism_challenge` fails, `saslmechanism_challenge` shall fail and return a non-zero value.**]** 
