// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stddef.h>

void* mock_malloc(size_t size);
void* mock_realloc(void* ptr, size_t size);
void mock_free(void* ptr);

#define umockalloc_malloc(size) mock_malloc(size)
#define umockalloc_realloc(ptr, size) mock_realloc(ptr, size)
#define umockalloc_free(ptr) mock_free(ptr)

/* include code under test */
#include "../../src/umocktypes_c.c"
