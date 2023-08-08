// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Parameters for verify a key.
 *
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {
  namespace _detail {

    /**
     * @brief Parameters for verify a key.
     *
     */
    struct KeyVerifyParameters final
    {
      std::string Algorithm;
      std::vector<uint8_t> Digest;
      std::vector<uint8_t> Signature;

      KeyVerifyParameters() = delete;

      KeyVerifyParameters(
          std::string algorithm,
          std::vector<uint8_t> digest,
          std::vector<uint8_t> signature)
          : Algorithm(std::move(algorithm)), Digest(std::move(digest)),
            Signature(std::move(signature))
      {
      }
    };
}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail
