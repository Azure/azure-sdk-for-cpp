// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/recover_deleted_key_operation.hpp"
#include "azure/keyvault/keys/details/key_constants.hpp"
#include "azure/keyvault/keys/details/key_serializers.hpp"

using namespace Azure::Security::KeyVault::Keys;

namespace {

// For delete key, the LRO ends when we can retreive the Key from the deleted keys list from the
// server.
inline Azure::Core::OperationStatus CheckCompleted(Azure::Core::Http::RawResponse const& response)
{
  auto const code = response.GetStatusCode();
  switch (code)
  {
    case Azure::Core::Http::HttpStatusCode::Ok:
    // Access denied but proof the key was deleted.
    case Azure::Core::Http::HttpStatusCode::Forbidden:
      return Azure::Core::OperationStatus::Succeeded;
    case Azure::Core::Http::HttpStatusCode::NotFound:
      return Azure::Core::OperationStatus::Running;
    default:
      throw Azure::Security::KeyVault::Common::KeyVaultException::CreateFromResponse(response);
  }
}
} // namespace

std::unique_ptr<Azure::Core::Http::RawResponse>
Azure::Security::KeyVault::Keys::RecoverDeletedKeyOperation::PollInternal(
    Azure::Core::Context& context)
{
  std::unique_ptr<Azure::Core::Http::RawResponse> rawResponse;
  if (!IsDone())
  {
    rawResponse = m_pipeline->Send(
        context,
        Azure::Core::Http::HttpMethod::Get,
        {_detail::KeysPath, m_value.Name(), m_value.Properties.Version});
    m_status = CheckCompleted(*rawResponse);
    if (m_status == Azure::Core::OperationStatus::Succeeded)
    {
      m_value
          = _detail::KeyVaultKeySerializer::KeyVaultKeyDeserialize(m_value.Name(), *rawResponse);
    }
  }

  // To ensure the success of calling Poll multiple times, even after operation is completed, a
  // copy of the raw http response is returned instead of transfering the ownership of the raw
  // response inside the Operation.
  return rawResponse;
}

Azure::Security::KeyVault::Keys::RecoverDeletedKeyOperation::RecoverDeletedKeyOperation(
    std::shared_ptr<Azure::Security::KeyVault::Common::_internal::KeyVaultPipeline>
        keyvaultPipeline,
    Azure::Response<Azure::Security::KeyVault::Keys::KeyVaultKey> response)
    : m_pipeline(keyvaultPipeline)
{
  // The response becomes useless and the value and rawResponse are now owned by the
  // DeleteKeyOperation. This is fine because the DeleteKeyOperation is what the delete key api
  // will return.
  m_value = response.Value;
  m_rawResponse = std::move(response.RawResponse);

  // Build the full url for continuation token. It is only used in case customers wants to use
  // it on their own. The Operation uses the KeyVaultPipeline from the client which knows how to
  // build this url.
  m_continuationToken = m_pipeline->GetVaultUrl() + "/" + std::string(_detail::DeletedKeysPath)
      + "/" + m_value.Name();
}
