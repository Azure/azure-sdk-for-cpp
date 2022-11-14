//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/secrets/keyvault_deleted_secret.hpp"
#include "azure/keyvault/secrets/keyvault_secret.hpp"
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <gtest/gtest.h>
#include <string>

using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Core::Http::_internal;

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets { namespace _test {
  struct BackupHelpers
  {
    static Azure::Core::Http::RawResponse GetEmptyResponse()
    {
      auto response
          = Azure::Core::Http::RawResponse(1, 1, Azure::Core::Http::HttpStatusCode::Ok, "OK");

      constexpr static const uint8_t responseBody[] = R"json({
        "value": ""
    }
)json";

      response.SetHeader(HttpShared::ContentType, "application/json");
      response.SetHeader(HttpShared::MsRequestId, "1");
      response.SetHeader(HttpShared::MsClientRequestId, "2");
      response.SetBody(std::vector<uint8_t>(responseBody, responseBody + sizeof(responseBody)));
      response.SetBodyStream(std::make_unique<Azure::Core::IO::MemoryBodyStream>(
          responseBody, sizeof(responseBody) - 1));

      return response;
    }

    static Azure::Core::Http::RawResponse GetFullResponse()
    {
      auto response
          = Azure::Core::Http::RawResponse(1, 1, Azure::Core::Http::HttpStatusCode::Ok, "OK");

      // cspell: disable-next-line
      constexpr static const uint8_t responseBody[] = R"json({"value": "bXkgbmFtZSBpcw=="})json";

      response.SetHeader(HttpShared::ContentType, "application/json");
      response.SetHeader(HttpShared::MsRequestId, "1");
      response.SetHeader(HttpShared::MsClientRequestId, "2");
      response.SetBody(std::vector<uint8_t>(responseBody, responseBody + sizeof(responseBody)));
      response.SetBodyStream(std::make_unique<Azure::Core::IO::MemoryBodyStream>(
          responseBody, sizeof(responseBody) - 1));

      return response;
    }

    static Azure::Core::Http::RawResponse GetIncorrectResponse()
    {
      auto response
          = Azure::Core::Http::RawResponse(1, 1, Azure::Core::Http::HttpStatusCode::Ok, "OK");

      constexpr static const uint8_t responseBody[] = R"json({
        "value": "my name is"
      }
)json";

      response.SetHeader(HttpShared::ContentType, "application/json");
      response.SetHeader(HttpShared::MsRequestId, "1");
      response.SetHeader(HttpShared::MsClientRequestId, "2");
      response.SetBody(std::vector<uint8_t>(responseBody, responseBody + sizeof(responseBody)));
      response.SetBodyStream(std::make_unique<Azure::Core::IO::MemoryBodyStream>(
          responseBody, sizeof(responseBody) - 1));

      return response;
    }
  };
}}}}} // namespace Azure::Security::KeyVault::Secrets::_test
