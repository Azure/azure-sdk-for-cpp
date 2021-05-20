// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Compute the hash value for the input binary data, using
 * SHA256, SHA384 and SHA512.
 *
 */

#pragma once

#include <azure/core/cryptography/hash.hpp>

#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault {

  /**
   * @brief Defines #SHA256.
   *
   */
  class SHA256 final : public Azure::Core::Cryptography::Hash {
  public:
    /**
     * @brief Construct a default instance of #SHA256.
     *
     */
    SHA256();

    /**
     * @brief Cleanup any state when destroying the instance of #SHA256.
     *
     */
    ~SHA256(){};

  private:
    /**
     * @brief Underline implementation based on the OS.
     *
     */
    std::unique_ptr<Azure::Core::Cryptography::Hash> m_portableImplementation;

    /**
     * @brief Computes the hash value of the specified binary input data, including any previously
     * appended.
     * @param data The pointer to binary data to compute the hash value for.
     * @param length The size of the data provided.
     * @return The computed SHA256 hash value corresponding to the input provided including any
     * previously appended.
     */
    std::vector<uint8_t> OnFinal(const uint8_t* data, std::size_t length) override
    {
      return m_portableImplementation->Final(data, length);
    }

    /**
     * @brief Used to append partial binary input data to compute the SHA256 hash in a streaming
     * fashion.
     * @remark Once all the data has been added, call #Final() to get the computed hash value.
     * @param data The pointer to the current block of binary data that is used for hash
     * calculation.
     * @param length The size of the data provided.
     */
    void OnAppend(const uint8_t* data, std::size_t length) override
    {
      return m_portableImplementation->Append(data, length);
    }
  };

  /**
   * @brief Defines #SHA384.
   *
   */
  class SHA384 final : public Azure::Core::Cryptography::Hash {
  public:
    /**
     * @brief Construct a default instance of #SHA384.
     *
     */
    SHA384();

    /**
     * @brief Cleanup any state when destroying the instance of #SHA384.
     *
     */
    ~SHA384(){};

  private:
    /**
     * @brief Underline implementation based on the OS.
     *
     */
    std::unique_ptr<Azure::Core::Cryptography::Hash> m_portableImplementation;

    /**
     * @brief Computes the hash value of the specified binary input data, including any previously
     * appended.
     * @param data The pointer to binary data to compute the hash value for.
     * @param length The size of the data provided.
     * @return The computed SHA384 hash value corresponding to the input provided including any
     * previously appended.
     */
    std::vector<uint8_t> OnFinal(const uint8_t* data, std::size_t length) override
    {
      return m_portableImplementation->Final(data, length);
    }

    /**
     * @brief Used to append partial binary input data to compute the SHA384 hash in a streaming
     * fashion.
     * @remark Once all the data has been added, call #Final() to get the computed hash value.
     * @param data The pointer to the current block of binary data that is used for hash
     * calculation.
     * @param length The size of the data provided.
     */
    void OnAppend(const uint8_t* data, std::size_t length) override
    {
      return m_portableImplementation->Append(data, length);
    }
  };

  /**
   * @brief Defines #SHA512.
   *
   */
  class SHA512 final : public Azure::Core::Cryptography::Hash {
  public:
    /**
     * @brief Construct a default instance of #SHA512.
     *
     */
    SHA512();

    /**
     * @brief Cleanup any state when destroying the instance of #SHA512.
     *
     */
    ~SHA512(){};

  private:
    /**
     * @brief Underline implementation based on the OS.
     *
     */
    std::unique_ptr<Azure::Core::Cryptography::Hash> m_portableImplementation;

    /**
     * @brief Computes the hash value of the specified binary input data, including any previously
     * appended.
     * @param data The pointer to binary data to compute the hash value for.
     * @param length The size of the data provided.
     * @return The computed SHA512 hash value corresponding to the input provided including any
     * previously appended.
     */
    std::vector<uint8_t> OnFinal(const uint8_t* data, std::size_t length) override
    {
      return m_portableImplementation->Final(data, length);
    }

    /**
     * @brief Used to append partial binary input data to compute the SHA512 hash in a streaming
     * fashion.
     * @remark Once all the data has been added, call #Final() to get the computed hash value.
     * @param data The pointer to the current block of binary data that is used for hash
     * calculation.
     * @param length The size of the data provided.
     */
    void OnAppend(const uint8_t* data, std::size_t length) override
    {
      return m_portableImplementation->Append(data, length);
    }
  };

}}} // namespace Azure::Security::KeyVault
