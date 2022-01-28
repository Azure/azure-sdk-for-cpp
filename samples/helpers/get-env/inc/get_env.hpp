// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

// Since `getenv()` may generate warnings on MSVC, and is not available on UWP, sample code
// gets cluttered with insignificant nuances. This file makes it so that `getenv()` compiles and
// works the same on Linux, macOS, Win32, and UWP.

#pragma once

#if !defined(_MSC_VER)

// Linux and macOS
#include <stdlib.h>

#else
#define _CRT_SECURE_NO_WARNINGS

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>

#if !defined(WINAPI_PARTITION_DESKTOP) || WINAPI_PARTITION_DESKTOP
// Win32
#include <stdlib.h>
#else
// UWP
char* getenv(const char* name);
#endif

#endif
