// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "transport_adapter.hpp"
#include <azure/core/context.hpp>
#include <azure/core/response.hpp>
#include <iostream>
#include <string>
#include <thread>

namespace Azure { namespace Core { namespace Test {

  static std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> CreatePolicies()
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> p;
    std::shared_ptr<Azure::Core::Http::HttpTransport> transport
        = std::make_shared<Azure::Core::Http::CurlTransport>();
    Azure::Core::Http::RetryOptions opt;
    opt.RetryDelay = std::chrono::milliseconds(10);

    // Retry policy will help to prevent server-occasionally-errors
    p.push_back(std::make_unique<Azure::Core::Http::RetryPolicy>(opt));
    p.push_back(std::make_unique<Azure::Core::Http::TransportPolicy>(std::move(transport)));
    return p;
  }

  std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> TransportAdapter::policies
      = CreatePolicies();

  Azure::Core::Http::HttpPipeline TransportAdapter::pipeline(policies);
  Azure::Core::Context TransportAdapter::context = Azure::Core::GetApplicationContext();

// Connection pool feature is curl-implementation only. No other transport adapter would have the
// connection pool
#ifdef BUILD_CURL_HTTP_TRANSPORT_ADAPTER
    // connectionPoolTest requires `ConnectionsOnPool` hook which is only available when building
    // BUILD_TESTING. This test is only built when that case is true.
    TEST_F(TransportAdapter, connectionPoolTest)
    {
      Azure::Core::Http::Url host("http://httpbin.org/get");
      Azure::Core::Http::CurlConnectionPool::ClearIndex();

      auto threadRoutine = [host]() {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);
        auto response = pipeline.Send(context, request);
        checkResponseCode(response->GetStatusCode());
        auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
        CheckBodyFromBuffer(*response, expectedResponseBodySize);
      };

      std::thread t1(threadRoutine);
      std::thread t2(threadRoutine);
      t1.join();
      t2.join();

      // 2 connections must be available at this point
      EXPECT_EQ(Http::CurlConnectionPool::ConnectionsOnPool("httpbin.org"), 2);

      std::thread t3(threadRoutine);
      std::thread t4(threadRoutine);
      std::thread t5(threadRoutine);
      t3.join();
      t4.join();
      t5.join();

      // Two connections re-used plus one connection created
      EXPECT_EQ(Http::CurlConnectionPool::ConnectionsOnPool("httpbin.org"), 3);

#ifdef RUN_LONG_UNIT_TESTS
      {
        // Test pool clean routine
        std::cout << "Running Connection Pool Cleaner Test. This test takes more than 3 minutes to "
                     "complete."
                  << std::endl
                  << "Add compiler option -DRUN_LONG_UNIT_TESTS=OFF when building if you want to "
                     "skip this "
                     "test."
                  << std::endl;

        // Wait for 180 secs to make sure any previous connection is removed by the cleaner
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 180));

        std::cout << "First wait time done. Validating state." << std::endl;

        // index is not affected by cleaner. It does not remove index
        EXPECT_EQ(Http::CurlConnectionPool::ConnectionsIndexOnPool(), 1);
        // cleaner should have removed connections
        EXPECT_EQ(Http::CurlConnectionPool::ConnectionsOnPool("httpbin.org"), 0);

        std::thread t1(threadRoutine);
        std::thread t2(threadRoutine);
        t1.join();
        t2.join();

        // wait for connection to be moved back to pool
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // 2 connections must be available at this point and one index
        EXPECT_EQ(Http::CurlConnectionPool::ConnectionsIndexOnPool(), 1);
        EXPECT_EQ(Http::CurlConnectionPool::ConnectionsOnPool("httpbin.org"), 2);
      }
#endif
    }
#endif

    TEST_F(TransportAdapter, get)
    {
      Azure::Core::Http::Url host("http://httpbin.org/get");

      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);
      auto response = pipeline.Send(context, request);
      checkResponseCode(response->GetStatusCode());
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
      CheckBodyFromBuffer(*response, expectedResponseBodySize);

      // Need to init request again, since retry would be on after it is sent
      request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);
      // Add a header and send again. RawResponse should return that header in the body
      request.AddHeader("123", "456");
      response = pipeline.Send(context, request);
      checkResponseCode(response->GetStatusCode());
      // header length is 6 (data) + 13 (formating) -> `    "123": "456"\r\n,`
      CheckBodyFromBuffer(*response, expectedResponseBodySize + 6 + 13);
    }

    TEST_F(TransportAdapter, get204)
    {
      Azure::Core::Http::Url host("http://mt3.google.com/generate_204");

      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);
      auto response = pipeline.Send(context, request);
      checkResponseCode(response->GetStatusCode(), Azure::Core::Http::HttpStatusCode::NoContent);
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
      CheckBodyFromBuffer(*response, expectedResponseBodySize);
    }

    TEST_F(TransportAdapter, getLoop)
    {
      Azure::Core::Http::Url host("http://httpbin.org/get");

      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);

      // loop sending request
      for (auto i = 0; i < 50; i++)
      {
        auto response = pipeline.Send(context, request);
        auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
        checkResponseCode(response->GetStatusCode());
        CheckBodyFromBuffer(*response, expectedResponseBodySize);
      }
    }

    TEST_F(TransportAdapter, head)
    {
      Azure::Core::Http::Url host("http://httpbin.org/get");
      auto expectedResponseBodySize = 0;

      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, host);
      auto response = pipeline.Send(context, request);
      checkResponseCode(response->GetStatusCode());
      CheckBodyFromBuffer(*response, expectedResponseBodySize);

      // Check content-length header to be greater than 0
      int64_t contentLengthHeader = std::stoull(response->GetHeaders().at("content-length"));
      EXPECT_TRUE(contentLengthHeader > 0);
    }

    TEST_F(TransportAdapter, put)
    {
      Azure::Core::Http::Url host("http://httpbin.org/put");

      // PUT 1K
      auto requestBodyVector = std::vector<uint8_t>(1024, 'x');
      auto bodyRequest = Azure::Core::Http::MemoryBodyStream(requestBodyVector);
      auto request
          = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, host, &bodyRequest);
      auto response = pipeline.Send(context, request);
      checkResponseCode(response->GetStatusCode());
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));

      CheckBodyFromBuffer(*response, expectedResponseBodySize);
    }

    TEST_F(TransportAdapter, deleteRequest)
    {
      Azure::Core::Http::Url host("http://httpbin.org/delete");

      // Delete with 1k payload
      auto requestBodyVector = std::vector<uint8_t>(1024, 'x');
      auto bodyRequest = Azure::Core::Http::MemoryBodyStream(requestBodyVector);
      auto request
          = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Delete, host, &bodyRequest);
      auto response = pipeline.Send(context, request);
      checkResponseCode(response->GetStatusCode());

      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
      CheckBodyFromBuffer(*response, expectedResponseBodySize);
    }

    TEST_F(TransportAdapter, patch)
    {
      Azure::Core::Http::Url host("http://httpbin.org/patch");

      // Patch with 1kb payload
      auto requestBodyVector = std::vector<uint8_t>(1024, 'x');
      auto bodyRequest = Azure::Core::Http::MemoryBodyStream(requestBodyVector);
      auto request
          = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Patch, host, &bodyRequest);
      auto response = pipeline.Send(context, request);
      checkResponseCode(response->GetStatusCode());

      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
      CheckBodyFromBuffer(*response, expectedResponseBodySize);
    }

    TEST_F(TransportAdapter, getChunk)
    {
      Azure::Core::Http::Url host("http://anglesharp.azurewebsites.net/Chunked");
      auto expectedResponseBodySize = -1; // chunked will return unknown body length
      auto expectedChunkResponse = std::string(
          "<!DOCTYPE html>\r\n<html lang=en>\r\n<head>\r\n<meta charset='utf-8'>\r\n<title>Chunked "
          "transfer encoding test</title>\r\n</head>\r\n<body><h1>Chunked transfer encoding "
          "test</h1><h5>This is a chunked response after 100 ms.</h5><h5>This is a chunked "
          "response after 1 second. The server should not close the stream before all chunks are "
          "sent to a client.</h5></body></html>");

      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);
      auto response = pipeline.Send(context, request);

      checkResponseCode(response->GetStatusCode());
      CheckBodyFromBuffer(*response, expectedResponseBodySize, expectedChunkResponse);
    }

    TEST_F(TransportAdapter, putErrorResponse)
    {
      Azure::Core::Http::Url host("http://httpbin.org/get");

      // Try to make a PUT to a GET url. This will return an error code from server.
      // This test makes sure that the connection is not re-used (because it gets closed by server)
      // and next request is not hang
      for (auto i = 0; i < 10; i++)
      {
        auto requestBodyVector = std::vector<uint8_t>(10, 'x');
        auto bodyRequest = Azure::Core::Http::MemoryBodyStream(requestBodyVector);
        auto request
            = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, host, &bodyRequest);
        auto response = pipeline.Send(context, request);
      }
    }

    // **********************
    // ***Same tests but getting stream to pull from socket, simulating the Download Op
    // **********************

    TEST_F(TransportAdapter, getWithStream)
    {
      Azure::Core::Http::Url host("http://httpbin.org/get");

      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host, true);
      auto response = pipeline.Send(context, request);
      checkResponseCode(response->GetStatusCode());
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
      CheckBodyFromStream(*response, expectedResponseBodySize);

      request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host, true);
      // Add a header and send again. Response should return that header in the body
      request.AddHeader("123", "456");
      response = pipeline.Send(context, request);
      checkResponseCode(response->GetStatusCode());
      // header length is 6 (data) + 13 (formating) -> `    "123": "456"\r\n,`
      CheckBodyFromStream(*response, expectedResponseBodySize + 6 + 13);
    }

    TEST_F(TransportAdapter, getLoopWithStream)
    {
      Azure::Core::Http::Url host("http://httpbin.org/get");

      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host, true);

      // loop sending request
      for (auto i = 0; i < 50; i++)
      {
        auto response = pipeline.Send(context, request);
        auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
        checkResponseCode(response->GetStatusCode());
        CheckBodyFromStream(*response, expectedResponseBodySize);
      }
    }

    TEST_F(TransportAdapter, headWithStream)
    {
      Azure::Core::Http::Url host("http://httpbin.org/get");
      auto expectedResponseBodySize = 0;

      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Head, host, true);
      auto response = pipeline.Send(context, request);
      checkResponseCode(response->GetStatusCode());
      CheckBodyFromStream(*response, expectedResponseBodySize);

      // Check content-length header to be greater than 0
      int64_t contentLengthHeader = std::stoull(response->GetHeaders().at("content-length"));
      EXPECT_TRUE(contentLengthHeader > 0);
    }

    TEST_F(TransportAdapter, putWithStream)
    {
      Azure::Core::Http::Url host("http://httpbin.org/put");

      // PUT 1k
      auto requestBodyVector = std::vector<uint8_t>(1024, 'x');
      auto bodyRequest = Azure::Core::Http::MemoryBodyStream(requestBodyVector);
      auto request = Azure::Core::Http::Request(
          Azure::Core::Http::HttpMethod::Put, host, &bodyRequest, true);
      auto response = pipeline.Send(context, request);
      checkResponseCode(response->GetStatusCode());
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));

      CheckBodyFromStream(*response, expectedResponseBodySize);
    }

    TEST_F(TransportAdapter, deleteRequestWithStream)
    {
      Azure::Core::Http::Url host("http://httpbin.org/delete");

      // Delete with 1k payload
      auto requestBodyVector = std::vector<uint8_t>(1024, 'x');
      auto bodyRequest = Azure::Core::Http::MemoryBodyStream(requestBodyVector);
      auto request = Azure::Core::Http::Request(
          Azure::Core::Http::HttpMethod::Delete, host, &bodyRequest, true);
      auto response = pipeline.Send(context, request);
      checkResponseCode(response->GetStatusCode());

      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
      CheckBodyFromStream(*response, expectedResponseBodySize);
    }

    TEST_F(TransportAdapter, patchWithStream)
    {
      Azure::Core::Http::Url host("http://httpbin.org/patch");

      // Patch with 1kb payload
      auto requestBodyVector = std::vector<uint8_t>(1024, 'x');
      auto bodyRequest = Azure::Core::Http::MemoryBodyStream(requestBodyVector);
      auto request = Azure::Core::Http::Request(
          Azure::Core::Http::HttpMethod::Patch, host, &bodyRequest, true);
      auto response = pipeline.Send(context, request);
      checkResponseCode(response->GetStatusCode());

      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
      CheckBodyFromStream(*response, expectedResponseBodySize);
    }

    TEST_F(TransportAdapter, getChunkWithStream)
    {
      Azure::Core::Http::Url host("http://anglesharp.azurewebsites.net/Chunked");
      auto expectedResponseBodySize = -1; // chunked will return unknown body length
      auto expectedChunkResponse = std::string(
          "<!DOCTYPE html>\r\n<html lang=en>\r\n<head>\r\n<meta charset='utf-8'>\r\n<title>Chunked "
          "transfer encoding test</title>\r\n</head>\r\n<body><h1>Chunked transfer encoding "
          "test</h1><h5>This is a chunked response after 100 ms.</h5><h5>This is a chunked "
          "response after 1 second. The server should not close the stream before all chunks are "
          "sent to a client.</h5></body></html>");

      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host, true);
      auto response = pipeline.Send(context, request);

      checkResponseCode(response->GetStatusCode());
      CheckBodyFromStream(*response, expectedResponseBodySize, expectedChunkResponse);
    }

    TEST_F(TransportAdapter, createResponseT)
    {
      Azure::Core::Http::Url host("http://httpbin.org/get");
      std::string expectedType("This is the Response Type");

      auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host, false);
      auto response = pipeline.Send(context, request);

      Azure::Core::Response<std::string> responseT(expectedType, std::move(response));
      auto& r = responseT.GetRawResponse();

      EXPECT_TRUE(r.GetStatusCode() == Azure::Core::Http::HttpStatusCode::Ok);
      auto expectedResponseBodySize = std::stoull(r.GetHeaders().at("content-length"));
      CheckBodyFromBuffer(r, expectedResponseBodySize);

      // Direct access
      EXPECT_STREQ((*responseT).data(), expectedType.data());
      EXPECT_STREQ(responseT->data(), expectedType.data());
      // extracting T out of response
      auto result = responseT.ExtractValue();
      EXPECT_STREQ(result.data(), expectedType.data());
      // Test that calling getValue again will return empty
      result = responseT.ExtractValue();
      EXPECT_STREQ(result.data(), std::string("").data());
    }

    TEST_F(TransportAdapter, customSizePut)
    {
      Azure::Core::Http::Url host("http://httpbin.org/put");

      // PUT 1MB
      auto requestBodyVector = std::vector<uint8_t>(1024 * 1024, 'x');
      auto bodyRequest = Azure::Core::Http::MemoryBodyStream(requestBodyVector);
      auto request
          = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, host, &bodyRequest);
      // Make transport adapter to read all stream content for uploading instead of chunks
      request.SetUploadChunkSize(1024 * 1024);
      {
        auto response = pipeline.Send(context, request);
        checkResponseCode(response->GetStatusCode());
        auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));
        CheckBodyFromBuffer(*response, expectedResponseBodySize);
      }
    }

    TEST_F(TransportAdapter, putWithStreamOnFail)
    {
      // point to bad address pah to generate server MethodNotAllowed error
      Azure::Core::Http::Url host("http://httpbin.org/get");

      // PUT 1k
      auto requestBodyVector = std::vector<uint8_t>(1024, 'x');
      auto bodyRequest = Azure::Core::Http::MemoryBodyStream(requestBodyVector);
      auto request = Azure::Core::Http::Request(
          Azure::Core::Http::HttpMethod::Put, host, &bodyRequest, true);
      auto response = pipeline.Send(context, request);
      checkResponseCode(
          response->GetStatusCode(), Azure::Core::Http::HttpStatusCode::MethodNotAllowed);
      auto expectedResponseBodySize = std::stoull(response->GetHeaders().at("content-length"));

      CheckBodyFromBuffer(*response, expectedResponseBodySize);
    }

    TEST_F(TransportAdapter, cancelTransferUpload)
    {
      Azure::Core::Http::Url host("http://httpbin.org/put");
      Azure::Core::Context cancelThis;

      auto threadRoutine = [host, cancelThis]() {
        // Start a big upload and expect it to throw cancelation
        std::vector<uint8_t> bigBuffer(1024 * 1024 * 200, 'x'); // upload 200 Mb
        auto stream = Azure::Core::Http::MemoryBodyStream(bigBuffer);
        auto request
            = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Put, host, &stream);

        // Request will be canceled from main thread throwing the exception
        EXPECT_THROW(pipeline.Send(cancelThis, request), Azure::Core::OperationCanceledException);
      };

      // Start request
      std::thread t1(threadRoutine);

      // Wait 100 ms so we know upload has started
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

      cancelThis.Cancel();
      t1.join();
    }

    TEST_F(TransportAdapter, cancelTransferDownload)
    {
      // public big blob (321MB)
      Azure::Core::Http::Url host("https://bigtestfiles.blob.core.windows.net/cpptestfiles/321MB");
      Azure::Core::Context cancelThis;

      auto threadRoutine = [host, cancelThis]() {
        auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);

        // Request will be canceled from main thread throwing the exception
        EXPECT_THROW(pipeline.Send(cancelThis, request), Azure::Core::OperationCanceledException);
      };

      // Start request
      std::thread t1(threadRoutine);

      // Wait 100 ms so we know download has started
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

      cancelThis.Cancel();
      t1.join();
    }

  TEST_F(TransportAdapter, requestFailedException)
  {
    Azure::Core::Http::Url host("http://unresolvedHost.org/get");

    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);
    EXPECT_THROW(pipeline.Send(context, request), Azure::Core::RequestFailedException);
  }

  TEST_F(TransportAdapter, dynamicCast)
  {
    Azure::Core::Http::Url host("http://unresolvedHost.org/get");
    auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, host);

    // test dynamic cast
    try
    {
      auto result = pipeline.Send(context, request);
    }
    catch (Azure::Core::RequestFailedException& err)
    {
      // if ref can't be cast, it throws
      EXPECT_NO_THROW(dynamic_cast<Azure::Core::Http::TransportException&>(err));
      EXPECT_NO_THROW(dynamic_cast<std::runtime_error&>(err));
      EXPECT_THROW(dynamic_cast<std::range_error&>(err), std::bad_cast);
    }
  }

}}} // namespace Azure::Core::Test
