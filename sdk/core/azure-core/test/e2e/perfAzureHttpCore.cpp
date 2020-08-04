// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Test speed of libcurl uploading 8Mb with easy_send() and without it.
 *
 */

#include <cstring>
#include <iostream>
#include <string>

#include <http/body_stream.hpp>
#include <http/curl/curl.hpp>
#include <http/http.hpp>

#define UPLOAD_SIZE 8 * 1024 * 1024
#define CONTENT_LENGTH "Content-Length: "

int main()
{
  std::cout << "Size: " << UPLOAD_SIZE << " ---- " << CLOCKS_PER_SEC;

  uint8_t* buffer = new uint8_t[UPLOAD_SIZE];
  auto memStream = Azure::Core::Http::MemoryBodyStream(buffer, UPLOAD_SIZE);
  auto transport = Azure::Core::Http::CurlTransport();

  std::string url = "https://httpbin.org/put";
  auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, url, &memStream);
  request.AddHeader("x-ms-version", "2019-02-02");
  request.AddHeader("Content-Length", std::to_string(UPLOAD_SIZE));
  request.SetUploadChunkSize(UPLOAD_SIZE);

  clock_t begin = clock();
  auto rawResponse = transport.Send(Azure::Core::GetApplicationContext(), request);
  clock_t end = clock();

  auto x = rawResponse->GetStatusCode();
  (void)x;

  std::cout << std::endl
            << static_cast<double>((end - begin) * 1000) / CLOCKS_PER_SEC << "ms" << std::endl
            << std::flush;

  delete[] buffer;
}