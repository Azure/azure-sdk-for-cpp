// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>

void* mock_malloc(size_t size);
void* mock_calloc(size_t nmemb, size_t size);
void* mock_realloc(void* ptr, size_t size);
void mock_free(void* ptr);

#define malloc(size) mock_malloc(size)
#define calloc(nmemb, size) mock_calloc(nmemb, size)
#define realloc(ptr, size) mock_realloc(ptr, size)
#define free(ptr) mock_free(ptr)

/* include code under test */
#include "../../src/umockalloc.c"
