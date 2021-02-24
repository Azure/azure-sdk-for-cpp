// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief A `map<string, T>` with case-insensitive keys.
 */

#pragma once

#include "azure/core/internal/strings.hpp"

#include <map>
#include <string>

namespace Azure { namespace Core {

  /**
   * @brief A `map<string, T>` with case-insensitive keys.
   */
  template <
      typename T,
      typename Allocator
      = std::map<std::string, T, Internal::Strings::CaseInsensitiveComparator>::allocator_type>
  using CaseInsensitiveMap
      = std::map<std::string, T, Internal::Strings::CaseInsensitiveComparator, Allocator>;

}} // namespace Azure::Core
