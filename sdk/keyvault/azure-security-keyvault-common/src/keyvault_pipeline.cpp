// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/common/keyvault_pipeline.hpp"
#include "azure/keyvault/common/keyvault_exception.hpp"

using namespace Azure::Security::KeyVault::Common;

Azure::Core::Http::Request Internal::KeyVaultPipeline::CreateRequest(
    Azure::Core::Http::HttpMethod method,
    std::vector<std::string> const& path) const
{
  Azure::Core::Http::Request request(method, m_vaultUrl);

  request.AddHeader("Content-Type", "application/json");
  request.AddHeader("Accept", "application/json");

  request.GetUrl().AppendQueryParameter("api-version", m_apiVersion);

  for (std::string const& p : path)
  {
    if (!p.empty())
    {
      request.GetUrl().AppendPath(p);
    }
  }
  return request;
}

std::unique_ptr<Azure::Core::Http::RawResponse> Internal::KeyVaultPipeline::SendRequest(
    Azure::Core::Context const& context,
    Azure::Core::Http::Request& request) const
{
  auto response = m_pipeline.Send(context, request);
  auto responseCode = response->GetStatusCode();
  switch (responseCode)
  {
    // 200, 2001, 202, 204 are accepted responses
    case Azure::Core::Http::HttpStatusCode::Ok:
    case Azure::Core::Http::HttpStatusCode::Created:
    case Azure::Core::Http::HttpStatusCode::Accepted:
    case Azure::Core::Http::HttpStatusCode::NoContent:
      break;
    default:
      throw KeyVaultException::CreateFromResponse(std::move(response));
  }
  return response;
}
