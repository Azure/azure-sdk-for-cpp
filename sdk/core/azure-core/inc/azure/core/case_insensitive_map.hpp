// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief A `map<string, string>` with case-insensitive key comparison.
 */

#pragma once

#include "azure/core/internal/strings.hpp"

#include <map>
#include <string>

namespace Azure { namespace Core {

  /**
   * @brief A type alias of `std::map<std::string, std::string>` with case-insensitive key
   * comparison.
   */
  using CaseInsensitiveMap
      = std::map<std::string, std::string, Internal::Strings::CaseInsensitiveComparator>;

}} // namespace Azure::Core
