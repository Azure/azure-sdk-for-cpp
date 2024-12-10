// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief DLL export macro.
 */

// For explanation, see the comment in azure/core/dll_import_export.hpp

#pragma once

/**
 * @def AZURE_CORE_TRACING_OPENTELEMETRY_DLLEXPORT
 * @brief Applies DLL export attribute, when applicable.
 * @note See https://docs.microsoft.com/cpp/cpp/dllexport-dllimport?view=msvc-160.
 */

#if defined(AZURE_CORE_TRACING_OPENTELEMETRY_DLL) \
    || (0 /*@AZURE_CORE_TRACING_OPENTELEMETRY_DLL_INSTALLED_AS_PACKAGE@*/)
#define AZURE_CORE_TRACING_OPENTELEMETRY_BUILT_AS_DLL 1
#else
#define AZURE_CORE_TRACING_OPENTELEMETRY_BUILT_AS_DLL 0
#endif

#if AZURE_CORE_TRACING_OPENTELEMETRY_BUILT_AS_DLL
#if defined(_MSC_VER)
#if defined(AZURE_CORE_TRACING_OPENTELEMETRY_BEING_BUILT)
#define AZURE_CORE_TRACING_OPENTELEMETRY_DLLEXPORT __declspec(dllexport)
#else // !defined(AZURE_CORE_TRACING_OPENTELEMETRY_BEING_BUILT)
#define AZURE_CORE_TRACING_OPENTELEMETRY_DLLEXPORT __declspec(dllimport)
#endif // AZURE_CORE_TRACING_OPENTELEMETRY_BEING_BUILT
#else // !defined(_MSC_VER)
#define AZURE_CORE_TRACING_OPENTELEMETRY_DLLEXPORT
#endif // _MSC_VER
#else // !AZURE_CORE_TRACING_OPENTELEMETRY_BUILT_AS_DLL
#define AZURE_CORE_TRACING_OPENTELEMETRY_DLLEXPORT
#endif // AZURE_CORE_TRACING_OPENTELEMETRY_BUILT_AS_DLL

#undef AZURE_CORE_TRACING_OPENTELEMETRY_BUILT_AS_DLL

/**
 * @brief Azure SDK abstractions.
 *
 */
namespace Azure {
/**
 * @brief Abstractions commonly used by Azure SDK client libraries.
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
