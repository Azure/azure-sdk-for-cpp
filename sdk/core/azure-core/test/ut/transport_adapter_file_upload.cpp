// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifdef POSIX
#include <fcntl.h>
#endif // Posix

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif // Windows

#include "azure/core/http/http.hpp"

#include "transport_adapter.hpp"

#include <string>

namespace Azure { namespace Core { namespace Test {
  namespace Datails {
    constexpr int64_t c_fileSize = 1024 * 100;
  }

  void TransportAdapter::checkResponseCode(
      Azure::Core::Http::HttpStatusCode code,
      Azure::Core::Http::HttpStatusCode expectedCode)
  {
    EXPECT_TRUE(code == expectedCode);
  }

  void TransportAdapter::CheckBodyFromBuffer(
      Azure::Core::Http::RawResponse& response,
      int64_t size,
      std::string expectedBody)
  {
    auto body = response.GetBodyStream();
    EXPECT_EQ(body, nullptr);
    std::vector<uint8_t> bodyVector = response.GetBody();
    int64_t bodySize = bodyVector.size();

    if (size > 0)
    { // only for known body size
      EXPECT_EQ(bodySize, size);
    }

    if (expectedBody.size() > 0)
    {
      auto bodyString = std::string(bodyVector.begin(), bodyVector.end());
      EXPECT_STREQ(expectedBody.data(), bodyString.data());
    }
  }

  void TransportAdapter::CheckBodyFromStream(
      Azure::Core::Http::RawResponse& response,
      int64_t size,
      std::string expectedBody)
  {
    auto body = response.GetBodyStream();
    EXPECT_NE(body, nullptr);

    std::vector<uint8_t> bodyVector = Azure::Core::Http::BodyStream::ReadToEnd(context, *body);
    int64_t bodySize = body->Length();
    EXPECT_EQ(bodySize, size);
    bodySize = bodyVector.size();

    if (size > 0)
    { // only for known body size
      EXPECT_EQ(bodyVector.size(), size);
    }

    if (expectedBody.size() > 0)
    {
      auto bodyString = std::string(bodyVector.begin(), bodyVector.end());
      EXPECT_STREQ(expectedBody.data(), bodyString.data());
    }
  }

  TEST_F(TransportAdapter, SizePutFromFile)
  {
    Azure::Core::Http::Url host("http://httpbin.org/put");
    std::string testDataPath(AZURE_TEST_DATA_PATH);

#ifdef POSIX
    testDataPath.append("/fileData");
    int f = open(testDataPath.data(), O_RDONLY);
#endif
#ifdef WINDOWS
    testDataPath.append("\\fileData");
    HANDLE f = CreateFile(
        testDataPath.data(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN,
        NULL);
#endif
    auto requestBodyStream
        = Azure::Core::Http::FileBodyStream(f, 0, Azure::Core::Test::Datails::c_fileSize);
    auto request = Azure::Core::Http::Request(
        Azure::Core::Http::HttpMethod::Put, host, &requestBodyStream, true);
    // Make transport adapter to read all stream content for uploading instead of chunks
    request.SetUploadChunkSize(Azure::Core::Test::Datails::c_fileSize);
    {
      auto response = pipeline.Send(context, request);
      checkResponseCode(response->GetStatusCode());
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));

      CheckBodyFromStream(*response, expectedResponseBodySize);
    }
  }

  TEST_F(TransportAdapter, SizePutFromFileDefault)
  {
    Azure::Core::Http::Url host("http://httpbin.org/put");
    std::string testDataPath(AZURE_TEST_DATA_PATH);

#ifdef POSIX
    testDataPath.append("/fileData");
    int f = open(testDataPath.data(), O_RDONLY);
#endif
#ifdef WINDOWS
    testDataPath.append("\\fileData");
    HANDLE f = CreateFile(
        testDataPath.data(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN,
        NULL);
#endif
    auto requestBodyStream
        = Azure::Core::Http::FileBodyStream(f, 0, Azure::Core::Test::Datails::c_fileSize);
    auto request = Azure::Core::Http::Request(
        Azure::Core::Http::HttpMethod::Put, host, &requestBodyStream, true);
    // Make transport adapter to read default chunk size
    {
      auto response = pipeline.Send(context, request);
      checkResponseCode(response->GetStatusCode());
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));

      CheckBodyFromStream(*response, expectedResponseBodySize);
    }
  }

  TEST_F(TransportAdapter, SizePutFromFileBiggerPage)
  {
    Azure::Core::Http::Url host("http://httpbin.org/put");
    std::string testDataPath(AZURE_TEST_DATA_PATH);

#ifdef POSIX
    testDataPath.append("/fileData");
    int f = open(testDataPath.data(), O_RDONLY);
#endif
#ifdef WINDOWS
    testDataPath.append("\\fileData");
    HANDLE f = CreateFile(
        testDataPath.data(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN,
        NULL);
#endif
    auto requestBodyStream
        = Azure::Core::Http::FileBodyStream(f, 0, Azure::Core::Test::Datails::c_fileSize);
    auto request = Azure::Core::Http::Request(
        Azure::Core::Http::HttpMethod::Put, host, &requestBodyStream, true);
    // Make transport adapter to read more than file size (5Mb)
    request.SetUploadChunkSize(Azure::Core::Test::Datails::c_fileSize * 5);
    {
      auto response = pipeline.Send(context, request);
      checkResponseCode(response->GetStatusCode());
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));

      CheckBodyFromStream(*response, expectedResponseBodySize);
    }
  }

}}} // namespace Azure::Core::Test
