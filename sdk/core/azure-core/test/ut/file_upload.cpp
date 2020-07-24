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

#include "transport_adapter.hpp"
#include <response.hpp>
#include <string>

namespace Azure { namespace Core { namespace Test {
  namespace Datails {
    constexpr int64_t c_fileSize = 1024 * 100;
  }

  TEST_F(TransportAdapter, customSizePutFromFile)
  {
    std::string host("http://httpbin.org/put");
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
    auto request
        = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, host, &requestBodyStream);
    // Make transport adapter to read all stream content for uploading instead of chunks
    request.SetUploadChunkSize(Azure::Core::Test::Datails::c_fileSize);
    {
      auto response = pipeline.Send(context, request);
      EXPECT_TRUE(response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));

      auto body = response->GetBodyStream();
      CheckBodyStreamLength(*body, expectedResponseBodySize);
    }
  }

  TEST_F(TransportAdapter, customSizePutFromFileDefault)
  {
    std::string host("http://httpbin.org/put");
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
    auto request
        = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, host, &requestBodyStream);
    // Make transport adapter to read default chunk size
    {
      auto response = pipeline.Send(context, request);
      EXPECT_TRUE(response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));

      auto body = response->GetBodyStream();
      CheckBodyStreamLength(*body, expectedResponseBodySize);
    }
  }

  TEST_F(TransportAdapter, customSizePutFromFileBiggerPage)
  {
    std::string host("http://httpbin.org/put");
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
    auto request
        = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, host, &requestBodyStream);
    // Make transport adapter to read more than file size (5Mb)
    request.SetUploadChunkSize(Azure::Core::Test::Datails::c_fileSize * 5);
    {
      auto response = pipeline.Send(context, request);
      EXPECT_TRUE(response->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));

      auto body = response->GetBodyStream();
      CheckBodyStreamLength(*body, expectedResponseBodySize);
    }
  }

}}} // namespace Azure::Core::Test
