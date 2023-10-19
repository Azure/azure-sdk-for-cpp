// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef SAFE_MATH_H
#define SAFE_MATH_H

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)((size_t)~(size_t)0))
#endif

#define safe_add_size_t(a, b) ((((size_t)(a)) < ((size_t)(SIZE_MAX - ((size_t)(b))))) ? ((size_t)(a) + (size_t)(b)) : SIZE_MAX)

#define safe_subtract_size_t(a, b) (((a) >= (b)) ? ((size_t)(a) - (size_t)(b)) : SIZE_MAX)

#define safe_multiply_size_t(a, b) (((a) == 0 || (b) == 0) ? 0 : (((SIZE_MAX / (size_t)(a)) >= (size_t)(b)) ? (size_t)(a) * (size_t)(b) : SIZE_MAX))

#endif // SAFE_MATH_H
