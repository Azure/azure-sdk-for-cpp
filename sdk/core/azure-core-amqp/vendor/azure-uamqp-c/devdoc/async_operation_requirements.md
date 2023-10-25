# `async_operation` requirements

## Overview

`async_operation` is module that encapsulates an asyncronous cancellable operation.

## Exposed API

```C
typedef struct ASYNC_OPERATION_INSTANCE_TAG* ASYNC_OPERATION_HANDLE;

typedef void(*ASYNC_OPERATION_CANCEL_HANDLER_FUNC)(void* context);

#define DEFINE_ASYNC_OPERATION_CONTEXT(type) \
struct C3(ASYNC_OPERATION_CONTEXT_, type, _TAG)(type) \
{ \
    ASYNC_OPERATION_CANCEL_HANDLER_FUNC async_operation_cancel_handler; \
    type context; \
};

#define GET_ASYNC_OPERATION_CONTEXT(type, async_operation) \
    (type*)((unsigned char*)async_operation + offsetof(type, context))

#define CREATE_ASYNC_OPERATION(type, async_operation_cancel_handler) \
    async_operation_create(async_operation_cancel_handler, sizeof(type))

MOCKABLE_FUNCTION(, ASYNC_OPERATION_HANDLE, async_operation_create, ASYNC_OPERATION_CANCEL_HANDLER_FUNC, async_operation_cancel_handler, size_t, context_size);
MOCKABLE_FUNCTION(, void, async_operation_destroy, ASYNC_OPERATION_HANDLE, async_operation);
MOCKABLE_FUNCTION(, int, async_operation_cancel, ASYNC_OPERATION_HANDLE, async_operation);
```

### async_operation_create

```C
MOCKABLE_FUNCTION(, ASYNC_OPERATION_HANDLE, async_operation_create, ASYNC_OPERATION_CANCEL_HANDLER_FUNC, async_operation_cancel_handler, size_t, context_size);
```

**SRS_ASYNC_OPERATION_01_001: [** `async_operation_create` shall return a non-NULL handle to a newly created asynchronous operation instance.**]**
**SRS_ASYNC_OPERATION_01_002: [** If `async_operation_cancel_handler` is NULL, `async_operation_create` shall fail and return NULL.**]**
**SRS_ASYNC_OPERATION_01_003: [** If `context_size` is less than the size of the `async_operation_cancel_handler` argument, `async_operation_create` shall fail and return NULL.**]**
**SRS_ASYNC_OPERATION_01_004: [** If allocating memory for the new asynchronous operation instance fails, `async_operation_create` shall fail and return NULL.**]**

### async_operation_destroy

```C
MOCKABLE_FUNCTION(, void, async_operation_destroy, ASYNC_OPERATION_HANDLE, async_operation);
```

**SRS_ASYNC_OPERATION_01_005: [** `async_operation_destroy` shall free all recources associated with the asyncronous operation instance.**]**
**SRS_ASYNC_OPERATION_01_006: [** If `async_operation` is NULL, `async_operation_destroy` shall do nothing.**]**

### async_operation_cancel

```C
MOCKABLE_FUNCTION(, int, async_operation_cancel, ASYNC_OPERATION_HANDLE, async_operation);
```

**SRS_ASYNC_OPERATION_01_007: [** `async_operation_cancel` shall cancel the operation by calling the cancel handler function passed to `async_operation_create`.**]**
**SRS_ASYNC_OPERATION_01_008: [** On success `async_operation_cancel` shall return 0.**]**
**SRS_ASYNC_OPERATION_01_009: [** If `async_operation` is NULL, `async_operation_cancel` shall fail and return a non-zero value.**]**
