// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Internal utility functions for strings.
 *
 */
#pragma once

#include <algorithm>
#include <cstring>
#include <string>

namespace Azure { namespace Core { namespace _internal {

  /**
   * @brief Extend the functionality of std::string by offering static methods for string
   * operations.
   */
  struct StringExtensions final
  {
    static constexpr char ToUpper(char c) noexcept
    {
      return (c < 'a' || c > 'z') ? c : c - ('a' - 'A');
    }

    static constexpr char ToLower(char c) noexcept
    {
      return (c < 'A' || c > 'Z') ? c : c + ('a' - 'A');
    }

    struct CaseInsensitiveComparator final
    {
      bool operator()(std::string const& lhs, std::string const& rhs) const
      {
        return std::lexicographical_compare(
            lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [](auto l, auto r) {
              return ToLower(l) < ToLower(r);
            });
      }
    };

    static bool LocaleInvariantCaseInsensitiveEqual(
        std::string const& lhs,
        std::string const& rhs) noexcept
    {
      auto const rhsSize = rhs.size();
      if (lhs.size() != rhsSize)
      {
        return false;
      }

      auto const lhsData = lhs.c_str();
      auto const rhsData = rhs.c_str();
      for (size_t i = 0; i < rhsSize; ++i)
      {
        if (lhsData[i] != rhsData[i])
        {
          return false;
        }
      }

      return true;
    }

    static std::string ToLower(std::string src)
    {
      std::transform(src.begin(), src.end(), src.begin(), [](auto c) { return ToLower(c); });
      return src;
    }

    static std::string ToUpper(std::string src)
    {
      std::transform(src.begin(), src.end(), src.begin(), [](auto c) { return ToUpper(c); });
      return src;
    }
  };

}}} // namespace Azure::Core::_internal
