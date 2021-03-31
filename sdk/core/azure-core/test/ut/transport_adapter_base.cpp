// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/platform.hpp>

#if defined(AZ_PLATFORM_POSIX)
#include <fcntl.h>
#elif defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "transport_adapter_base.hpp"
#include <azure/core/context.hpp>
#include <azure/core/response.hpp>
#include <iostream>
#include <string>
#include <thread>

namespace Azure { namespace Core { namespace Test {

  namespace Datails {
    constexpr int64_t FileSize = 1024 * 100;
  }

  TEST_P(TransportAdapter, get)
  {
    Azure::Core::Url host("http://httpbin.org/get");

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);
    auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
    checkResponseCode(response->GetStatusCode());
    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
    CheckBodyFromBuffer(*response, expectedResponseBodySize);

    // Need to init request again, since retry would be on after it is sent
    request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);
    // Add a header and send again. RawResponse should return that header in the body
    request.SetHeader("123", "456");
    response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
    checkResponseCode(response->GetStatusCode());
    // header length is 6 (data) + 13 (formating) -> `    "123": "456"\r\n,`
    CheckBodyFromBuffer(*response, expectedResponseBodySize + 6 + 13);
  }

  TEST_P(TransportAdapter, get204)
  {
    Azure::Core::Url host("http://mt3.google.com/generate_204");

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);
    auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
    checkResponseCode(response->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
    CheckBodyFromBuffer(*response, expectedResponseBodySize);
  }

  TEST_P(TransportAdapter, getLoop)
  {
    Azure::Core::Url host("http://httpbin.org/get");

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);

    // loop sending request
    for (auto i = 0; i < 50; i++)
    {
      auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
      checkResponseCode(response->GetStatusCode());
      CheckBodyFromBuffer(*response, expectedResponseBodySize);
    }
  }

  TEST_P(TransportAdapter, head)
  {
    Azure::Core::Url host("http://httpbin.org/get");
    auto expectedResponseBodySize = 0;

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, host);
    auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
    checkResponseCode(response->GetStatusCode());
    CheckBodyFromBuffer(*response, expectedResponseBodySize);

    // Check content-length header to be greater than 0
    int64_t contentLengthHeader = std::stoull(response->GetHeaders().at("content-length"));
    EXPECT_TRUE(contentLengthHeader > 0);
  }

  TEST_P(TransportAdapter, put)
  {
    Azure::Core::Url host("http://httpbin.org/put");

    // PUT 1K
    auto requestBodyVector = std::vector<uint8_t>(1024, 'x');
    auto bodyRequest = Azure::Core::IO::MemoryBodyStream(requestBodyVector);
    auto request
        = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, host, &bodyRequest);
    auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
    checkResponseCode(response->GetStatusCode());
    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));

    CheckBodyFromBuffer(*response, expectedResponseBodySize);
  }

  TEST_P(TransportAdapter, deleteRequest)
  {
    Azure::Core::Url host("http://httpbin.org/delete");

    // Delete with 1k payload
    auto requestBodyVector = std::vector<uint8_t>(1024, 'x');
    auto bodyRequest = Azure::Core::IO::MemoryBodyStream(requestBodyVector);
    auto request
        = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Delete, host, &bodyRequest);
    auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
    checkResponseCode(response->GetStatusCode());

    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
    CheckBodyFromBuffer(*response, expectedResponseBodySize);
  }

  TEST_P(TransportAdapter, patch)
  {
    Azure::Core::Url host("http://httpbin.org/patch");

    // Patch with 1kb payload
    auto requestBodyVector = std::vector<uint8_t>(1024, 'x');
    auto bodyRequest = Azure::Core::IO::MemoryBodyStream(requestBodyVector);
    auto request
        = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Patch, host, &bodyRequest);
    auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
    checkResponseCode(response->GetStatusCode());

    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
    CheckBodyFromBuffer(*response, expectedResponseBodySize);
  }

  TEST_P(TransportAdapter, getChunk)
  {
    Azure::Core::Url host("http://anglesharp.azurewebsites.net/Chunked");
    auto expectedResponseBodySize = -1; // chunked will return unknown body length
    auto expectedChunkResponse = std::string(
        "<!DOCTYPE html>\r\n<html lang=en>\r\n<head>\r\n<meta charset='utf-8'>\r\n<title>Chunked "
        "transfer encoding test</title>\r\n</head>\r\n<body><h1>Chunked transfer encoding "
        "test</h1><h5>This is a chunked response after 100 ms.</h5><h5>This is a chunked "
        "response after 1 second. The server should not close the stream before all chunks are "
        "sent to a client.</h5></body></html>");

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);
    auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());

    checkResponseCode(response->GetStatusCode());
    CheckBodyFromBuffer(*response, expectedResponseBodySize, expectedChunkResponse);
  }

  TEST_P(TransportAdapter, putErrorResponse)
  {
    Azure::Core::Url host("http://httpbin.org/get");

    // Try to make a PUT to a GET url. This will return an error code from server.
    // This test makes sure that the connection is not re-used (because it gets closed by server)
    // and next request is not hang
    for (auto i = 0; i < 10; i++)
    {
      auto requestBodyVector = std::vector<uint8_t>(10, 'x');
      auto bodyRequest = Azure::Core::IO::MemoryBodyStream(requestBodyVector);
      auto request
          = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, host, &bodyRequest);
      auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
    }
  }

  // **********************
  // ***Same tests but getting stream to pull from socket, simulating the Download Op
  // **********************

  TEST_P(TransportAdapter, getWithStream)
  {
    Azure::Core::Url host("http://httpbin.org/get");

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host, true);
    auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
    checkResponseCode(response->GetStatusCode());
    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
    CheckBodyFromStream(*response, expectedResponseBodySize);

    request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host, true);
    // Add a header and send again. Response should return that header in the body
    request.SetHeader("123", "456");
    response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
    checkResponseCode(response->GetStatusCode());
    // header length is 6 (data) + 13 (formating) -> `    "123": "456"\r\n,`
    CheckBodyFromStream(*response, expectedResponseBodySize + 6 + 13);
  }

  TEST_P(TransportAdapter, getLoopWithStream)
  {
    Azure::Core::Url host("http://httpbin.org/get");

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host, true);

    // loop sending request
    for (auto i = 0; i < 50; i++)
    {
      auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
      checkResponseCode(response->GetStatusCode());
      CheckBodyFromStream(*response, expectedResponseBodySize);
    }
  }

  TEST_P(TransportAdapter, headWithStream)
  {
    Azure::Core::Url host("http://httpbin.org/get");
    auto expectedResponseBodySize = 0;

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, host, true);
    auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
    checkResponseCode(response->GetStatusCode());
    CheckBodyFromStream(*response, expectedResponseBodySize);

    // Check content-length header to be greater than 0
    int64_t contentLengthHeader = std::stoull(response->GetHeaders().at("content-length"));
    EXPECT_TRUE(contentLengthHeader > 0);
  }

  TEST_P(TransportAdapter, putWithStream)
  {
    Azure::Core::Url host("http://httpbin.org/put");

    // PUT 1k
    auto requestBodyVector = std::vector<uint8_t>(1024, 'x');
    auto bodyRequest = Azure::Core::IO::MemoryBodyStream(requestBodyVector);
    auto request
        = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, host, &bodyRequest, true);
    auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
    checkResponseCode(response->GetStatusCode());
    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));

    CheckBodyFromStream(*response, expectedResponseBodySize);
  }

  TEST_P(TransportAdapter, deleteRequestWithStream)
  {
    Azure::Core::Url host("http://httpbin.org/delete");

    // Delete with 1k payload
    auto requestBodyVector = std::vector<uint8_t>(1024, 'x');
    auto bodyRequest = Azure::Core::IO::MemoryBodyStream(requestBodyVector);
    auto request = Azure::Core::Http::Request(
        Azure::Core::Http::HttpMethod::Delete, host, &bodyRequest, true);
    auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
    checkResponseCode(response->GetStatusCode());

    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
    CheckBodyFromStream(*response, expectedResponseBodySize);
  }

  TEST_P(TransportAdapter, patchWithStream)
  {
    Azure::Core::Url host("http://httpbin.org/patch");

    // Patch with 1kb payload
    auto requestBodyVector = std::vector<uint8_t>(1024, 'x');
    auto bodyRequest = Azure::Core::IO::MemoryBodyStream(requestBodyVector);
    auto request = Azure::Core::Http::Request(
        Azure::Core::Http::HttpMethod::Patch, host, &bodyRequest, true);
    auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
    checkResponseCode(response->GetStatusCode());

    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
    CheckBodyFromStream(*response, expectedResponseBodySize);
  }

  TEST_P(TransportAdapter, getChunkWithStream)
  {
    Azure::Core::Url host("http://anglesharp.azurewebsites.net/Chunked");
    auto expectedResponseBodySize = -1; // chunked will return unknown body length
    auto expectedChunkResponse = std::string(
        "<!DOCTYPE html>\r\n<html lang=en>\r\n<head>\r\n<meta charset='utf-8'>\r\n<title>Chunked "
        "transfer encoding test</title>\r\n</head>\r\n<body><h1>Chunked transfer encoding "
        "test</h1><h5>This is a chunked response after 100 ms.</h5><h5>This is a chunked "
        "response after 1 second. The server should not close the stream before all chunks are "
        "sent to a client.</h5></body></html>");

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host, true);
    auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());

    checkResponseCode(response->GetStatusCode());
    CheckBodyFromStream(*response, expectedResponseBodySize, expectedChunkResponse);
  }

  TEST_P(TransportAdapter, createResponseT)
  {
    Azure::Core::Url host("http://httpbin.org/get");
    std::string expectedType("This is the Response Type");

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host, false);
    auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());

    Azure::Response<std::string> responseT(expectedType, std::move(response));
    auto& r = responseT.RawResponse;

    EXPECT_TRUE(r->GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);
    auto expectedResponseBodySize = std::stoull(r->GetHeaders().at("content-length"));
    CheckBodyFromBuffer(*r, expectedResponseBodySize);

    // Direct access
    auto result = responseT.Value;
    EXPECT_STREQ(result.data(), expectedType.data());
    // Test that calling getValue again will return empty
    result = std::move(responseT.Value);
    EXPECT_STREQ(result.data(), expectedType.data());
    result = responseT.Value;
    EXPECT_STREQ(result.data(), std::string("").data());
  }

  TEST_P(TransportAdapter, customSizePut)
  {
    Azure::Core::Url host("http://httpbin.org/put");

    // PUT 1MB
    auto requestBodyVector = std::vector<uint8_t>(1024 * 1024, 'x');
    auto bodyRequest = Azure::Core::IO::MemoryBodyStream(requestBodyVector);
    auto request
        = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, host, &bodyRequest);
    // Make transport adapter to read all stream content for uploading instead of chunks
    request.SetUploadChunkSize(1024 * 1024);
    {
      auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
      checkResponseCode(response->GetStatusCode());
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
      CheckBodyFromBuffer(*response, expectedResponseBodySize);
    }
  }

  TEST_P(TransportAdapter, putWithStreamOnFail)
  {
    // point to bad address pah to generate server MethodNotAllowed error
    Azure::Core::Url host("http://httpbin.org/get");

    // PUT 1k
    auto requestBodyVector = std::vector<uint8_t>(1024, 'x');
    auto bodyRequest = Azure::Core::IO::MemoryBodyStream(requestBodyVector);
    auto request
        = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, host, &bodyRequest, true);
    auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
    checkResponseCode(
        response->GetStatusCode(), Azure::Core::Http::HttpStatusCode::MethodNotAllowed);
    auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));

    CheckBodyFromBuffer(*response, expectedResponseBodySize);
  }

  TEST_P(TransportAdapter, cancelTransferUpload)
  {
    Azure::Core::Url host("http://httpbin.org/put");
    Azure::Core::Context cancelThis;

    auto threadRoutine = [&]() {
      // Start a big upload and expect it to throw cancelation
      std::vector<uint8_t> bigBuffer(1024 * 1024 * 200, 'x'); // upload 200 Mb
      auto stream = Azure::Core::IO::MemoryBodyStream(bigBuffer);
      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, host, &stream);

      // Request will be cancelled from main thread throwing the exception
      EXPECT_THROW(m_pipeline->Send(request, cancelThis), Azure::Core::OperationCancelledException);
    };

    // Start request
    std::thread t1(threadRoutine);

    // Wait 100 ms so we know upload has started
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    cancelThis.Cancel();
    t1.join();
  }

  TEST_P(TransportAdapter, cancelTransferDownload)
  {
    // public big blob (321MB)
    Azure::Core::Url host("https://bigtestfiles.blob.core.windows.net/cpptestfiles/321MB");
    Azure::Core::Context cancelThis;

    auto threadRoutine = [&]() {
      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);

      // Request will be cancelled from main thread throwing the exception
      EXPECT_THROW(m_pipeline->Send(request, cancelThis), Azure::Core::OperationCancelledException);
    };

    // Start request
    std::thread t1(threadRoutine);

    // Wait 100 ms so we know download has started
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    cancelThis.Cancel();
    t1.join();
  }

  TEST_P(TransportAdapter, requestFailedException)
  {
    Azure::Core::Url host("http://unresolvedHost.org/get");

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);
    EXPECT_THROW(
        m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext()),
        Azure::Core::RequestFailedException);
  }

  TEST_P(TransportAdapter, dynamicCast)
  {
    Azure::Core::Url host("http://unresolvedHost.org/get");
    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);

    // test dynamic cast
    try
    {
      auto result = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
    }
    catch (Azure::Core::RequestFailedException& err)
    {
      // if ref can't be cast, it throws
      EXPECT_NO_THROW((void)dynamic_cast<Azure::Core::Http::TransportException&>(err));
      EXPECT_NO_THROW((void)dynamic_cast<std::runtime_error&>(err));
      EXPECT_THROW((void)dynamic_cast<std::range_error&>(err), std::bad_cast);
    }
  }

  TEST_P(TransportAdapter, SizePutFromFile)
  {
    Azure::Core::Url host("http://httpbin.org/put");
    std::string testDataPath(AZURE_TEST_DATA_PATH);

#if defined(AZ_PLATFORM_POSIX)
    testDataPath.append("/fileData");
#elif defined(AZ_PLATFORM_WINDOWS)
    testDataPath.append("\\fileData");
#else
#error "Unknown platform"
#endif

    Azure::Core::IO::FileBodyStream requestBodyStream(testDataPath);
    auto request = Azure::Core::Http::Request(
        Azure::Core::Http::HttpMethod::Put, host, &requestBodyStream, true);
    // Make transport adapter to read all stream content for uploading instead of chunks
    request.SetUploadChunkSize(Azure::Core::Test::Datails::FileSize);
    {
      auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
      checkResponseCode(response->GetStatusCode());
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));

      CheckBodyFromStream(*response, expectedResponseBodySize);
    }
  }

  TEST_P(TransportAdapter, SizePutFromFileDefault)
  {
    Azure::Core::Url host("http://httpbin.org/put");
    std::string testDataPath(AZURE_TEST_DATA_PATH);

#if defined(AZ_PLATFORM_POSIX)
    testDataPath.append("/fileData");
#elif defined(AZ_PLATFORM_WINDOWS)
    testDataPath.append("\\fileData");
#else
#error "Unknown platform"
#endif

    Azure::Core::IO::FileBodyStream requestBodyStream(testDataPath);
    auto request = Azure::Core::Http::Request(
        Azure::Core::Http::HttpMethod::Put, host, &requestBodyStream, true);
    // Make transport adapter to read default chunk size
    {
      auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
      checkResponseCode(response->GetStatusCode());
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));

      CheckBodyFromStream(*response, expectedResponseBodySize);
    }
  }

  TEST_P(TransportAdapter, SizePutFromFileBiggerPage)
  {
    Azure::Core::Url host("http://httpbin.org/put");
    std::string testDataPath(AZURE_TEST_DATA_PATH);

#if defined(AZ_PLATFORM_POSIX)
    testDataPath.append("/fileData");
#elif defined(AZ_PLATFORM_WINDOWS)
    testDataPath.append("\\fileData");
#else
#error "Unknown platform"
#endif

    Azure::Core::IO::FileBodyStream requestBodyStream(testDataPath);
    auto request = Azure::Core::Http::Request(
        Azure::Core::Http::HttpMethod::Put, host, &requestBodyStream, true);
    // Make transport adapter to read more than file size (5Mb)
    request.SetUploadChunkSize(Azure::Core::Test::Datails::FileSize * 5);
    {
      auto response = m_pipeline->Send(request, Azure::Core::Context::GetApplicationContext());
      checkResponseCode(response->GetStatusCode());
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));

      CheckBodyFromStream(*response, expectedResponseBodySize);
    }
  }

  /*****************  Test Utils *************************/
  void TransportAdapter::checkResponseCode(
      Azure::Core::Http::HttpStatusCode code,
      Azure::Core::Http::HttpStatusCode expectedCode)
  {
    EXPECT_PRED2(
        [](Azure::Core::Http::HttpStatusCode a, Azure::Core::Http::HttpStatusCode b) {
          return a == b;
        },
        code,
        expectedCode);
  }

  void TransportAdapter::CheckBodyFromBuffer(
      Azure::Core::Http::RawResponse& response,
      int64_t size,
      std::string expectedBody)
  {
    auto body = response.ExtractBodyStream();
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
    auto body = response.ExtractBodyStream();
    EXPECT_NE(body, nullptr);

    std::vector<uint8_t> bodyVector
        = body->ReadToEnd(Azure::Core::Context::GetApplicationContext());
    int64_t bodySize = body->Length();
    EXPECT_EQ(bodySize, size);
    bodySize = bodyVector.size();

    if (size > 0)
    { // only for known body size
      EXPECT_EQ(bodyVector.size(), static_cast<size_t>(size));
    }

    if (expectedBody.size() > 0)
    {
      auto bodyString = std::string(bodyVector.begin(), bodyVector.end());
      EXPECT_STREQ(expectedBody.data(), bodyString.data());
    }
  }

}}} // namespace Azure::Core::Test
