// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Common definitions.
 */

#pragma once

#include <azure/core/internal/contract.hpp>
#include <string>

/**
 * @brief Used in implementations to mark an unreferenced function parameter.
 */
#define AZURE_UNREFERENCED_PARAMETER(x) ((void)(x));

namespace Azure { namespace Core { namespace Details {

  bool LocaleInvariantCaseInsensitiveEqual(const std::string& lhs, const std::string& rhs) noexcept;
  std::string const ToLower(const std::string& src) noexcept;
  unsigned char ToLower(const unsigned char src) noexcept;

}}} // namespace Azure::Core::Details
