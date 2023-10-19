// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef REAL_STRING_TOKENIZER_H
#define REAL_STRING_TOKENIZER_H

#define REGISTER_STRING_TOKENIZER_GLOBAL_MOCK_HOOK \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_TOKENIZER_create, real_STRING_TOKENIZER_create); \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_TOKENIZER_create_from_char, real_STRING_TOKENIZER_create_from_char); \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_TOKENIZER_get_next_token, real_STRING_TOKENIZER_get_next_token); \
    REGISTER_GLOBAL_MOCK_HOOK(STRING_TOKENIZER_destroy, real_STRING_TOKENIZER_destroy);

#define STRING_TOKENIZER_create               real_STRING_TOKENIZER_create
#define STRING_TOKENIZER_create_from_char     real_STRING_TOKENIZER_create_from_char
#define STRING_TOKENIZER_get_next_token       real_STRING_TOKENIZER_get_next_token
#define STRING_TOKENIZER_destroy              real_STRING_TOKENIZER_destroy

#undef STRING_TOKENIZER_H
#include "azure_c_shared_utility/string_tokenizer.h"

#ifndef COMPILING_REAL_REAL_STRING_TOKENIZER_H_C

#undef STRING_TOKENIZER_create
#undef STRING_TOKENIZER_create_from_char
#undef STRING_TOKENIZER_get_next_token
#undef STRING_TOKENIZER_destroy

#endif

#undef STRING_TOKENIZER_H

#endif
