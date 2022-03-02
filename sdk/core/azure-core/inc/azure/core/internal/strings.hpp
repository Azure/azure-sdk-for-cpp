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

namespace Azure { namespace Core { namespace _internal {

  /**
   * @brief Extend the functionality of std::string by offering static methods for string
   * operations.
   */
  struct StringExtensions final
  {
    struct CaseInsensitiveComparator final
    {
      bool operator()(const std::string& lhs, const std::string& rhs) const
      {
        return std::lexicographical_compare(
            lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [](char c1, char c2) {
              return ToLower(c1) < ToLower(c2);
            });
      }
    };

    static bool LocaleInvariantCaseInsensitiveEqual(
        const std::string& lhs,
        const std::string& rhs) noexcept;
    static std::string const ToLower(std::string const& src) noexcept;
    static unsigned char ToLower(unsigned char const src) noexcept;
    static std::string const ToUpper(std::string const& src) noexcept;
    static unsigned char ToUpper(unsigned char const src) noexcept;
  };

}}} // namespace Azure::Core::_internal
