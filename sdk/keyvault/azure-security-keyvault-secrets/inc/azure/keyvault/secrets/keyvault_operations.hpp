// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Provides a wrapper class for the Azure Core Operation
 *
 */

#pragma once

#include <azure/core/http/http.hpp>
#include <azure/core/operation.hpp>
#include <azure/core/operation_status.hpp>
#include <azure/core/response.hpp>
#include <azure/keyvault/secrets/keyvault_secret.hpp>
#include <azure/keyvault/secrets/secret_client.hpp>

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets {
  template <class T> class KeyVaultSecretsOperations final : public Azure::Core::Operation<T> {

  private:
    friend class SecretClient;
    std::shared_ptr<SecretClient> m_keyClient;
    T m_value;
    std::string m_continuationToken;

    Azure::Response<T> PollUntilDoneInternal(
        std::chrono::milliseconds period,
        Azure::Core::Context& context) override
    {
      while (true)
      {
        // Poll will update the raw response.
        this->Poll(context);
        if (this->IsDone())
        {
          break;
        }
        std::this_thread::sleep_for(period);
      }

      return Azure::Response<T>(
          m_value, std::make_unique<Azure::Core::Http::RawResponse>(*this->m_rawResponse));
    }

    std::unique_ptr<Azure::Core::Http::RawResponse> PollInternal(
        Azure::Core::Context const& context) override
    {
      std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse;
      if (!this->IsDone())
      {
        if (context.IsCancelled())
        {
          throw std::exception("dsds");
        }
        /* try
        {
          rawResponse = m_keyClient->GetDeletedKey(m_value.Name(), context).RawResponse;
        }
        catch (Azure::Core::RequestFailedException& error)
        {
          rawResponse = std::move(error.RawResponse);
        }*/

        switch (rawResponse->GetStatusCode())
        {
          case Azure::Core::Http::HttpStatusCode::Ok:
          case Azure::Core::Http::HttpStatusCode::Forbidden: // Access denied but proof the
                                                             // key was deleted.
          {
            this->m_status = Azure::Core::OperationStatus::Succeeded;
            break;
          }
          case Azure::Core::Http::HttpStatusCode::NotFound: {
            this->m_status = Azure::Core::OperationStatus::Running;
            break;
          }
          default:
            throw Azure::Core::RequestFailedException(rawResponse);
        }

        if (this->m_status == Azure::Core::OperationStatus::Succeeded)
        {
          /* m_value
              = _detail::DeletedKeySerializer::DeletedKeyDeserialize(m_value.Name(),
             *rawResponse);*/
        }
      }
      return rawResponse;
    }

    /*
     * Only friend classes are permitted to construct a DeleteOperation. This is because a
     * KeyVaultPipelne is required and it is not exposed to customers.
     *
     * Since C++ doesn't offer `internal` access, we use friends-only instead.
     */
    KeyVaultSecretsOperations(std::shared_ptr<SecretClient> keyClient, Azure::Response<T> response)
        : m_keyClient(keyClient)
    {
      // The response becomes useless and the value and rawResponse are now owned by the
      // DeleteKeyOperation. This is fine because the DeleteKeyOperation is what the delete key api
      // will return.
      m_value = response.Value;
      this->m_rawResponse = std::move(response.RawResponse);

      // The key name is enough to be used as continuation token.
      //      m_continuationToken = m_value.Name();

      // The recoveryId is only returned if soft-delete is enabled.
      // The LRO is considered completed for non soft-delete (key will be eventually removed).
      /* if (m_value.RecoveryId.empty())
      {
        this->m_status = Azure::Core::OperationStatus::Succeeded;
      }*/
    };

    KeyVaultSecretsOperations(std::string resumeToken, std::shared_ptr<SecretClient> keyClient)
        : m_keyClient(keyClient), m_value(T(resumeToken)),
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
      return *this->m_rawResponse;
    }

  public:
    /**
     * @brief Get the #Azure::Security::KeyVault::Keys::DeletedKey object.
     *
     * @remark The deleted key contains the recovery id if the key can be recovered.
     *
     * @return A deleted key object.
     */
    T Value() const override { return m_value; }

    /**
     * @brief Get an Url as string which can be used to get the status of the delete key operation.
     *
     * @return std::string
     */
    std::string GetResumeToken() const override { return m_continuationToken; }

    /**
     * @brief Create a #DeleteKeyOperation from the \p resumeToken fetched from another
     * `Operation<T>`, updated to the the latest operation status.
     *
     * @remark After the operation is initialized, it is used to poll the last update from the
     * server using the \p context.
     *
     * @param resumeToken A previously generated token used to resume the polling of the operation.
     * @param client A #KeyClient that is used for getting status updates.
     * @param context A #Azure::Core::Context controlling the request lifetime.
     * @return DeleteKeyOperation
     */
    static KeyVaultSecretsOperations CreateFromResumeToken(
        std::string const& resumeToken,
        SecretClient const& client,
        Azure::Core::Context const& context = Azure::Core::Context())
    {
      KeyVaultSecretsOperations operation(resumeToken, std::make_shared<SecretClient>(client));
      operation.Poll(context);
      return operation;
    };
  };
}}}} // namespace Azure::Security::KeyVault::Secrets
