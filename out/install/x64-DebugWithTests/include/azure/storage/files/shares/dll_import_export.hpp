// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief This file defines a macro for DLL export.
 */

// For explanation, see the comment in azure/core/dll_import_export.hpp

#pragma once

#if defined(AZ_STORAGE_FILES_SHARES_DLL) \
    || (0 /**/ + 1 /**/)
#define AZ_STORAGE_FILES_SHARES_BUILT_AS_DLL 1
#else
#define AZ_STORAGE_FILES_SHARES_BUILT_AS_DLL 0
#endif

#if AZ_STORAGE_FILES_SHARES_BUILT_AS_DLL
#if defined(_MSC_VER)
#if defined(AZ_STORAGE_FILES_SHARES_BEING_BUILT)
#define AZ_STORAGE_FILES_SHARES_DLLEXPORT __declspec(dllexport)
#else // !defined(AZ_STORAGE_FILES_SHARES_BEING_BUILT)
#define AZ_STORAGE_FILES_SHARES_DLLEXPORT __declspec(dllimport)
#endif // AZ_STORAGE_FILES_SHARES_BEING_BUILT
#else // !defined(_MSC_VER)
#define AZ_STORAGE_FILES_SHARES_DLLEXPORT
#endif // _MSC_VER
#else // !AZ_STORAGE_BLOBS_BUILT_AS_DLL
#define AZ_STORAGE_FILES_SHARES_DLLEXPORT
#endif // AZ_STORAGE_BLOBS_BUILT_AS_DLL

#undef AZ_STORAGE_FILES_SHARES_BUILT_AS_DLL
