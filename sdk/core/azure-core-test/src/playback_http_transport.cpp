//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/http/http.hpp>

#include "azure/core/test/interceptor_manager.hpp"
#include "azure/core/test/playback_http_client.hpp"

#include <cstdlib>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

using namespace Azure::Core::Http;
using namespace Azure::Core::Test;

/**
 * @brief Infomation about special behavior headers.
 *
 * @details This structure helps to describe how to handle the playback response when there are
 * special headers. For example, for unique id headers, the playback transport adapter can take one
 * unique id comming from the request and use it as part of the playback response.
 *
 */
struct UniqueIdInfo
{
  /**
   * @brief If this header key is found in the request, the response should override the header
   * defined by `ReplaceResponseHeader` in the response.
   *
   */
  std::string RequestHeader;

  /**
   * @brief This field can be used to condition the replacement of the response header to only when
   * the request header is equal to this value.
   *
   */
  std::string RequestHeaderOnlyIfValue;

  /**
   * @brief This is the header in the response to be replaced.
   *
   */
  std::string ReplaceResponseHeader;

  /**
   * @brief Use this field to override the response value with another header value from the
   * request. Leave it empty to use the value from `RequestHeader`.
   *
   */
  std::string ReplacedValueWithHeader;
};

/**
 * @brief Define the special headers
 *
 * @details Current rules:
 *
 * - If header x-ms-proposed-lease-id is in the request, then use its value for the header
 * x-ms-lease-id in the response.
 *
 * - If header x-ms-lease-action is in the request and its value is equals to renew, then use the
 * value from request header x-ms-lease-id for the response header value of x-ms-lease-id
 *
 */
std::vector<UniqueIdInfo> UniqueHeaders(
    {{"x-ms-proposed-lease-id", "", "x-ms-lease-id", ""},
     {"x-ms-lease-action", "renew", "x-ms-lease-id", "x-ms-lease-id"}});

std::mutex PlaybackClientMutex;

std::unique_ptr<RawResponse> PlaybackClient::Send(
    Request& request,
    Azure::Core::Context const& context)
{
  context.ThrowIfCancelled();

  {
    // This mutex obligates the playbackClient to run `Send` method from just one thread at a time.
    // PlaybackClient forces program to distach one request at a time
    // This is how playback client can support concurrency, if multiple threads uses the same
    // pipeline to perform a request, the playback client will dispatch one at a time.
    std::unique_lock<std::mutex> lock(PlaybackClientMutex);

    // The test name can't be known before the test is started. That's why the test data is loaded
    // up to this point instead of loading it on test SetUp. The test data will be loaded just one
    // time.
    m_interceptorManager->LoadTestData();

    auto& recordedData = m_interceptorManager->GetRecordedData();
    Azure::Core::Url const redactedUrl = m_interceptorManager->RedactUrl(request.GetUrl());

    std::map<std::string, std::string> uniqueIds;
    auto const& requestHeaders = request.GetHeaders();
    for (auto const& requestHeader : requestHeaders)
    {
      auto const uniqueHeaderInRequest = std::find_if(
          UniqueHeaders.begin(),
          UniqueHeaders.end(),
          [requestHeader](UniqueIdInfo const& uniqueHeaderInfo) {
            if (uniqueHeaderInfo.RequestHeaderOnlyIfValue.empty())
            {
              return requestHeader.first == uniqueHeaderInfo.RequestHeader;
            }
            else
            {
              return requestHeader.first == uniqueHeaderInfo.RequestHeader
                  && requestHeader.second == uniqueHeaderInfo.RequestHeaderOnlyIfValue;
            }
          });
      if (uniqueHeaderInRequest != UniqueHeaders.end())
      {
        // header is a uniqueHeader, save the value to use in the response.
        auto const headerForReplacing = uniqueHeaderInRequest->ReplacedValueWithHeader.empty()
            ? requestHeader.second
            : requestHeaders.at(uniqueHeaderInRequest->ReplacedValueWithHeader);

        uniqueIds.emplace(uniqueHeaderInRequest->ReplaceResponseHeader, headerForReplacing);
      }
    }

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
        auto rpIt = record->Response.find("REASON_PHRASE");
        auto rp = rpIt != record->Response.end() ? rpIt->second : "recorded response";
        auto response = std::make_unique<RawResponse>(1, 1, statusCode, rp);

        // Headers
        for (auto const& header : record->Response)
        {
          if (header.first != "STATUS_CODE" && header.first != "BODY"
              && header.first != "REASON_PHRASE")
          {
            auto const replaceWithUnique = std::find_if(
                uniqueIds.begin(),
                uniqueIds.end(),
                [header](std::pair<std::string, std::string> const& uniqueHeaderInfo) {
                  return header.first == uniqueHeaderInfo.first;
                });

            if (replaceWithUnique == uniqueIds.end())
            {
              response->SetHeader(header.first, header.second);
            }
            else
            {
              response->SetHeader(header.first, uniqueIds[header.first]);
            }
          }
        }

        // Body
        auto const body = record->Response.find("BODY")->second;
        {
          std::string const bodyStreamSentinel(RECORDING_BODY_STREAM_SENTINEL);
          auto const bodyStreamSentinelLen = bodyStreamSentinel.length();
          if (body.length() > bodyStreamSentinelLen
              && bodyStreamSentinel
                  == std::string(body.begin(), body.begin() + bodyStreamSentinelLen))
          {
            // Sentinel found. Generate bodyStream
            std::string bodyStreamSettings(body.begin() + bodyStreamSentinelLen, body.end());
            auto const separator = bodyStreamSettings.find('_');
            auto const bodyStreamSize = std::atoi(
                std::string(bodyStreamSettings.begin(), bodyStreamSettings.begin() + separator)
                    .data());
            auto const bodyStreamFillWith = std::atoi(
                std::string(bodyStreamSettings.begin() + separator + 1, bodyStreamSettings.end())
                    .data());

            response->SetBodyStream(std::make_unique<CircularBodyStream>(
                bodyStreamSize, static_cast<uint8_t>(bodyStreamFillWith)));
          }
          else
          {
            // No special sentinel. Use the entire body from recording as the response.
            std::vector<uint8_t> bodyVector(body.begin(), body.end());
            response->SetBodyStream(std::make_unique<WithMemoryBodyStream>(bodyVector));
          }
        }

        // take the record out of the recording
        record = recordedData.NetworkCallRecords.erase(record);

        return response;
      }
      ++record;
    }
  }
  throw std::runtime_error("Did not found a response for the request in the recordings.");
}
