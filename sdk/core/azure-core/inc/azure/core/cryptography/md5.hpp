// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Represents the class for the MD5 hash function which maps binary data of an arbitrary
 * length to small binary data of a fixed length.
 */

#pragma once

#include "azure/core/cryptography/hash_base.hpp"

#include <memory>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <vector>

namespace Azure { namespace Core { namespace Cryptography {

  /**
   * @brief Represents the class for the MD5 hash function which maps binary data of an arbitrary
   * length to small binary data of a fixed length.
   */
  class Md5Hash : public Hash {

  public:
    /**
     * @brief Construct a default instance of #Azure::Core::Cryptography::Md5Hash.
     */
    Md5Hash();

    /**
     * @brief Cleanup any state when destroying the instance of #Azure::Core::Cryptography::Md5Hash.
     */
    ~Md5Hash() override;

  private:
    std::unique_ptr<Hash> m_implementation;

    /**
     * @brief Computes the hash value of the specified binary input data, including any previously
     * appended.
     * @param data The pointer to binary data to compute the hash value for.
     * @param length The size of the data provided.
     * @return The computed MD5 hash value corresponding to the input provided including any
     * previously appended.
     */
    std::vector<uint8_t> OnFinal(const uint8_t* data, std::size_t length) override;

    /**
     * @brief Used to append partial binary input data to compute the MD5 hash in a streaming
     * fashion.
     * @remark Once all the data has been added, call #Azure::Core::Cryptography::Hash::Final() to
     * get the computed hash value.
     * @param data The pointer to the current block of binary data that is used for hash
     * calculation.
     * @param length The size of the data provided.
     */
    void OnAppend(const uint8_t* data, std::size_t length) override;
  };

}}} // namespace Azure::Core::Cryptography
