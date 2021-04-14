// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief A long-running operation for recovering a Key.
 *
 */

#pragma once

#include <azure/core/http/http.hpp>
#include <azure/core/operation.hpp>
#include <azure/core/operation_status.hpp>
#include <azure/core/response.hpp>

#include <azure/keyvault/common/internal/keyvault_pipeline.hpp>
#include <azure/keyvault/common/keyvault_exception.hpp>

#include "azure/keyvault/keys/key_vault_key.hpp"

#include <memory>
#include <string>
#include <thread>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  /**
   * @brief A long running operation to recover a key.
   *
   */
  class RecoverDeletedKeyOperation : public Azure::Core::Operation<KeyVaultKey> {
  private:
    /* RecoverDeletedKeyOperation can be constructed only by friends classes (internal creation).
     * The constructor is private and requires internal components.*/
    friend class KeyClient;

    std::shared_ptr<Azure::Security::KeyVault::_internal::KeyVaultPipeline> m_pipeline;
    Azure::Security::KeyVault::Keys::KeyVaultKey m_value;
    std::string m_continuationToken;

    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        Azure::Core::Context& context) override;

    Azure::Response<Azure::Security::KeyVault::Keys::KeyVaultKey> PollUntilDoneInternal(
        std::chrono::milliseconds period,
        Azure::Core::Context& context) override
    {
      while (true)
      {
        // Poll will update the raw response.
        Poll(context);
        if (IsDone())
        {
          break;
        }
        std::this_thread::sleep_for(period);
      }

      return Azure::Response<Azure::Security::KeyVault::Keys::KeyVaultKey>(
          m_value, std::make_unique<Azure::Core::Http::RawResponse>(*m_rawResponse));
    }

    /*
     * Only friend classes are permitted to construct a RecoverDeletedKeyOperation. This is because
     * a KeyVaultPipelne is required and it is not exposed to customers.
     *
     * Since C++ doesn't offer `internal` access, we use friends-only instead.
     */
    RecoverDeletedKeyOperation(
        std::shared_ptr<Azure::Security::KeyVault::_internal::KeyVaultPipeline> keyvaultPipeline,
        Azure::Response<Azure::Security::KeyVault::Keys::KeyVaultKey> response);

    RecoverDeletedKeyOperation(
        std::shared_ptr<Azure::Security::KeyVault::_internal::KeyVaultPipeline> keyvaultPipeline,
        std::string resumeToken)
        : m_pipeline(keyvaultPipeline), m_value(DeletedKey(resumeToken)),
          m_continuationToken(std::move(resumeToken))
    {
    }

    /**
     * @brief Get the #Azure::Core::Http::RawResponse of the operation request.
     * @return A reference to an #Azure::Core::Http::RawResponse.
     * @note Does not give up ownership of the RawResponse.
     */
    Azure::Core::Http::RawResponse const& GetRawResponseInternal() const override
    {
      return *m_rawResponse;
    }

  public:
    /**
     * @brief Get the #Azure::Security::KeyVault::Keys::KeyVaultKey object.
     *
     * @remark The deleted key contains the recovery id if the key can be recovered.
     *
     * @return A deleted key object.
     */
    Azure::Security::KeyVault::Keys::KeyVaultKey Value() const override { return m_value; }

    /**
     * @brief Get an Url as string which can be used to get the status of the delete key operation.
     *
     * @return std::string
     */
    std::string GetResumeToken() const override { return m_continuationToken; }
  };

}}}} // namespace Azure::Security::KeyVault::Keys
