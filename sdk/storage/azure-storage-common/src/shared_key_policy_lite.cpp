// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/common/internal/shared_key_policy_lite.hpp"

#include "azure/storage/common/crypt.hpp"

#include <azure/core/http/http.hpp>
#include <azure/core/internal/strings.hpp>

#include <algorithm>

namespace Azure { namespace Storage { namespace _internal {

  std::string SharedKeyPolicyLite::GetSignature(const Core::Http::Request& request) const
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
      string_to_sign += "?comp=" + Azure::Core::Url::Decode(compValue) ;
    }
    string_to_sign += "\n";
    // remove last linebreak
    string_to_sign.pop_back();

    return Azure::Core::Convert::Base64Encode(_internal::HmacSha256(
        std::vector<uint8_t>(string_to_sign.begin(), string_to_sign.end()),
        Azure::Core::Convert::Base64Decode(m_credential->GetAccountKey())));
  }
}}} // namespace Azure::Storage::_internal
