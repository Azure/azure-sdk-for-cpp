// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

/**
 * @file
 * @brief Deprecated assert macros. It is only temporarily made available for compatibility and
 * should NOT be used by any callers outside of the SDK.
 *
 */

#include "azure/core/internal/azure_assert.hpp"

[[deprecated("The Azure Assert macros are meant for internal use within the SDK only, use the STL "
             "assert instead.")]]
#define AZURE_ASSERT(exp) _azure_ASSERT(exp)

[[deprecated("The Azure Assert macros are meant for internal use within the SDK only, use the STL "
             "assert instead.")]]
#define AZURE_ASSERT_MSG(exp, msg) _azure_ASSERT_MSG(exp, msg)

[[deprecated("The Azure Assert macros are meant for internal use within the SDK only, use the STL "
             "assert instead.")]]
#define AZURE_ASSERT_FALSE(exp) _azure_ASSERT_FALSE(exp)

[[deprecated("The Azure Assert macros are meant for internal use within the SDK only, use the STL "
             "assert instead.")]]
#define AZURE_UNREACHABLE_CODE() _azure_UNREACHABLE_CODE()

[[deprecated("The Azure Assert macros are meant for internal use within the SDK only, use the STL "
             "assert instead.")]]
#define AZURE_NOT_IMPLEMENTED() _azure_NOT_IMPLEMENTED()
