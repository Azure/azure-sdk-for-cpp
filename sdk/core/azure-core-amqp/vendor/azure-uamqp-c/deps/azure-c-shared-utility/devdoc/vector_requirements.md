VECTOR Requirements
================

## Overview

The VECTOR object is an index based collection of uniform size elements.

## Exposed API
```c

typedef struct VECTOR_TAG* VECTOR_HANDLE;

typedef bool(*PREDICATE_FUNCTION)(const void* element, const void* value);

/* creation */
extern VECTOR_HANDLE VECTOR_create(size_t elementSize);
extern VECTOR_HANDLE VECTOR_move(VECTOR_HANDLE handle);
extern void VECTOR_destroy(VECTOR_HANDLE handle);

/* insertion */
extern int VECTOR_push_back(VECTOR_HANDLE handle, const void* elements, size_t numElements);

/* removal */
extern void VECTOR_erase(VECTOR_HANDLE handle, void* elements, size_t numElements);
extern void VECTOR_clear(VECTOR_HANDLE handle);

/* access */
extern void* VECTOR_element(VECTOR_HANDLE handle, size_t index);
extern void* VECTOR_front(VECTOR_HANDLE handle);
extern void* VECTOR_back(VECTOR_HANDLE handle);
extern void* VECTOR_find_if(VECTOR_HANDLE handle, PREDICATE_FUNCTION pred, const void* value);

/* capacity */
extern size_t VECTOR_size(VECTOR_HANDLE handle);
```

###  PREDICATE_FUNCTION
```c
bool(*PREDICATE_FUNCTION)(const void* element, const void* value);
/**
 *  PREDICATE_FUNCTION defines a function prototype that is used in conjunction with `VECTOR_find_if()`.
 *     VECTOR_find_if will iterate through each element of the object and call the given function
 *     with two parameters. Once a match is found, a pointer to the matched element will be returned.
 *     NULL will be returned if no match is found.
 *     Care must be taken if one or both of the arguments are of type VECTOR_HANDLE. For example,
 *     calling the `VECTOR_move` from inside a `PREDICATE_FUNCTION` is not supported and may cause undefined
 *     behavior.
 *
 *     -  element: points to the member of the array being evaluated.
 *     -  value:   points to a variable whose contents will be used to find a matching element.
 **/
    
```

###  VECTOR_create
```c
VECTOR_HANDLE VECTOR_create(size_t elementSize)
```

**SRS_VECTOR_10_001: [** VECTOR_create shall allocate a VECTOR_HANDLE that will contain an empty vector. The size of each element is given in elementSize. **]**

**SRS_VECTOR_10_002: [** VECTOR_create shall fail and return NULL if elementsize is equal to 0. **]**

**SRS_VECTOR_10_033: [** VECTOR_create shall fail and return NULL if malloc fails. **]**

###  VECTOR_move
```c
VECTOR_HANDLE VECTOR_move(VECTOR_HANDLE handle)
```

**SRS_VECTOR_10_004: [** VECTOR_move shall allocate a VECTOR_HANLDE and move the data to it from the given handle. **]**

**SRS_VECTOR_10_005: [** VECTOR_move shall fail and return NULL if the given handle is NULL. **]**

**SRS_VECTOR_10_006: [** VECTOR_move shall fail and return NULL if malloc fails. **]**

###  VECTOR_destroy
```c
void VECTOR_destroy(VECTOR_HANDLE handle)
```

**SRS_VECTOR_10_008: [** VECTOR_destroy shall free the given handle and its internal storage. **]**

**SRS_VECTOR_10_009: [** VECTOR_destroy shall return if the given handle is NULL. **]**

###  VECTOR_push_back
```c
int VECTOR_push_back(VECTOR_HANDLE handle, const void* elements, size_t numElements)
```

**SRS_VECTOR_10_011: [** VECTOR_push_back shall fail and return non-zero if `handle` is NULL. **]**

**SRS_VECTOR_10_034: [** VECTOR_push_back shall fail and return non-zero if `elements` is NULL. **]**

**SRS_VECTOR_10_035: [** VECTOR_push_back shall fail and return non-zero if `numElements` is 0. **]**

**SRS_VECTOR_10_012: [** VECTOR_push_back shall fail and return non-zero if memory allocation fails. **]**

**SRS_VECTOR_10_013: [** VECTOR_push_back shall append the given elements and return 0 indicating success. **]**

###  VECTOR_erase
```c
void VECTOR_erase(VECTOR_HANDLE handle, void* elements, size_t numElements)
```

**SRS_VECTOR_10_014: [** VECTOR_erase shall remove the `numElements` starting at `elements` and reduce its internal storage. **]**

**SRS_VECTOR_10_015: [** VECTOR_erase shall return if `handle` is NULL. **]**

**SRS_VECTOR_10_038: [** VECTOR_erase shall return if `elements` is NULL. **]**

**SRS_VECTOR_10_040: [** VECTOR_erase shall return if `elements` is out of bound. **]**

**SRS_VECTOR_10_041: [** VECTOR_erase shall return if `elements` is misaligned. **]**

**SRS_VECTOR_10_039: [** VECTOR_erase shall return if `numElements` is 0. **]**

**SRS_VECTOR_10_027: [** VECTOR_erase shall return if `numElements` is out of bound. **]**


###  VECTOR_clear
```c
void VECTOR_clear(VECTOR_HANDLE handle)
```

**SRS_VECTOR_10_016: [** VECTOR_clear shall remove all elements from the object and release internal storage. **]**

**SRS_VECTOR_10_017: [** VECTOR_clear shall return if the object is NULL or empty. **]**

###  VECTOR_element
```c
void* VECTOR_element(VECTOR_HANDLE handle, size_t index)
```


**SRS_VECTOR_10_018: [** VECTOR_element shall return a pointer to the element at the given index. **]**

**SRS_VECTOR_10_019: [** VECTOR_element shall fail and return NULL if handle is NULL. **]**

**SRS_VECTOR_10_020: [** VECTOR_element shall fail and return NULL if the given index is out of range. **]**

###  VECTOR_front
```c
void* VECTOR_front(VECTOR_HANDLE handle)
```


**SRS_VECTOR_10_021: [** VECTOR_front shall return the element at index 0. **]**

**SRS_VECTOR_10_022: [** VECTOR_front shall fail and return NULL if handle is NULL. **]**

**SRS_VECTOR_10_028: [** VECTOR_front shall fail and return NULL if the vector is empty. **]**

###  VECTOR_back
```c
void* VECTOR_back(VECTOR_HANDLE handle)
```


**SRS_VECTOR_10_023: [** VECTOR_back shall return the last element of the vector. **]**

**SRS_VECTOR_10_024: [** VECTOR_back shall fail and return NULL if handle is NULL. **]**

**SRS_VECTOR_10_029: [** VECTOR_back shall fail and return NULL if the vector is empty. **]**

###  VECTOR_find_if
```c
void* VECTOR_find_if(VECTOR_HANDLE handle, PREDICATE_FUNCTION pred, const void* value)
```


**SRS_VECTOR_10_030: [** VECTOR_find_if shall fail and return NULL if `handle` is NULL. **]**

**SRS_VECTOR_10_036: [** VECTOR_find_if shall fail and return NULL if `pred` is NULL. **]**

**SRS_VECTOR_10_031: [** VECTOR_find_if shall return the first element in the vector that matches `pred`. **]**

**SRS_VECTOR_10_032: [** VECTOR_find_if shall return NULL if no matching element is found. **]**

###  VECTOR_size
```c
size_t VECTOR_size(VECTOR_HANDLE handle)
```

**SRS_VECTOR_10_025: [** VECTOR_size shall return the number of elements stored with the given handle. **]**

**SRS_VECTOR_10_026: [** VECTOR_size shall return 0 if the given handle is NULL. **]**