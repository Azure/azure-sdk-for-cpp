`constbuffer_array_batcher` requirements
================

## Overview

`constbuffer_array_batcher` is a module that batches/unbatches several `CONSTBUFFER_ARRAY`s.

`constbuffer_array_batcher` understands the layout of the batches as:

| Header                                                                                                                    | Payload 0                               |     |                          |
|----------------------|-------------------------------|-------------------------------|-----|------------------------------|--------------------|--------------------|-----|--------------------|-----|
| 4 byte payload count | 4 byte payload 0 buffer count | 4 byte payload 1 buffer count | ... |4 byte payload n buffer count | payload 0 buffer 0 | payload 0 buffer 1 | ... | payload 1 buffer 0 | ... |

## Exposed API

```c
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_batcher_batch, CONSTBUFFER_ARRAY_HANDLE*, payloads, uint32_t, count);
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_HANDLE*, constbuffer_array_batcher_unbatch, CONSTBUFFER_ARRAY_HANDLE, batch, uint32_t*, payload_count);
```

### constbuffer_array_batcher_batch

```c
CONSTBUFFER_ARRAY_HANDLE constbuffer_array_batcher_batch(CONSTBUFFER_ARRAY_HANDLE* payloads, uint32_t count);
```

`constbuffer_array_batcher_batch` batches several const buffer arrays.

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_001: [** If `payloads` is `NULL`, `constbuffer_array_batcher_batch` shall fail and return `NULL`. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_002: [** If `count` is 0, `constbuffer_array_batcher_batch` shall fail and return `NULL`. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_023: [** If any of the payload const buffer arrays is `NULL`, `constbuffer_array_batcher_batch` shall fail and return `NULL`. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_003: [** Otherwise `constbuffer_array_batcher_batch` shall obtain the number of buffers used by each CONSTBUFFER_ARRAY. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_004: [** `constbuffer_array_batcher_batch` shall allocate memory for the header buffer (enough to hold the entire batch header namingly (`count` + 1) `uint32_t` values). **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_005: [** `count` shall be written as the first `uint32_t` in the header memory. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_006: [** The count of buffers for each array in `payloads` shall also be written in the header. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_007: [** `constbuffer_array_batcher_batch` shall allocate enough memory for all the buffer handles in all the arrays + one extra header buffer handle. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_008: [** `constbuffer_array_batcher_batch` shall populate the first handle in the newly allocated handles array with the header buffer handle. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_009: [** `constbuffer_array_batcher_batch` shall populate the rest of the handles in the newly allocated handles array with the const buffer handles obtained from the arrays in `payloads`. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_010: [** If any error occurrs, `constbuffer_array_batcher_batch` shall fail and return NULL. **]**

### constbuffer_array_batcher_unbatch

```c
CONSTBUFFER_ARRAY_HANDLE* constbuffer_array_batcher_unbatch(CONSTBUFFER_ARRAY_HANDLE batch, uint32_t* payload_count);
```

`constbuffer_array_batcher_unbatch` unbatches a CONSTBUFFER_ARRAY and produces the originally batched payloads.

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_011: [** If `batch` is NULL, `constbuffer_array_batcher_unbatch` shall fail and return NULL. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_012: [** If `payload_count` is NULL, `constbuffer_array_batcher_unbatch` shall fail and return NULL. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_013: [** Otherwise, `constbuffer_array_batcher_unbatch` shall obtain the number of buffers in `batch`. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_014: [** `constbuffer_array_batcher_unbatch` shall obtain the content of first (header) buffer in `batch`. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_024: [** If the size of the first buffer is less than `uint32_t` or not a multiple of `uint32_t`, `constbuffer_array_batcher_unbatch` shall fail and return NULL. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_015: [** `constbuffer_array_batcher_unbatch` shall extract the number of buffer arrays batched by reading the first `uint32_t`. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_025: [** If the number of buffer arrays does not match the size of the first buffer, `constbuffer_array_batcher_unbatch` shall fail and return NULL. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_026: [** If the number of buffer arrays in the batch is 0, `constbuffer_array_batcher_unbatch` shall fail and return NULL. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_017: [** `constbuffer_array_batcher_unbatch` shall allocate enough memory to hold the handles for buffer arrays that will be unbatched. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_016: [** `constbuffer_array_batcher_unbatch` shall extract the number of buffers in each of the batched payloads reading the `uint32_t` values encoded in the rest of the first (header) buffer. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_018: [** `constbuffer_array_batcher_unbatch` shall create a const buffer array for each of the payloads in the batch. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_019: [** On success `constbuffer_array_batcher_unbatch` shall return the array of const buffer array handles that constitute the batch. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_020: [** On success `constbuffer_array_batcher_unbatch` shall write in `payload_count` the number of const buffer arrays that are in the batch. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_021: [** If there are not enough buffers in `batch` to properly create all the payloads, `constbuffer_array_batcher_unbatch` shall fail and return NULL. **]**

**SRS_CONSTBUFFER_ARRAY_BATCHER_01_022: [** If any error occurs, `constbuffer_array_batcher_unbatch` shall fail and return NULL. **]**
