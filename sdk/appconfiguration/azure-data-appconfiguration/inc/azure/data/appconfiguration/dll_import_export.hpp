// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief DLL export macro.
 */

// For explanation, see the comment in azure/core/dll_import_export.hpp

#pragma once

/**
 * @def AZURE_DATA_APPCONFIGURATION_DLLEXPORT
 * @brief Applies DLL export attribute, when applicable.
 * @note See https://docs.microsoft.com/cpp/cpp/dllexport-dllimport?view=msvc-160.
 */

#if defined(AZURE_DATA_APPCONFIGURATION_DLL) \
    || (0 /*@AZURE_DATA_APPCONFIGURATION_DLL_INSTALLED_AS_PACKAGE@*/)
#define AZURE_DATA_APPCONFIGURATION_BUILT_AS_DLL 1
#else
#define AZURE_DATA_APPCONFIGURATION_BUILT_AS_DLL 0
#endif

#if AZURE_DATA_APPCONFIGURATION_BUILT_AS_DLL
#if defined(_MSC_VER)
#if defined(AZURE_DATA_APPCONFIGURATION_BEING_BUILT)
#define AZURE_DATA_APPCONFIGURATION_DLLEXPORT __declspec(dllexport)
#else // !defined(AZURE_DATA_APPCONFIGURATION_BEING_BUILT)
#define AZURE_DATA_APPCONFIGURATION_DLLEXPORT __declspec(dllimport)
#endif // AZURE_DATA_APPCONFIGURATION_BEING_BUILT
#else // !defined(_MSC_VER)
#define AZURE_DATA_APPCONFIGURATION_DLLEXPORT
#endif // _MSC_VER
#else // !AZURE_DATA_APPCONFIGURATION_BUILT_AS_DLL
#define AZURE_DATA_APPCONFIGURATION_DLLEXPORT
#endif // AZURE_DATA_APPCONFIGURATION_BUILT_AS_DLL

#undef AZURE_DATA_APPCONFIGURATION_BUILT_AS_DLL
