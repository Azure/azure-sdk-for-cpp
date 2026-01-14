// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/http/http.hpp"

#include "azure/core/http/transport.hpp"
#include "azure/core/internal/strings.hpp"
#include "azure/core/url.hpp"

#include <algorithm>
#include <unordered_set>

using namespace Azure::Core;
using namespace Azure::Core::Http;

const Context::Key Http::_internal::HttpConnectionTimeout{};

char const Http::_internal::HttpShared::ContentType[] = "content-type";
char const Http::_internal::HttpShared::ApplicationJson[] = "application/json";
char const Http::_internal::HttpShared::Accept[] = "accept";
char const Http::_internal::HttpShared::MsRequestId[] = "x-ms-request-id";
char const Http::_internal::HttpShared::MsClientRequestId[] = "x-ms-client-request-id";

AZ_CORE_DLLEXPORT const HttpMethod HttpMethod::Get("GET");
AZ_CORE_DLLEXPORT const HttpMethod HttpMethod::Head("HEAD");
AZ_CORE_DLLEXPORT const HttpMethod HttpMethod::Post("POST");
AZ_CORE_DLLEXPORT const HttpMethod HttpMethod::Put("PUT");
AZ_CORE_DLLEXPORT const HttpMethod HttpMethod::Delete("DELETE");
AZ_CORE_DLLEXPORT const HttpMethod HttpMethod::Patch("PATCH");
AZ_CORE_DLLEXPORT const HttpMethod HttpMethod::Options("OPTIONS");

namespace {
bool IsInvalidHeaderNameChar(char c)
{
  static std::unordered_set<char> const HeaderNameExtraValidChars
      = {' ', '!', '#', '$', '%', '&', '\'', '*', '+', '-', '.', '^', '_', '`', '|', '~'};

  return !Azure::Core::_internal::StringExtensions::IsAlphaNumeric(c)
      && HeaderNameExtraValidChars.find(c) == HeaderNameExtraValidChars.end();
}
} // namespace

void Azure::Core::Http::_detail::RawResponseHelpers::InsertHeaderWithValidation(
    Azure::Core::CaseInsensitiveMap& headers,
    std::string const& headerName,
    std::string const& headerValue)
{

  // Check all chars in name are valid
  if (std::find_if(headerName.begin(), headerName.end(), IsInvalidHeaderNameChar)
      != headerName.end())
  {
    throw std::invalid_argument("Invalid header name: " + headerName);
  }

  // insert (override if duplicated)
  headers[headerName] = headerValue;
}
