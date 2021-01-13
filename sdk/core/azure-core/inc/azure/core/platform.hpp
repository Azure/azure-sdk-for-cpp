// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines platform-specific macros.
 */

#pragma once

#if defined(_WIN32)
#define AZ_PLATFORM_WINDOWS
// The macro will work as expected once windows.h is included, but not before that.
#define AZ_PLATFORM_IS_NOT_UWP() (!defined(WINAPI_FAMILY) || WINAPI_PARTITION_DESKTOP)
#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#define AZ_PLATFORM_POSIX
#endif
