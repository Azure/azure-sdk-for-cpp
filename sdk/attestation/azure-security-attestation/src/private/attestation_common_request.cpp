// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "attestation_common_request.hpp"
#include <azure/core/exception.hpp>
#include <azure/core/http/http.hpp>
#include <memory>

using namespace Azure::Security::Attestation;
using namespace Azure::Security::Attestation::_detail;
using namespace Azure::Core::Http::_internal;

std::unique_ptr<Azure::Core::Http::RawResponse> AttestationCommonRequest::SendRequest(
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

Azure::Core::Http::Request AttestationCommonRequest::CreateRequest(
    Azure::Core::Url url,
    Azure::Core::Http::HttpMethod method,
    std::vector<std::string> const& path,
    Azure::Core::IO::BodyStream* content)
{
  using namespace Azure::Core::Http;
  return CreateRequest(url, "", method, path, content);
}

Azure::Core::Http::Request AttestationCommonRequest::CreateRequest(
    Azure::Core::Url url,
    std::string const& apiVersion,
    Azure::Core::Http::HttpMethod method,
    std::vector<std::string> const& path,
    Azure::Core::IO::BodyStream* content)
{
  using namespace Azure::Core::Http;
  Request request = content == nullptr ? Request(method, url) : Request(method, url, content);

  request.SetHeader(ContentHeaderName, ApplicationJsonValue);
  if (!apiVersion.empty())
  {
    request.GetUrl().AppendQueryParameter(ApiVersionQueryParamName, apiVersion);
  }

  for (std::string const& P : path)
  {
    if (!P.empty())
    {
      request.GetUrl().AppendPath(P);
    }
  }
  return request;
}
