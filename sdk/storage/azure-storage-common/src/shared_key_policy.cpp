// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/shared_key_policy.hpp"

#include "azure/core/http/http.hpp"
#include "azure/core/strings.hpp"
#include "azure/storage/common/crypt.hpp"

#include <algorithm>
#include <cctype>

namespace Azure { namespace Storage {
  std::string SharedKeyPolicy::GetSignature(const Core::Http::Request& request) const
  {
    std::string string_to_sign;
    string_to_sign += Azure::Core::Http::HttpMethodToString(request.GetMethod()) + "\n";

    const auto& headers = request.GetHeaders();
    for (std::string headerName :
         {"Content-Encoding",
          "Content-Language",
          "Content-Length",
          "Content-MD5",
          "Content-Type",
          "Date",
          "If-Modified-Since",
          "If-Match",
          "If-None-Match",
          "If-Unmodified-Since",
          "Range"})
    {
      auto ite = headers.find(Azure::Core::Strings::ToLower(headerName));
      if (ite != headers.end())
      {
        if (headerName == "Content-Length" && ite->second == "0")
        {
          // do nothing
        }
        else
        {
          string_to_sign += ite->second;
        }
      }
      string_to_sign += "\n";
    }

    // canonicalized headers
    const std::string prefix = "x-ms-";
    std::vector<std::pair<std::string, std::string>> ordered_kv;
    for (auto ite = headers.lower_bound(prefix);
         ite != headers.end() && ite->first.substr(0, prefix.length()) == prefix;
         ++ite)
    {
      std::string key = Azure::Core::Strings::ToLower(ite->first);
      ordered_kv.emplace_back(std::make_pair(std::move(key), ite->second));
    }
    std::sort(ordered_kv.begin(), ordered_kv.end());
    for (const auto& p : ordered_kv)
    {
      string_to_sign += p.first + ":" + p.second + "\n";
    }
    ordered_kv.clear();

    // canonicalized resource
    string_to_sign += "/" + m_credential->AccountName + "/" + request.GetUrl().GetPath() + "\n";
    for (const auto& query : request.GetUrl().GetQueryParameters())
    {
      std::string key = Azure::Core::Strings::ToLower(query.first);
      ordered_kv.emplace_back(std::make_pair(
          Azure::Core::Http::Url::Decode(key), Azure::Core::Http::Url::Decode(query.second)));
    }
    std::sort(ordered_kv.begin(), ordered_kv.end());
    for (const auto& p : ordered_kv)
    {
      string_to_sign += p.first + ":" + p.second + "\n";
    }

    // remove last linebreak
    string_to_sign.pop_back();

    return Base64Encode(
        Details::HmacSha256(string_to_sign, Base64Decode(m_credential->GetAccountKey())));
  }
}} // namespace Azure::Storage
