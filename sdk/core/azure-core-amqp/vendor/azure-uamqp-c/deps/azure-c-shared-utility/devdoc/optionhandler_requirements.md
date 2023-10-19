# OptionHandler

# Overview

Option Handler is a module that builds a container of options relevant to a module. The options can be later retrieved.
It does so by asking the module to clone its options or to destroy them. `OptionHandler` is agnostic to the module it serves.


## Exposed API

```c
#define OPTIONHANDLER_RESULT_VALUES
OPTIONHANDLER_OK,
OPTIONHANDLER_ERROR,
OPTIONHANDLER_INVALIDARG

MU_DEFINE_ENUM(OPTIONHANDLER_RESULT, OPTIONHANDLER_RESULT_VALUES)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "umock_c/umock_c_prod.h"

typedef struct OPTIONHANDLER_HANDLE_DATA_TAG* OPTIONHANDLER_HANDLE;

/*the following function pointer points to a function that produces a clone of the option specified by name and value (that is, a clone of void* value)*/
/*returns NULL if it failed to produce a clone, otherwise returns a non-NULL value*/
/*to be implemented by every module*/
typedef void* (*pfCloneOption)(const char* name, const void* value);

/*the following function pointer points to a function that frees resources allocated for an option*/
/*to be implemented by every module*/
typedef void (*pfDestroyOption)(const char* name, const void* value);

/*the following function pointer points to a function that sets an option for a module*/
/*to be implemented by every module*/
/*returns 0 if _SetOption succeeded, any other value is error, if the option is not intended for that module, returns 0*/
typedef int (*pfSetOption)(void* handle, const char* name, const void* value);

MOCKABLE_FUNCTION(,OPTIONHANDLER_HANDLE, OptionHandler_Create, pfCloneOption, cloneOption, pfDestroyOption, destroyOption, pfSetOption setOption);
MOCKABLE_FUNCTION(, OPTIONHANDLER_HANDLE, OptionHandler_Clone, OPTIONHANDLER_HANDLE, handler);
MOCKABLE_FUNCTION(,OPTIONHANDLER_RESULT, OptionHandler_AddOption, OPTIONHANDLER_HANDLE, handle, const char*, name, void*, value);
MOCKABLE_FUNCTION(,OPTIONHANDLER_RESULT, OptionHandler_FeedOptions, OPTIONHANDLER_HANDLE, handle, void*, destinationHandle);
MOCKABLE_FUNCTION(,void, OptionHandler_Destroy, OPTIONHANDLER_HANDLE, handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

###  OptionHandler_Create
```c
OPTIONHANDLER_HANDLE OptionHandler_Create(pfCloneOption cloneOption, pfDestroyOption destroyOption, pfSetOption setOption)
```

**SRS_OPTIONHANDLER_02_001: [** `OptionHandler_Create` shall fail and retun `NULL` if any parameters are `NULL`. **]**

**SRS_OPTIONHANDLER_02_002: [** `OptionHandler_Create` shall create an empty VECTOR that will hold pairs of `const char*` and `void*`. **]**

**SRS_OPTIONHANDLER_02_003: [** If all the operations succeed then `OptionHandler_Create` shall succeed and return a non-`NULL` handle. **]**

**SRS_OPTIONHANDLER_02_004: [** Otherwise, `OptionHandler_Create` shall fail and return `NULL`. **]**

###  OptionHandler_Clone

```c
OPTIONHANDLER_HANDLE OptionHandler_Clone(OPTIONHANDLER_HANDLE handler);
```

**SRS_OPTIONHANDLER_01_001: [** `OptionHandler_Clone` shall clone an existing option handler instance. **]**

**SRS_OPTIONHANDLER_01_002: [** On success it shall return a non-NULL handle. **]**

**SRS_OPTIONHANDLER_01_010: [** If `handler` is NULL, OptionHandler_Clone shall fail and return NULL. **]**

**SRS_OPTIONHANDLER_01_003: [** `OptionHandler_Clone` shall allocate memory for the new option handler instance. **]**

**SRS_OPTIONHANDLER_01_004: [** If allocating memory fails, `OptionHandler_Clone` shall return NULL. **]**

**SRS_OPTIONHANDLER_01_005: [** `OptionHandler_Clone` shall iterate through all the options stored by the option handler to be cloned by using VECTOR's iteration mechanism. **]**

**SRS_OPTIONHANDLER_01_006: [** For each option the option name shall be cloned by calling `mallocAndStrcpy_s`. **]**

**SRS_OPTIONHANDLER_01_007: [** For each option the value shall be cloned by using the cloning function associated with the source option handler `handler`. **]**

**SRS_OPTIONHANDLER_01_008: [** If cloning one of the option names fails, `OptionHandler_Clone` shall return NULL. **]**

**SRS_OPTIONHANDLER_01_009: [** If cloning one of the option values fails, `OptionHandler_Clone` shall return NULL. **]**

**SRS_OPTIONHANDLER_01_011: [** When adding one of the newly cloned options to the option handler storage vector fails, `OptionHandler_Clone` shall return NULL. **]**

###  OptionHandler_AddOption
```c
OPTIONHANDLER_RESULT OptionHandler_AddOption(OPTIONHANDLER_HANDLE handle, const char* name, void* value)
```

**SRS_OPTIONHANDLER_02_005: [** `OptionHandler_AddOption` shall fail and return `OPTIONHANDLER_INVALIDARG` if any parameter is NULL. **]**

**SRS_OPTIONHANDLER_02_006: [** OptionHandler_AddOption shall call `pfCloneOption` passing `name` and `value`. **]**

**SRS_OPTIONHANDLER_02_007: [** OptionHandler_AddOption shall use `VECTOR` APIs to save the `name` and the newly created clone of `value`. **]**

**SRS_OPTIONHANDLER_02_008: [** If all the operations succed then `OptionHandler_AddOption` shall succeed and return `OPTIONHANDLER_OK`. **]**

**SRS_OPTIONHANDLER_02_009: [** Otherwise, `OptionHandler_AddOption` shall succeed and return `OPTIONHANDLER_ERROR`. **]**

###  OptionHandler_FeedOptions
```c
OPTIONHANDLER_RESULT OptionHandler_FeedOptions(OPTIONHANDLER_HANDLE handle, void* destinationHandle);
```

**SRS_OPTIONHANDLER_02_010: [** `OptionHandler_FeedOptions` shall fail and return `OPTIONHANDLER_INVALIDARG` if any argument is `NULL`. **]**

**SRS_OPTIONHANDLER_02_011: [** Otherwise, `OptionHandler_FeedOptions` shall use `VECTOR`'s iteration mechanisms to retrieve pairs of name, value (`const char*` and `void*`). **]**

**SRS_OPTIONHANDLER_02_012: [** `OptionHandler_FeedOptions` shall call for every pair of name,value `setOption` passing `destinationHandle`, name and value. **]**

**SRS_OPTIONHANDLER_02_013: [** If all the operations succeed then `OptionHandler_FeedOptions` shall succeed and return `OPTIONHANDLER_OK`. **]**

**SRS_OPTIONHANDLER_02_014: [** Otherwise, `OptionHandler_FeedOptions` shall fail and return `OPTIONHANDLER_ERROR`. **]**

###  OptionHandler_Destroy
```c
void OptionHandler_Destroy(OPTIONHANDLER_HANDLE handle)
```
**SRS_OPTIONHANDLER_02_015: [** OptionHandler_Destroy shall do nothing if parameter `handle` is `NULL`. **]**

**SRS_OPTIONHANDLER_02_016: [** Otherwise, OptionHandler_Destroy shall free all used resources. **]**
