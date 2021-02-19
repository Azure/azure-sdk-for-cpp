// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief A long-running operation for deleting a Key.
 *
 */

#pragma once

#include <azure/core/http/http.hpp>
#include <azure/core/operation.hpp>
#include <azure/core/operation_status.hpp>
#include <azure/core/response.hpp>

#include <azure/keyvault/common/internal/keyvault_pipeline.hpp>
#include <azure/keyvault/common/keyvault_exception.hpp>

#include "azure/keyvault/keys/deleted_key.hpp"

#include <memory>
#include <string>
#include <thread>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  /**
   * @brief A long running operation to delete a key.
   *
   */
  class DeleteKeyOperation
      : public Azure::Core::Operation<Azure::Security::KeyVault::Keys::DeletedKey> {
  private:
    /* DeleteKeyOperation can be constructed only by friends classes (internal creation). The
     * constructor is private and requires internal components.*/
    friend class KeyClient;

    std::shared_ptr<Azure::Security::KeyVault::Common::Internal::KeyVaultPipeline> m_pipeline;
    Azure::Security::KeyVault::Keys::DeletedKey m_value;
    std::unique_ptr<Azure::Core::Http::RawResponse> m_rawResponse;
    std::string m_continuationToken;

    /* This is the implementation for checking the status of a deleted key. The key is considered
     * deleted if querying /deletedkeys/keyName returns 200 from server. Or whenever soft-delete is
     * disabled.*/
    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        Azure::Core::Context& context) override;

    Azure::Core::Response<Azure::Security::KeyVault::Keys::DeletedKey> PollUntilDoneInternal(
        Azure::Core::Context& context,
        std::chrono::milliseconds period) override
    {
      while (true)
      {
        m_rawResponse = Poll(context);
        if (IsDone())
        {
          break;
        }
        std::this_thread::sleep_for(period);
      }

      return Azure::Core::Response<Azure::Security::KeyVault::Keys::DeletedKey>(
          m_value, std::make_unique<Azure::Core::Http::RawResponse>(*m_rawResponse));
    }

    /*
     * Only friend classes are permitted to construct a DeleteOperation. This is because a
     * KeyVaultPipelne is required and it is not exposed to customers.
     *
     * Since C++ doesn't offer `internal` access, we use friends-only instead.
     */
    DeleteKeyOperation(
        std::shared_ptr<Azure::Security::KeyVault::Common::Internal::KeyVaultPipeline>
            keyvaultPipeline,
        Azure::Core::Response<Azure::Security::KeyVault::Keys::DeletedKey> response)
        : m_pipeline(keyvaultPipeline)
    {
      if (!response.HasValue())
      {
        throw Azure::Security::KeyVault::Common::KeyVaultException(
            "The response does not contain a value.");
      }
      // The response becomes useless and the value and rawResponse are now owned by the
      // DeleteKeyOperation. This is fine because the DeleteKeyOperation is what the delete key api
      // will return.
      m_value = response.ExtractValue();
      m_rawResponse = response.ExtractRawResponse();

      // Build the full url for continuation token. It is only used in case customers wants to use
      // it on their own. The Operation uses the KeyVaultPipeline from the client which knows how to
      // build this url.
      m_continuationToken = m_pipeline->GetVaultUrl() + "/" + std::string(Details::DeletedKeysPath)
          + "/" + m_value.Name();

      // The recoveryId is only returned if soft-delete is enabled.
      // The LRO is considered completed for non soft-delete (key will be eventually removed).
      if (m_value.RecoveryId.empty())
      {
        m_status = Azure::Core::OperationStatus::Succeeded;
      }
    }

  public:
    /**
     * @brief Get the #Azure::Security::KeyVault::Keys::DeletedKey object.
     *
     * @remark The deleted key contains the recovery id if the key can be recovered.
     *
     * @return A deleted key object.
     */
    Azure::Security::KeyVault::Keys::DeletedKey Value() const override { return m_value; }

    /**
     * @brief Get an Url as string which can be used to get the status of the delete key operation.
     *
     * @return std::string
     */
    std::string GetResumeToken() const override { return m_continuationToken; }
  };

}}}} // namespace Azure::Security::KeyVault::Keys
