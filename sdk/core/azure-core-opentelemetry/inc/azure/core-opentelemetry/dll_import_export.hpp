// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief DLL export macro.
 */

// For explanation, see the comment in azure/core/dll_import_export.hpp

#pragma once

/**
 * @def AZ_IDENTITY_DLLEXPORT
 * @brief Applies DLL export attribute, when applicable.
 * @note See https://docs.microsoft.com/cpp/cpp/dllexport-dllimport?view=msvc-160.
 */

#if defined(AZ_CORE_TRACING_OPENTELEMETRY_DLL) || (0 /*@AZ_IDENTITY_DLL_INSTALLED_AS_PACKAGE@*/)
#define AZ_CORE_TRACING_OPENTELEMETRY_BUILT_AS_DLL 1
#else
#define AZ_CORE_TRACING_OPENTELEMETRY_BUILT_AS_DLL 0
#endif

#if AZ_CORE_TRACING_OPENTELEMETRY_BUILT_AS_DLL
#if defined(_MSC_VER)
#if defined(AZ_CORE_TRACING_OPENTELEMETRY_BEING_BUILT)
#define AZ_CORE_TRACING_OPENTELEMETRY_DLLEXPORT __declspec(dllexport)
#else // !defined(AZ_CORE_TRACING_OPENTELEMETRY_BEING_BUILT)
#define AZ_CORE_TRACING_OPENTELEMETRY_DLLEXPORT __declspec(dllimport)
#endif // AZ_CORE_TRACING_OPENTELEMETRY_BEING_BUILT
#else // !defined(_MSC_VER)
#define AZ_CORE_TRACING_OPENTELEMETRY_DLLEXPORT
#endif // _MSC_VER
#else // !AZ_CORE_TRACING_OPENTELEMETRY_BUILT_AS_DLL
#define AZ_CORE_TRACING_OPENTELEMETRY_DLLEXPORT
#endif // AZ_CORE_TRACING_OPENTELEMETRY_BUILT_AS_DLL

#undef AZ_CORE_TRACING_OPENTELEMETRY_BUILT_AS_DLL

/**
 * @brief Azure SDK abstractions.
 *
 */
namespace Azure {
/**
 * @brief Azure Identity SDK abstractions.
 *
 */
namespace Core {
  /** @brief Azure Tracing Abstractions
   */
  namespace Tracing {
    /** @brief OpenTelemetry Tracing Abstractions
     */
    namespace OpenTelemetry {
    }
  } // namespace Tracing
} // namespace Core
} // namespace Azure
