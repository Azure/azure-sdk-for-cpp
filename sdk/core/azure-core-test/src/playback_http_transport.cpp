// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/http/http.hpp>

#include "azure/core/test/interceptor_manager.hpp"
#include "azure/core/test/playback_http_client.hpp"

#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

using namespace Azure::Core::Http;
using namespace Azure::Core::Test;

std::unique_ptr<RawResponse> PlaybackClient::Send(
    Request& request,
    Azure::Core::Context const& context)
{
  context.ThrowIfCancelled();

  // The test name can't be known before the test is started. That's why the test data is loaded up
  // to this point instead of loading it on test SetUp.
  // The test data will be loaded just one time.
  m_interceptorManager->LoadTestData();

  auto& recordedData = m_interceptorManager->GetRecordedData();
  Azure::Core::Url const redactedUrl = m_interceptorManager->RedactUrl(request.GetUrl());

  for (auto record = recordedData.NetworkCallRecords.begin();
       record != recordedData.NetworkCallRecords.end();)
  {
    auto url = redactedUrl.GetAbsoluteUrl();
    auto m = request.GetMethod().ToString();
    // Use the first occurrence and remove it from the recording.
    if (m == record->Method && url == record->Url)
    {
      // StatusCode
      auto const statusCode
          = HttpStatusCode(std::stoi(record->Response.find("STATUS_CODE")->second));
      auto response = std::make_unique<RawResponse>(1, 1, statusCode, "recorded response");

      // Headers
      for (auto const& header : record->Response)
      {
        if (header.first != "STATUS_CODE" && header.first != "BODY")
        {
          response->SetHeader(header.first, header.second);
        }
      }

      // Body
      auto body = record->Response.find("BODY")->second;
      std::vector<uint8_t> bodyVector(body.begin(), body.end());
      response->SetBodyStream(std::make_unique<WithMemoryBodyStream>(bodyVector));

      // take the record out of the recording
      record = recordedData.NetworkCallRecords.erase(record);

      return response;
    }
    ++record;
  }

  throw std::runtime_error("Did not found a response for the request in the recordings.");
}
