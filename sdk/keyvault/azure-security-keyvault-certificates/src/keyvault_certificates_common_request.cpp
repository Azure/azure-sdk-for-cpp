// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

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
  auto response = pipeline.Send(request, context);
  auto responseCode = response->GetStatusCode();

  switch (responseCode)
  {

    // 200, 201, 202, 204 are accepted responses
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
  Request request = content == nullptr ? Request(std::move(method), std::move(url))
                                       : Request(std::move(method), std::move(url), content);

  request.SetHeader(ContentHeaderName, ApplicationJsonValue);
  request.GetUrl().AppendQueryParameter(ApiVersionQueryParamName, apiVersion);

  for (std::string const& p : path)
  {
    if (!p.empty())
    {
      request.GetUrl().AppendPath(p);
    }
  }
  return request;
}
