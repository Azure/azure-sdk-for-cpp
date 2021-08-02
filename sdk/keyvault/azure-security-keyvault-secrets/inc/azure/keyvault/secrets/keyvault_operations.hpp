// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Provides a wrapper class for the Azure Core Operation
 *
 */

#pragma once
#include "../src/private/secret_serializers.hpp"
#include "azure/keyvault/secrets/keyvault_deleted_secret.hpp"
#include <azure/core/http/http.hpp>
#include <azure/core/operation.hpp>
#include <azure/core/operation_status.hpp>
#include <azure/core/response.hpp>
#include <azure/keyvault/secrets/keyvault_secret.hpp>
#include <azure/keyvault/secrets/secret_client.hpp>
#include <thread>

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets {

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

      return Azure::Response<KeyVaultSecret>(
          m_value, std::make_unique<Azure::Core::Http::RawResponse>(*m_rawResponse));
    }

    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        Azure::Core::Context const& context) override
    {
      std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse;
      if (IsDone())
      {
        try
        {
          rawResponse
              = m_secretClient->GetSecret(m_value.Name, GetSecretOptions(), context).RawResponse;
        }
        catch (Azure::Core::RequestFailedException& error)
        {
          rawResponse = std::move(error.RawResponse);
        }

        switch (rawResponse->GetStatusCode())
        {
          case Azure::Core::Http::HttpStatusCode::Ok:
          case Azure::Core::Http::HttpStatusCode::Forbidden: {
            m_status = Azure::Core::OperationStatus::Succeeded;
            break;
          }
          case Azure::Core::Http::HttpStatusCode::NotFound: {
            m_status = Azure::Core::OperationStatus::Running;
            break;
          }
          default:
            throw Azure::Core::RequestFailedException(rawResponse);
        }

        if (m_status == Azure::Core::OperationStatus::Succeeded)
        {
          m_value = _detail::KeyVaultSecretSerializer::KeyVaultSecretDeserialize(
              m_value.Name, *rawResponse);
        }
      }
      return rawResponse;
    }

    /*
     * Only friend classes are permitted to construct an Operation. This is because a
     * KeyVaultPipelne is required and it is not exposed to customers.
     *
     * Since C++ doesn't offer `internal` access, we use friends-only instead.
     */
    KeyVaultRestoreDeletedSecretOperation(
        std::shared_ptr<SecretClient> secretClient,
        Azure::Response<KeyVaultSecret> response)
        : m_secretClient(secretClient)
    {
      m_value = response.Value;

      m_rawResponse = std::move(response.RawResponse);

      m_continuationToken = ((KeyVaultSecret)m_value).Name;

      if (m_value.Name.empty() == false)
      {
        m_status = Azure::Core::OperationStatus::Succeeded;
      }
    };

    KeyVaultRestoreDeletedSecretOperation(
        std::string resumeToken,
        std::shared_ptr<SecretClient> secretClient)
        : m_secretClient(secretClient), m_continuationToken(std::move(resumeToken))
    {
      m_value.Name = resumeToken;
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
        Azure::Core::Context const& context = Azure::Core::Context())
    {
      KeyVaultRestoreDeletedSecretOperation operation(
          resumeToken, std::make_shared<SecretClient>(client));
      operation.Poll(context);
      return operation;
    };
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
        Azure::Core::Context& context) override
    {
      while (true)
      {
        Poll(context);
        if (IsDone())
        {
          break;
        }
        std::this_thread::sleep_for(period);
      }

      return Azure::Response<KeyVaultDeletedSecret>(
          m_value, std::make_unique<Azure::Core::Http::RawResponse>(*m_rawResponse));
    }

    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        Azure::Core::Context const& context) override
    {
      std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse;
      if (!IsDone())
      {
        try
        {
          rawResponse = m_secretClient->GetDeletedSecret(m_value.Name, context).RawResponse;
        }
        catch (Azure::Core::RequestFailedException& error)
        {
          rawResponse = std::move(error.RawResponse);
        }

        switch (rawResponse->GetStatusCode())
        {
          case Azure::Core::Http::HttpStatusCode::Ok:
          case Azure::Core::Http::HttpStatusCode::Forbidden: {
            m_status = Azure::Core::OperationStatus::Succeeded;
            break;
          }
          case Azure::Core::Http::HttpStatusCode::NotFound: {
            m_status = Azure::Core::OperationStatus::Running;
            break;
          }
          default:
            throw Azure::Core::RequestFailedException(rawResponse);
        }

        if (m_status == Azure::Core::OperationStatus::Succeeded)
        {
          m_value = _detail::KeyVaultDeletedSecretSerializer::KeyVaultDeletedSecretDeserialize(
              m_value.Name, *rawResponse);
        }
      }
      return rawResponse;
    }

    /*
     * Only friend classes are permitted to call the constructor . This is because a
     * KeyVaultPipelne is required and it is not exposed to customers.
     *
     * Since C++ doesn't offer `internal` access, we use friends-only instead.
     */
    KeyVaultDeleteSecretOperation(
        std::shared_ptr<SecretClient> secretClient,
        Azure::Response<KeyVaultDeletedSecret> response)
        : m_secretClient(secretClient)
    {
      m_value = response.Value;
      m_rawResponse = std::move(response.RawResponse);
      m_continuationToken = m_value.Name;

      if (m_value.Name.empty() == false)
      {
        m_status = Azure::Core::OperationStatus::Succeeded;
      }
    };

    KeyVaultDeleteSecretOperation(
        std::string resumeToken,
        std::shared_ptr<SecretClient> secretClient)
        : m_secretClient(secretClient), m_continuationToken(std::move(resumeToken))
    {
      m_value.Name = resumeToken;
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
        Azure::Core::Context const& context = Azure::Core::Context())
    {
      KeyVaultDeleteSecretOperation operation(resumeToken, std::make_shared<SecretClient>(client));
      operation.Poll(context);
      return operation;
    };
  };
}}}} // namespace Azure::Security::KeyVault::Secrets
