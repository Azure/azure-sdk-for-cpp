
# umocktypes_wcharptr requirements

# Overview

`umocktypes_wcharptr` is a module that exposes out of the box functionality for `wchar_t*` and `const wchar_t*` types for `umockc`.

# Exposed API

```c
int umocktypes_wcharptr_register_types(void);

char* umocktypes_stringify_wcharptr(const wchar_t** value);
int umocktypes_are_equal_wcharptr(const wchar_t** left, const wchar_t** right);
int umocktypes_copy_wcharptr(wchar_t** destination, const wchar_t** source);
void umocktypes_free_wcharptr(wchar_t** value);

char* umocktypes_stringify_const_wcharptr(const wchar_t** value);
int umocktypes_are_equal_const_wcharptr(const wchar_t** left, const wchar_t** right);
int umocktypes_copy_const_wcharptr(const wchar_t** destination, const wchar_t** source);
void umocktypes_free_const_wcharptr(const wchar_t** value);
```

## umocktypes_wcharptr_register_types

```c
int umocktypes_wcharptr_register_types(void);
```

**SRS_UMOCKTYPES_WCHARPTR_01_001: [** `umocktypes_wcharptr_register_types` shall register support for the types `wchar_t*` and `const wchar_t*` by using the `REGISTER_UMOCK_VALUE_TYPE` macro provided by `umockc`. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_038: [** On success, `umocktypes_wcharptr_register_types` shall return 0. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_039: [** If registering any of the types fails, `umocktypes_wcharptr_register_types` shall fail and return a non-zero value. **]**

## umocktypes_stringify_wcharptr

```c
char* umocktypes_stringify_wcharptr(const wchar_t** value);
```

**SRS_UMOCKTYPES_WCHARPTR_01_002: [** `umocktypes_stringify_wcharptr` shall return a string containing the string representation of `value`, enclosed by quotes ("value"). **]**

**SRS_UMOCKTYPES_WCHARPTR_01_004: [** If `value` is `NULL`, `umocktypes_stringify_wcharptr` shall return `NULL`. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_003: [** If allocating a new string to hold the string representation fails, `umocktypes_stringify_wcharptr` shall return `NULL`. **]**

## umocktypes_are_equal_wcharptr

```c
int umocktypes_are_equal_wcharptr(const wchar_t** left, const wchar_t** right);
```
**SRS_UMOCKTYPES_WCHARPTR_42_001: [** If `left` is `NULL`, `umocktypes_are_equal_wcharptr` shall return -1. **]**

**SRS_UMOCKTYPES_WCHARPTR_42_002: [** If `right` is `NULL`, `umocktypes_are_equal_wcharptr` shall return -1. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_005: [** `umocktypes_are_equal_wcharptr` shall compare the 2 strings pointed to by `left` and `right`. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_006: [** The comparison shall be case sensitive. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_007: [** If `left` and `right` are equal, `umocktypes_are_equal_wcharptr` shall return 1. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_008: [** If only one of the `left` and `right` argument is `NULL`, `umocktypes_are_equal_wcharptr` shall return 0. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_009: [** If the string pointed to by `left` is equal to the string pointed to by `right`, `umocktypes_are_equal_wcharptr` shall return 1. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_010: [** If the string pointed to by `left` is different than the string pointed to by `right`, `umocktypes_are_equal_wcharptr` shall return 0. **]**

## umocktypes_copy_wcharptr

```c
int umocktypes_copy_wcharptr(wchar_t** destination, const wchar_t** source);
```

**SRS_UMOCKTYPES_WCHARPTR_01_011: [** `umocktypes_copy_wcharptr` shall allocate a new sequence of chars by using `umockalloc_malloc`. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_012: [** The number of bytes allocated shall accommodate the string pointed to by `source`. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_014: [** `umocktypes_copy_wcharptr` shall copy the string pointed to by `source` to the newly allocated memory. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_015: [** The newly allocated string shall be returned in the `destination` argument. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_016: [** On success `umocktypes_copy_wcharptr` shall return 0. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_013: [** If `source` or `destination` are `NULL`, `umocktypes_copy_wcharptr` shall return a non-zero value. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_036: [** If allocating the memory for the new string fails, `umocktypes_copy_wcharptr` shall fail and return a non-zero value. **]**

## umocktypes_free_wcharptr

```c
void umocktypes_free_wcharptr(wchar_t** value);
```

**SRS_UMOCKTYPES_WCHARPTR_01_017: [** `umocktypes_free_wcharptr` shall free the string pointed to by `value`. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_018: [** If `value` is `NULL`, `umocktypes_free_wcharptr` shall do nothing. **]**

## umocktypes_stringify_const_wcharptr

```c
char* umocktypes_stringify_const_wcharptr(const wchar_t** value);
```

**SRS_UMOCKTYPES_WCHARPTR_01_019: [** `umocktypes_stringify_const_wcharptr` shall return a string containing the string representation of `value`, enclosed by quotes ("value"). **]**

**SRS_UMOCKTYPES_WCHARPTR_01_020: [** If `value` is `NULL`, `umocktypes_stringify_const_wcharptr` shall return `NULL`. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_021: [** If allocating a new string to hold the string representation fails, `umocktypes_stringify_const_wcharptr` shall return `NULL`. **]**

## umocktypes_are_equal_const_wcharptr

```c
int umocktypes_are_equal_const_wcharptr(const wchar_t** left, const wchar_t** right);
```

**SRS_UMOCKTYPES_WCHARPTR_01_022: [** `umocktypes_are_equal_const_wcharptr` shall compare the 2 strings pointed to by `left` and `right`. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_023: [** The comparison shall be case sensitive. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_024: [** If `left` and `right` are equal, `umocktypes_are_equal_const_wcharptr` shall return 1. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_025: [** If only one of the `left` and `right` argument is `NULL`, `umocktypes_are_equal_const_wcharptr` shall return 0. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_026: [** If the string pointed to by `left` is equal to the string pointed to by `right`, `umocktypes_are_equal_const_wcharptr` shall return 1. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_027: [** If the string pointed to by `left` is different than the string pointed to by `right`, `umocktypes_are_equal_const_wcharptr` shall return 0. **]**

## umocktypes_copy_const_wcharptr

```c
int umocktypes_copy_const_wcharptr(const wchar_t** destination, const wchar_t** source);
```

**SRS_UMOCKTYPES_WCHARPTR_01_028: [** `umocktypes_copy_const_wcharptr` shall allocate a new sequence of chars by using `umockalloc_malloc`. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_029: [** The number of bytes allocated shall accommodate the string pointed to by `source`. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_030: [** `umocktypes_copy_const_wcharptr` shall copy the string pointed to by `source` to the newly allocated memory. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_031: [** The newly allocated string shall be returned in the `destination` argument. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_032: [** On success `umocktypes_copy_const_wcharptr` shall return 0. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_033: [** If `source` or `destination` are `NULL`, `umocktypes_copy_const_wcharptr` shall return a non-zero value. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_037: [** If allocating the memory for the new string fails, `umocktypes_copy_const_wcharptr` shall fail and return a non-zero value. **]**

## umocktypes_free_const_wcharptr

```c
void umocktypes_free_const_wcharptr(const wchar_t** value);
```

**SRS_UMOCKTYPES_WCHARPTR_01_034: [** `umocktypes_free_const_wcharptr` shall free the string pointed to by `value`. **]**

**SRS_UMOCKTYPES_WCHARPTR_01_035: [** If `value` is `NULL`, `umocktypes_free_const_wcharptr` shall do nothing. **]**
