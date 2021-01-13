// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines platform-specific macros.
 */

#pragma once

#if defined(_WIN32)
#define AZ_PLATFORM_WINDOWS
#if defined(WINAPI_FAMILY) && WINAPI_PARTITION_APP
#define AZ_PLATFORM_WINDOWS_UWP
#endif
#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#define AZ_PLATFORM_POSIX
#endif
