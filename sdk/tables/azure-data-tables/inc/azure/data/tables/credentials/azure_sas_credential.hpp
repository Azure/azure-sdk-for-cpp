// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/http/http.hpp>

#include <memory>
#include <mutex>
#include <string>

namespace Azure { namespace Data { namespace Tables { namespace Credentials {
  /**
   * @brief Azure Shared Access Signature (SAS) credential.
   */
  class AzureSasCredential final {
  private:
    std::string m_signature;
    mutable std::mutex m_mutex;

  public:
    /**
     * @brief Initializes a new instance of the AzureSasCredential.
     *
     * @param signature The signature for the SAS token.
     */
    explicit AzureSasCredential(std::string signature) : m_signature(std::move(signature)) {}

    /**
     * @brief Get the signature for the SAS token.
     */
    std::string GetSignature() const
    {
      std::lock_guard<std::mutex> guard(m_mutex);
      return m_signature;
    }

    /**
     * @brief Update the signature for the SAS token.
     */
    void Update(std::string signature)
    {
      std::lock_guard<std::mutex> guard(m_mutex);
      m_signature = std::move(signature);
    }
  };
}}}} // namespace Azure::Data::Tables::Credentials
