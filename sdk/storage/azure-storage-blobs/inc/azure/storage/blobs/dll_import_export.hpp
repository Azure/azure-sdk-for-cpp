// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief DLL export macro.
 */

// For explanation, see the comment in azure/core/dll_import_export.hpp

#pragma once

/**
 * @def AZURE_STORAGE_BLOBS_DLLEXPORT
 * @brief Applies DLL export attribute, when applicable.
 * @note See https://docs.microsoft.com/cpp/cpp/dllexport-dllimport?view=msvc-160.
 */

#if defined(AZURE_STORAGE_BLOBS_DLL) || (0 /*@AZURE_STORAGE_BLOBS_DLL_INSTALLED_AS_PACKAGE@*/)
#define AZURE_STORAGE_BLOBS_BUILT_AS_DLL 1
#else
#define AZURE_STORAGE_BLOBS_BUILT_AS_DLL 0
#endif

#if AZURE_STORAGE_BLOBS_BUILT_AS_DLL
#if defined(_MSC_VER)
#if defined(AZURE_STORAGE_BLOBS_BEING_BUILT)
#define AZURE_STORAGE_BLOBS_DLLEXPORT __declspec(dllexport)
#else // !defined(AZURE_STORAGE_BLOBS_BEING_BUILT)
#define AZURE_STORAGE_BLOBS_DLLEXPORT __declspec(dllimport)
#endif // AZURE_STORAGE_BLOBS_BEING_BUILT
#else // !defined(_MSC_VER)
#define AZURE_STORAGE_BLOBS_DLLEXPORT
#endif // _MSC_VER
#else // !AZURE_STORAGE_BLOBS_BUILT_AS_DLL
#define AZURE_STORAGE_BLOBS_DLLEXPORT
#endif // AZURE_STORAGE_BLOBS_BUILT_AS_DLL

#undef AZURE_STORAGE_BLOBS_BUILT_AS_DLL
