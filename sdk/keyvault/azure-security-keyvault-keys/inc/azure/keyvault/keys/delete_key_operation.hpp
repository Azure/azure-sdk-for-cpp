// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief A long-running operation for deleting a Key.
 *
 */

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/operation.hpp>
#include <azure/core/operation_status.hpp>
#include <azure/core/response.hpp>

#include "azure/keyvault/keys/deleted_key.hpp"

#include <memory>
#include <string>
#include <thread>

namespace Azure { namespace Security { namespace KeyVault { namespace Keys {

  class KeyClient;

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

    std::shared_ptr<Azure::Security::KeyVault::Keys::KeyClient> m_keyClient;
    Azure::Security::KeyVault::Keys::DeletedKey m_value;
    std::string m_continuationToken;

    /* This is the implementation for checking the status of a deleted key. The key is considered
     * deleted if querying /deletedkeys/keyName returns 200 from server. Or whenever soft-delete is
     * disabled.*/
    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        Azure::Core::Context const& context) override;

    Azure::Response<Azure::Security::KeyVault::Keys::DeletedKey> PollUntilDoneInternal(
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

      return Azure::Response<Azure::Security::KeyVault::Keys::DeletedKey>(
          m_value, std::make_unique<Azure::Core::Http::RawResponse>(*m_rawResponse));
    }

    /*
     * Only friend classes are permitted to construct a DeleteOperation. This is because a
     * KeyVaultPipelne is required and it is not exposed to customers.
     *
     * Since C++ doesn't offer `internal` access, we use friends-only instead.
     */
    DeleteKeyOperation(
        std::shared_ptr<Azure::Security::KeyVault::Keys::KeyClient> keyClient,
        Azure::Response<Azure::Security::KeyVault::Keys::DeletedKey> response);

    DeleteKeyOperation(
        std::shared_ptr<Azure::Security::KeyVault::Keys::KeyClient> keyClient,
        std::string resumeToken)
        : m_keyClient(keyClient), m_value(DeletedKey(resumeToken)),
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

    /**
     * @brief Create a #DeleteKeyOperation from the \p resumeToken fetched from another `Operation<T>`, updated to the
     * the latest operation status.
     *
     * @remark After the operation is initialized, it is used to poll the last update from the server using
     * the \p context.
     *
     * @param client A #KeyClient that is used for getting status updates.
     * @param resumeToken A previously generated token used to resume the polling of the operation.
     * @param context A #Azure::Core::Context controlling the request lifetime.
     * @return DeleteKeyOperation
     */
    static DeleteKeyOperation CreateFromResumeToken(
        Azure::Security::KeyVault::Keys::KeyClient const& client,
        std::string const& resumeToken,
        Azure::Core::Context const& context = Azure::Core::Context())
    {

      DeleteKeyOperation operation(
          std::make_shared<Azure::Security::KeyVault::Keys::KeyClient>(client), resumeToken);
      operation.Poll(context);
      return operation;
    }
  };

}}}} // namespace Azure::Security::KeyVault::Keys
