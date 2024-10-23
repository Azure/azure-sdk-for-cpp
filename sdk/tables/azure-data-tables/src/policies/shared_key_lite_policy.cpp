// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/data/tables/internal/policies/shared_key_lite_policy.hpp"

#include "../private/hmacsha256.hpp"

#include <azure/core/base64.hpp>
#include <azure/core/cryptography/hash.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/internal/strings.hpp>

#include <algorithm>

namespace Azure { namespace Data { namespace Tables { namespace _detail { namespace Policies {

  std::string SharedKeyLitePolicy::GetSignature(const Core::Http::Request& request) const
  {
    std::string string_to_sign;

    const auto& headers = request.GetHeaders();

    // canonical date header headers
    const std::string dateHeader = headers.at("x-ms-date");
    string_to_sign += dateHeader + "\n";

    // If the request URI addresses a component of the resource, append the appropriate query
    // string. The query string should include the question mark and the comp parameter (for
    // example, ?comp=metadata). No other parameters should be included on the query string.
    // https://docs.microsoft.com/en-us/rest/api/storageservices/authorize-with-shared-key#shared-key-lite-and-table-service-format-for-2009-09-19-and-later
    // canonicalized resource
    string_to_sign += "/" + m_credential->AccountName + "/" + request.GetUrl().GetPath();
    auto queryParameters = request.GetUrl().GetQueryParameters();
    if (queryParameters.count("comp") > 0)
    {
      const std::string keyValue = Azure::Core::Url::Encode("comp");
      auto compValue = queryParameters.at(keyValue);
      string_to_sign += "?comp=" + Azure::Core::Url::Decode(compValue);
    }

    return Azure::Core::Convert::Base64Encode(
        Azure::Data::Tables::_detail::Cryptography::HmacSha256::Compute(
            std::vector<uint8_t>(string_to_sign.begin(), string_to_sign.end()),
            Azure::Core::Convert::Base64Decode(m_credential->GetAccountKey())));
  }
}}}}} // namespace Azure::Data::Tables::_detail::Policies
