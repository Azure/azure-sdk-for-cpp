
# umockcallrecorder requirements

# Overview

umockcallrecorder is a module that implements recording the expected and actual calls.

# Exposed API

```c
    typedef struct UMOCKCALLRECORDER_TAG* UMOCKCALLRECORDER_HANDLE;

    UMOCKCALLRECORDER_HANDLE umockcallrecorder_create(void);
    void umockcallrecorder_destroy(UMOCKCALLRECORDER_HANDLE umock_call_recorder);
    int umockcallrecorder_reset_all_calls(UMOCKCALLRECORDER_HANDLE umock_call_recorder);
    int umockcallrecorder_add_expected_call(UMOCKCALLRECORDER_HANDLE umock_call_recorder, UMOCKCALL_HANDLE mock_call);
    int umockcallrecorder_add_actual_call(UMOCKCALLRECORDER_HANDLE umock_call_recorder, UMOCKCALL_HANDLE mock_call, UMOCKCALL_HANDLE* matched_call);
    const char* umockcallrecorder_get_actual_calls(UMOCKCALLRECORDER_HANDLE umock_call_recorder);
    const char* umockcallrecorder_get_expected_calls(UMOCKCALLRECORDER_HANDLE umock_call_recorder);
    UMOCKCALL_HANDLE umockcallrecorder_get_last_expected_call(UMOCKCALLRECORDER_HANDLE umock_call_recorder);
    UMOCKCALLRECORDER_HANDLE umockcallrecorder_clone(UMOCKCALLRECORDER_HANDLE umock_call_recorder);
    int umockcallrecorder_get_expected_call_count(UMOCKCALLRECORDER_HANDLE umock_call_recorder, size_t* expected_call_count);
    int umockcallrecorder_fail_call(UMOCKCALLRECORDER_HANDLE umock_call_recorder, size_t index);
    int umockcallrecorder_can_call_fail(UMOCKCALLRECORDER_HANDLE umock_call_recorder, size_t index, int* can_call_fail);
```

## umockcallrecorder_create

```c
UMOCKCALLRECORDER_HANDLE umockcallrecorder_create(void);
```

**SRS_UMOCKCALLRECORDER_01_001: [** umockcallrecorder_create shall create a new instance of a call recorder and return a non-NULL handle to it on success. **]**

**SRS_UMOCKCALLRECORDER_01_002: [** If allocating memory for the call recorder fails, umockcallrecorder_create shall return NULL. **]**

## umockcallrecorder_destroy

```c
void umockcallrecorder_destroy(UMOCKCALLRECORDER_HANDLE umock_call_recorder);
```

**SRS_UMOCKCALLRECORDER_01_003: [** umockcallrecorder_destroy shall free the resources associated with a the call recorder identified by the umock_call_recorder argument. **]**

**SRS_UMOCKCALLRECORDER_01_004: [** If umock_call_recorder is NULL, umockcallrecorder_destroy shall do nothing. **]**

## umockcallrecorder_reset_all_calls

```c
int umockcallrecorder_reset_all_calls(UMOCKCALLRECORDER_HANDLE umock_call_recorder);
```

**SRS_UMOCKCALLRECORDER_01_005: [** umockcallrecorder_reset_all_calls shall free all the expected and actual calls for the call recorder identified by umock_call_recorder. **]**

**SRS_UMOCKCALLRECORDER_01_006: [** On success umockcallrecorder_reset_all_calls shall return 0. **]**

**SRS_UMOCKCALLRECORDER_01_007: [** If umock_call_recorder is NULL, umockcallrecorder_reset_all_calls shall fail and return a non-zero value. **]**

## umockcallrecorder_add_expected_call

```c
int umockcallrecorder_add_expected_call(UMOCKCALLRECORDER_HANDLE umock_call_recorder, UMOCKCALL_HANDLE mock_call);
```

**SRS_UMOCKCALLRECORDER_01_008: [** umockcallrecorder_add_expected_call shall add the mock_call call to the expected call list maintained by the call recorder identified by umock_call_recorder. **]**

**SRS_UMOCKCALLRECORDER_01_009: [** On success umockcallrecorder_add_expected_call shall return 0. **]**

**SRS_UMOCKCALLRECORDER_01_012: [** If any of the arguments is NULL, umockcallrecorder_add_expected_call shall fail and return a non-zero value. **]**

**SRS_UMOCKCALLRECORDER_01_013: [** If allocating memory for the expected calls fails, umockcallrecorder_add_expected_call shall fail and return a non-zero value. **]**

## umockcallrecorder_add_actual_call

```c
int umockcallrecorder_add_actual_call(UMOCKCALLRECORDER_HANDLE umock_call_recorder, UMOCKCALL_HANDLE mock_call, UMOCKCALL_HANDLE* matched_call);
```

**SRS_UMOCKCALLRECORDER_01_014: [** umockcallrecorder_add_actual_call shall check whether the call mock_call matches any of the expected calls maintained by umock_call_recorder. **]**

**SRS_UMOCKCALLRECORDER_01_015: [** If the call does not match any of the expected calls, then umockcallrecorder_add_actual_call shall add the mock_call call to the actual call list maintained by umock_call_recorder. **]**

**SRS_UMOCKCALLRECORDER_01_016: [** If the call matches one of the expected calls, a handle to the matched call shall be filled into the matched_call argument. **]**

**SRS_UMOCKCALLRECORDER_01_017: [** Comparing the calls shall be done by calling umockcall_are_equal. **]**

**SRS_UMOCKCALLRECORDER_01_018: [** When no error is encountered, umockcallrecorder_add_actual_call shall return 0. **]**

**SRS_UMOCKCALLRECORDER_01_019: [** If any of the arguments is NULL, umockcallrecorder_add_actual_call shall fail and return a non-zero value. **]**

**SRS_UMOCKCALLRECORDER_01_020: [** If allocating memory for the actual calls fails, umockcallrecorder_add_actual_call shall fail and return a non-zero value. **]**

**SRS_UMOCKCALLRECORDER_01_021: [** If umockcall_are_equal fails, umockcallrecorder_add_actual_call shall fail and return a non-zero value. **]**

**SRS_UMOCKCALLRECORDER_01_057: [** If any expected call has `ignore_all_calls` set and the actual call is equal to it when comparing the 2 calls, then the call shall be considered matched and not added to the actual calls list. **]**

**SRS_UMOCKCALLRECORDER_01_058: [** If getting `ignore_all_calls` by calling `umockcall_get_ignore_all_calls` fails, `umockcallrecorder_add_actual_call` shall fail and return a non-zero value. **]**

## umockcallrecorder_get_actual_calls

```c
const char* umockcallrecorder_get_actual_calls(UMOCKCALLRECORDER_HANDLE umock_call_recorder);
```

**SRS_UMOCKCALLRECORDER_01_022: [** umockcallrecorder_get_actual_calls shall return a pointer to the string representation of all the actual calls. **]**

**SRS_UMOCKCALLRECORDER_01_023: [** The string for each call shall be obtained by calling umockcall_stringify. **]**

**SRS_UMOCKCALLRECORDER_01_024: [** If the umock_call_recorder is NULL, umockcallrecorder_get_actual_calls shall fail and return NULL. **]**

**SRS_UMOCKCALLRECORDER_01_025: [** If umockcall_stringify fails, umockcallrecorder_get_actual_calls shall fail and return NULL. **]**

**SRS_UMOCKCALLRECORDER_01_026: [** If allocating memory for the resulting string fails, umockcallrecorder_get_actual_calls shall fail and return NULL. **]**

## umockcallrecorder_get_expected_calls

```c
const char* umockcallrecorder_get_expected_calls(UMOCKCALLRECORDER_HANDLE umock_call_recorder);
```

**SRS_UMOCKCALLRECORDER_01_027: [** umockcallrecorder_get_expected_calls shall return a pointer to the string representation of all the expected calls. **]**

**SRS_UMOCKCALLRECORDER_01_028: [** The string for each call shall be obtained by calling umockcall_stringify. **]**

**SRS_UMOCKCALLRECORDER_01_029: [** If the umock_call_recorder is NULL, umockcallrecorder_get_expected_calls shall fail and return NULL. **]**

**SRS_UMOCKCALLRECORDER_01_030: [** If umockcall_stringify fails, umockcallrecorder_get_expected_calls shall fail and return NULL. **]**

**SRS_UMOCKCALLRECORDER_01_031: [** If allocating memory for the resulting string fails, umockcallrecorder_get_expected_calls shall fail and return NULL. **]**

**SRS_UMOCKCALLRECORDER_01_054: [** Calls that have the `ignore_all_calls` property set shall not be reported in the expected call list. **]**

**SRS_UMOCKCALLRECORDER_01_055: [** Getting the `ignore_all_calls` property shall be done by calling `umockcall_get_ignore_all_calls`. **]**

**SRS_UMOCKCALLRECORDER_01_056: [** If `umockcall_get_ignore_all_calls` returns a negative value then `umockcallrecorder_get_expected_calls` shall fail and return NULL. **]**

## umockcallrecorder_get_last_expected_call

```c
UMOCKCALL_HANDLE umockcallrecorder_get_last_expected_call(UMOCKCALLRECORDER_HANDLE umock_call_recorder);
```

**SRS_UMOCKCALLRECORDER_01_032: [** umockcallrecorder_get_last_expected_call shall return the last expected call for the umock_call_recorder call recorder. **]**

**SRS_UMOCKCALLRECORDER_01_033: [** If umock_call_recorder is NULL, umockcallrecorder_get_last_expected_call shall fail and return NULL. **]**

**SRS_UMOCKCALLRECORDER_01_034: [** If no expected call has been recorded for umock_call_recorder then umockcallrecorder_get_last_expected_call shall fail and return NULL. **]**

## umockcallrecorder_clone

```c
UMOCKCALLRECORDER_HANDLE umockcallrecorder_clone(UMOCKCALLRECORDER_HANDLE umock_call_recorder);
```

**SRS_UMOCKCALLRECORDER_01_035: [** umockcallrecorder_clone shall clone a call recorder and return a handle to the newly cloned call recorder. **]**

**SRS_UMOCKCALLRECORDER_01_036: [** If the umock_call_recorder argument is NULL, umockcallrecorder_clone shall fail and return NULL. **]**

**SRS_UMOCKCALLRECORDER_01_037: [** If allocating memory for the new umock call recorder instance fails, umockcallrecorder_clone shall fail and return NULL. **]**

**SRS_UMOCKCALLRECORDER_01_038: [** umockcallrecorder_clone shall clone all the expected calls. **]**

**SRS_UMOCKCALLRECORDER_01_039: [** Each expected call shall be cloned by calling umockcall_clone. **]**

**SRS_UMOCKCALLRECORDER_01_040: [** If cloning an expected call fails, umockcallrecorder_clone shall fail and return NULL. **]**

**SRS_UMOCKCALLRECORDER_01_041: [** umockcallrecorder_clone shall clone all the actual calls. **]**

**SRS_UMOCKCALLRECORDER_01_042: [** Each actual call shall be cloned by calling umockcall_clone. **]**

**SRS_UMOCKCALLRECORDER_01_043: [** If cloning an actual call fails, umockcallrecorder_clone shall fail and return NULL. **]**

**SRS_UMOCKCALLRECORDER_01_052: [** If allocating memory for the expected calls fails, umockcallrecorder_clone shall fail and return NULL. **]**

**SRS_UMOCKCALLRECORDER_01_053: [** If allocating memory for the actual calls fails, umockcallrecorder_clone shall fail and return NULL. **]**

## umockcallrecorder_get_expected_call_count

```c
int umockcallrecorder_get_expected_call_count(UMOCKCALLRECORDER_HANDLE umock_call_recorder, size_t* expected_call_count);
```

**SRS_UMOCKCALLRECORDER_01_044: [** umockcallrecorder_get_expected_call_count shall return in the expected_call_count argument the number of expected calls associated with the call recorder. **]**

**SRS_UMOCKCALLRECORDER_01_045: [** On success umockcallrecorder_get_expected_call_count shall return 0. **]**

**SRS_UMOCKCALLRECORDER_01_046: [** If any of the arguments is NULL, umockcallrecorder_get_expected_call_count shall return a non-zero value. **]**

## umockcallrecorder_fail_call

```c
int umockcallrecorder_fail_call(UMOCKCALLRECORDER_HANDLE umock_call_recorder, size_t index);
```

**SRS_UMOCKCALLRECORDER_01_047: [** umockcallrecorder_fail_call shall mark an expected call as to be failed by calling umockcall_set_fail_call with a 1 value for fail_call. **]**

**SRS_UMOCKCALLRECORDER_01_048: [** On success, umockcallrecorder_fail_call shall return 0. **]**

**SRS_UMOCKCALLRECORDER_01_049: [** If umock_call_recorder is NULL, umockcallrecorder_fail_call shall return a non-zero value. **]**

**SRS_UMOCKCALLRECORDER_01_050: [** If index is invalid, umockcallrecorder_fail_call shall return a non-zero value. **]**

**SRS_UMOCKCALLRECORDER_01_051: [** If umockcall_set_fail_call fails, umockcallrecorder_fail_call shall return a non-zero value. **]**


## umockcallrecorder_can_call_fail

```c
int umockcallrecorder_can_call_fail(UMOCKCALLRECORDER_HANDLE umock_call_recorder, size_t index, int* can_call_fail);
```


**SRS_UMOCKCALLRECORDER_31_056: [** If umock_call_recorder or call_can_fail is NULL, umockcallrecorder_can_call_fail shall return a non-zero value. **]**

**SRS_UMOCKCALLRECORDER_31_057: [** If index is greater or equal to the current expected calls count, umockcallrecorder_can_call_fail shall return a non-zero value. **]**

**SRS_UMOCKCALLRECORDER_31_061: [** umockcallrecorder_can_call_fail shall determine whether given call can fail or not by calling umockcall_get_call_can_fail. **]**

**SRS_UMOCKCALLRECORDER_31_058: [** If umockcall_get_call_can_fail returns -1, umockcallrecorder_can_call_fail shall return an error to the caller  **]**

**SRS_UMOCKCALLRECORDER_31_059: [** umockcallrecorder_can_call_fail shall return in the can_call_fail argument whether the call can fail or not. **]**

**SRS_UMOCKCALLRECORDER_31_060: [** On success umockcallrecorder_can_call_fail shall return 0. ]*/

