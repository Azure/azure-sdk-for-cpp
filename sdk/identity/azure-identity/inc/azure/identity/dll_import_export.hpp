// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief DLL export macro.
 */

// For explanation, see the comment in azure/core/dll_import_export.hpp

#pragma once

/**
 * @def AZURE_IDENTITY_DLLEXPORT
 * @brief Applies DLL export attribute, when applicable.
 * @note See https://learn.microsoft.com/cpp/cpp/dllexport-dllimport?view=msvc-160.
 */

#if defined(AZURE_IDENTITY_DLL) || (0 /*@AZURE_IDENTITY_DLL_INSTALLED_AS_PACKAGE@*/)
#define AZURE_IDENTITY_BUILT_AS_DLL 1
#else
#define AZURE_IDENTITY_BUILT_AS_DLL 0
#endif

#if AZURE_IDENTITY_BUILT_AS_DLL
#if defined(_MSC_VER)
#if defined(AZURE_IDENTITY_BEING_BUILT)
#define AZURE_IDENTITY_DLLEXPORT __declspec(dllexport)
#else // !defined(AZURE_IDENTITY_BEING_BUILT)
#define AZURE_IDENTITY_DLLEXPORT __declspec(dllimport)
#endif // AZURE_IDENTITY_BEING_BUILT
#else // !defined(_MSC_VER)
#define AZURE_IDENTITY_DLLEXPORT
#endif // _MSC_VER
#else // !AZURE_IDENTITY_BUILT_AS_DLL
#define AZURE_IDENTITY_DLLEXPORT
#endif // AZURE_IDENTITY_BUILT_AS_DLL

#undef AZURE_IDENTITY_BUILT_AS_DLL

/**
 * @brief Azure SDK abstractions.
 *
 */
namespace Azure {

/**
 * @brief Azure Identity SDK abstractions.
 *
 */
namespace Identity {
}
} // namespace Azure
