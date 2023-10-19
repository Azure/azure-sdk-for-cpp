# constbuffer_array_requirements
================

## Overview

`constbuffer_array` is a module that stiches several `CONSTBUFFER_HANDLE`s together. `constbuffer_array` can add/remove a `CONSTBUFFER_HANDLE` at the beginning (front) of the already constructed stitch. `constbuffer_array` can merge with another `constbuffer_array` by appending the contents of one array to the other.

`CONSTBUFFER_ARRAY_HANDLE`s are immutable, that is, adding/removing a `CONSTBUFFER_HANDLE` to/from an existing `CONSTBUFFER_ARRAY_HANDLE` will result in a new `CONSTBUFFER_ARRAY_HANDLE`.

## Exposed API

```c
typedef struct CONSTBUFFER_ARRAY_HANDLE_DATA_TAG* CONSTBUFFER_ARRAY_HANDLE;

MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_create, const CONSTBUFFER_HANDLE*, buffers, uint32_t, buffer_count);
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_create_with_move_buffers, CONSTBUFFER_HANDLE*, buffers, uint32_t, buffer_count);
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_create_empty);
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_create_from_array_array, const CONSTBUFFER_ARRAY_HANDLE*, buffer_arrays, uint32_t, buffer_array_count);

MOCKABLE_FUNCTION(, void, constbuffer_array_inc_ref, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_handle);
MOCKABLE_FUNCTION(, void, constbuffer_array_dec_ref, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_handle);

/*add in front*/
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_add_front, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_handle, CONSTBUFFER_HANDLE, constbuffer_handle);

/*remove front*/
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_remove_front, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_handle, CONSTBUFFER_HANDLE *const_buffer_handle);

/* getters */
MOCKABLE_FUNCTION(, int, constbuffer_array_get_buffer_count, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_handle, uint32_t*, buffer_count);
MOCKABLE_FUNCTION(, CONSTBUFFER_HANDLE, constbuffer_array_get_buffer, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_handle, uint32_t, buffer_index);
MOCKABLE_FUNCTION(, const CONSTBUFFER*, constbuffer_array_get_buffer_content, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_handle, uint32_t, buffer_index);
MOCKABLE_FUNCTION(, int, constbuffer_array_get_all_buffers_size, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_handle, uint32_t*, all_buffers_size);
MOCKABLE_FUNCTION(, const CONSTBUFFER_HANDLE*, constbuffer_array_get_const_buffer_handle_array, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_handle);

/*compare*/
MOCKABLE_FUNCTION(, bool, CONSTBUFFER_ARRAY_HANDLE_contain_same, CONSTBUFFER_ARRAY_HANDLE, left, CONSTBUFFER_ARRAY_HANDLE, right);
```

### constbuffer_array_create

```c
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_create, const CONSTBUFFER_HANDLE*, buffers, uint32_t, buffer_count);
```

`constbuffer_array_create` creates a new const buffer array made of the const buffers in `buffers`.

**SRS_CONSTBUFFER_ARRAY_01_009: [** `constbuffer_array_create` shall allocate memory for a new `CONSTBUFFER_ARRAY_HANDLE` that can hold `buffer_count` buffers. **]**

**SRS_CONSTBUFFER_ARRAY_01_010: [** `constbuffer_array_create` shall clone the buffers in `buffers` and store them. **]**

**SRS_CONSTBUFFER_ARRAY_01_011: [** On success `constbuffer_array_create` shall return a non-NULL handle. **]**

**SRS_CONSTBUFFER_ARRAY_01_012: [** If `buffers` is NULL and `buffer_count` is not 0, `constbuffer_array_create` shall fail and return NULL. **]**

**SRS_CONSTBUFFER_ARRAY_01_014: [** If any error occurs, `constbuffer_array_create` shall fail and return NULL. **]**

### constbuffer_array_create_with_move_buffers

```c
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_create_with_move_buffers, CONSTBUFFER_HANDLE*, buffers, uint32_t, buffer_count);
```

`constbuffer_array_create_with_move_buffers` creates a new const buffer array made of the const buffers in `buffers` with move memory semantics for `buffers`.

Note: `constbuffer_array_create_with_move_buffers` does not increment the reference count of the buffer handles in `buffers`.

**SRS_CONSTBUFFER_ARRAY_01_028: [** If `buffers` is `NULL` and `buffer_count` is not 0, `constbuffer_array_create_with_move_buffers` shall fail and return `NULL`. **]**

**SRS_CONSTBUFFER_ARRAY_01_029: [** Otherwise, `constbuffer_array_create_with_move_buffers` shall allocate memory for a new `CONSTBUFFER_ARRAY_HANDLE` that holds the const buffers in `buffers`. **]**

**SRS_CONSTBUFFER_ARRAY_01_031: [** On success `constbuffer_array_create_with_move_buffers` shall return a non-`NULL` handle. **]**

**SRS_CONSTBUFFER_ARRAY_01_030: [** If any error occurs, `constbuffer_array_create_with_move_buffers` shall fail and return `NULL`. **]**

### constbuffer_array_create_empty

```c
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_create_empty);
```

`constbuffer_array_create_empty` creates a new, empty CONSTBUFFER_ARRAY_HANDLE.

**SRS_CONSTBUFFER_ARRAY_02_004: [** `constbuffer_array_create_empty` shall allocate memory for a new `CONSTBUFFER_ARRAY_HANDLE`. **]**

**SRS_CONSTBUFFER_ARRAY_02_041: [** `constbuffer_array_create_empty` shall succeed and return a non-`NULL` value. **]**

**SRS_CONSTBUFFER_ARRAY_02_001: [** If are any failure is encountered, `constbuffer_array_create_empty` shall fail and return `NULL`. **]**

### constbuffer_array_create_from_array_array

```c
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_create_from_array_array, const CONSTBUFFER_ARRAY_HANDLE*, buffer_arrays, uint32_t, buffer_array_count);
```

`constbuffer_array_create_from_array_array` creates a new const buffer array made of all the const buffers in `buffer_arrays`.

**SRS_CONSTBUFFER_ARRAY_42_009: [** If `buffer_arrays` is `NULL` and `buffer_array_count` is not 0 then `constbuffer_array_create_from_array_array` shall fail and return `NULL`. **]**

**SRS_CONSTBUFFER_ARRAY_42_001: [** If `buffer_arrays` is `NULL` or `buffer_array_count` is 0 then `constbuffer_array_create_from_array_array` shall create a new, empty `CONSTBUFFER_ARRAY_HANDLE`. **]**

**SRS_CONSTBUFFER_ARRAY_42_002: [** If any const buffer array in `buffer_arrays` is `NULL` then `constbuffer_array_create_from_array_array` shall fail and return `NULL`. **]**

**SRS_CONSTBUFFER_ARRAY_42_003: [** `constbuffer_array_create_from_array_array` shall allocate memory to hold all of the `CONSTBUFFER_HANDLES` from `buffer_arrays`. **]**

**SRS_CONSTBUFFER_ARRAY_42_004: [** `constbuffer_array_create_from_array_array` shall copy all of the `CONSTBUFFER_HANDLES` from each const buffer array in `buffer_arrays` to the newly constructed array by calling `CONSTBUFFER_IncRef`. **]**

**SRS_CONSTBUFFER_ARRAY_42_007: [** `constbuffer_array_create_from_array_array` shall succeed and return a non-`NULL` value. **]**

**SRS_CONSTBUFFER_ARRAY_42_008: [** If there are any failures then `constbuffer_array_create_from_array_array` shall fail and return `NULL`. **]**

### constbuffer_array_inc_ref

```c
MOCKABLE_FUNCTION(, void, constbuffer_array_inc_ref, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_handle);
```

`constbuffer_array_inc_ref` increments the reference count for `constbuffer_array_handle`.

**SRS_CONSTBUFFER_ARRAY_01_017: [** If `constbuffer_array_handle` is `NULL` then `constbuffer_array_inc_ref` shall return. **]**

**SRS_CONSTBUFFER_ARRAY_01_018: [** Otherwise `constbuffer_array_inc_ref` shall increment the reference count for `constbuffer_array_handle`. **]**

### constbuffer_array_dec_ref

```c
MOCKABLE_FUNCTION(, void, constbuffer_array_dec_ref, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_handle);
```

`constbuffer_array_dec_ref` decrements the reference count and frees all used resources if needed.

**SRS_CONSTBUFFER_ARRAY_02_039: [** If `constbuffer_array_handle` is `NULL` then `constbuffer_array_dec_ref` shall return. **]**

**SRS_CONSTBUFFER_ARRAY_01_016: [** Otherwise `constbuffer_array_dec_ref` shall decrement the reference count for `constbuffer_array_handle`. **]**

**SRS_CONSTBUFFER_ARRAY_02_038: [** If the reference count reaches 0, `constbuffer_array_dec_ref` shall free all used resources. **]**

### constbuffer_array_add_front

```c
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_add_front, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_handle, CONSTBUFFER_HANDLE, constbuffer_handle);
```

`constbuffer_array_add_front` adds a new `CONSTBUFFER_HANDLE` at the front of the already stored `CONSTBUFFER_HANDLE`s.

**SRS_CONSTBUFFER_ARRAY_02_006: [** If `constbuffer_array_handle` is `NULL` then `constbuffer_array_add_front` shall fail and return `NULL` **]**

**SRS_CONSTBUFFER_ARRAY_02_007: [** If `constbuffer_handle` is `NULL` then `constbuffer_array_add_front` shall fail and return `NULL` **]**

**SRS_CONSTBUFFER_ARRAY_02_042: [** `constbuffer_array_add_front` shall allocate enough memory to hold all of `constbuffer_array_handle` existing `CONSTBUFFER_HANDLE` and `constbuffer_handle`. **]**

**SRS_CONSTBUFFER_ARRAY_02_043: [** `constbuffer_array_add_front` shall copy `constbuffer_handle` and all of `constbuffer_array_handle` existing `CONSTBUFFER_HANDLE`. **]**

**SRS_CONSTBUFFER_ARRAY_02_044: [** `constbuffer_array_add_front` shall inc_ref all the `CONSTBUFFER_HANDLE` it had copied. **]**

**SRS_CONSTBUFFER_ARRAY_02_010: [** `constbuffer_array_add_front` shall succeed and return a non-`NULL` value. **]**

**SRS_CONSTBUFFER_ARRAY_02_011: [** If there any failures `constbuffer_array_add_front` shall fail and return `NULL`. **]**

### constbuffer_array_remove_front

```c
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_remove_front, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_handle, CONSTBUFFER_HANDLE* const_buffer_handle);
```

`constbuffer_array_remove_front` removes the front `CONSTBUFFER_HANDLE` and hands it over to the caller.

**SRS_CONSTBUFFER_ARRAY_02_012: [** If `constbuffer_array_handle` is `NULL` then `constbuffer_array_remove_front` shall fail and return `NULL`. **]**

**SRS_CONSTBUFFER_ARRAY_02_045: [** If `constbuffer_handle` is `NULL` then `constbuffer_array_remove_front` shall fail and return `NULL`. **]**

**SRS_CONSTBUFFER_ARRAY_02_013: [** If there is no front `CONSTBUFFER_HANDLE` then `constbuffer_array_remove_front` shall fail and return `NULL`. **]**

**SRS_CONSTBUFFER_ARRAY_02_002: [** `constbuffer_array_remove_front` shall fail when called on a newly constructed `CONSTBUFFER_ARRAY_HANDLE`. **]**

**SRS_CONSTBUFFER_ARRAY_02_046: [** `constbuffer_array_remove_front` shall allocate memory to hold all of `constbuffer_array_handle` `CONSTBUFFER_HANDLE`s except the front one. **]**

**SRS_CONSTBUFFER_ARRAY_02_047: [** `constbuffer_array_remove_front` shall copy all of `constbuffer_array_handle` `CONSTBUFFER_HANDLE`s except the front one. **]**

**SRS_CONSTBUFFER_ARRAY_02_048: [** `constbuffer_array_remove_front` shall inc_ref all the copied `CONSTBUFFER_HANDLE`s. **]**

**SRS_CONSTBUFFER_ARRAY_01_001: [** `constbuffer_array_remove_front` shall inc_ref the removed buffer. **]**

**SRS_CONSTBUFFER_ARRAY_02_049: [** `constbuffer_array_remove_front` shall succeed and return a non-`NULL` value. **]**

**SRS_CONSTBUFFER_ARRAY_02_036: [** If there are any failures then `constbuffer_array_remove_front` shall fail and return `NULL`. **]**

### constbuffer_array_get_buffer_count

```c
MOCKABLE_FUNCTION(, int, constbuffer_array_get_buffer_count, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_handle, uint32_t*, buffer_count);
```

`constbuffer_array_get_buffer_count` gets the count of const buffers held by the const buffer array.

**SRS_CONSTBUFFER_ARRAY_01_002: [** On success, `constbuffer_array_get_buffer_count` shall return 0 and write the buffer count in `buffer_count`. **]**

**SRS_CONSTBUFFER_ARRAY_01_003: [** If `constbuffer_array_handle` is NULL, `constbuffer_array_get_buffer_count` shall fail and return a non-zero value. **]**

**SRS_CONSTBUFFER_ARRAY_01_004: [** If `buffer_count` is NULL, `constbuffer_array_get_buffer_count` shall fail and return a non-zero value. **]**

### constbuffer_array_get_buffer

```c
MOCKABLE_FUNCTION(, CONSTBUFFER_HANDLE, constbuffer_array_get_buffer, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_handle, uint32_t, buffer_index);
```

`constbuffer_array_get_buffer` returns the buffer at the `buffer_index`-th given index in the array.

**SRS_CONSTBUFFER_ARRAY_01_005: [** On success, `constbuffer_array_get_buffer` shall return a non-NULL handle to the `buffer_index`-th const buffer in the array. **]**

**SRS_CONSTBUFFER_ARRAY_01_006: [** The returned handle shall have its reference count incremented. **]**

**SRS_CONSTBUFFER_ARRAY_01_007: [** If `constbuffer_array_handle` is NULL, `constbuffer_array_get_buffer` shall fail and return NULL. **]**

**SRS_CONSTBUFFER_ARRAY_01_008: [** If `buffer_index` is greater or equal to the number of buffers in the array, `constbuffer_array_get_buffer` shall fail and return NULL. **]**

### constbuffer_array_get_buffer_content

```c
MOCKABLE_FUNCTION(, const CONSTBUFFER*, constbuffer_array_get_buffer_content, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_handle, uint32_t, buffer_index);
```

`constbuffer_array_get_buffer_content` gets the buffer content for the buffer at the `buffer_index`-th given index in the array.

**SRS_CONSTBUFFER_ARRAY_01_023: [** If `constbuffer_array_handle` is NULL, `constbuffer_array_get_buffer_content` shall fail and return NULL. **]**

**SRS_CONSTBUFFER_ARRAY_01_024: [** If `buffer_index` is greater or equal to the number of buffers in the array, `constbuffer_array_get_buffer_content` shall fail and return NULL. **]**

**SRS_CONSTBUFFER_ARRAY_01_025: [** Otherwise `constbuffer_array_get_buffer_content` shall call `CONSTBUFFER_GetContent` for the `buffer_index`-th buffer and return its result. **]**

### constbuffer_array_get_all_buffers_size

```c
MOCKABLE_FUNCTION(, int, constbuffer_array_get_all_buffers_size, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_handle, uint32_t*, all_buffers_size);
```

`constbuffer_array_get_all_buffers_size` gets the size for all buffers (how much memory is held by all buffers in the array).

**SRS_CONSTBUFFER_ARRAY_01_019: [** If `constbuffer_array_handle` is NULL, `constbuffer_array_get_all_buffers_size` shall fail and return a non-zero value. **]**

**SRS_CONSTBUFFER_ARRAY_01_020: [** If `all_buffers_size` is NULL, `constbuffer_array_get_all_buffers_size` shall fail and return a non-zero value. **]**

**SRS_CONSTBUFFER_ARRAY_01_021: [** If summing up the sizes results in an `uint32_t` overflow, shall fail and return a non-zero value. **]**

**SRS_CONSTBUFFER_ARRAY_01_022: [** Otherwise `constbuffer_array_get_all_buffers_size` shall write in `all_buffers_size` the total size of all buffers in the array and return 0. **]**

### constbuffer_array_get_const_buffer_handle_array

```c
MOCKABLE_FUNCTION(, const CONSTBUFFER_HANDLE*, constbuffer_array_get_const_buffer_handle_array, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_handle);
```

`constbuffer_array_get_const_buffer_handle_array` gets a const array with the handles for all the const buffers in the array.

**SRS_CONSTBUFFER_ARRAY_01_026: [** If `constbuffer_array_handle` is NULL, `constbuffer_array_get_const_buffer_handle_array` shall fail and return NULL. **]**

**SRS_CONSTBUFFER_ARRAY_01_027: [** Otherwise `constbuffer_array_get_const_buffer_handle_array` shall return the array of const buffer handles backing the const buffer array. **]**

### CONSTBUFFER_ARRAY_HANDLE_contain_same
```c
MOCKABLE_FUNCTION(, bool, CONSTBUFFER_ARRAY_HANDLE_contain_same, CONSTBUFFER_ARRAY_HANDLE, left, CONSTBUFFER_ARRAY_HANDLE, right);
```

`CONSTBUFFER_ARRAY_HANDLE_contain_same` returns `true` if `left` and `right` have the some content.

**SRS_CONSTBUFFER_ARRAY_02_050: [** If `left` is `NULL` and `right` is `NULL` then `CONSTBUFFER_ARRAY_HANDLE_contain_same` shall return `true`. **]**

**SRS_CONSTBUFFER_ARRAY_02_051: [** If `left` is `NULL` and `right` is not `NULL` then `CONSTBUFFER_ARRAY_HANDLE_contain_same` shall return `false`. **]**

**SRS_CONSTBUFFER_ARRAY_02_052: [** If `left` is not `NULL` and `right` is `NULL` then `CONSTBUFFER_ARRAY_HANDLE_contain_same` shall return `false`. **]**

**SRS_CONSTBUFFER_ARRAY_02_053: [** If the number of `CONSTBUFFER_HANDLE`s in `left` is different then the number of `CONSTBUFFER_HANDLE`s in `right` then  `CONSTBUFFER_ARRAY_HANDLE_contain_same` shall return `false`. **]**

**SRS_CONSTBUFFER_ARRAY_02_054: [** If `left` and `right` `CONSTBUFFER_HANDLE`s at same index are different (as indicated by `CONSTBUFFER_HANDLE_contain_same` call) then `CONSTBUFFER_ARRAY_HANDLE_contain_same` shall return `false`. **]**

**SRS_CONSTBUFFER_ARRAY_02_055: [** `CONSTBUFFER_ARRAY_HANDLE_contain_same` shall return `true`. **]**

