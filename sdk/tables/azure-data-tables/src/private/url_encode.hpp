// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <memory>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <vector>

namespace Azure { namespace Data { namespace Tables { namespace _detail { namespace Cryptography {

  class UrlUtils final {
  public:
    static std::string UrlEncodeQueryParameter(const std::string& value)
    {
      const static std::string DoNotEncodeCharacters = []() {
        // Core::Url::Encode won't encode unreserved characters.
        std::string doNotEncodeCharacters = "!$&'()*+,;=";
        doNotEncodeCharacters += "/:@?";
        doNotEncodeCharacters.erase(
            std::remove_if(
                doNotEncodeCharacters.begin(),
                doNotEncodeCharacters.end(),
                [](char x) {
                  // we also encode + and &
                  // Surprisingly, '=' also needs to be encoded because Azure Storage server side is
                  // so strict. We are applying this function to query key and value respectively,
                  // so this won't affect that = used to separate key and query.
                  return x == '+' || x == '=' || x == '&';
                }),
            doNotEncodeCharacters.end());
        return doNotEncodeCharacters;
      }();
      return Core::Url::Encode(value, DoNotEncodeCharacters);
    }

    static std::string UrlEncodePath(const std::string& value)
    {
      const static std::string DoNotEncodeCharacters = []() {
        // Core::Url::Encode won't encode unreserved characters.
        std::string doNotEncodeCharacters = "!$&'()*+,;=";
        doNotEncodeCharacters += "/:@";
        doNotEncodeCharacters.erase(
            std::remove_if(
                doNotEncodeCharacters.begin(),
                doNotEncodeCharacters.end(),
                [](char x) {
                  // we also encode +
                  return x == '+';
                }),
            doNotEncodeCharacters.end());
        return doNotEncodeCharacters;
      }();
      return Core::Url::Encode(value, DoNotEncodeCharacters);
    }
  };
}}}}} // namespace Azure::Data::Tables::_detail::Cryptography
