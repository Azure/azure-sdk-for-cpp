// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief run-time type info enable or disable.
 *
 * @details Checks whenever RTTI is enabled and exports the symbol `AZ_STORAGE_FILES_DATALAKE_WITH_RTTI`. When
 * the symbol is not exported, RTTI is disabled.
 *
 * @details This headers works together with the CMake module `AzureConfigRTTI` for installing the
 * header with the right value to resolve whether the SDK package was built and installed with or
 * without RTTI. The CMake module will patch this file for installation.
 */

#pragma once

/**
 * @def AZ_STORAGE_FILES_DATALAKE_WITH_RTTI
 * @brief Define the symbol to enable or disable RTTI.
 *
 * @details `BUILD_RTTI` can be set when building the Azure SDK with CMake, however, installing the
 * header requires to resolve `AZ_STORAGE_FILES_DATALAKE_WITH_RTTI` which will become ` + 1 ` when the SDK was built
 * with RTTI and `false` otherwise.
 */

#if AZ_BUILD_RTTI || (0 /*@AZ_STORAGE_FILES_DATALAKE_WITH_RTTI@*/)
#define AZ_STORAGE_FILES_DATALAKE_WITH_RTTI 1
#endif
