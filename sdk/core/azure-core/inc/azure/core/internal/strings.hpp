// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Internal utility functions for strings.
 *
 */
#pragma once

#include <algorithm>
#include <string>

namespace Azure { namespace Core { namespace _internal { namespace Strings {

  bool LocaleInvariantCaseInsensitiveEqual(const std::string& lhs, const std::string& rhs) noexcept;
  std::string const ToLower(const std::string& src) noexcept;
  unsigned char ToLower(const unsigned char src) noexcept;

  struct CaseInsensitiveComparator
  {
    bool operator()(const std::string& lhs, const std::string& rhs) const
    {
      return std::lexicographical_compare(
          lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [](char c1, char c2) {
            return ToLower(c1) < ToLower(c2);
          });
    }
  };

}}}} // namespace Azure::Core::_internal::Strings
