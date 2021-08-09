// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Keyvault operations definition.
 *
 */

#include "azure/keyvault/secrets/keyvault_operations.hpp"
#include "azure/keyvault/secrets/secret_client.hpp"
#include "private/secret_serializers.hpp"
// KeyVaultRestoreDeletedSecretOperation

Azure::Response<Secret> KeyVaultRestoreDeletedSecretOperation::PollUntilDoneInternal(
    std::chrono::milliseconds period,
    Azure::Core::Context& context)
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

  return Azure::Response<Secret>(
      m_value, std::make_unique<Azure::Core::Http::RawResponse>(*m_rawResponse));
}

std::unique_ptr<Azure::Core::Http::RawResponse> KeyVaultRestoreDeletedSecretOperation::PollInternal(
    Azure::Core::Context const& context)
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

KeyVaultRestoreDeletedSecretOperation::KeyVaultRestoreDeletedSecretOperation(
    std::shared_ptr<SecretClient> secretClient,
    Azure::Response<Secret> response)
    : m_secretClient(secretClient)
{
  m_value = response.Value;

  m_rawResponse = std::move(response.RawResponse);

  m_continuationToken = m_value.Name;

  if (m_value.Name.empty() == false)
  {
    m_status = Azure::Core::OperationStatus::Succeeded;
  }
}

KeyVaultRestoreDeletedSecretOperation::KeyVaultRestoreDeletedSecretOperation(
    std::string resumeToken,
    std::shared_ptr<SecretClient> secretClient)
    : m_secretClient(secretClient), m_continuationToken(std::move(resumeToken))
{
  m_value.Name = resumeToken;
}

KeyVaultRestoreDeletedSecretOperation KeyVaultRestoreDeletedSecretOperation::CreateFromResumeToken(
    std::string const& resumeToken,
    SecretClient const& client,
    Azure::Core::Context const& context)
{
  KeyVaultRestoreDeletedSecretOperation operation(
      resumeToken, std::make_shared<SecretClient>(client));
  operation.Poll(context);
  return operation;
}
// KeyVaultDeleteSecretOperation
Azure::Response<DeletedSecret> KeyVaultDeleteSecretOperation::PollUntilDoneInternal(
    std::chrono::milliseconds period,
    Azure::Core::Context& context)
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

  return Azure::Response<DeletedSecret>(
      m_value, std::make_unique<Azure::Core::Http::RawResponse>(*m_rawResponse));
}

std::unique_ptr<Azure::Core::Http::RawResponse> KeyVaultDeleteSecretOperation::PollInternal(
    Azure::Core::Context const& context)
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

KeyVaultDeleteSecretOperation::KeyVaultDeleteSecretOperation(
    std::shared_ptr<SecretClient> secretClient,
    Azure::Response<DeletedSecret> response)
    : m_secretClient(secretClient)
{
  m_value = response.Value;
  m_rawResponse = std::move(response.RawResponse);
  m_continuationToken = m_value.Name;

  if (m_value.Name.empty() == false)
  {
    m_status = Azure::Core::OperationStatus::Succeeded;
  }
}

KeyVaultDeleteSecretOperation::KeyVaultDeleteSecretOperation(
    std::string resumeToken,
    std::shared_ptr<SecretClient> secretClient)
    : m_secretClient(secretClient), m_continuationToken(std::move(resumeToken))
{
  m_value.Name = resumeToken;
}

KeyVaultDeleteSecretOperation KeyVaultDeleteSecretOperation::CreateFromResumeToken(
    std::string const& resumeToken,
    SecretClient const& client,
    Azure::Core::Context const& context)
{
  KeyVaultDeleteSecretOperation operation(resumeToken, std::make_shared<SecretClient>(client));
  operation.Poll(context);
  return operation;
}
