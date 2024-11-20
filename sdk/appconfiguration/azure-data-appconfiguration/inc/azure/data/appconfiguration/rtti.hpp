// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Run-time type info enable or disable.
 *
 * @details When RTTI is enabled, defines a macro `AZURE_DATA_APPCONFIGURATION_RTTI`. When
 * the macro is not defined, RTTI is disabled.
 *
 * @details Each library has this header file. These headers are being configured by
 * `AZURE_rtti_setup()` CMake macro. CMake install will patch this file during installation,
 * depending on the build flags.
 */

#pragma once

/**
 * @def AZURE_DATA_APPCONFIGURATION_RTTI
 * @brief A macro indicating whether the code is built with RTTI or not.
 *
 * @details `AZURE_RTTI` could be defined while building the Azure SDK with CMake, however, after
 * the build is completed, that information is not preserved for the code that consumes Azure SDK
 * headers, unless the code that consumes the SDK is the part of the same build process. To address
 * this issue, CMake install would patch the header it places in the installation directory, so that
 * condition:
 * `#if defined(AZURE_RTTI) || (0)`
 * becomes, effectively,
 * `#if defined(AZURE_RTTI) || (0 + 1)`
 * when the library was built with RTTI support, and will make no changes to the
 * condition when it was not.
 */

#if defined(AZURE_RTTI) || (0 /*@AZURE_DATA_APPCONFIGURATION_RTTI@*/)
#define AZURE_DATA_APPCONFIGURATION_RTTI
#endif