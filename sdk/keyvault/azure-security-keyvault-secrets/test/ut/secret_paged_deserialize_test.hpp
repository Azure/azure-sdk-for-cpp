// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/secret_constants.hpp"
#include "private/secret_serializers.hpp"

#include "../src/private/secret_serializers.hpp"
#include "azure/keyvault/secrets/secret_client.hpp"
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/internal/json/json_serializable.hpp>

#include "azure/keyvault/secrets/keyvault_secret_paged_response.hpp"
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <cstddef>
#include <gtest/gtest.h>
#include <string>

using namespace Azure::Security::KeyVault::Secrets;
using namespace Azure::Core::Http::_internal;

namespace Azure { namespace Security { namespace KeyVault { namespace Secrets { namespace _test {
  struct PagedHelpers
  {

    static Azure::Core::Http::RawResponse GetFirstResponse()
    {
      auto response
          = Azure::Core::Http::RawResponse(1, 1, Azure::Core::Http::HttpStatusCode::Ok, "OK");

      constexpr static const uint8_t responseBody[] = R"json({
	"nextLink": "https://gearama-test2.vault.azure.net:443/secrets?api-version=7.2&$skiptoken=eyJOZXh0TWFya2VyIjoiMiE4NCFNREF3TURFM0lYTmxZM0psZEM5VFQwMUZVMFZEVWtWVUlUQXdNREF5T0NFNU9UazVMVEV5TFRNeFZESXpPalU1T2pVNUxqazVPVGs1T1RsYUlRLS0iLCJUYXJnZXRMb2NhdGlvbiI6MH0&maxresults=1",
	"value": [{
		"attributes": {
			"created": 1627404049,
			"enabled": true,
			"recoverableDays": 90,
			"recoveryLevel": "Recoverable+Purgeable",
			"updated": 1627404049
		},
		"id": "https://gearama-test2.vault.azure.net/secrets/magic"
	}]
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

    static Azure::Core::Http::RawResponse GetMultipleResponse()
    {
      auto response
          = Azure::Core::Http::RawResponse(1, 1, Azure::Core::Http::HttpStatusCode::Ok, "OK");

      constexpr static const uint8_t responseBody[] = R"json({
	"nextLink": null,
	"value": [{
		"attributes": {
			"created": 1628101925,
			"enabled": true,
			"recoverableDays": 90,
			"recoveryLevel": "Recoverable+Purgeable",
			"updated": 1628101925
		},
		"contentType": "content",
		"id": "https://gearama-test2.vault.azure.net/secrets/magic/5a0fdd819481420eac6f3282ce722461",
		"tags": {}
	}, {
		"attributes": {
			"created": 1627404049,
			"enabled": true,
			"recoverableDays": 90,
			"recoveryLevel": "Recoverable+Purgeable",
			"updated": 1627404049
		},
		"id": "https://gearama-test2.vault.azure.net/secrets/magic/8faafbb99216484dbbd75f9dd6bcaadf"
	}, {
		"attributes": {
			"created": 1628101911,
			"enabled": true,
			"recoverableDays": 90,
			"recoveryLevel": "Recoverable+Purgeable",
			"updated": 1628101911
		},
		"id": "https://gearama-test2.vault.azure.net/secrets/magic/d75080822f03400ab4d658bd0e988ac5",
		"tags": {}
	}]
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

    static Azure::Core::Http::RawResponse GetEmptyResponse()
    {
      auto response
          = Azure::Core::Http::RawResponse(1, 1, Azure::Core::Http::HttpStatusCode::Ok, "OK");

      constexpr static const uint8_t responseBody[] = R"json({
	"nextLink": null,
	"value": []
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

    static Azure::Core::Http::RawResponse GetDeletedFirstResponse()
    {
      auto response
          = Azure::Core::Http::RawResponse(1, 1, Azure::Core::Http::HttpStatusCode::Ok, "OK");

      constexpr static const uint8_t responseBody[] = R"json({
	"nextLink": "nextLink",
	"value": [{
		"attributes": {
			"created": 1628110306,
			"enabled": true,
			"recoverableDays": 90,
			"recoveryLevel": "Recoverable+Purgeable",
			"updated": 1628110306
		},
		"deletedDate": 1628110318,
		"id": "https://gearama-test2.vault.azure.net/secrets/eqwewq",
		"recoveryId": "https://gearama-test2.vault.azure.net/deletedsecrets/eqwewq",
		"scheduledPurgeDate": 1635886318,
		"tags": {}
	}]
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

    static Azure::Core::Http::RawResponse GetDeletedMultipleResponse()
    {
      auto response
          = Azure::Core::Http::RawResponse(1, 1, Azure::Core::Http::HttpStatusCode::Ok, "OK");

      constexpr static const uint8_t responseBody[] = R"json({
	"nextLink": null,
	"value": [{
		"attributes": {
			"created": 1628110306,
			"enabled": true,
			"recoverableDays": 90,
			"recoveryLevel": "Recoverable+Purgeable",
			"updated": 1628110306
		},
		"deletedDate": 1628110318,
		"id": "https://gearama-test2.vault.azure.net/secrets/eqwewq",
		"recoveryId": "https://gearama-test2.vault.azure.net/deletedsecrets/eqwewq",
		"scheduledPurgeDate": 1635886318,
		"tags": {}
	}, {
		"attributes": {
			"created": 1626967532,
			"enabled": true,
			"recoverableDays": 90,
			"recoveryLevel": "Recoverable+Purgeable",
			"updated": 1626967532
		},
		"deletedDate": 1628110252,
		"id": "https://gearama-test2.vault.azure.net/secrets/someSecret",
		"recoveryId": "https://gearama-test2.vault.azure.net/secrets/someSecret",
		"scheduledPurgeDate": 1635886252
	}, {
		"attributes": {
			"created": 1627101774,
			"enabled": true,
			"recoverableDays": 90,
			"recoveryLevel": "Recoverable+Purgeable",
			"updated": 1627101774
		},
		"deletedDate": 1628110259,
		"id": "https://gearama-test2.vault.azure.net/secrets/someSecret2",
		"recoveryId": "https://gearama-test2.vault.azure.net/deletedsecrets/someSecret2",
		"scheduledPurgeDate": 1635886259
	}]
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
