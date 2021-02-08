// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Utility functions to help compute the MD5 hash value for the input binary data.
 */

#pragma once

#include <cstdint>
#include <vector>

namespace Azure { namespace Core {

  /**
   * @brief Represents the class for the MD5 hash function which maps binary data of an arbitrary
   * length to small binary data of a fixed length.
   */
  class Md5 {
  public:
    /**
     * @brief Construct a default instance of @Md5.
     */
    explicit Md5();

    /**
     * @brief Cleanup any state when destroying the instance of @Md5.
     */
    ~Md5();

    /**
     * @brief Used to append partial binary input data to compute the hash in a streaming fashion.
     * @remark Once all the data has been added, call #Digest() to get the computed hash value.
     * @param data The pointer to the current block of binary data that is used for hash
     * calculation.
     * @param length The size of the data provided.
     */
    void Update(const uint8_t* data, std::size_t length);

    /**
     * @brief Computes the hash value of all the binary input data appended to the instance so far.
     * @remark Use #Update() to add more partial data before calling this function.
     * @return The computed MD5 hash value corresponding to the input provided.
     */
    std::vector<uint8_t> Digest() const;

    /**
     * @brief Computes the hash value of the specified binary input data.
     * @param data The pointer to binary data to compute the hash value for.
     * @param length The size of the data provided.
     * @return The computed MD5 hash value corresponding to the input provided.
     */
    static std::vector<uint8_t> Hash(const uint8_t* data, std::size_t length)
    {
      Md5 instance;
      instance.Update(data, length);
      return instance.Digest();
    }

    /**
     * @brief Computes the hash value of the specified binary input data.
     * @param data The input vector to compute the hash value for.
     * @return The computed MD5 hash value corresponding to the input provided.
     */
    static std::vector<uint8_t> Hash(const std::vector<uint8_t>& data)
    {
      return Hash(data.data(), data.size());
    }

  private:
    void* m_md5Context;
  };

}} // namespace Azure::Core
