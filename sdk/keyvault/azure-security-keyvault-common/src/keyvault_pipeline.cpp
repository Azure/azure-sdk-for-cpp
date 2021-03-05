// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/common/internal/keyvault_pipeline.hpp"
#include "azure/keyvault/common/keyvault_constants.hpp"
#include "azure/keyvault/common/keyvault_exception.hpp"

using namespace Azure::Security::KeyVault::Common;

namespace {
inline Azure::Core::Http::Request InitRequest(
    Azure::Core::Http::HttpMethod method,
    Azure::IO::BodyStream* content,
    Azure::Core::Http::Url const& url)
{
  if (content == nullptr)
  {
    return Azure::Core::Http::Request(method, url);
  }
  return Azure::Core::Http::Request(method, url, content);
}
} // namespace

Azure::Core::Http::Request Internal::KeyVaultPipeline::CreateRequest(
    Azure::Core::Http::HttpMethod method,
    Azure::IO::BodyStream* content,
    std::vector<std::string> const& path) const
{

  auto request = ::InitRequest(method, content, m_vaultUrl);

  request.AddHeader(Details::ContentType, Details::ApplicationJson);
  request.AddHeader(Details::Accept, Details::ApplicationJson);

  request.GetUrl().AppendQueryParameter(Details::ApiVersion, m_apiVersion);

  for (std::string const& p : path)
  {
    if (!p.empty())
    {
      request.GetUrl().AppendPath(p);
    }
  }
  return request;
}

Azure::Core::Http::Request Internal::KeyVaultPipeline::CreateRequest(
    Azure::Core::Http::HttpMethod method,
    std::vector<std::string> const& path) const
{
  return CreateRequest(method, nullptr, path);
}

std::unique_ptr<Azure::Core::Http::RawResponse> Internal::KeyVaultPipeline::SendRequest(
    Azure::Core::Context const& context,
    Azure::Core::Http::Request& request) const
{
  auto response = m_pipeline.Send(request, context);
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
