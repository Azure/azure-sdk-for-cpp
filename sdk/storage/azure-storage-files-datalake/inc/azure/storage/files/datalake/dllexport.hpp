// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief This file defines a macro for DLL export.
 */

// For explanation, see the comment in azure/core/dllexport.hpp

#pragma once

#if defined(_MSC_VER)
#if defined(AZ_STORAGE_FILES_DATALAKE_BEING_BUILT)
#define AZ_STORAGE_FILES_DATALAKE_DLLEXPORT __declspec(dllexport)
#else
#define AZ_STORAGE_FILES_DATALAKE_DLLEXPORT __declspec(dllimport)
#endif
#else
#define AZ_STORAGE_FILES_DATALAKE_DLLEXPORT
#endif
