// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief DLL export macro.
 */

// For explanation, see the comment in azure/core/dll_import_export.hpp

#pragma once

/**
 * @def AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT
 * @brief Applies DLL export attribute, when applicable.
 * @note See https://docs.microsoft.com/cpp/cpp/dllexport-dllimport?view=msvc-160.
 */

#if defined(AZ_SECURITY_KEYVAULT_CERTIFICATES_DLL) \
    || (0 /*@AZ_SECURITY_KEYVAULT_CERTIFICATES_DLL_INSTALLED_AS_PACKAGE@*/)
#define AZ_SECURITY_KEYVAULT_CERTIFICATES_BUILT_AS_DLL 1
#else
#define AZ_SECURITY_KEYVAULT_CERTIFICATES_BUILT_AS_DLL 0
#endif

#if AZ_SECURITY_KEYVAULT_CERTIFICATES_BUILT_AS_DLL
#if defined(_MSC_VER)
#if defined(AZ_SECURITY_KEYVAULT_CERTIFICATES_BEING_BUILT)
#define AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT __declspec(dllexport)
#else // !defined(AZ_SECURITY_KEYVAULT_CERTIFICATES_BEING_BUILT)
#define AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT __declspec(dllimport)
#endif // AZ_SECURITY_KEYVAULT_CERTIFICATES_BEING_BUILT
#else // !defined(_MSC_VER)
#define AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT
#endif // _MSC_VER
#else // !AZ_SECURITY_KEYVAULT_CERTIFICATES_BUILT_AS_DLL
#define AZ_SECURITY_KEYVAULT_CERTIFICATES_DLLEXPORT
#endif // AZ_SECURITY_KEYVAULT_CERTIFICATES_BUILT_AS_DLL

#undef AZ_SECURITY_KEYVAULT_CERTIFICATES_BUILT_AS_DLL