// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Utility functions to help convert between binary data and UTF-8 encoded text that is
 * represented in base 64.
 */

#pragma once

#include <string>
#include <vector>

namespace Azure { namespace Core {

  /**
   * @brief Encodes the vector of binary data into UTF-8 encoded text represented as base 64.
   *
   * @param data The input vector that contains binary data that needs to be encoded.
   * @return The UTF-8 encoded text in base 64.
   */
  std::string Base64Encode(const std::vector<uint8_t>& data);

  /**
   * @brief Decodes the UTF-8 encoded text represented as base 64 into binary data.
   *
   * @param text The input UTF-8 encoded text in base 64 that needs to be decoded.
   * @return The decoded binary data.
   */
  std::vector<uint8_t> Base64Decode(const std::string& text);

}} // namespace Azure::Core
