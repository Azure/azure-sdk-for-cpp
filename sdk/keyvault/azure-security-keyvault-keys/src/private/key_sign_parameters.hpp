// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Parameters for sign a key.
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
     * @brief Parameters for sign a key.
     *
     */
    struct KeySignParameters final
    {
      std::string Algorithm;
      std::vector<uint8_t> Digest;

      KeySignParameters() = delete;

      KeySignParameters(std::string algorithm, std::vector<uint8_t> digest)
          : Algorithm(std::move(algorithm)), Digest(std::move(digest))
      {
      }
    };
}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail
