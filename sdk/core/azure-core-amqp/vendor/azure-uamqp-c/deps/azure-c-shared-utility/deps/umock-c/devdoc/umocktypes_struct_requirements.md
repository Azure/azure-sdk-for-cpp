
# umocktypes_struct requirements

# Overview

umocktypes_struct is a module that exposes stringify, an equality comparer, copy, and free for structs in umock_c. It performs member-by-member operations.

# Exposed API

```c
#define UMOCK_DEFINE_TYPE_STRUCT_STRINGIFY(type, ...) \
    static char* MU_C2(umocktypes_stringify_, type)(const type* value) \
/*...*/

#define UMOCK_DEFINE_TYPE_STRUCT_ARE_EQUAL(type, ...) \
    static int MU_C2(umocktypes_are_equal_, type)(const type* left, const type* right) \
/*...*/

#define UMOCK_DEFINE_TYPE_STRUCT_COPY(type, ...) \
    static int MU_C2(umocktypes_copy_, type)(type* destination, const type* source) \
/*...*/

#define UMOCK_DEFINE_TYPE_STRUCT_FREE(type, ...) \
    static void MU_C2(umocktypes_free_, type)(type* value) \
/*...*/

#define UMOCK_DEFINE_TYPE_STRUCT(type, ...) \
    UMOCK_DEFINE_TYPE_STRUCT_STRINGIFY(type, __VA_ARGS__) \
    UMOCK_DEFINE_TYPE_STRUCT_ARE_EQUAL(type, __VA_ARGS__) \
    UMOCK_DEFINE_TYPE_STRUCT_COPY(type, __VA_ARGS__) \
    UMOCK_DEFINE_TYPE_STRUCT_FREE(type, __VA_ARGS__)
```

# Usage

```c
/// Header

#define MY_STRUCT_FIELDS \
    int, foo, \
    char, bar

MU_DEFINE_STRUCT(MY_STRUCT, MY_STRUCT_FIELDS)

/// Test file

// Define the umock c functions
UMOCK_DEFINE_TYPE_STRUCT(MY_STRUCT, MY_STRUCT_FIELDS)

// ...

// Register the functions with umock
REGISTER_TYPE(MY_STRUCT, MY_STRUCT);

// ...

// Now MY_STRUCT can be used in mock expectations

```

## umocktypes_stringify_\<type\>

```c
static char* MU_C2(umocktypes_stringify_, type)(const type* value)
```

**SRS_UMOCKTYPES_STRUCT_42_001: [** `umocktypes_stringify_<type>` shall call `umocktypes_stringify` for each field in `type`. **]**

**SRS_UMOCKTYPES_STRUCT_42_002: [** `umocktypes_stringify_<type>` shall generate a string containing all stringified fields in `type` and return it. **]**

**SRS_UMOCKTYPES_STRUCT_42_003: [** `umocktypes_stringify_<type>` shall free all of the stringified fields. **]**

**SRS_UMOCKTYPES_STRUCT_42_004: [** If there are any errors then `umocktypes_stringify_<type>` shall fail and return `NULL`. **]**

## umocktypes_are_equal_\<type\>

```c
static int MU_C2(umocktypes_are_equal_, type)(const type* left, const type* right)
```

**SRS_UMOCKTYPES_STRUCT_42_005: [** If `left` is `NULL` then `umocktypes_are_equal_<type>` shall fail and return `-1`. **]**

**SRS_UMOCKTYPES_STRUCT_42_006: [** If `right` is `NULL` then `umocktypes_are_equal_<type>` shall fail and return `-1`. **]**

**SRS_UMOCKTYPES_STRUCT_42_007: [** `umocktypes_are_equal_<type>` shall call `umocktypes_are_equal` for each field in `type`. **]**

**SRS_UMOCKTYPES_STRUCT_42_008: [** If any call to `umocktypes_are_equal` does not return `1` then `umocktypes_are_equal_<type>` shall return `0`. **]**

**SRS_UMOCKTYPES_STRUCT_42_009: [** Otherwise, `umocktypes_are_equal_<type>` shall return `1`. **]**

## umocktypes_copy_\<type\>

```c
static int MU_C2(umocktypes_copy_, type)(type* destination, const type* source)
```

**SRS_UMOCKTYPES_STRUCT_42_010: [** If `destination` is `NULL` then `umocktypes_copy_<type>` shall fail and return a non-zero value. **]**

**SRS_UMOCKTYPES_STRUCT_42_011: [** If `source` is `NULL` then `umocktypes_copy_<type>` shall fail and return a non-zero value. **]**

**SRS_UMOCKTYPES_STRUCT_42_012: [** `umocktypes_copy_<type>` shall call `umocktypes_copy` for each field in `type`. **]**

**SRS_UMOCKTYPES_STRUCT_42_013: [** If any call to `umocktypes_copy` does not return `0` then `umocktypes_copy_<type>` shall return a non-zero value. **]**

**SRS_UMOCKTYPES_STRUCT_42_014: [** Otherwise `umocktypes_copy_<type>` shall return `0`. **]**


## umocktypes_free_\<type\>

```c
static void MU_C2(umocktypes_free_, type)(type* value)
```

**SRS_UMOCKTYPES_STRUCT_42_015: [** `umocktypes_free_<type>` shall call `umocktypes_free` for each field in `type`. **]**
