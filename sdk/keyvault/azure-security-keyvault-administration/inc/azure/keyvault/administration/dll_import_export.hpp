// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief DLL export macro.
 */

// For explanation, see the comment in azure/core/dll_import_export.hpp

#pragma once

/**
 * @def AZURE_SECURITY_KEYVAULT_ADMINISTRATION_DLLEXPORT
 * @brief Applies DLL export attribute, when applicable.
 * @note See https://docs.microsoft.com/cpp/cpp/dllexport-dllimport?view=msvc-160.
 */

#if defined(AZURE_SECURITY_KEYVAULT_ADMINISTRATION_DLL) \
    || (0 /*@AZURE_SECURITY_KEYVAULT_ADMINISTRATION_DLL_INSTALLED_AS_PACKAGE@*/)
#define AZURE_SECURITY_KEYVAULT_ADMINISTRATION_BUILT_AS_DLL 1
#else
#define AZURE_SECURITY_KEYVAULT_ADMINISTRATION_BUILT_AS_DLL 0
#endif

#if AZURE_SECURITY_KEYVAULT_ADMINISTRATION_BUILT_AS_DLL
#if defined(_MSC_VER)
#if defined(AZURE_SECURITY_KEYVAULT_ADMINISTRATION_BEING_BUILT)
#define AZURE_SECURITY_KEYVAULT_ADMINISTRATION_DLLEXPORT __declspec(dllexport)
#else // !defined(AZURE_SECURITY_KEYVAULT_ADMINISTRATION_BEING_BUILT)
#define AZURE_SECURITY_KEYVAULT_ADMINISTRATION_DLLEXPORT __declspec(dllimport)
#endif // AZURE_SECURITY_KEYVAULT_ADMINISTRATION_BEING_BUILT
#else // !defined(_MSC_VER)
#define AZURE_SECURITY_KEYVAULT_ADMINISTRATION_DLLEXPORT
#endif // _MSC_VER
#else // !AZURE_SECURITY_KEYVAULT_ADMINISTRATION_BUILT_AS_DLL
#define AZURE_SECURITY_KEYVAULT_ADMINISTRATION_DLLEXPORT
#endif // AZURE_SECURITY_KEYVAULT_ADMINISTRATION_BUILT_AS_DLL

#undef AZURE_SECURITY_KEYVAULT_ADMINISTRATION_BUILT_AS_DLL

/**
 * @brief Azure SDK abstractions.
 *
 */
namespace Azure {

/**
 * @brief Azure Template SDK abstractions.
 *
 */
namespace Template {
}
} // namespace Azure
