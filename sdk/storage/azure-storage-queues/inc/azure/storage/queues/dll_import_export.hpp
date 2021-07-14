// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief DLL export macro.
 */

// For explanation, see the comment in azure/core/dll_import_export.hpp

#pragma once

/**
 * @def AZ_STORAGE_QUEUES_DLLEXPORT
 * @brief Applies DLL export attribute, when applicable.
 * @note See https://docs.microsoft.com/cpp/cpp/dllexport-dllimport?view=msvc-160.
 */

#if defined(AZ_STORAGE_QUEUES_DLL) || (0 /*@AZ_STORAGE_QUEUES_DLL_INSTALLED_AS_PACKAGE@*/)
#define AZ_STORAGE_QUEUES_BUILT_AS_DLL 1
#else
#define AZ_STORAGE_QUEUES_BUILT_AS_DLL 0
#endif

#if AZ_STORAGE_QUEUES_BUILT_AS_DLL
#if defined(_MSC_VER)
#if defined(AZ_STORAGE_QUEUES_BEING_BUILT)
#define AZ_STORAGE_QUEUES_DLLEXPORT __declspec(dllexport)
#else // !defined(AZ_STORAGE_QUEUES_BEING_BUILT)
#define AZ_STORAGE_QUEUES_DLLEXPORT __declspec(dllimport)
#endif // AZ_STORAGE_QUEUES_BEING_BUILT
#else // !defined(_MSC_VER)
#define AZ_STORAGE_QUEUES_DLLEXPORT
#endif // _MSC_VER
#else // !AZ_STORAGE_QUEUES_BUILT_AS_DLL
#define AZ_STORAGE_QUEUES_DLLEXPORT
#endif // AZ_STORAGE_QUEUES_BUILT_AS_DLL

#undef AZ_STORAGE_QUEUES_BUILT_AS_DLL
