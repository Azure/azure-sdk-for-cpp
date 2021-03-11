// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Utility functions to help compute the hash value for the input binary data, using
 * algorithms such as MD5.
 */

#pragma once

#include <stdexcept>
#include <stdint.h>
#include <string>
#include <vector>

namespace Azure { namespace Core { namespace Cryptography {

  /**
   * @brief Represents the base class for hash algorithms which map binary data of an arbitrary
   * length to small binary data of a fixed length.
   */
  class Hash {
  private:
    /**
     * @brief Used to append partial binary input data to compute the hash in a streaming fashion.
     * @remark Once all the data has been added, call #Azure::Core::Cryptography::Hash::Final() to
     * get the computed hash value.
     * @param data The pointer to the current block of binary data that is used for hash
     * calculation.
     * @param length The size of the data provided.
     */
    virtual void OnAppend(const uint8_t* data, std::size_t length) = 0;

    /**
     * @brief Computes the hash value of the specified binary input data, including any previously
     * appended.
     * @param data The pointer to binary data to compute the hash value for.
     * @param length The size of the data provided.
     * @return The computed hash value corresponding to the input provided including any previously
     * appended.
     */
    virtual std::vector<uint8_t> OnFinal(const uint8_t* data, std::size_t length) = 0;

  public:
    /**
     * @brief Construct a default instance of #Azure::Core::Cryptography::Hash.
     */
    Hash() = default;

    /**
     * @brief Used to append partial binary input data to compute the hash in a streaming fashion.
     * @remark Once all the data has been added, call #Azure::Core::Cryptography::Hash::Final() to
     * get the computed hash value.
     * @remark Do not call this function after a call to #Azure::Core::Cryptography::Hash::Final().
     * @param data The pointer to the current block of binary data that is used for hash
     * calculation.
     * @param length The size of the data provided.
     */
    void Append(const uint8_t* data, std::size_t length)
    {
      if (!data && length != 0)
      {
        throw std::invalid_argument(
            "Length cannot be " + std::to_string(length) + " if the data pointer is null.");
      }
      if (m_isDone)
      {
        throw std::runtime_error("Cannot call Append after calling Final().");
      }
      OnAppend(data, length);
    }

    /**
     * @brief Computes the hash value of the specified binary input data, including any previously
     * appended.
     * @remark Do not call this function multiple times.
     * @param data The pointer to the last block of binary data to compute the hash value for.
     * @param length The size of the data provided.
     * @return The computed hash value corresponding to the input provided, including any previously
     * appended.
     */
    std::vector<uint8_t> Final(const uint8_t* data, std::size_t length)
    {
      if (!data && length != 0)
      {
        throw std::invalid_argument(
            "Length cannot be " + std::to_string(length) + " if the data pointer is null.");
      }
      if (m_isDone)
      {
        throw std::runtime_error("Cannot call Final() multiple times.");
      }
      m_isDone = true;
      return OnFinal(data, length);
    }

    /**
     * @brief Computes the hash value of all the binary input data appended to the instance so far.
     * @remark Use #Azure::Core::Cryptography::Hash::Append() to add more partial data before
     * calling this function.
     * @remark Do not call this function multiple times.
     * @return The computed hash value corresponding to the input provided.
     */
    std::vector<uint8_t> Final() { return Final(nullptr, 0); }

    /**
     * @brief Cleanup any state when destroying the instance of #Azure::Core::Cryptography::Hash.
     */
    virtual ~Hash() = default;

  private:
    bool m_isDone = false;

    // Delete the copy constructor, along with the assignment operator.
    Hash(Hash const&) = delete;
    void operator=(Hash const&) = delete;
  };

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
    void* m_md5Context;

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
