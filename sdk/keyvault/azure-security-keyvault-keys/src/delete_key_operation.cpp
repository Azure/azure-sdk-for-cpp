// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/keys/delete_key_operation.hpp"

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
