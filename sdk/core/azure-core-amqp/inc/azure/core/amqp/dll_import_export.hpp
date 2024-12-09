// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief DLL export macro.
 */

// For explanation, see the comment in azure/core/dll_import_export.hpp
#pragma once

/**
 * @def AZ_CORE_AMQP_DLLEXPORT
 * @brief Applies DLL export attribute, when applicable.
 * @note See https://learn.microsoft.com/cpp/cpp/dllexport-dllimport?view=msvc-160.
 */

#if defined(AZ_CORE_AMQP_DLL) || (0 /*@AZ_CORE_AMQP_DLL_INSTALLED_AS_PACKAGE@*/)
#define AZ_CORE_AMQP_BUILT_AS_DLL 1
#else
#define AZ_CORE_AMQP_BUILT_AS_DLL 0
#endif

#if AZ_CORE_AMQP_BUILT_AS_DLL
#if defined(_MSC_VER)
#if defined(AZ_CORE_AMQP_BEING_BUILT)
#define AZ_CORE_AMQP_DLLEXPORT __declspec(dllexport)
#else // !defined(AZ_CORE_AMQP_BEING_BUILT)
#define AZ_CORE_AMQP_DLLEXPORT __declspec(dllimport)
#endif // AZ_CORE_AMQP_BEING_BUILT
#else // !defined(_MSC_VER)
#define AZ_CORE_AMQP_DLLEXPORT
#endif // _MSC_VER
#else // !AZ_CORE_BUILT_AS_DLL
#define AZ_CORE_AMQP_DLLEXPORT
#endif // AZ_CORE_BUILT_AS_DLL

#undef AZ_CORE_AMQP_BUILT_AS_DLL

/**
 * @brief Azure SDK abstractions.
 */
namespace Azure {

/**
 * @brief Abstractions commonly used by Azure SDK client libraries.
 */
namespace Core {

  /**
   * @brief AMQP-related abstractions.
   */
  namespace Amqp {

    /**
     * @brief AMQP Public model types.
     */
    namespace Models {
    }
  } // namespace Amqp
} // namespace Core
} // namespace Azure
