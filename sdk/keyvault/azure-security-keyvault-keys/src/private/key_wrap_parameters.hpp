//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Parameters for wrap a key.
 *
 */

#pragma once

#include <string>
#include <vector>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {
  namespace _detail {

    /**
     * @brief Parameters for wrap a key.
     *
     */
    struct KeyWrapParameters final
    {
      std::string Algorithm;
      std::vector<uint8_t> Key;

      KeyWrapParameters() = delete;

      KeyWrapParameters(std::string algorithm, std::vector<uint8_t> key)
          : Algorithm(std::move(algorithm)), Key(std::move(key))
      {
      }
    };
}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail
