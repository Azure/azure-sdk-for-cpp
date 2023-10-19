
# umockcallpairs requirements

# Overview

umockcallpairs is a module that handles tracking of call pairs.

# Exposed API

```c
    typedef struct PAIRED_HANDLE_TAG
    {
        void* handle_value;
        char* handle_type;
    } PAIRED_HANDLE;

    typedef struct PAIRED_HANDLES_TAG
    {
        PAIRED_HANDLE* paired_handles;
        size_t paired_handle_count;
    } PAIRED_HANDLES;

    int umockcallpairs_track_create_paired_call(PAIRED_HANDLES* paired_handles, const void* handle, const char* handle_type, size_t handle_type_size);
    int umockcallpairs_track_destroy_paired_call(PAIRED_HANDLES* paired_handles, const void* handle);
```

## umockcallpairs_track_create_paired_call

```c
int umockcallpairs_track_create_paired_call(PAIRED_HANDLES* paired_handles, const void* handle, const char* handle_type, size_t handle_type_size);
```

**SRS_UMOCKCALLPAIRS_01_001: [** umockcallpairs_track_create_paired_call shall add a new entry to the PAIRED_HANDLES array and on success it shall return 0. **]**

**SRS_UMOCKCALLPAIRS_01_002: [** umockcallpairs_track_create_paired_call shall copy the handle_value to the handle_value member of the new entry. **]**

**SRS_UMOCKCALLPAIRS_01_003: [** umockcallpairs_track_create_paired_call shall allocate a memory block and store a pointer to it in the memory field of the new entry. **]**

**SRS_UMOCKCALLPAIRS_01_004: [** If any of the arguments paired_handles, handle or handle_type is NULL, umockcallpairs_track_create_paired_call shallfail and return a non-zero value. **]**

**SRS_UMOCKCALLPAIRS_01_005: [** If allocating memory fails, umockcallpairs_track_create_paired_call shall fail and return a non-zero value. **]**

**SRS_UMOCKCALLPAIRS_01_006: [** The handle value shall be copied by using umocktypes_copy. **]**

**SRS_UMOCKCALLPAIRS_01_007: [** If umocktypes_copy fails, umockcallpairs_track_create_paired_call shall fail and return a non-zero value. **]**

## umockcallpairs_track_destroy_paired_call

```c
int umockcallpairs_track_destroy_paired_call(PAIRED_HANDLES* paired_handles, const void* handle);
```

**SRS_UMOCKCALLPAIRS_01_008: [** umockcallpairs_track_destroy_paired_call shall remove from the paired handles array pointed by the paired_handles field the entry that is associated with the handle passed in the handle argument. **]**

**SRS_UMOCKCALLPAIRS_01_009: [** On success umockcallpairs_track_destroy_paired_call shall return 0. **]**

**SRS_UMOCKCALLPAIRS_01_010: [** If any of the arguments is NULL, umockcallpairs_track_destroy_paired_call shall fail and return a non-zero value. **]**

**SRS_UMOCKCALLPAIRS_01_011: [** umockcallpairs_track_destroy_paired_call shall free the memory pointed by the memory field in the PAIRED_HANDLES array entry associated with handle. **]**

**SRS_UMOCKCALLPAIRS_01_012: [** If the array paired handles array is empty after removing the entry, the paired_handles field shall be freed and set to NULL. **]**

**SRS_UMOCKCALLPAIRS_01_013: [** When looking up which entry to remove, the comparison of the handle values shall be done by calling umocktypes_are_equal. **]**

**SRS_UMOCKCALLPAIRS_01_014: [** If umocktypes_are_equal fails, umockcallpairs_track_destroy_paired_call shall fail and return a non-zero value. **]**

**SRS_UMOCKCALLPAIRS_01_015: [** If the handle is not found in the array then umockcallpairs_track_destroy_paired_call shall fail and return a non-zero value. **]**
