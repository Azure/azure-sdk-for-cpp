// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Provides a wrapper class for the Azure Core Operation
 *
 */

#pragma once
#include "azure/keyvault/secrets/keyvault_deleted_secret.hpp"
#include "azure/keyvault/secrets/keyvault_secret.hpp"
#include <azure/core/http/http.hpp>
#include <azure/core/operation.hpp>
#include <azure/core/operation_status.hpp>
#include <azure/core/response.hpp>
#include <thread>

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets {
  class SecretClient;
  /**
   * @brief Represents a long running operation to restore a deleted secret.
   */
  class KeyVaultRestoreDeletedSecretOperation final
      : public Azure::Core::Operation<KeyVaultSecret> {

  private:
    friend class SecretClient;
    std::shared_ptr<SecretClient> m_secretClient;
    KeyVaultSecret m_value;
    std::string m_continuationToken;

    Azure::Response<KeyVaultSecret> PollUntilDoneInternal(
        std::chrono::milliseconds period,
        Azure::Core::Context& context) override;

    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        Azure::Core::Context const& context) override;

    /*
     * Only friend classes are permitted to construct an Operation. This is because a
     * KeyVaultPipelne is required and it is not exposed to customers.
     *
     * Since C++ doesn't offer `internal` access, we use friends-only instead.
     */
    KeyVaultRestoreDeletedSecretOperation(
        std::shared_ptr<SecretClient> secretClient,
        Azure::Response<KeyVaultSecret> response);

    KeyVaultRestoreDeletedSecretOperation(
        std::string resumeToken,
        std::shared_ptr<SecretClient> secretClient);

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
     * @brief Get the #Azure::Security::KeyVault::Secrets::KeyVaultSecret object.
     *
     * @return A KeyVaultSecret object.
     */
    KeyVaultSecret Value() const override { return m_value; }

    /**
     * @brief Get an Url as string which can be used to get the status of the operation.
     *
     * @return std::string
     */
    std::string GetResumeToken() const override { return m_continuationToken; }

    /**
     * @brief Create a #KeyVaultRestoreDeletedSecretOperation from the \p resumeToken fetched from
     * another `Operation<T>`, updated to the the latest operation status.
     *
     * @remark After the operation is initialized, it is used to poll the last update from the
     * server using the \p context.
     *
     * @param resumeToken A previously generated token used to resume the polling of the
     * operation.
     * @param client A #secretClient that is used for getting status updates.
     * @param context A #Azure::Core::Context controlling the request lifetime.
     * @return KeyVaultRestoreDeletedSecretOperation
     */
    static KeyVaultRestoreDeletedSecretOperation CreateFromResumeToken(
        std::string const& resumeToken,
        SecretClient const& client,
        Azure::Core::Context const& context = Azure::Core::Context());
  };

  /**
   * @brief Represents a delete secret long running operation
   */
  class KeyVaultDeleteSecretOperation final : public Azure::Core::Operation<KeyVaultDeletedSecret> {

  private:
    friend class SecretClient;
    std::shared_ptr<SecretClient> m_secretClient;
    KeyVaultDeletedSecret m_value;
    std::string m_continuationToken;

    Azure::Response<KeyVaultDeletedSecret> PollUntilDoneInternal(
        std::chrono::milliseconds period,
        Azure::Core::Context& context) override;

    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        Azure::Core::Context const& context) override;

    /*
     * Only friend classes are permitted to call the constructor . This is because a
     * KeyVaultPipelne is required and it is not exposed to customers.
     *
     * Since C++ doesn't offer `internal` access, we use friends-only instead.
     */
    KeyVaultDeleteSecretOperation(
        std::shared_ptr<SecretClient> secretClient,
        Azure::Response<KeyVaultDeletedSecret> response);

    KeyVaultDeleteSecretOperation(
        std::string resumeToken,
        std::shared_ptr<SecretClient> secretClient);

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
     * @brief Get the #Azure::Security::KeyVault::Secrets::KeyVaultDeletedSecret object.
     *
     * @remark The deleted secret contains the recovery id if the key can be recovered.
     *
     * @return A deleted secret object.
     */
    KeyVaultDeletedSecret Value() const override { return m_value; }

    /**
     * @brief Get an Url as string which can be used to get the status of the delete secret
     * operation.
     *
     * @return std::string
     */
    std::string GetResumeToken() const override { return m_continuationToken; }

    /**
     * @brief Create a #KeyVaultDeleteSecretOperation from the \p resumeToken fetched from another
     * `Operation<T>`, updated to the the latest operation status.
     *
     * @remark After the operation is initialized, it is used to poll the last update from the
     * server using the \p context.
     *
     * @param resumeToken A previously generated token used to resume the polling of the
     * operation.
     * @param client A #secretClient that is used for getting status updates.
     * @param context A #Azure::Core::Context controlling the request lifetime.
     * @return KeyVaultDeleteSecretOperation
     */
    static KeyVaultDeleteSecretOperation CreateFromResumeToken(
        std::string const& resumeToken,
        SecretClient const& client,
        Azure::Core::Context const& context = Azure::Core::Context());
  };
}}}} // namespace Azure::Security::KeyVault::Secrets
