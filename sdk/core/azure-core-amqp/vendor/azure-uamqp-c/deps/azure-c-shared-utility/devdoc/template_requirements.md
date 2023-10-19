template
=========

## Overview

Template is a fake project to demonstrate how to create a new code for the Azure IoT SDKs using the unit test suite together with mockable functions. This sample simulate a target module that calls functions in the callee module. The unit test will test the target functions emulating different behaviors for the callee mockable functions.
To complete this sample, this document describes the requirements for the target functions. If you are creating a new module for the SDK, it is mandatory that you create the requirements, each requirement must show up in the code, and must have, at least, one unity test to cover it.
Note that it is just a sample, so, this requirement does not contemplate all possible situations and we only have unit tests for the requirements that helped to demonstrate a test suite feature. 

## References

Add any reference that can support this implementation here. For example, it the target implement a TLS protocol, it can have the follow references:

[TLS Protocol (RFC2246)](https://www.ietf.org/rfc/rfc2246.txt)

[TLS Protocol (generic information)](https://en.wikipedia.org/wiki/Transport_Layer_Security)


## Exposed API

**SRS_TEMPLATE_21_011: [** The target shall return the errors defined by the TARGET_RESULT in target.h. **]**
```c
#define TARGET_RESULT_VALUES         \
    TARGET_RESULT_OK,                \
    TARGET_RESULT_FAIL,              \
    TARGET_RESULT_OUT_OF_MEMORY
MU_DEFINE_ENUM(TARGET_RESULT, TARGET_RESULT_VALUES);
```
 **]**  

**SRS_TEMPLATE_21_012: [** The target shall implement the APIs defined in target.h:
```c
TARGET_RESULT target_create(size_t size);
void target_destroy(void);
TARGET_RESULT target_foo(void);
```
 **]**  


###   target_create
```c
TARGET_RESULT target_create(size_t size);
```

**SRS_TEMPLATE_21_001: [** The target_create shall call callee_open to do stuff and allocate the memory. **]**

**SRS_TEMPLATE_21_002: [** If callee_open return error, the target_create shall return TARGET_RESULT_FAIL. **]**

**SRS_TEMPLATE_21_003: [** If there is no memory to control the target_create information, it shall return TARGET_RESULT_OUT_OF_MEMORY. **]**

**SRS_TEMPLATE_21_008: [** If callee_open got success, it shall return TARGET_RESULT_OK. **]**

**SRS_TEMPLATE_21_009: [** If callee_open is called but the connection is already created, it shall return TARGET_RESULT_OK. **]**  


###   target_destroy
```c
void target_destroy(void);
```

**SRS_TEMPLATE_21_006: [** The target_destroy shall call callee_close to do stuff and free the memory. **]**

**SRS_TEMPLATE_21_007: [** If target_destroy is called but the connection is not created, the target_destroy shall not do anything. **]**  


###   target_foo
```c
TARGET_RESULT target_foo(void);
```

**SRS_TEMPLATE_21_004: [** The target_foo shall do stuff calling callee_bar_1 and callee_bar_2. **]**

**SRS_TEMPLATE_21_005: [** If target_foo is called but the connection is not created, the target_foo shall return TARGET_RESULT_FAIL. **]**

**SRS_TEMPLATE_21_010: [** If target_foo cannot execute foo, the target_foo shall return TARGET_RESULT_FAIL. **]**  

        