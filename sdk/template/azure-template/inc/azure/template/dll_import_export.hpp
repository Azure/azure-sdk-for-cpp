// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief DLL export macro.
 */

// For explanation, see the comment in azure/core/dll_import_export.hpp

#pragma once

/**
 * @def AZ_TEMPLATE_DLLEXPORT
 * @brief Applies DLL export attribute, when applicable.
 * @note See https://docs.microsoft.com/cpp/cpp/dllexport-dllimport?view=msvc-160.
 */

#if defined(AZ_TEMPLATE_DLL) || (0 /*@AZ_TEMPLATE_DLL_INSTALLED_AS_PACKAGE@*/)
#define AZ_TEMPLATE_BUILT_AS_DLL 1
#else
#define AZ_TEMPLATE_BUILT_AS_DLL 0
#endif

#if AZ_TEMPLATE_BUILT_AS_DLL
#if defined(_MSC_VER)
#if defined(AZ_TEMPLATE_BEING_BUILT)
#define AZ_TEMPLATE_DLLEXPORT __declspec(dllexport)
#else // !defined(AZ_TEMPLATE_BEING_BUILT)
#define AZ_TEMPLATE_DLLEXPORT __declspec(dllimport)
#endif // AZ_TEMPLATE_BEING_BUILT
#else // !defined(_MSC_VER)
#define AZ_TEMPLATE_DLLEXPORT
#endif // _MSC_VER
#else // !AZ_TEMPLATE_BUILT_AS_DLL
#define AZ_TEMPLATE_DLLEXPORT
#endif // AZ_TEMPLATE_BUILT_AS_DLL

#undef AZ_TEMPLATE_BUILT_AS_DLL

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