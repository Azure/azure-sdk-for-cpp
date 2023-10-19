# lock


## Overview

The **lock** adapter implements synchronization needed with the **threadapi** adapter.

A functional Lock adapter implementation is only required if the optional threadapi adapter is being used.

## Exposed API
**SRS_LOCK_10_001: [** `Lock` interface exposes the following APIs **]**
```c
typedef enum LOCK_RESULT_TAG
{
    LOCK_OK,
    LOCK_ERROR
} LOCK_RESULT;
```

```c
typedef void* HANDLE_LOCK;
```
**SRS_LOCK_10_015: [** This is the handle to the different lock instances **]**

```c
HANDLE_LOCK Lock_Init (void);
```
**SRS_LOCK_30_001: [** If the **lock** adapter is not implemented, `Lock_Init` shall return NULL. **]**

**SRS_LOCK_10_002: [** `Lock_Init` on success shall return a valid lock handle which should be a non-`NULL` value **]**

**SRS_LOCK_10_003: [** `Lock_Init` on error shall return `NULL` **]**

```c
LOCK_RESULT Lock(HANDLE_LOCK handle);
```
**SRS_LOCK_30_001: [** If the **lock** adapter is not implemented, `Lock` shall return `LOCK_ERROR`. **]**

**SRS_LOCK_10_004: [** `Lock` shall be implemented as a non-recursive lock **]**

**SRS_LOCK_10_005: [** `Lock` on success shall return `LOCK_OK` **]**

**SRS_LOCK_10_006: [** `Lock` on error shall return `LOCK_ERROR` **]**

**SRS_LOCK_10_007: [** `Lock` on `NULL` handle passed returns `LOCK_ERROR` **]**

```c
LOCK_RESULT Unlock(HANDLE_LOCK handle);
```
**SRS_LOCK_30_001: [** If the **lock** adapter is not implemented, `Unlock` shall return `LOCK_ERROR`. **]**

**SRS_LOCK_10_008: [** `Unlock` shall be implemented as a mutex unlock **]**

**SRS_LOCK_10_009: [** `Unlock` on success shall return `LOCK_OK` **]**

**SRS_LOCK_10_010: [** `Unlock` on error shall return `LOCK_ERROR` **]**

**SRS_LOCK_10_011: [** `Unlock` on `NULL` handle passed returns `LOCK_ERROR` **]**

```c
LOCK_RESULT Lock_Deinit(HANDLE_LOCK handle);
```
**SRS_LOCK_30_001: [** If the **lock** adapter is not implemented, `Lock_Deinit` shall do nothing. **]**

**SRS_LOCK_10_012: [** `Lock_Deinit` frees all resources associated with `handle` **]**

**SRS_LOCK_10_013: [** `Lock_Deinit` on NULL `handle` passed returns `LOCK_ERROR` **]**
