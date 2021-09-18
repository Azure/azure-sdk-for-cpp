// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/keyvault_certificates_common_request.hpp"

#include <azure/core/exception.hpp>
#include <azure/core/http/http.hpp>

#include <memory>

using namespace Azure::Security::KeyVault;
using namespace Azure::Core::Http::_internal;

std::unique_ptr<Azure::Core::Http::RawResponse>
_detail::KeyVaultCertificatesCommonRequest::SendRequest(
    Azure::Core::Http::_internal::HttpPipeline const& pipeline,
    Azure::Core::Http::Request& request,
    Azure::Core::Context const& context)
{
  request.SetHeader("content-type", "application/json");
  auto response = pipeline.Send(request, context);
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

Azure::Core::Http::Request _detail::KeyVaultCertificatesCommonRequest::CreateRequest(
    Azure::Core::Url url,
    std::string const& apiVersion,
    Azure::Core::Http::HttpMethod method,
    std::vector<std::string> const& path,
    Azure::Core::IO::BodyStream* content)
{
  using namespace Azure::Core::Http;
  Request request = content == nullptr ? Request(method, url) : Request(method, url, content);

  request.GetUrl().AppendQueryParameter("api-version", apiVersion);

  for (std::string const& p : path)
  {
    if (!p.empty())
    {
      request.GetUrl().AppendPath(p);
    }
  }
  return request;
}
