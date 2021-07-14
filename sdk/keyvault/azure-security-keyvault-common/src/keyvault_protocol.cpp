// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/common/internal/keyvault_protocol.hpp"
#include "private/keyvault_constants.hpp"

#include <azure/core/exception.hpp>
#include <azure/core/http/http.hpp>

using namespace Azure::Security::KeyVault;
using namespace Azure::Core::Http::_internal;

Azure::Core::Http::Request _internal::KeyVaultProtocolClient::CreateRequest(
    Azure::Core::Http::HttpMethod method,
    Azure::Core::IO::BodyStream* content,
    std::vector<std::string> const& path) const
{
  Azure::Core::Http::Request request = content == nullptr
      ? Azure::Core::Http::Request(method, m_vaultUrl)
      : Azure::Core::Http::Request(method, m_vaultUrl, content);

  request.SetHeader(HttpShared::ContentType, HttpShared::ApplicationJson);
  request.SetHeader(HttpShared::Accept, HttpShared::ApplicationJson);

  request.GetUrl().AppendQueryParameter(_detail::ApiVersion, m_apiVersion);

  for (std::string const& p : path)
  {
    if (!p.empty())
    {
      request.GetUrl().AppendPath(p);
    }
  }
  return request;
}

Azure::Core::Http::Request _internal::KeyVaultProtocolClient::CreateRequest(
    Azure::Core::Http::HttpMethod method,
    std::vector<std::string> const& path) const
{
  return CreateRequest(method, nullptr, path);
}

std::unique_ptr<Azure::Core::Http::RawResponse> _internal::KeyVaultProtocolClient::SendRequest(
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
      throw Azure::Core::RequestFailedException(response);
  }
  return response;
}
