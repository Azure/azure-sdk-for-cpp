// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief This file defines a macro for DLL export.
 */

// For explanation, see the comment in azure/core/dllexport.hpp

#pragma once

#if defined(_MSC_VER)
#if defined(AZ_IDENTITY_BEING_BUILT)
#define AZ_IDENTITY_DLLEXPORT __declspec(dllexport)
#else
#define AZ_IDENTITY_DLLEXPORT __declspec(dllimport)
#endif
#else
#define AZ_IDENTITY_DLLEXPORT
#endif
