// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief run-time type info enable or disable.
 *
 * @details Checks whenever RTTI is enabled and exports the symbol `AZURE_SDK_RTTI_ENABLED`. When
 * the symbol is not exported, RTTI is disabled.
 *
 * @details This headers works together with the CMake module `AzureConfigRTTI` for installing the
 * header with the right value to resolve whether the SDK was built and installed with or without
 * RTTI. The CMake module will patch this file for installation.
 */

#pragma once

/**
 * @def AZURE_SDK_RTTI_ENABLED
 * @brief Define the symbol to enable or disable RTTI.
 *
 * @details `BUILD_RTTI` can be set when building the Azure SDK with CMake, however, installing the
 * header requires to resolve `AZ_CORE_WITH_RTTI` which will become ` + 1 ` when the SDK was built
 * with RTTI and `false` otherwise.
 */

#if !defined(AZURE_SDK_RTTI_ENABLED) && (BUILD_RTTI || (0 /*@AZ_BUILD_WITH_RTTI@*/))
#define AZURE_SDK_RTTI_ENABLED 1
#endif
