// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include "azure/keyvault/certificates/certificate_client_operations.hpp"
#include "azure/keyvault/certificates/certificate_client.hpp"
#include "azure/keyvault/certificates/certificate_client_models.hpp"
#include "private/certificate_constants.hpp"
#include "private/certificate_serializers.hpp"
#include <thread>

using namespace Azure::Security::KeyVault::Certificates;

Azure::Response<KeyVaultCertificate> CreateCertificateOperation::PollUntilDoneInternal(
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

  return Azure::Response<KeyVaultCertificate>(
      m_value, std::make_unique<Azure::Core::Http::RawResponse>(*m_rawResponse));
}

std::unique_ptr<Azure::Core::Http::RawResponse> CreateCertificateOperation::PollInternal(
    Azure::Core::Context const& context)
{
  std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse;

  try
  {
    rawResponse = m_certificateClient->GetCertificateOperation(Properties.Name, context)
                      .RawResponse;
  }
  catch (Azure::Core::RequestFailedException& error)
  {
    rawResponse = std::move(error.RawResponse);
  }

  switch (rawResponse->GetStatusCode())
  {
    case Azure::Core::Http::HttpStatusCode::Ok: {
      Properties = _detail::CertificateOperationSerializer::Deserialize(*rawResponse);
      // the operation returns completed for crete certificate, thus success is success when the
      // operation returns completed not on operation query success
      if (Properties.Status.HasValue())
      {
        m_status = Properties.Status.Value() == _detail::CompletedValue
            ? Azure::Core::OperationStatus::Succeeded
            : Azure::Core::OperationStatus::Running;
      }
      else
      {
        // no status code, we're in no mans land , assume failed
        m_status = Azure::Core::OperationStatus::Failed;
      }

      break;
    }
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
    auto finalReponse = m_certificateClient->GetCertificate(Properties.Name);
    m_value = finalReponse.Value;
  }

  return rawResponse;
}

CreateCertificateOperation::CreateCertificateOperation(
    std::shared_ptr<CertificateClient> certificateClient,
    Azure::Response<CertificateOperationProperties> response)
    : m_certificateClient(certificateClient)
{
  Properties = response.Value;
  m_rawResponse = std::move(response.RawResponse);
  m_continuationToken = m_value.Name();

  if (!m_value.Name().empty())
  {
    m_status = Azure::Core::OperationStatus::Succeeded;
  }
}

CreateCertificateOperation::CreateCertificateOperation(
    std::string resumeToken,
    std::shared_ptr<CertificateClient> certificateClient)
    : m_certificateClient(certificateClient), m_continuationToken(std::move(resumeToken))
{
  m_value.Properties.Name = resumeToken;
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
