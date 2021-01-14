// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief This file defines a macro for DLL export.
 */

// We use CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS which does most of the job for us, but static data
// members do still need to be declared as __declspec(dllimport) for the client code. (See
// https://cmake.org/cmake/help/v3.13/prop_tgt/WINDOWS_EXPORT_ALL_SYMBOLS.html)

#pragma once

#if defined(_MSC_VER)
#if defined(AZ_TEMPLATE_EXPORTS)
#define AZ_TEMPLATE_EXPORT __declspec(dllexport)
#else
#define AZ_TEMPLATE_EXPORT __declspec(dllimport)
#endif
#else
#define AZ_TEMPLATE_EXPORT
#endif
