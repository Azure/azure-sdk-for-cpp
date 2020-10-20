// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Public utility functions for strings.
 *
 */
#pragma once

#include <string>

namespace Azure { namespace Core { namespace Strings {

  bool LocaleInvariantCaseInsensitiveEqual(const std::string& lhs, const std::string& rhs) noexcept;
  std::string const ToLower(const std::string& src) noexcept;
  unsigned char ToLower(const unsigned char src) noexcept;

}}} // namespace Azure::Core::Strings
