// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/delete_key_operation.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"

using namespace Azure::Security::KeyVault::Keys;

namespace {

// For delete key, the LRO ends when we can retreive the Key from the deleted keys list from the
// server.
inline Azure::Core::OperationStatus CheckCompleted(Azure::Core::Http::RawResponse const& response)
{
  auto code = response.GetStatusCode();
  switch (code)
  {
    case Azure::Core::Http::HttpStatusCode::Ok:
    case Azure::Core::Http::HttpStatusCode::Forbidden: // Access denied but proof the key was
                                                       // deleted.
      return Azure::Core::OperationStatus::Succeeded;
    case Azure::Core::Http::HttpStatusCode::NotFound:
      return Azure::Core::OperationStatus::Running;
    default:
      throw Azure::Security::KeyVault::Common::KeyVaultException::CreateFromResponse(response);
  }
}
} // namespace

std::unique_ptr<Azure::Core::Http::RawResponse>
Azure::Security::KeyVault::Keys::DeleteKeyOperation::PollInternal(Azure::Core::Context& context)
{
  if (!IsDone())
  {
    m_rawResponse = m_pipeline->GetResponse(
        context, Azure::Core::Http::HttpMethod::Get, {Details::DeletedKeysPath, m_value.Name()});
    m_status = CheckCompleted(*m_rawResponse);
  }

  // To ensure the success of calling Poll multiple times, even after operation is completed, a
  // copy of the raw http response is returned instead of transfering the ownership of the raw
  // response inside the Operation.
  return std::make_unique<Azure::Core::Http::RawResponse>(*m_rawResponse);
}

Azure::Security::KeyVault::Keys::DeleteKeyOperation::DeleteKeyOperation(
    std::shared_ptr<Azure::Security::KeyVault::Common::Internal::KeyVaultPipeline> keyvaultPipeline,
    Azure::Response<Azure::Security::KeyVault::Keys::DeletedKey> response)
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
