// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief This file defines a macro for DLL export.
 */

// For explanation, see the comment in azure/core/dll_import_export.hpp

#pragma once

#if defined(AZ_IDENTITY_DLL) || (0 /**/ + 1 /**/)
#define AZ_IDENTITY_BUILT_AS_DLL 1
#else
#define AZ_IDENTITY_BUILT_AS_DLL 0
#endif

#if AZ_IDENTITY_BUILT_AS_DLL
#if defined(_MSC_VER)
#if defined(AZ_IDENTITY_BEING_BUILT)
#define AZ_IDENTITY_DLLEXPORT __declspec(dllexport)
#else // !defined(AZ_IDENTITY_BEING_BUILT)
#define AZ_IDENTITY_DLLEXPORT __declspec(dllimport)
#endif // AZ_IDENTITY_BEING_BUILT
#else // !defined(_MSC_VER)
#define AZ_IDENTITY_DLLEXPORT
#endif // _MSC_VER
#else // !AZ_IDENTITY_BUILT_AS_DLL
#define AZ_IDENTITY_DLLEXPORT
#endif // AZ_IDENTITY_BUILT_AS_DLL

#undef AZ_IDENTITY_BUILT_AS_DLL
