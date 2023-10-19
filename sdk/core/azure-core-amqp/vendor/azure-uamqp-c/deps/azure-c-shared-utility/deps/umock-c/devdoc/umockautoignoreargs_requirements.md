`umockautoignoreargs` requirements

# Overview

`umockautoignoreargs` is a module that provides the functionality of inspecting the arguments of a function to determine whether they should be ignored or not.

# Exposed API

```c
int umockautoignoreargs_is_call_argument_ignored(const char* call, size_t argument_index, int* is_argument_ignored);
```

## umockautoignoreargs_is_call_argument_ignored

```c
int umockautoignoreargs_is_call_argument_ignored(const char* call, size_t argument_index, int* is_argument_ignored);
```

**SRS_UMOCKAUTOIGNOREARGS_01_001: [** `umockautoignoreargs_is_call_argument_ignored` shall determine whether argument `argument_index` shall be ignored or not. **]**

**SRS_UMOCKAUTOIGNOREARGS_01_002: [** If `call` or `is_argument_ignored` is NULL, `umockautoignoreargs_is_call_argument_ignored` shall fail and return a non-zero value. **]**

**SRS_UMOCKAUTOIGNOREARGS_01_003: [** `umockautoignoreargs_is_call_argument_ignored` shall parse the `call` string as a function call: function_name(arg1, arg2, ...). **]**

**SRS_UMOCKAUTOIGNOREARGS_01_010: [** `umockautoignoreargs_is_call_argument_ignored` shall look for the arguments as being the string contained in the scope of the rightmost parenthesis set in `call`. **]**

Note: nesting is allowed. An example of arguments that would have nested parenthesis scope:

```c
function_name(arg1, some_call(x), ...)
```

For this example, the arg list would be `arg1, some_call(x), ...`.

**SRS_UMOCKAUTOIGNOREARGS_01_011: [** If a valid scope of the rightmost parenthesis set cannot be formed (imbalanced parenthesis for example), `umockautoignoreargs_is_call_argument_ignored` shall fail and return a non-zero value. **]**

**SRS_UMOCKAUTOIGNOREARGS_01_004: [** If `umockautoignoreargs_is_call_argument_ignored` fails parsing the `call` argument it shall fail and return a non-zero value. **]**

**SRS_UMOCKAUTOIGNOREARGS_01_009: [** If the number of arguments parsed from `call` is less than `argument_index`, `umockautoignoreargs_is_call_argument_ignored` shall fail and return a non-zero value. **]**

**SRS_UMOCKAUTOIGNOREARGS_01_005: [** If `umockautoignoreargs_is_call_argument_ignored` was able to parse the `argument_index`th argument it shall succeed and return 0, while writing whether the argument is ignored in the `is_argument_ignored` output argument. **]**

**SRS_UMOCKAUTOIGNOREARGS_01_006: [** If the argument value is `IGNORED_PTR_ARG` then `is_argument_ignored` shall be set to 1. **]**

**SRS_UMOCKAUTOIGNOREARGS_01_007: [** If the argument value is `IGNORED_NUM_ARG` then `is_argument_ignored` shall be set to 1. **]**

**SRS_UMOCKAUTOIGNOREARGS_01_008: [** If the argument value is any other value then `is_argument_ignored` shall be set to 0. **]**
