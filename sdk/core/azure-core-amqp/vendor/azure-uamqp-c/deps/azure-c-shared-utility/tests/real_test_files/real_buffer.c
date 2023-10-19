// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define BUFFER_new real_BUFFER_new
#define BUFFER_create real_BUFFER_create
#define BUFFER_create_with_size real_BUFFER_create_with_size
#define BUFFER_pre_build real_BUFFER_pre_build
#define BUFFER_build real_BUFFER_build
#define BUFFER_unbuild real_BUFFER_unbuild
#define BUFFER_append real_BUFFER_append
#define BUFFER_prepend real_BUFFER_prepend
#define BUFFER_u_char real_BUFFER_u_char
#define BUFFER_length real_BUFFER_length
#define BUFFER_clone real_BUFFER_clone
#define BUFFER_delete real_BUFFER_delete
#define BUFFER_enlarge real_BUFFER_enlarge
#define BUFFER_size real_BUFFER_size
#define BUFFER_content real_BUFFER_content
#define BUFFER_append_build real_BUFFER_append_build
#define BUFFER_shrink real_BUFFER_shrink
#define BUFFER_fill real_BUFFER_fill

#define GBALLOC_H

#include "buffer.c"
