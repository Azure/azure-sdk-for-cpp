// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Test speed of libcurl uploading 8Mb with easy_send() and without it.
 *
 */
#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#endif // Windows

#include <cstring>
#include <iostream>
#include <string>
#include <algorithm>

#include <http/body_stream.hpp>
#include <http/curl/curl.hpp>
#include <http/http.hpp>
#include <http/pipeline.hpp>
#include <http/policy.hpp>

#include <chrono>

#define UPLOAD_SIZE 8 * 1024 * 1024
#define CONTENT_LENGTH "Content-Length: "
#define CYCLE_COUNT 5

int main()
{
  std::cout << "Size: " << UPLOAD_SIZE << ". Will run " << CYCLE_COUNT << " Times." << std::endl
            << std::flush;

  uint8_t* buffer = new uint8_t[UPLOAD_SIZE];
  auto memStream = Azure::Core::Http::MemoryBodyStream(buffer, UPLOAD_SIZE);
  std::shared_ptr<Azure::Core::Http::HttpTransport> transport
      = std::make_unique<Azure::Core::Http::CurlTransport>();
  std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
  // Add the transport policy
  policies.push_back(std::make_unique<Azure::Core::Http::TransportPolicy>(std::move(transport)));
  auto httpPipeline = Azure::Core::Http::HttpPipeline(policies);

  std::string url = "https://httpbin.org/put";
  auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, &memStream);
  request.AddHeader("x-ms-version", "2019-02-02");
  request.AddHeader("Content-Length", std::to_string(UPLOAD_SIZE));
  request.SetUploadChunkSize(UPLOAD_SIZE);

  int64_t accumulator = 0;
  for (int64_t i = 1; i <= CYCLE_COUNT; i++)
  {
    memStream.Rewind();
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    auto rawResponse = httpPipeline.Send(Azure::Core::GetApplicationContext(), request);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    auto statusCode = rawResponse->GetStatusCode();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

    std::cout
        << "Time difference (" << i << ") = " << time << "[ms]. Status code: "
        << static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
               statusCode)
        << std::endl;

    if (statusCode == Azure::Core::Http::HttpStatusCode::Ok)
    {
      accumulator += time;
    }
  }
  std::cout << std::endl << "Average: " << accumulator / CYCLE_COUNT << std::endl;

  delete[] buffer;
}
