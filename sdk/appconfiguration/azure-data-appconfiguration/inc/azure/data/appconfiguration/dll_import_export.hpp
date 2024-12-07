// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief DLL export macro.
 */

// For explanation, see the comment in azure/core/dll_import_export.hpp

#pragma once

/**
 * @def AZ_DATA_APPCONFIGURATION_DLLEXPORT
 * @brief Applies DLL export attribute, when applicable.
 * @note See https://learn.microsoft.com/cpp/cpp/dllexport-dllimport?view=msvc-160.
 */

#if defined(AZ_DATA_APPCONFIGURATION_DLL) \
    || (0 /*@AZ_DATA_APPCONFIGURATION_DLL_INSTALLED_AS_PACKAGE@*/)
#define AZ_DATA_APPCONFIGURATION_BUILT_AS_DLL 1
#else
#define AZ_DATA_APPCONFIGURATION_BUILT_AS_DLL 0
#endif

#if AZ_DATA_APPCONFIGURATION_BUILT_AS_DLL
#if defined(_MSC_VER)
#if defined(AZ_DATA_APPCONFIGURATION_BEING_BUILT)
#define AZ_DATA_APPCONFIGURATION_DLLEXPORT __declspec(dllexport)
#else // !defined(AZ_DATA_APPCONFIGURATION_BEING_BUILT)
#define AZ_DATA_APPCONFIGURATION_DLLEXPORT __declspec(dllimport)
#endif // AZ_DATA_APPCONFIGURATION_BEING_BUILT
#else // !defined(_MSC_VER)
#define AZ_DATA_APPCONFIGURATION_DLLEXPORT
#endif // _MSC_VER
#else // !AZ_DATA_APPCONFIGURATION_BUILT_AS_DLL
#define AZ_DATA_APPCONFIGURATION_DLLEXPORT
#endif // AZ_DATA_APPCONFIGURATION_BUILT_AS_DLL

#undef AZ_DATA_APPCONFIGURATION_BUILT_AS_DLL
