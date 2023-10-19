`refcount` requirements
================

## Overview

`refcount` is a module that provides reference counting for a given structure.
It wraps the structure that needs to be ref counted into another structure that contains an additional field (ref count).

## Exposed API

```c
#define REFCOUNT_TYPE_CREATE(type) MU_C2(REFCOUNT_SHORT_TYPE(type), _Create)()
#define REFCOUNT_TYPE_CREATE_WITH_EXTRA_SIZE(type, size) MU_C2(REFCOUNT_SHORT_TYPE(type), _Create_With_Extra_Size)(size)
#define REFCOUNT_TYPE_DESTROY(type, var) MU_C2(REFCOUNT_SHORT_TYPE(type), _Destroy)(var)

#define DEFINE_REFCOUNT_TYPE(type) \
...
```

### DEFINE_REFCOUNT_TYPE

**SRS_REFCOUNT_01_001: [** `DEFINE_REFCOUNT_TYPE` shall define the create/create_with_Extra_size/destroy functions for the type `type`. **]**

### REFCOUNT_TYPE_CREATE

```c
REFCOUNT_TYPE_CREATE(type)
```

`REFCOUNT_TYPE_CREATE` creates a ref counted object instance.

**SRS_REFCOUNT_01_002: [** `REFCOUNT_TYPE_CREATE` shall allocate memory for the type that is ref counted. **]**

**SRS_REFCOUNT_01_003: [** On success it shall return a non-NULL handle to the allocated ref counted type `type`. **]**

**SRS_REFCOUNT_01_004: [** If any error occurs, `REFCOUNT_TYPE_CREATE` shall return NULL. **]**

### REFCOUNT_TYPE_CREATE_WITH_EXTRA_SIZE

```c
REFCOUNT_TYPE_CREATE_WITH_EXTRA_SIZE(type, size)
```

`REFCOUNT_TYPE_CREATE_WITH_EXTRA_SIZE` creates a ref counted object instance while allocating extra memory (useful for flexible arrays).

**SRS_REFCOUNT_01_005: [** `REFCOUNT_TYPE_CREATE_WITH_EXTRA_SIZE` shall allocate memory for the type that is ref counted (`type`) plus extra memory enough to hold `size` bytes. **]**

**SRS_REFCOUNT_01_006: [** On success it shall return a non-NULL handle to the allocated ref counted type `type`. **]**

**SRS_REFCOUNT_01_007: [** If any error occurs, `REFCOUNT_TYPE_CREATE_WITH_EXTRA_SIZE` shall return NULL. **]**

### REFCOUNT_TYPE_DESTROY

```c
REFCOUNT_TYPE_DESTROY(type, counted_type)
```

`REFCOUNT_TYPE_DESTROY` frees the memory for a ref counted type.

**SRS_REFCOUNT_01_008: [** `REFCOUNT_TYPE_DESTROY` shall free the memory allocated by `REFCOUNT_TYPE_CREATE` or `REFCOUNT_TYPE_CREATE_WITH_EXTRA_SIZE`. **]**

**SRS_REFCOUNT_01_009: [** If `counted_type` is NULL, `REFCOUNT_TYPE_DESTROY` shall return. **]**

