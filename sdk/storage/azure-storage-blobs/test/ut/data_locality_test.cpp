// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../src/private/data_locality.hpp"
#include "test/ut/test_base.hpp"

#include <azure/storage/blobs.hpp>

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <fstream>
#include <limits>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace Azure { namespace Storage { namespace Test {

  namespace {
    struct LocalityRequest final
    {
      std::string Host;
      std::string HostHeader;
      std::string IfMatch;
      std::string Range;
      bool IsLayout = false;
    };

    struct LocalityState final
    {
      std::mutex Mutex;
      std::vector<LocalityRequest> Requests;
      Core::Http::HttpStatusCode LayoutStatus = Core::Http::HttpStatusCode::Ok;
      bool PaginateLayout = false;
      bool ReturnDownloadHint = true;
      std::string DownloadHint = "layout";
    };

    class LocalityTransportPolicy final : public Core::Http::Policies::HttpPolicy {
    public:
      LocalityTransportPolicy(
          std::shared_ptr<LocalityState> state,
          std::shared_ptr<std::string> data)
          : m_state(std::move(state)), m_data(std::move(data))
      {
      }

      std::unique_ptr<HttpPolicy> Clone() const override
      {
        return std::make_unique<LocalityTransportPolicy>(*this);
      }

      std::unique_ptr<Core::Http::RawResponse> Send(
          Core::Http::Request& request,
          Core::Http::Policies::NextHttpPolicy nextPolicy,
          Core::Context const& context) const override
      {
        (void)nextPolicy;
        (void)context;

        const auto query = request.GetUrl().GetQueryParameters();
        const bool isLayout = query.find("comp") != query.end() && query.at("comp") == "layout";
        const auto& headers = request.GetHeaders();
        const auto hostHeader = headers.find("host");
        const auto ifMatchHeader = headers.find("if-match");
        const auto rangeHeader = headers.find("x-ms-range");

        {
          std::lock_guard<std::mutex> lock(m_state->Mutex);
          m_state->Requests.push_back(LocalityRequest{
              request.GetUrl().GetHost(),
              hostHeader == headers.end() ? std::string() : hostHeader->second,
              ifMatchHeader == headers.end() ? std::string() : ifMatchHeader->second,
              rangeHeader == headers.end() ? std::string() : rangeHeader->second,
              isLayout});
        }

        if (isLayout)
        {
          auto setLayoutPropertyHeaders = [&](Core::Http::RawResponse& response) {
            response.SetHeader("etag", "\"locality-etag\"");
            response.SetHeader("last-modified", "Thu, 23 Aug 2001 07:00:00 GMT");
            response.SetHeader("x-ms-creation-time", "Thu, 22 Aug 2002 07:00:00 GMT");
            response.SetHeader("x-ms-meta-layout", "metadata");
            response.SetHeader("x-ms-blob-content-length", std::to_string(m_data->size()));
            response.SetHeader("x-ms-blob-content-type", "application/octet-stream");
            response.SetHeader("x-ms-blob-content-encoding", "identity");
            response.SetHeader("x-ms-blob-type", "BlockBlob");
            response.SetHeader("x-ms-server-encrypted", "true");
          };
          if (m_state->LayoutStatus == Core::Http::HttpStatusCode::NoContent)
          {
            auto response = std::make_unique<Core::Http::RawResponse>(
                1, 1, Core::Http::HttpStatusCode::NoContent, "No Content");
            setLayoutPropertyHeaders(*response);
            return response;
          }
          if (m_state->LayoutStatus != Core::Http::HttpStatusCode::Ok)
          {
            const std::string errorBody
                = "<Error><Code>MockLayoutError</Code><Message>layout error</Message></Error>";
            auto response = std::make_unique<Core::Http::RawResponse>(
                1, 1, m_state->LayoutStatus, "layout error");
            response->SetBody(std::vector<uint8_t>(errorBody.begin(), errorBody.end()));
            response->SetHeader("content-type", "application/xml");
            response->SetHeader("x-ms-error-code", "MockLayoutError");
            return response;
          }

          const bool secondPage
              = query.find("marker") != query.end() && query.at("marker") == "page2";
          std::string ranges;
          std::string nextMarker;
          if (!m_state->PaginateLayout)
          {
            ranges = "<Range Start=\"0\" End=\"262143\" EndpointIndex=\"0\"/>"
                     "<Range Start=\"262144\" End=\"524287\" EndpointIndex=\"1\"/>"
                     "<Range Start=\"524288\" End=\"786431\" EndpointIndex=\"0\"/>"
                     "<Range Start=\"786432\" End=\"1048575\" EndpointIndex=\"1\"/>";
          }
          else if (!secondPage)
          {
            ranges = "<Range Start=\"0\" End=\"262143\" EndpointIndex=\"0\"/>"
                     "<Range Start=\"262144\" End=\"524287\" EndpointIndex=\"1\"/>";
            nextMarker = "page2";
          }
          else
          {
            ranges = "<Range Start=\"524288\" End=\"786431\" EndpointIndex=\"0\"/>"
                     "<Range Start=\"786432\" End=\"1048575\" EndpointIndex=\"1\"/>";
          }
          const std::string layout = "<BlobLayout><Ranges>" + ranges
              + "</Ranges><Endpoints>"
                "<Endpoint Index=\"0\" Value=\"locality0.test:443\"/>"
                "<Endpoint Index=\"1\" Value=\"locality1.test:443\"/>"
                "</Endpoints><Marker></Marker><NextMarker>"
              + nextMarker + "</NextMarker><MaxResults>4</MaxResults></BlobLayout>";
          auto response = std::make_unique<Core::Http::RawResponse>(
              1, 1, Core::Http::HttpStatusCode::Ok, "OK");
          response->SetBody(std::vector<uint8_t>(layout.begin(), layout.end()));
          response->SetHeader("content-type", "application/xml");
          setLayoutPropertyHeaders(*response);
          return response;
        }

        int64_t offset = 0;
        int64_t length = static_cast<int64_t>(m_data->size());
        if (rangeHeader != headers.end())
        {
          const auto separator = rangeHeader->second.find('-');
          offset = std::stoll(rangeHeader->second.substr(6, separator - 6));
          const auto end = std::stoll(rangeHeader->second.substr(separator + 1));
          length = (std::min)(end - offset + 1, static_cast<int64_t>(m_data->size()) - offset);
        }

        auto response = std::make_unique<Core::Http::RawResponse>(
            1,
            1,
            rangeHeader == headers.end() ? Core::Http::HttpStatusCode::Ok
                                         : Core::Http::HttpStatusCode::PartialContent,
            "OK");
        response->SetBodyStream(std::make_unique<Core::IO::MemoryBodyStream>(
            reinterpret_cast<const uint8_t*>(m_data->data() + offset),
            static_cast<size_t>(length)));
        response->SetHeader("content-length", std::to_string(length));
        if (rangeHeader != headers.end())
        {
          response->SetHeader(
              "content-range",
              "bytes " + std::to_string(offset) + "-" + std::to_string(offset + length - 1) + "/"
                  + std::to_string(m_data->size()));
        }
        response->SetHeader("etag", "\"locality-etag\"");
        response->SetHeader("last-modified", "Thu, 23 Aug 2001 07:00:00 GMT");
        response->SetHeader("x-ms-creation-time", "Thu, 22 Aug 2002 07:00:00 GMT");
        response->SetHeader("x-ms-blob-content-length", std::to_string(m_data->size()));
        response->SetHeader("x-ms-blob-type", "BlockBlob");
        response->SetHeader("x-ms-server-encrypted", "true");
        if (m_state->ReturnDownloadHint)
        {
          response->SetHeader("x-ms-download-hint", m_state->DownloadHint);
        }
        return response;
      }

    private:
      std::shared_ptr<LocalityState> m_state;
      std::shared_ptr<std::string> m_data;
    };

    Blobs::BlobClient CreateLocalityClient(
        const std::shared_ptr<LocalityState>& state,
        const std::shared_ptr<std::string>& data)
    {
      Blobs::BlobClientOptions options;
      options.PerRetryPolicies.emplace_back(std::make_unique<LocalityTransportPolicy>(state, data));
      return Blobs::BlobClient("https://primary.test/container/blob", options);
    }

    Blobs::BlobClient CreateLocalityClientFromService(
        const std::shared_ptr<LocalityState>& state,
        const std::shared_ptr<std::string>& data)
    {
      Blobs::BlobClientOptions options;
      options.PerRetryPolicies.emplace_back(std::make_unique<LocalityTransportPolicy>(state, data));
      return Blobs::BlobServiceClient("https://primary.test", options)
          .GetBlobContainerClient("container")
          .GetBlobClient("blob");
    }

    Blobs::DownloadBlobToOptions CreateDownloadOptions()
    {
      Blobs::DownloadBlobToOptions options;
      options.LayoutAwareRouting = Blobs::LayoutAwareRouting::Enabled;
      options.TransferOptions.InitialChunkSize = 128 * 1024;
      options.TransferOptions.ChunkSize = 64 * 1024;
      options.TransferOptions.Concurrency = 4;
      return options;
    }

    Blobs::Models::BlobLayoutRange MakeLayoutRange(
        int64_t offset,
        int64_t length,
        std::string endpoint)
    {
      Blobs::Models::BlobLayoutRange range;
      range.Range.Offset = offset;
      range.Range.Length = length;
      range.Endpoint = std::move(endpoint);
      return range;
    }

    Blobs::_detail::DataLocalityLayout CreateLayout(const std::string& endpoint)
    {
      Blobs::_detail::DataLocalityLayout layout;
      layout.Ranges = {MakeLayoutRange(0, 1024 * 1024, endpoint)};
      return layout;
    }
  } // namespace

  class DataLocalityRecordedTest : public StorageTest {
  };

  TEST(DataLocalityTest, SelectsIdealEndpoint)
  {
    using Range = Blobs::Models::BlobLayoutRange;
    struct TestCase final
    {
      const char* Name;
      int64_t Offset;
      int64_t Length;
      std::vector<Range> Ranges;
      std::string ExpectedEndpoint;
    };

    const auto maxOffset = (std::numeric_limits<int64_t>::max)();
    const std::vector<TestCase> testCases{
        {"NegativeOffset", -1, 1, {MakeLayoutRange(0, 10, "A")}, ""},
        {"ZeroLength", 0, 0, {MakeLayoutRange(0, 10, "A")}, ""},
        {"NegativeLength", 0, -1, {MakeLayoutRange(0, 10, "A")}, ""},
        {"EmptyLayout", 0, 1, {}, ""},
        {"BeforeLayout", 0, 5, {MakeLayoutRange(10, 10, "A")}, ""},
        {"AfterLayout", 20, 1, {MakeLayoutRange(10, 10, "A")}, ""},
        {"RequestStartsInGap", 5, 10, {MakeLayoutRange(10, 10, "A")}, "A"},
        {"ExactRangeStart", 10, 1, {MakeLayoutRange(10, 10, "A")}, "A"},
        {"ExactRangeEnd", 19, 1, {MakeLayoutRange(10, 10, "A")}, "A"},
        {"SkipsEarlierRanges",
         100,
         1,
         {MakeLayoutRange(0, 10, "A"), MakeLayoutRange(100, 10, "B")},
         "B"},
        {"LargestOverlap",
         200 * 1024,
         400 * 1024,
         {MakeLayoutRange(0, 256 * 1024, "A"),
          MakeLayoutRange(256 * 1024, 256 * 1024, "B"),
          MakeLayoutRange(512 * 1024, 256 * 1024, "A")},
         "B"},
        {"AggregatesDisjointRanges",
         5,
         25,
         {MakeLayoutRange(0, 10, "A"), MakeLayoutRange(10, 10, "B"), MakeLayoutRange(20, 10, "A")},
         "A"},
        {"IgnoresLayoutGaps",
         0,
         30,
         {MakeLayoutRange(0, 5, "A"), MakeLayoutRange(20, 10, "B")},
         "B"},
        {"RequestEndAtMax", maxOffset - 1, 2, {MakeLayoutRange(maxOffset - 9, 10, "A")}, "A"},
        {"SingleByteAtMax", maxOffset, 1, {MakeLayoutRange(maxOffset, 1, "A")}, "A"},
        {"RequestEndOverflow", maxOffset - 4, 10, {MakeLayoutRange(maxOffset - 9, 10, "A")}, "A"},
    };

    for (const auto& testCase : testCases)
    {
      SCOPED_TRACE(testCase.Name);
      Blobs::_detail::DataLocalityLayout layout;
      layout.Ranges = testCase.Ranges;
      EXPECT_EQ(
          Blobs::_detail::GetIdealDataLocalityEndpoint(testCase.Offset, testCase.Length, layout),
          testCase.ExpectedEndpoint);
    }
  }

  TEST(DataLocalityTest, SelectsEitherEndpointForEqualOverlap)
  {
    Blobs::_detail::DataLocalityLayout layout;
    layout.Ranges = {MakeLayoutRange(0, 10, "A"), MakeLayoutRange(10, 10, "B")};

    const auto endpoint = Blobs::_detail::GetIdealDataLocalityEndpoint(5, 10, layout);
    EXPECT_TRUE(endpoint == "A" || endpoint == "B");
  }

  TEST(DataLocalityTest, FreshLayoutDoesNotRefresh)
  {
    std::atomic<int> refreshCount{0};
    Blobs::_detail::DataLocalityLayoutState state([&]() {
      ++refreshCount;
      return CreateLayout("A");
    });

    state.WaitForInitialLayout();
    EXPECT_EQ(refreshCount, 1);
    EXPECT_EQ(state.GetEndpoint(0, 1), "A");
    EXPECT_EQ(refreshCount, 1);
  }

  TEST(DataLocalityTest, GetLayoutNoContentReturnsBlobProperties)
  {
    auto state = std::make_shared<LocalityState>();
    state->LayoutStatus = Core::Http::HttpStatusCode::NoContent;
    auto data = std::make_shared<std::string>(1024 * 1024, 'n');
    auto client = CreateLocalityClient(state, data);

    auto page = client.GetLayout();

    EXPECT_TRUE(page.Layout.Ranges.empty());
    EXPECT_EQ(page.Layout.Properties.ETag.ToString(), "\"locality-etag\"");
    EXPECT_EQ(page.Layout.Properties.BlobSize, static_cast<int64_t>(data->size()));
    EXPECT_EQ(page.Layout.Properties.Metadata.at("layout"), "metadata");
    EXPECT_EQ(page.RawResponse->GetStatusCode(), Core::Http::HttpStatusCode::NoContent);
  }

  TEST_F(DataLocalityRecordedTest, GetLayoutMultipleEndpoints_PLAYBACKONLY_)
  {
    auto clientOptions = InitStorageClientOptions<Blobs::BlobClientOptions>();
    auto blobClient = Blobs::BlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        "data-locality-test",
        "data-locality-multi-endpoint.bin",
        clientOptions);

    std::vector<Blobs::Models::BlobLayoutRange> ranges;
    std::set<std::string> endpoints;
    auto page = blobClient.GetLayout();
    EXPECT_EQ(page.Layout.Properties.BlobSize, 64 * 1024 * 1024);
    EXPECT_EQ(page.Layout.Properties.BlobType, Blobs::Models::BlobType::BlockBlob);
    EXPECT_EQ(page.Layout.Properties.HttpHeaders.ContentType, "application/octet-stream");
    EXPECT_TRUE(page.Layout.Properties.IsServerEncrypted);
    for (; page.HasPage(); page.MoveToNextPage())
    {
      for (const auto& range : page.Layout.Ranges)
      {
        EXPECT_GE(range.Range.Offset, 0);
        ASSERT_TRUE(range.Range.Length.HasValue());
        EXPECT_GT(range.Range.Length.Value(), 0);
        EXPECT_FALSE(range.Endpoint.empty());
        ranges.push_back(range);
        endpoints.insert(range.Endpoint);
      }
    }

    if (m_testContext.IsLiveMode() && endpoints.size() < 2)
    {
      GTEST_SKIP() << "The configured live account does not return a multi-endpoint layout.";
    }
    ASSERT_GT(ranges.size(), 1U);
    EXPECT_GE(endpoints.size(), 2U);

    const auto differentEndpoint
        = std::adjacent_find(ranges.begin(), ranges.end(), [](auto const& left, auto const& right) {
            return left.Endpoint != right.Endpoint;
          });
    ASSERT_NE(differentEndpoint, ranges.end());
    const auto nextRange = differentEndpoint + 1;
    ASSERT_GE(nextRange->Range.Offset, 2);

    constexpr int64_t downloadLength = 5;
    const int64_t downloadOffset = nextRange->Range.Offset - 2;
    Blobs::DownloadBlobOptions baselineOptions;
    baselineOptions.Range = Core::Http::HttpRange();
    baselineOptions.Range.Value().Offset = downloadOffset;
    baselineOptions.Range.Value().Length = downloadLength;
    auto downloadResponse = blobClient.Download(baselineOptions);
    auto content = downloadResponse.Value.BodyStream->ReadToEnd();
    ASSERT_EQ(content.size(), static_cast<size_t>(downloadLength));
    ASSERT_TRUE(downloadResponse.Value.Details.DownloadHint.HasValue());
    EXPECT_EQ(
        downloadResponse.Value.Details.DownloadHint.Value(), Blobs::Models::DownloadHint::Layout);
  }

  TEST_F(DataLocalityRecordedTest, GetLayoutRangeReturnsFullBlobSize_PLAYBACKONLY_)
  {
    auto clientOptions = InitStorageClientOptions<Blobs::BlobClientOptions>();
    auto client = Blobs::BlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(),
        "data-locality-test",
        "data-locality-multi-endpoint.bin",
        clientOptions);
    constexpr int64_t blobSize = 64 * 1024 * 1024;
    for (bool bounded : {true, false})
    {
      SCOPED_TRACE(bounded);
      Blobs::GetBlobLayoutOptions options;
      options.Range = Core::Http::HttpRange();
      options.Range.Value().Offset = bounded ? 8 * 1024 * 1024 - 2 : blobSize - 16;
      if (bounded)
      {
        options.Range.Value().Length = 5;
      }
      const auto offset = options.Range.Value().Offset;
      bool coversOffset = false;
      size_t rangeCount = 0;
      for (auto page = client.GetLayout(options); page.HasPage(); page.MoveToNextPage())
      {
        EXPECT_EQ(page.Layout.Properties.BlobSize, blobSize);
        EXPECT_GT(page.Layout.Properties.BlobSize, options.Range.Value().Length.ValueOr(16));
        EXPECT_TRUE(page.Layout.Properties.ETag.HasValue());
        for (const auto& range : page.Layout.Ranges)
        {
          ASSERT_TRUE(range.Range.Length.HasValue());
          ASSERT_GT(range.Range.Length.Value(), 0);
          ASSERT_GE(range.Range.Offset, 0);
          ASSERT_LE(range.Range.Offset, blobSize);
          ASSERT_LE(range.Range.Length.Value(), blobSize - range.Range.Offset);
          EXPECT_FALSE(range.Endpoint.empty());
          coversOffset = coversOffset
              || (range.Range.Offset <= offset
                  && offset - range.Range.Offset < range.Range.Length.Value());
          ++rangeCount;
        }
      }
      EXPECT_GT(rangeCount, 0U);
      EXPECT_TRUE(coversOffset);
    }
  }

  TEST(DataLocalityTest, RoutesDownloadChunks)
  {
    auto state = std::make_shared<LocalityState>();
    auto data = std::make_shared<std::string>(1024 * 1024, '\0');
    for (size_t i = 0; i < data->size(); ++i)
    {
      (*data)[i] = static_cast<char>(i % 127);
    }
    auto client = CreateLocalityClient(state, data);

    std::vector<uint8_t> buffer(data->size());
    auto response = client.DownloadTo(buffer.data(), buffer.size(), CreateDownloadOptions());

    EXPECT_EQ(response.Value.BlobSize, static_cast<int64_t>(data->size()));
    EXPECT_TRUE(std::equal(data->begin(), data->end(), buffer.begin()));

    std::lock_guard<std::mutex> lock(state->Mutex);
    ASSERT_EQ(state->Requests.size(), 16U);
    EXPECT_FALSE(state->Requests[0].IsLayout);
    EXPECT_EQ(state->Requests[0].Host, "primary.test");
    EXPECT_TRUE(state->Requests[0].Range.empty());
    EXPECT_TRUE(state->Requests[1].IsLayout);
    EXPECT_EQ(state->Requests[1].Range, "bytes=131072-1048575");
    EXPECT_EQ(state->Requests[1].IfMatch, "\"locality-etag\"");
    EXPECT_EQ(state->Requests.front().Host, "primary.test");
    for (auto request = state->Requests.begin() + 2; request != state->Requests.end(); ++request)
    {
      EXPECT_FALSE(request->IsLayout);
      EXPECT_TRUE(request->Host == "locality0.test" || request->Host == "locality1.test");
      EXPECT_EQ(request->HostHeader, "primary.test");
      EXPECT_EQ(request->IfMatch, "\"locality-etag\"");
      EXPECT_FALSE(request->Range.empty());
      const auto separator = request->Range.find('-');
      const auto offset = std::stoll(request->Range.substr(6, separator - 6));
      EXPECT_EQ(
          request->Host, (offset / (256 * 1024)) % 2 == 0 ? "locality0.test" : "locality1.test");
    }
  }

  TEST(DataLocalityTest, RoutesFileDownloadChunks)
  {
    auto state = std::make_shared<LocalityState>();
    auto data = std::make_shared<std::string>(1024 * 1024, 'f');
    auto client = CreateLocalityClientFromService(state, data);
    const std::string fileName = Core::Uuid::CreateUuid().ToString() + ".tmp";

    auto response = client.DownloadTo(fileName, CreateDownloadOptions());
    EXPECT_EQ(response.Value.BlobSize, static_cast<int64_t>(data->size()));

    std::ifstream file(fileName, std::ios::binary);
    const std::string downloaded((std::istreambuf_iterator<char>(file)), {});
    file.close();
    std::remove(fileName.c_str());
    EXPECT_EQ(downloaded, *data);

    std::lock_guard<std::mutex> lock(state->Mutex);
    ASSERT_EQ(state->Requests.size(), 16U);
    EXPECT_FALSE(state->Requests[0].IsLayout);
    EXPECT_EQ(state->Requests[0].Host, "primary.test");
    EXPECT_TRUE(state->Requests[0].Range.empty());
    EXPECT_TRUE(state->Requests[1].IsLayout);
    for (auto request = state->Requests.begin() + 2; request != state->Requests.end(); ++request)
    {
      EXPECT_TRUE(request->Host == "locality0.test" || request->Host == "locality1.test");
      EXPECT_EQ(request->HostHeader, "primary.test");
      EXPECT_EQ(request->IfMatch, "\"locality-etag\"");
    }
  }

  TEST(DataLocalityTest, CompletesSmallBufferDownloadBeforeLayout)
  {
    auto state = std::make_shared<LocalityState>();
    auto data = std::make_shared<std::string>(4 * 1024, 's');
    auto client = CreateLocalityClient(state, data);

    std::vector<uint8_t> buffer(data->size());
    client.DownloadTo(buffer.data(), buffer.size(), CreateDownloadOptions());
    EXPECT_TRUE(std::equal(buffer.begin(), buffer.end(), data->begin()));

    std::lock_guard<std::mutex> lock(state->Mutex);
    ASSERT_EQ(state->Requests.size(), 1U);
    EXPECT_FALSE(state->Requests.front().IsLayout);
    EXPECT_EQ(state->Requests.front().Host, "primary.test");
    EXPECT_TRUE(state->Requests.front().Range.empty());
  }

  TEST(DataLocalityTest, DownloadsEmptyBlobWithoutRange)
  {
    auto data = std::make_shared<std::string>();

    {
      auto state = std::make_shared<LocalityState>();
      auto client = CreateLocalityClient(state, data);
      std::vector<uint8_t> buffer;

      auto response = client.DownloadTo(buffer.data(), buffer.size(), CreateDownloadOptions());
      EXPECT_EQ(response.Value.BlobSize, 0);
      EXPECT_EQ(response.Value.ContentRange.Length.Value(), 0);

      std::lock_guard<std::mutex> lock(state->Mutex);
      ASSERT_EQ(state->Requests.size(), 1U);
      EXPECT_FALSE(state->Requests.front().IsLayout);
      EXPECT_TRUE(state->Requests.front().Range.empty());
    }

    {
      auto state = std::make_shared<LocalityState>();
      auto client = CreateLocalityClient(state, data);
      const std::string fileName = Core::Uuid::CreateUuid().ToString() + ".tmp";

      auto response = client.DownloadTo(fileName, CreateDownloadOptions());
      EXPECT_EQ(response.Value.BlobSize, 0);
      EXPECT_EQ(response.Value.ContentRange.Length.Value(), 0);

      std::ifstream file(fileName, std::ios::binary);
      const std::string downloaded((std::istreambuf_iterator<char>(file)), {});
      file.close();
      std::remove(fileName.c_str());
      EXPECT_TRUE(downloaded.empty());

      std::lock_guard<std::mutex> lock(state->Mutex);
      ASSERT_EQ(state->Requests.size(), 1U);
      EXPECT_FALSE(state->Requests.front().IsLayout);
      EXPECT_TRUE(state->Requests.front().Range.empty());
    }
  }

  TEST(DataLocalityTest, CompletesSingleRangeDownloadBeforeLayout)
  {
    auto state = std::make_shared<LocalityState>();
    auto data = std::make_shared<std::string>(1024 * 1024, 'o');
    auto client = CreateLocalityClient(state, data);
    constexpr int64_t offset = 200 * 1024;
    constexpr int64_t length = 400 * 1024;
    auto options = CreateDownloadOptions();
    options.Range = Core::Http::HttpRange();
    options.Range.Value().Offset = offset;
    options.Range.Value().Length = length;
    options.TransferOptions.InitialChunkSize = length;

    std::vector<uint8_t> buffer(length);
    client.DownloadTo(buffer.data(), buffer.size(), options);
    EXPECT_TRUE(std::equal(buffer.begin(), buffer.end(), data->begin() + offset));

    std::lock_guard<std::mutex> lock(state->Mutex);
    ASSERT_EQ(state->Requests.size(), 1U);
    EXPECT_FALSE(state->Requests.front().IsLayout);
    EXPECT_EQ(state->Requests.front().Range, "bytes=204800-614399");
    EXPECT_EQ(state->Requests.front().Host, "primary.test");
  }

  TEST(DataLocalityTest, CompletesSmallFileDownloadBeforeLayout)
  {
    auto state = std::make_shared<LocalityState>();
    auto data = std::make_shared<std::string>(4 * 1024, 'f');
    auto client = CreateLocalityClient(state, data);
    const std::string fileName = Core::Uuid::CreateUuid().ToString() + ".tmp";
    auto options = CreateDownloadOptions();
    options.Range = Core::Http::HttpRange();
    options.Range.Value().Offset = 0;
    options.Range.Value().Length = static_cast<int64_t>(data->size());

    client.DownloadTo(fileName, options);
    std::ifstream file(fileName, std::ios::binary);
    const std::string downloaded((std::istreambuf_iterator<char>(file)), {});
    file.close();
    std::remove(fileName.c_str());
    EXPECT_EQ(downloaded, *data);

    std::lock_guard<std::mutex> lock(state->Mutex);
    ASSERT_EQ(state->Requests.size(), 1U);
    EXPECT_FALSE(state->Requests.front().IsLayout);
    EXPECT_EQ(state->Requests.front().Host, "primary.test");
  }

  TEST(DataLocalityTest, DoesNotFetchLayoutWithoutDownloadHint)
  {
    auto state = std::make_shared<LocalityState>();
    state->ReturnDownloadHint = false;
    auto data = std::make_shared<std::string>(1024 * 1024, 'h');
    auto client = CreateLocalityClient(state, data);

    std::vector<uint8_t> buffer(data->size());
    client.DownloadTo(buffer.data(), buffer.size(), CreateDownloadOptions());
    EXPECT_TRUE(std::equal(buffer.begin(), buffer.end(), data->begin()));

    std::lock_guard<std::mutex> lock(state->Mutex);
    ASSERT_GT(state->Requests.size(), 1U);
    for (const auto& request : state->Requests)
    {
      EXPECT_FALSE(request.IsLayout);
      EXPECT_EQ(request.Host, "primary.test");
    }
  }

  TEST(DataLocalityTest, AutoEnablesLayoutAwareRouting)
  {
    auto state = std::make_shared<LocalityState>();
    auto data = std::make_shared<std::string>(1024 * 1024, 'a');
    auto client = CreateLocalityClient(state, data);
    auto options = CreateDownloadOptions();
    options.LayoutAwareRouting = Blobs::LayoutAwareRouting::Auto;

    std::vector<uint8_t> buffer(data->size());
    client.DownloadTo(buffer.data(), buffer.size(), options);

    std::lock_guard<std::mutex> lock(state->Mutex);
    ASSERT_GT(state->Requests.size(), 1U);
    EXPECT_TRUE(state->Requests[1].IsLayout);
  }

  TEST(DataLocalityTest, DefaultAndDisabledSkipLayoutAwareRouting)
  {
    for (bool useDefault : {true, false})
    {
      for (bool toFile : {false, true})
      {
        SCOPED_TRACE(useDefault);
        SCOPED_TRACE(toFile);
        auto state = std::make_shared<LocalityState>();
        auto data = std::make_shared<std::string>(1024 * 1024, 'd');
        auto client = CreateLocalityClient(state, data);
        Blobs::DownloadBlobToOptions options;
        EXPECT_EQ(options.LayoutAwareRouting, Blobs::LayoutAwareRouting::Disabled);
        options.TransferOptions = CreateDownloadOptions().TransferOptions;
        if (!useDefault)
        {
          options.LayoutAwareRouting = Blobs::LayoutAwareRouting::Disabled;
        }

        std::vector<uint8_t> content(data->size());
        if (toFile)
        {
          const std::string fileName = Core::Uuid::CreateUuid().ToString() + ".tmp";
          client.DownloadTo(fileName, options);
          std::ifstream file(fileName, std::ios::binary);
          content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
          file.close();
          std::remove(fileName.c_str());
        }
        else
        {
          client.DownloadTo(content.data(), content.size(), options);
        }
        EXPECT_TRUE(std::equal(content.begin(), content.end(), data->begin(), data->end()));

        std::lock_guard<std::mutex> lock(state->Mutex);
        ASSERT_GT(state->Requests.size(), 1U);
        for (const auto& request : state->Requests)
        {
          EXPECT_FALSE(request.IsLayout);
          EXPECT_EQ(request.Host, "primary.test");
        }
      }
    }
  }

  TEST(DataLocalityTest, AcceptsCaseInsensitiveDownloadHint)
  {
    auto state = std::make_shared<LocalityState>();
    state->DownloadHint = "LAYOUT";
    auto data = std::make_shared<std::string>(1024 * 1024, 'h');
    auto client = CreateLocalityClient(state, data);

    std::vector<uint8_t> buffer(data->size());
    client.DownloadTo(buffer.data(), buffer.size(), CreateDownloadOptions());
    EXPECT_TRUE(std::equal(buffer.begin(), buffer.end(), data->begin()));

    std::lock_guard<std::mutex> lock(state->Mutex);
    ASSERT_GT(state->Requests.size(), 1U);
    EXPECT_FALSE(state->Requests[0].IsLayout);
    EXPECT_TRUE(state->Requests[1].IsLayout);
  }

  TEST(DataLocalityTest, RoutesToEndpointWithLargestOverlap)
  {
    auto state = std::make_shared<LocalityState>();
    auto data = std::make_shared<std::string>(1024 * 1024, 'o');
    auto client = CreateLocalityClient(state, data);
    constexpr int64_t offset = 200 * 1024;
    constexpr int64_t length = 400 * 1024;
    constexpr int64_t initialLength = 64 * 1024;
    auto options = CreateDownloadOptions();
    options.Range = Core::Http::HttpRange();
    options.Range.Value().Offset = offset;
    options.Range.Value().Length = length;
    options.TransferOptions.InitialChunkSize = initialLength;
    options.TransferOptions.ChunkSize = length - initialLength;

    std::vector<uint8_t> buffer(length);
    client.DownloadTo(buffer.data(), buffer.size(), options);
    EXPECT_TRUE(std::equal(buffer.begin(), buffer.end(), data->begin() + offset));

    std::lock_guard<std::mutex> lock(state->Mutex);
    ASSERT_EQ(state->Requests.size(), 3U);
    EXPECT_FALSE(state->Requests[0].IsLayout);
    EXPECT_EQ(state->Requests[0].Range, "bytes=204800-270335");
    EXPECT_TRUE(state->Requests[1].IsLayout);
    EXPECT_EQ(state->Requests[1].Range, "bytes=270336-614399");
    EXPECT_EQ(state->Requests[1].IfMatch, "\"locality-etag\"");
    EXPECT_FALSE(state->Requests[2].IsLayout);
    EXPECT_EQ(state->Requests[2].Range, "bytes=270336-614399");
    EXPECT_EQ(state->Requests[2].Host, "locality1.test");
  }

  TEST(DataLocalityTest, ReadsAllLayoutPages)
  {
    auto state = std::make_shared<LocalityState>();
    state->PaginateLayout = true;
    auto data = std::make_shared<std::string>(1024 * 1024, 'p');
    auto client = CreateLocalityClient(state, data);

    std::vector<uint8_t> buffer(data->size());
    client.DownloadTo(buffer.data(), buffer.size(), CreateDownloadOptions());
    EXPECT_TRUE(std::equal(data->begin(), data->end(), buffer.begin()));

    std::lock_guard<std::mutex> lock(state->Mutex);
    ASSERT_EQ(state->Requests.size(), 17U);
    EXPECT_FALSE(state->Requests[0].IsLayout);
    EXPECT_TRUE(state->Requests[1].IsLayout);
    EXPECT_TRUE(state->Requests[2].IsLayout);
    EXPECT_EQ(state->Requests[1].IfMatch, "\"locality-etag\"");
    EXPECT_EQ(state->Requests[2].IfMatch, "\"locality-etag\"");
  }

  TEST(DataLocalityTest, RoutesOneShotDownloadToLayoutEndpoint)
  {
    auto state = std::make_shared<LocalityState>();
    auto data = std::make_shared<std::string>(4 * 1024, 'o');
    auto client = CreateLocalityClient(state, data);

    Blobs::DownloadBlobOptions options;
    options.LayoutEndpoint = "locality0.test:443";
    auto response = client.Download(options);

    EXPECT_TRUE(response.Value.Details.DownloadHint.HasValue());
    EXPECT_EQ(response.Value.Details.DownloadHint.Value(), Blobs::Models::DownloadHint::Layout);

    std::lock_guard<std::mutex> lock(state->Mutex);
    ASSERT_EQ(state->Requests.size(), 1U);
    EXPECT_FALSE(state->Requests.front().IsLayout);
    EXPECT_EQ(state->Requests.front().Host, "locality0.test");
    EXPECT_EQ(state->Requests.front().HostHeader, "primary.test");
  }

  TEST(DataLocalityTest, FallsBackOnUnsupportedLayout)
  {
    for (const auto status :
         {Core::Http::HttpStatusCode::BadRequest, Core::Http::HttpStatusCode::InternalServerError})
    {
      auto state = std::make_shared<LocalityState>();
      state->LayoutStatus = status;
      auto data = std::make_shared<std::string>(1024 * 1024, 'x');
      auto client = CreateLocalityClient(state, data);

      std::vector<uint8_t> buffer(data->size());
      EXPECT_NO_THROW(client.DownloadTo(buffer.data(), buffer.size(), CreateDownloadOptions()));
      EXPECT_TRUE(std::equal(data->begin(), data->end(), buffer.begin()));

      std::lock_guard<std::mutex> lock(state->Mutex);
      ASSERT_GT(state->Requests.size(), 1U);
      EXPECT_FALSE(state->Requests[0].IsLayout);
      EXPECT_TRUE(state->Requests[1].IsLayout);
      for (auto request = state->Requests.begin(); request != state->Requests.end(); ++request)
      {
        if (!request->IsLayout)
        {
          EXPECT_EQ(request->Host, "primary.test");
          EXPECT_TRUE(request->HostHeader.empty());
        }
      }
    }
  }

  TEST(DataLocalityTest, PropagatesNonFallbackLayoutError)
  {
    auto state = std::make_shared<LocalityState>();
    state->LayoutStatus = Core::Http::HttpStatusCode::NotFound;
    auto data = std::make_shared<std::string>(1024 * 1024, 'x');
    auto client = CreateLocalityClient(state, data);

    std::vector<uint8_t> buffer(data->size());
    EXPECT_THROW(
        client.DownloadTo(buffer.data(), buffer.size(), CreateDownloadOptions()), StorageException);
  }

  TEST(DataLocalityTest, FallsBackWhenBlobHasNoLayout)
  {
    auto state = std::make_shared<LocalityState>();
    state->LayoutStatus = Core::Http::HttpStatusCode::NoContent;
    auto data = std::make_shared<std::string>(1024 * 1024, 'x');
    auto client = CreateLocalityClient(state, data);

    std::vector<uint8_t> buffer(data->size());
    EXPECT_NO_THROW(client.DownloadTo(buffer.data(), buffer.size(), CreateDownloadOptions()));
    EXPECT_TRUE(std::equal(data->begin(), data->end(), buffer.begin()));

    std::lock_guard<std::mutex> lock(state->Mutex);
    ASSERT_GT(state->Requests.size(), 1U);
    for (auto request = state->Requests.begin() + 1; request != state->Requests.end(); ++request)
    {
      EXPECT_EQ(request->Host, "primary.test");
    }
  }

}}} // namespace Azure::Storage::Test
