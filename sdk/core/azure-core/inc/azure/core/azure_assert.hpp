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

#define AZURE_ASSERT(exp) _azure_ASSERT(exp)
#define AZURE_ASSERT_MSG(exp, msg) _azure_ASSERT_MSG(exp, msg)
#define AZURE_ASSERT_FALSE(exp) _azure_ASSERT_FALSE(exp)
#define AZURE_UNREACHABLE_CODE() _azure_UNREACHABLE_CODE()
#define AZURE_NOT_IMPLEMENTED() _azure_NOT_IMPLEMENTED()
