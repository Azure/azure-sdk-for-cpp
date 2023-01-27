//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include "azure/keyvault/certificates/certificate_client_operations.hpp"
#include "azure/keyvault/certificates/certificate_client.hpp"
#include "azure/keyvault/certificates/certificate_client_models.hpp"
#include "private/certificate_constants.hpp"
#include "private/certificate_serializers.hpp"
#include <thread>

using namespace Azure::Security::KeyVault::Certificates;

Azure::Response<CertificateOperationProperties> CreateCertificateOperation::PollUntilDoneInternal(
    std::chrono::milliseconds period,
    Azure::Core::Context& context)
{
  while (true)
  {
    Poll(context);
    if (IsDone() && IsCompleted())
    {
      break;
    }
    std::this_thread::sleep_for(period);
  }

  return Azure::Response<CertificateOperationProperties>(
      m_value, std::make_unique<Azure::Core::Http::RawResponse>(*m_rawResponse));
}

std::unique_ptr<Azure::Core::Http::RawResponse> CreateCertificateOperation::PollInternal(
    Azure::Core::Context const& context)
{
  std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse;

  try
  {
    rawResponse = m_certificateClient->GetPendingCertificateOperation(m_continuationToken, context)
                      .RawResponse;
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
    m_value = _detail::CertificateOperationSerializer::Deserialize(*rawResponse);
  }

  return rawResponse;
}

CreateCertificateOperation::CreateCertificateOperation(
    std::shared_ptr<CertificateClient> certificateClient,
    Azure::Response<CertificateOperationProperties> response)
    : m_certificateClient(certificateClient)
{
  m_value = response.Value;
  m_rawResponse = std::move(response.RawResponse);
  m_continuationToken = m_value.Name;

  if (!m_value.Name.empty())
  {
    m_status = Azure::Core::OperationStatus::Succeeded;
  }
}

CreateCertificateOperation::CreateCertificateOperation(
    std::string resumeToken,
    std::shared_ptr<CertificateClient> certificateClient)
    : m_certificateClient(certificateClient), m_continuationToken(std::move(resumeToken))
{
}

CreateCertificateOperation CreateCertificateOperation::CreateFromResumeToken(
    std::string const& resumeToken,
    CertificateClient const& client,
    Azure::Core::Context const& context)
{
  CreateCertificateOperation operation(resumeToken, std::make_shared<CertificateClient>(client));
  operation.Poll(context);
  return operation;
}

void CreateCertificateOperation::Cancel(Azure::Core::Context const& context)
{
  auto response
      = m_certificateClient->CancelPendingCertificateOperation(m_continuationToken, context);
  m_value = response.Value;
}
void CreateCertificateOperation::Delete(Azure::Core::Context const& context)
{
  auto response
      = m_certificateClient->DeletePendingCertificateOperation(m_continuationToken, context);
  m_value = response.Value;
}

bool CreateCertificateOperation::IsCompleted() const
{
  bool completed = false;

  if (m_value.Status
      && (m_value.Status.Value() == _detail::CompletedValue
          || m_value.Status.Value() == _detail::DeletedValue))
  {
    completed = true;
  }
  if (m_value.Error.HasValue())
  {
    completed = true;
  }

  return completed;
}

Azure::Response<DeletedCertificate> DeleteCertificateOperation::PollUntilDoneInternal(
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

  return Azure::Response<DeletedCertificate>(
      m_value, std::make_unique<Azure::Core::Http::RawResponse>(*m_rawResponse));
}

std::unique_ptr<Azure::Core::Http::RawResponse> DeleteCertificateOperation::PollInternal(
    Azure::Core::Context const& context)
{
  std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse;

  try
  {
    rawResponse
        = m_certificateClient->GetDeletedCertificate(m_continuationToken, context).RawResponse;
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
    m_value = _detail::DeletedCertificateSerializer::Deserialize(m_value.Name(), *rawResponse);
  }
  return rawResponse;
}

DeleteCertificateOperation::DeleteCertificateOperation(
    std::shared_ptr<CertificateClient> certificateClient,
    Azure::Response<DeletedCertificate> response)
    : m_certificateClient(certificateClient)
{
  m_value = response.Value;
  m_rawResponse = std::move(response.RawResponse);
  m_continuationToken = m_value.Name();

  if (!m_value.Name().empty())
  {
    m_status = Azure::Core::OperationStatus::Succeeded;
  }
}

DeleteCertificateOperation::DeleteCertificateOperation(
    std::string resumeToken,
    std::shared_ptr<CertificateClient> certificateClient)
    : m_certificateClient(certificateClient), m_continuationToken(std::move(resumeToken))
{
}

DeleteCertificateOperation DeleteCertificateOperation::CreateFromResumeToken(
    std::string const& resumeToken,
    CertificateClient const& client,
    Azure::Core::Context const& context)
{
  DeleteCertificateOperation operation(resumeToken, std::make_shared<CertificateClient>(client));
  operation.Poll(context);
  return operation;
}

Azure::Response<KeyVaultCertificateWithPolicy> RecoverDeletedCertificateOperation::
    PollUntilDoneInternal(std::chrono::milliseconds period, Azure::Core::Context& context)
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

  return Azure::Response<KeyVaultCertificateWithPolicy>(
      m_value, std::make_unique<Azure::Core::Http::RawResponse>(*m_rawResponse));
}

std::unique_ptr<Azure::Core::Http::RawResponse> RecoverDeletedCertificateOperation::PollInternal(
    Azure::Core::Context const& context)
{
  std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse;

  try
  {
    rawResponse = m_certificateClient->GetCertificate(m_continuationToken, context).RawResponse;
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
    m_value = _detail::KeyVaultCertificateSerializer::Deserialize(m_value.Name(), *rawResponse);
  }
  return rawResponse;
}

RecoverDeletedCertificateOperation::RecoverDeletedCertificateOperation(
    std::shared_ptr<CertificateClient> certificateClient,
    Azure::Response<KeyVaultCertificateWithPolicy> response)
    : m_certificateClient(certificateClient)
{
  m_value = response.Value;
  m_rawResponse = std::move(response.RawResponse);
  m_continuationToken = m_value.Name();

  if (!m_value.Name().empty())
  {
    m_status = Azure::Core::OperationStatus::Succeeded;
  }
}

RecoverDeletedCertificateOperation::RecoverDeletedCertificateOperation(
    std::string resumeToken,
    std::shared_ptr<CertificateClient> certificateClient)
    : m_certificateClient(certificateClient), m_continuationToken(std::move(resumeToken))
{
}

RecoverDeletedCertificateOperation RecoverDeletedCertificateOperation::CreateFromResumeToken(
    std::string const& resumeToken,
    CertificateClient const& client,
    Azure::Core::Context const& context)
{
  RecoverDeletedCertificateOperation operation(
      resumeToken, std::make_shared<CertificateClient>(client));
  operation.Poll(context);
  return operation;
}
