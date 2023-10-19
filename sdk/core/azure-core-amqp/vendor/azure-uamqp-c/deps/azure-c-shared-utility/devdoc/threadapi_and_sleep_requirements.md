# threadapi and sleep

## Overview

This document defines the behavior of the Azure IoT's **sleep** and **threadapi** adapters. These two
adapters are specifed together because they are declared in the same
[threadapi.h](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/threadapi.h)
header file.

The _sleep_ and _threadapi_ adapters are related because they share a header file and they
both deal with thread functionality, but
they are described as distinct adapters in these documents because the _sleep_ adapter is mandatory for
correct SDK operation, while the _threadapi_ adapter is optional.

## References

[Azure IoT porting guide](https://github.com/Azure/azure-c-shared-utility/blob/master/devdoc/porting_guide.md)<br/>
[threadapi.h](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/threadapi.h)

## Exposed API

The _threadapi_ adapter uses these types specified in threadapi.h:
```
typedef void* THREAD_HANDLE;

typedef enum
{
    THREADAPI_OK,
    THREADAPI_INVALID_ARG,
    THREADAPI_NO_MEMORY,
    THREADAPI_ERROR,
} THREADAPI_RESULT;
```
##   sleep Adapter

###   ThreadAPI_Sleep
The _sleep_ adapter is a single exposed function, `ThreadAPI_Sleep`. This function is required for correct SDK
operation.

```c
void ThreadAPI_Sleep(unsigned int milliseconds);
```

**SRS_THREADAPI_30_001: [** ThreadAPI_Sleep shall suspend the thread for at least the supplied value of `milliseconds`. **]**  

## threadapi Adapter

###   ThreadAPI_Create

Creates a thread with an entry point specified by the `func` argument. The concrete type of the
`void* THREAD_HANDLE` is at the discretion of the developer.

```c
THREADAPI_RESULT ThreadAPI_Create(THREAD_HANDLE* threadHandle, THREAD_START_FUNC func, void* arg);
```

**SRS_THREADAPI_30_010: [** If the **threadapi** adapter is not implemented, `ThreadAPI_Create` shall return `THREADAPI_ERROR`. **]**

**SRS_THREADAPI_30_011: [** If `threadHandle` is NULL `ThreadAPI_Create` shall return `THREADAPI_INVALID_ARG`. **]**

**SRS_THREADAPI_30_012: [** If `func` is NULL `ThreadAPI_Create` shall return `THREADAPI_INVALID_ARG`. **]**

**SRS_THREADAPI_30_013: [** If `ThreadAPI_Create` is unable to create a thread it shall return `THREADAPI_ERROR` or `THREADAPI_NO_MEMORY`, whichever seems more appropriate. **]**

**SRS_THREADAPI_30_014: [** On success, `ThreadAPI_Create` shall return the created thread object in `threadHandle`. **]**

**SRS_THREADAPI_30_015: [** On success, `ThreadAPI_Create` shall return `THREADAPI_OK`. **]**


###   ThreadAPI_Join

Waits for the thread identified by the `threadHandle` argument to complete. When the
`threadHandle` thread completes, all resources associated with the thread must be released and
the thread handle will no longer be valid.

```c
THREADAPI_RESULT ThreadAPI_Join(THREAD_HANDLE threadHandle, int* result);
```
**SRS_THREADAPI_30_020: [** If the **threadapi** adapter is not implemented, `ThreadAPI_Join` shall return `THREADAPI_ERROR`. **]**

**SRS_THREADAPI_30_021: [** If `threadHandle` is NULL `ThreadAPI_Join` shall return `THREADAPI_INVALID_ARG`. **]**

**SRS_THREADAPI_30_022: [** If `ThreadAPI_Join` fails  it shall return `THREADAPI_ERROR`. **]**

**SRS_THREADAPI_30_023: [** On success, `ThreadAPI_Join` shall wait for the thread identified by the `threadHandle` argument to complete. **]**

**SRS_THREADAPI_30_024: [** On success, all resources associated with the thread shall be released. **]**

**SRS_THREADAPI_30_025: [** On success, if `result` is non-NULL then the result of the `THREAD_START_FUNC` shall be returned in `result`. **]**

**SRS_THREADAPI_30_026: [** On success, `ThreadAPI_Join` shall return `THREADAPI_OK`. **]**

###   ThreadAPI_Exit

This function is called by the `THREAD_START_FUNC` when it exits. If another thread has called
`ThreadAPI_Join` then that thread will be unblocked when `ThreadAPI_Exit` is called. 

The implementation must maintain the relationship between the
`THREAD_HANDLE` passed into `ThreadAPI_Join` and the running thread so that
`ThreadAPI_Exit` unblocks the correct waiting thread when when more than one `THREAD_HANDLE`
has been created. This relationship will need to be maintained in thread-local storage
or the equivalent; the thread function cannot be used for disambiguation
because there is no guarantee that multiple threads are not using the same thread function.


```c
void ThreadAPI_Exit(int result);
```
For this `ThreadAPI_Exit` specification, the phrase _"associated `ThreadAPI_Join`"_ means a
`ThreadAPI_Join` which was provided with the same `THREAD_HANDLE` parameter 
that was returned during the creation of the thread from which `ThreadAPI_Exit` was called.

**SRS_THREADAPI_30_030: [** If the **threadapi** adapter is not implemented, `ThreadAPI_Exit` shall do nothing. **]**

**SRS_THREADAPI_30_031: [** If another thread has made an _associated `ThreadAPI_Join`_ call, then that thread shall be unblocked when `ThreadAPI_Exit` is called. **]**

**SRS_THREADAPI_30_032: [** The `result` parameter supplied to `ThreadAPI_Exit` shall be returned in the `result` parameter of the _associated `ThreadAPI_Join`_ call. **]**
