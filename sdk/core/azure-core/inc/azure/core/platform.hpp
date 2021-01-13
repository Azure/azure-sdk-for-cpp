// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines platform-specific macros.
 */

#pragma once

#if defined(_WIN32)
#define AZ_PLATFORM_WINDOWS
#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#define AZ_PLATFORM_POSIX
#endif
