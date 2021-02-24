// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief A `map<string, T>` with case-insensitive key comparison.
 */

#pragma once

#include "azure/core/internal/strings.hpp"

#include <map>
#include <string>

namespace Azure { namespace Core {

  /**
   * @brief A type alias of `std::map<std::string, T>` with case-insensitive key comparison.
   *
   * @tparam T Type of values being stored in the map.
   *
   * @tparam[optional] STL container allocator type
   * (default is `std::map<std::string, T>::allocator_type`).
   */
  template <
      typename T,
      typename Allocator
      = std::map<std::string, T, typename Internal::Strings::CaseInsensitiveComparator>::
          allocator_type>
  using CaseInsensitiveMap
      = std::map<std::string, T, Internal::Strings::CaseInsensitiveComparator, Allocator>;

}} // namespace Azure::Core
