// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief DLL export macro.
 */

// For explanation, see the comment in azure/core/dll_import_export.hpp

#pragma once

/**
 * @def AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT
 * @brief Applies DLL export attribute, when applicable.
 * @note See https://learn.microsoft.com/cpp/cpp/dllexport-dllimport?view=msvc-160.
 */

#if defined(AZ_SECURITY_KEYVAULT_KEYS_DLL) \
    || (0 /*@AZ_SECURITY_KEYVAULT_KEYS_DLL_INSTALLED_AS_PACKAGE@*/)
#define AZ_SECURITY_KEYVAULT_KEYS_BUILT_AS_DLL 1
#else
#define AZ_SECURITY_KEYVAULT_KEYS_BUILT_AS_DLL 0
#endif

#if AZ_SECURITY_KEYVAULT_KEYS_BUILT_AS_DLL
#if defined(_MSC_VER)
#if defined(AZ_SECURITY_KEYVAULT_KEYS_BEING_BUILT)
#define AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT __declspec(dllexport)
#else // !defined(AZ_SECURITY_KEYVAULT_KEYS_BEING_BUILT)
#define AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT __declspec(dllimport)
#endif // AZ_SECURITY_KEYVAULT_KEYS_BEING_BUILT
#else // !defined(_MSC_VER)
#define AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT
#endif // _MSC_VER
#else // !AZ_SECURITY_KEYVAULT_KEYS_BUILT_AS_DLL
#define AZ_SECURITY_KEYVAULT_KEYS_DLLEXPORT
#endif // AZ_SECURITY_KEYVAULT_KEYS_BUILT_AS_DLL

#undef AZ_SECURITY_KEYVAULT_KEYS_BUILT_AS_DLL
