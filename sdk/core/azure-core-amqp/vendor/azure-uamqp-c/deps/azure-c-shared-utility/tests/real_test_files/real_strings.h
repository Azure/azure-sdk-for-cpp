// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef REAL_STRINGS_H
#define REAL_STRINGS_H

#define REGISTER_STRING_GLOBAL_MOCK_HOOK \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_new, real_STRING_new); \
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_new, NULL); \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_clone, real_STRING_clone); \
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_clone, NULL); \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_construct, real_STRING_construct); \
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_construct, NULL); \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_construct_n, real_STRING_construct_n); \
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_construct_n, NULL); \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_new_with_memory, real_STRING_new_with_memory); \
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_new_with_memory, NULL); \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_new_quoted, real_STRING_new_quoted); \
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_new_quoted, NULL); \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_new_JSON, real_STRING_new_JSON); \
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_new_JSON, NULL); \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_from_byte_array, real_STRING_from_byte_array); \
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_from_byte_array, NULL); \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_delete, real_STRING_delete); \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_concat, real_STRING_concat); \
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_concat, __LINE__); \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_concat_with_STRING, real_STRING_concat_with_STRING); \
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_concat_with_STRING, __LINE__); \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_quote, real_STRING_quote); \
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_quote, __LINE__); \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_copy, real_STRING_copy); \
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_copy, __LINE__); \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_copy_n, real_STRING_copy_n); \
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_copy_n, __LINE__); \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_c_str, real_STRING_c_str); \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_empty, real_STRING_empty); \
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_empty, __LINE__); \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_length, real_STRING_length); \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_compare, real_STRING_compare); \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_replace, real_STRING_replace); \
    REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_replace, __LINE__);

#define STRING_new                      real_STRING_new
#define STRING_clone                    real_STRING_clone
#define STRING_construct                real_STRING_construct
#define STRING_construct_n              real_STRING_construct_n
#define STRING_new_with_memory          real_STRING_new_with_memory
#define STRING_new_quoted               real_STRING_new_quoted
#define STRING_new_JSON                 real_STRING_new_JSON
#define STRING_from_byte_array          real_STRING_from_byte_array
#define STRING_delete                   real_STRING_delete
#define STRING_concat                   real_STRING_concat
#define STRING_concat_with_STRING       real_STRING_concat_with_STRING
#define STRING_quote                    real_STRING_quote
#define STRING_copy                     real_STRING_copy
#define STRING_copy_n                   real_STRING_copy_n
#define STRING_c_str                    real_STRING_c_str
#define STRING_empty                    real_STRING_empty
#define STRING_length                   real_STRING_length
#define STRING_compare                  real_STRING_compare
#define STRING_replace                  real_STRING_replace


#undef STRINGS_H
#include "azure_c_shared_utility/strings.h"

#ifndef COMPILING_REAL_STRINGS_C

#undef STRING_new
#undef STRING_clone
#undef STRING_construct
#undef STRING_construct_n
#undef STRING_new_with_memory
#undef STRING_new_quoted
#undef STRING_new_JSON
#undef STRING_from_byte_array
#undef STRING_delete
#undef STRING_concat
#undef STRING_concat_with_STRING
#undef STRING_quote
#undef STRING_copy
#undef STRING_copy_n
#undef STRING_c_str
#undef STRING_empty
#undef STRING_length
#undef STRING_compare
#undef STRING_replace

#endif

#undef STRINGS_H

#endif