// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/http/transport.hpp>
#include <azure/core/io/body_stream.hpp>
#include <azure/storage/blobs.hpp>

#include <gtest/gtest.h>

namespace Azure { namespace Storage { namespace Test {
  namespace {
    const std::string BatchBoundary = "batch_response";
    const std::string BatchContentType = "multipart/mixed; boundary=" + BatchBoundary;

    class BatchResponseTransport final : public Core::Http::HttpTransport {
    public:
      explicit BatchResponseTransport(
          std::string body,
          Nullable<std::string> contentType,
          Core::Http::HttpStatusCode status)
          : m_body(std::move(body)), m_contentType(std::move(contentType)), m_status(status)
      {
      }

      std::unique_ptr<Core::Http::RawResponse> Send(Core::Http::Request&, const Core::Context&)
          override
      {
        auto response = std::make_unique<Core::Http::RawResponse>(1, 1, m_status, "Batch response");
        if (m_contentType.HasValue())
        {
          response->SetHeader("Content-Type", m_contentType.Value());
        }
        response->SetHeader("Content-Length", std::to_string(m_body.size()));
        response->SetBodyStream(
            std::make_unique<Core::IO::MemoryBodyStream>(
                reinterpret_cast<const uint8_t*>(m_body.data()), m_body.size()));
        return response;
      }

    private:
      std::string m_body;
      Nullable<std::string> m_contentType;
      Core::Http::HttpStatusCode m_status;
    };

    std::string Part(
        const std::string& headers,
        const std::string& status = "202 Accepted",
        const std::string& responseHeaders = "")
    {
      return "--" + BatchBoundary
          + "\r\nContent-Type: application/http\r\n"
            "Content-Transfer-Encoding: binary\r\n"
          + headers + "\r\nHTTP/1.1 " + status + "\r\nContent-Length: 0\r\n" + responseHeaders
          + "\r\n";
    }

    std::string Finish(const std::string& parts) { return parts + "--" + BatchBoundary + "--"; }

    class BlobBatchResponseTest : public testing::TestWithParam<bool> {
    protected:
      template <class Test>
      void WithBatch(
          const std::string& body,
          Test test,
          Nullable<std::string> contentType = BatchContentType,
          Core::Http::HttpStatusCode status = Core::Http::HttpStatusCode::Accepted)
      {
        Blobs::BlobClientOptions options;
        options.Transport.Transport
            = std::make_shared<BatchResponseTransport>(body, std::move(contentType), status);
        options.Retry.MaxRetries = 0;
        if (GetParam())
        {
          auto client = Blobs::BlobContainerClient(
              "https://account.blob.core.windows.net/container", options);
          auto batch = client.CreateBatch();
          test(client, batch);
        }
        else
        {
          auto client = Blobs::BlobServiceClient("https://account.blob.core.windows.net", options);
          auto batch = client.CreateBatch();
          test(client, batch);
        }
      }

      void ExpectInvalidResponse(
          const std::string& body,
          Nullable<std::string> contentType = BatchContentType)
      {
        WithBatch(
            body,
            [](auto& client, auto& batch) {
              batch.DeleteBlobUrl("https://account.blob.core.windows.net/container/first");
              batch.DeleteBlobUrl("https://account.blob.core.windows.net/container/second");
              EXPECT_THROW(client.SubmitBatch(batch), StorageException);
            },
            std::move(contentType));
      }
    };
  } // namespace

  TEST_P(BlobBatchResponseTest, RejectsNumericPrefixContentId)
  {
    ExpectInvalidResponse(Finish(Part("Content-ID: 0junk\r\n") + Part("Content-ID: 1\r\n")));
  }

  TEST_P(BlobBatchResponseTest, RejectsInvalidMultipartContentType)
  {
    const auto body = Finish(Part("Content-ID: 0\r\n") + Part("Content-ID: 1\r\n"));
    const std::string prefix = "multipart/mixed; boundary=";
    for (size_t length = 0; length <= prefix.size(); ++length)
    {
      SCOPED_TRACE(length);
      ExpectInvalidResponse(body, prefix.substr(0, length));
    }
    for (const auto& contentType : {
             std::string("text/plain"),
             std::string("application/json"),
             std::string("multipart/mixed"),
             std::string(prefix.size(), 'x') + BatchBoundary,
             std::string("multipart/mixed; boundary"),
             std::string("multipart/mixed; boundary=\"\""),
         })
    {
      SCOPED_TRACE(contentType);
      ExpectInvalidResponse(body, contentType);
    }
    ExpectInvalidResponse(body, Nullable<std::string>());
  }

  TEST_P(BlobBatchResponseTest, PreservesNonMultipartBatchError)
  {
    WithBatch(
        "",
        [](auto& client, auto& batch) {
          batch.DeleteBlobUrl("https://account.blob.core.windows.net/container/first");
          try
          {
            client.SubmitBatch(batch);
            FAIL() << "Expected the original HTTP failure.";
          }
          catch (const StorageException& exception)
          {
            EXPECT_EQ(exception.StatusCode, Core::Http::HttpStatusCode::BadRequest);
            ASSERT_NE(exception.RawResponse, nullptr);
            EXPECT_EQ(
                exception.RawResponse->GetStatusCode(), Core::Http::HttpStatusCode::BadRequest);
          }
        },
        Nullable<std::string>(),
        Core::Http::HttpStatusCode::BadRequest);
  }

  TEST_P(BlobBatchResponseTest, RejectsInvalidContentIds)
  {
    const std::vector<std::string> ids{
        "",
        "-1",
        "-2",
        "+0",
        " 0",
        "0 ",
        "0junk",
        "junk",
        "0.0",
        "2",
        "2147483647",
        "2147483648",
        "18446744073709551615",
        "18446744073709551616",
        std::string(1000, '9')};
    for (const auto& id : ids)
    {
      SCOPED_TRACE(id);
      ExpectInvalidResponse(Finish(Part("Content-ID: " + id + "\r\n") + Part("Content-ID: 1\r\n")));
    }
  }

  TEST_P(BlobBatchResponseTest, RejectsDuplicateAndIncompleteResponses)
  {
    const std::vector<std::string> parts{
        "",
        Part("Content-ID: 0\r\n"),
        Part("Content-ID: 1\r\n"),
        Part("Content-ID: 0\r\n") + Part("Content-ID: 0\r\n"),
        Part("Content-ID: 1\r\n") + Part("Content-ID: 1\r\n"),
        Part("Content-ID: 0\r\nContent-ID: 1\r\n") + Part("Content-ID: 1\r\n"),
        Part("Content-ID: 0\r\n") + Part("Content-ID: 1\r\n") + Part("Content-ID: 0\r\n"),
    };
    for (const auto& responseParts : parts)
    {
      SCOPED_TRACE(responseParts);
      ExpectInvalidResponse(Finish(responseParts));
    }
  }

  TEST_P(BlobBatchResponseTest, RejectsMissingContentId)
  {
    const std::vector<std::string> parts{
        Part("") + Part("Content-ID: 1\r\n"),
        Part("Content-ID: 0\r\n") + Part(""),
        Part("") + Part("Content-ID: 1\r\n", "400 Bad Request"),
        Part("", "202 Accepted", "Content-ID: 0\r\n") + Part("Content-ID: 1\r\n"),
        Part("X-Content-ID: 0\r\n") + Part("Content-ID: 1\r\n"),
        Part("", "400 Bad Request") + Part("Content-ID: 1\r\n"),
        Part("Content-ID: 0\r\n") + Part("", "400 Bad Request"),
    };
    for (const auto& responseParts : parts)
    {
      SCOPED_TRACE(responseParts);
      ExpectInvalidResponse(Finish(responseParts));
    }
  }

  TEST_P(BlobBatchResponseTest, MapsResponsesByContentId)
  {
    for (const auto& ending : {std::string(), std::string("\r\n")})
    {
      WithBatch(
          Finish(
              Part("Content-ID: 1\r\n", "200 OK", "x-test-response: second\r\n")
              + Part("Content-ID: 0\r\n", "202 Accepted", "x-test-response: first\r\n"))
              + ending,
          [](auto& client, auto& batch) {
            auto first
                = batch.DeleteBlobUrl("https://account.blob.core.windows.net/container/first");
            auto second = batch.SetBlobAccessTierUrl(
                "https://account.blob.core.windows.net/container/second",
                Blobs::Models::AccessTier::Cool);
            EXPECT_NO_THROW(client.SubmitBatch(batch));
            EXPECT_EQ(first.GetResponse().RawResponse->GetHeaders().at("x-test-response"), "first");
            EXPECT_EQ(
                second.GetResponse().RawResponse->GetHeaders().at("x-test-response"), "second");
          });
    }
  }

  TEST_P(BlobBatchResponseTest, MapsMaximumBatchInReverseOrder)
  {
    std::string parts;
    for (size_t i = 256; i > 0; --i)
    {
      const auto id = std::to_string(i - 1);
      parts
          += Part("Content-ID: " + id + "\r\n", "202 Accepted", "x-test-response: " + id + "\r\n");
    }
    WithBatch(Finish(parts), [](auto& client, auto& batch) {
      std::vector<DeferredResponse<Blobs::Models::DeleteBlobResult>> responses;
      for (size_t i = 0; i < 256; ++i)
      {
        responses.push_back(batch.DeleteBlobUrl(
            "https://account.blob.core.windows.net/container/blob" + std::to_string(i)));
      }
      EXPECT_NO_THROW(client.SubmitBatch(batch));
      for (size_t i = 0; i < responses.size(); ++i)
      {
        EXPECT_EQ(
            responses[i].GetResponse().RawResponse->GetHeaders().at("x-test-response"),
            std::to_string(i));
      }
    });
  }

  TEST_P(BlobBatchResponseTest, RejectsResponseForEmptyBatch)
  {
    WithBatch(Finish(Part("Content-ID: 0\r\n")), [](auto& client, auto& batch) {
      EXPECT_THROW(client.SubmitBatch(batch), StorageException);
    });
  }

  TEST_P(BlobBatchResponseTest, RejectsTruncatedMultipartResponses)
  {
    for (const auto& body : {
             std::string(),
             Part("Content-ID: 0\r\n"),
             std::string("--") + BatchBoundary,
             std::string("--") + BatchBoundary + "\r\nContent-ID: 0\r\n",
         })
    {
      SCOPED_TRACE(body);
      ExpectInvalidResponse(body);
    }
  }

  TEST_P(BlobBatchResponseTest, PreservesSubrequestFailures)
  {
    WithBatch(
        Finish(
            Part("Content-ID: 1\r\n")
            + Part("Content-ID: 0\r\n", "404 Not Found", "x-ms-error-code: BlobNotFound\r\n")),
        [](auto& client, auto& batch) {
          auto first = batch.DeleteBlobUrl("https://account.blob.core.windows.net/container/first");
          auto second
              = batch.DeleteBlobUrl("https://account.blob.core.windows.net/container/second");
          EXPECT_NO_THROW(client.SubmitBatch(batch));
          EXPECT_THROW(first.GetResponse(), StorageException);
          EXPECT_NO_THROW(second.GetResponse());
        });
  }

  TEST_P(BlobBatchResponseTest, PreservesBatchLevelErrorWithoutContentId)
  {
    WithBatch(
        Finish(Part("", "400 Bad Request", "x-ms-error-code: InvalidInput\r\n")),
        [](auto& client, auto& batch) {
          batch.DeleteBlobUrl("https://account.blob.core.windows.net/container/first");
          try
          {
            client.SubmitBatch(batch);
            FAIL() << "Expected the batch-level storage error.";
          }
          catch (const StorageException& exception)
          {
            EXPECT_EQ(exception.StatusCode, Core::Http::HttpStatusCode::BadRequest);
            EXPECT_EQ(exception.ErrorCode, "InvalidInput");
          }
        });
  }

  INSTANTIATE_TEST_SUITE_P(
      ServiceAndContainer,
      BlobBatchResponseTest,
      testing::Values(false, true));
}}} // namespace Azure::Storage::Test
