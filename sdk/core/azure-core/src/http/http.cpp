// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/http.hpp"
#include "azure/core/http/policies/policy.hpp"
#include "azure/core/internal/io/null_body_stream.hpp"
#include "azure/core/url.hpp"

#include <algorithm>
#include <unordered_set>
#include <utility>

using namespace Azure::Core;
using namespace Azure::Core::Http;
using namespace Azure::Core::IO::_internal;

char const Azure::Core::Http::_internal::HttpShared::ContentType[] = "content-type";
char const Azure::Core::Http::_internal::HttpShared::ApplicationJson[] = "application/json";
char const Azure::Core::Http::_internal::HttpShared::Accept[] = "accept";
char const Azure::Core::Http::_internal::HttpShared::MsRequestId[] = "x-ms-request-id";
char const Azure::Core::Http::_internal::HttpShared::MsClientRequestId[] = "x-ms-client-request-id";

const HttpMethod HttpMethod::Get("GET");
const HttpMethod HttpMethod::Head("HEAD");
const HttpMethod HttpMethod::Post("POST");
const HttpMethod HttpMethod::Put("PUT");
const HttpMethod HttpMethod::Delete("DELETE");
const HttpMethod HttpMethod::Patch("PATCH");

namespace {
std::unordered_set<char> const ValidHeaderNameChars = {
    ' ', '!', '#', '$', '%', '&', '\'', '*', '+', '-', '.', '0', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', 'a', 'b', 'c', 'd',  'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v',  'w', 'x', 'y', 'z', '^', '_', '`', '|', '~',
};
}

void Azure::Core::Http::_detail::RawResponseHelpers::InsertHeaderWithValidation(
    Azure::Core::CaseInsensitiveMap& headers,
    std::string const& headerName,
    std::string const& headerValue)
{
  // Check all chars in name are valid
  if (std::find_if(
          headerName.begin(),
          headerName.end(),
          [](auto c) { return ValidHeaderNameChars.find(c) == ValidHeaderNameChars.end(); })
      != headerName.end())
  {
    throw std::invalid_argument("Invalid header name: " + headerName);
  }

  // insert (override if duplicated)
  headers[headerName] = headerValue;
}

Request::Request(HttpMethod httpMethod, Url url, bool shouldBufferResponse)
    : Request(httpMethod, std::move(url), NullBodyStream::GetNullBodyStream(), shouldBufferResponse)
{
}

Request::Request(HttpMethod httpMethod, Url url)
    : Request(httpMethod, std::move(url), NullBodyStream::GetNullBodyStream(), true)
{
}
