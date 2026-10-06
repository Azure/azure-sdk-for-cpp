// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "test/ut/test_base.hpp"

#include <azure/storage/files/datalake.hpp>

#include <cstdio>
#include <fstream>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Test {

  namespace {
    namespace DataLake = Files::DataLake;

    struct LocalityRequest final
    {
      std::string Host;
      std::string HostHeader;
      std::string Range;
      std::string IfMatch;
      std::string Marker;
      bool IsLayout;
    };

    struct LocalityState final
    {
      std::vector<LocalityRequest> Requests;
      bool NoLayout = false;
    };

    class LocalityTransportPolicy final : public Core::Http::Policies::HttpPolicy {
    public:
      explicit LocalityTransportPolicy(std::shared_ptr<LocalityState> state)
          : m_state(std::move(state))
      {
      }

      std::unique_ptr<HttpPolicy> Clone() const override
      {
        return std::make_unique<LocalityTransportPolicy>(*this);
      }

      std::unique_ptr<Core::Http::RawResponse> Send(
          Core::Http::Request& request,
          Core::Http::Policies::NextHttpPolicy,
          Core::Context const&) const override
      {
        const auto& query = request.GetUrl().GetQueryParameters();
        const auto& headers = request.GetHeaders();
        const auto header = [&](const char* name) {
          auto value = headers.find(name);
          return value == headers.end() ? std::string() : value->second;
        };
        const bool isLayout = query.count("comp") && query.at("comp") == "layout";
        const auto marker = query.count("marker") ? query.at("marker") : std::string();
        m_state->Requests.push_back(LocalityRequest{
            request.GetUrl().GetHost(),
            header("host"),
            header("x-ms-range"),
            header("if-match"),
            marker,
            isLayout});

        auto response = std::make_unique<Core::Http::RawResponse>(
            1,
            1,
            isLayout
                ? (m_state->NoLayout ? Core::Http::HttpStatusCode::NoContent
                                     : Core::Http::HttpStatusCode::Ok)
                : Core::Http::HttpStatusCode::PartialContent,
            "OK");
        response->SetHeader("etag", "\"layout-etag\"");
        response->SetHeader("last-modified", "Thu, 23 Aug 2001 07:00:00 GMT");
        response->SetHeader("x-ms-creation-time", "Thu, 22 Aug 2002 07:00:00 GMT");
        response->SetHeader("x-ms-blob-type", "BlockBlob");
        response->SetHeader("x-ms-server-encrypted", "true");
        response->SetHeader("x-ms-blob-content-length", "12");
        response->SetHeader("x-ms-blob-content-type", "application/octet-stream");
        response->SetHeader("x-ms-meta-layout", "metadata");
        response->SetHeader("x-ms-owner", "owner");
        response->SetHeader("x-ms-group", "group");
        response->SetHeader("x-ms-permissions", "rw-r-----");
        response->SetHeader("x-ms-acl", "user::rw-,group::r--,other::---");
        response->SetHeader("x-ms-encryption-context", "context");
        if (isLayout)
        {
          if (!m_state->NoLayout)
          {
            const std::string xml = marker.empty()
                ? "<BlobLayout><Ranges><Range Start=\"0\" End=\"5\" EndpointIndex=\"0\"/>"
                  "</Ranges><Endpoints><Endpoint Index=\"0\" Value=\"locality0.test:443\"/>"
                  "</Endpoints><NextMarker>page2</NextMarker></BlobLayout>"
                : "<BlobLayout><Ranges><Range Start=\"6\" End=\"11\" EndpointIndex=\"0\"/>"
                  "</Ranges><Endpoints><Endpoint Index=\"0\" Value=\"locality1.test:443\"/>"
                  "</Endpoints><NextMarker/></BlobLayout>";
            response->SetBody(std::vector<uint8_t>(xml.begin(), xml.end()));
            response->SetHeader("content-type", "application/xml");
          }
          return response;
        }

        const auto range = header("x-ms-range");
        const auto separator = range.find('-');
        const auto offset = std::stoll(range.substr(6, separator - 6));
        const auto end = std::stoll(range.substr(separator + 1));
        static const std::string data = "abcdefghijkl";
        response->SetHeader("content-length", std::to_string(end - offset + 1));
        response->SetHeader(
            "content-range", "bytes " + std::to_string(offset) + "-" + std::to_string(end) + "/12");
        response->SetHeader("x-ms-download-hint", "layout");
        response->SetBodyStream(std::make_unique<Core::IO::MemoryBodyStream>(
            reinterpret_cast<const uint8_t*>(data.data() + offset),
            static_cast<size_t>(end - offset + 1)));
        return response;
      }

    private:
      std::shared_ptr<LocalityState> m_state;
    };

    DataLake::DataLakeFileClient CreateClient(
        const std::shared_ptr<LocalityState>& state,
        int origin)
    {
      DataLake::DataLakeClientOptions options;
      options.PerRetryPolicies.emplace_back(std::make_unique<LocalityTransportPolicy>(state));
      const std::string url = "https://account.dfs.core.windows.net";
      switch (origin)
      {
        case 0:
          return DataLake::DataLakeFileClient(url + "/filesystem/file", options);
        case 1:
          return DataLake::DataLakeServiceClient(url, options)
              .GetFileSystemClient("filesystem")
              .GetFileClient("file");
        case 2:
          return DataLake::DataLakeFileSystemClient(url + "/filesystem", options)
              .GetFileClient("file");
        default:
          return DataLake::DataLakeDirectoryClient(url + "/filesystem/directory", options)
              .GetFileClient("file");
      }
    }
  } // namespace

  class DataLakeLocalityRecordedTest : public StorageTest {
  };

  TEST_F(DataLakeLocalityRecordedTest, GetLayoutAndDownloadHint_PLAYBACKONLY_)
  {
    if (!m_testContext.IsPlaybackMode())
    {
      GTEST_SKIP() << "Reuses the Blob multi-endpoint recording.";
    }
    m_testContext.RecordingPath = std::string(AZURE_TEST_RECORDING_DIR)
        + "/../../../azure-storage-blobs/test/ut/recordings";
    m_testContext.RenameTest(
        "DataLocalityRecordedTest", "GetLayoutMultipleEndpoints_PLAYBACKONLY_");
    auto options = InitStorageClientOptions<DataLake::DataLakeClientOptions>();
    auto client = DataLake::DataLakeFileClient(
        "https://" + StandardStorageAccountName()
            + ".dfs.core.windows.net/data-locality-test/data-locality-multi-endpoint.bin",
        GetTestCredential(),
        options);
    auto page = client.GetLayout();
    EXPECT_EQ(page.Layout.Properties.FileSize, 64 * 1024 * 1024);
    EXPECT_EQ(page.Layout.Properties.HttpHeaders.ContentType, "application/octet-stream");
    ASSERT_TRUE(page.Layout.Properties.IsServerEncrypted.HasValue());
    EXPECT_TRUE(page.Layout.Properties.IsServerEncrypted.Value());
    EXPECT_TRUE(page.Layout.Properties.ETag.HasValue());
    EXPECT_FALSE(page.Layout.Properties.IsDirectory);
    std::set<std::string> endpoints;
    std::vector<DataLake::Models::FileLayoutRange> ranges;
    for (; page.HasPage(); page.MoveToNextPage())
    {
      for (const auto& range : page.Layout.Ranges)
      {
        ASSERT_TRUE(range.Range.Length.HasValue());
        EXPECT_GT(range.Range.Length.Value(), 0);
        EXPECT_FALSE(range.Endpoint.empty());
        ranges.push_back(range);
        endpoints.insert(range.Endpoint);
      }
    }
    EXPECT_GE(endpoints.size(), 2U);
    ASSERT_GT(ranges.size(), 1U);
    DataLake::DownloadFileOptions downloadOptions;
    downloadOptions.Range = Core::Http::HttpRange();
    downloadOptions.Range.Value().Offset = ranges[1].Range.Offset - 2;
    downloadOptions.Range.Value().Length = 5;
    auto response = client.Download(downloadOptions);
    EXPECT_EQ(response.Value.Body->ReadToEnd().size(), 5U);
    EXPECT_EQ(response.Value.FileSize, 64 * 1024 * 1024);
    ASSERT_TRUE(response.Value.Details.DownloadHint.HasValue());
    EXPECT_EQ(response.Value.Details.DownloadHint.Value(), Blobs::Models::DownloadHint::Layout);
  }

  TEST_F(DataLakeLocalityRecordedTest, GetLayoutRangeReturnsFullFileSize_PLAYBACKONLY_)
  {
    if (!m_testContext.IsPlaybackMode())
    {
      GTEST_SKIP() << "Reuses the Blob ranged layout recording.";
    }
    m_testContext.RecordingPath = std::string(AZURE_TEST_RECORDING_DIR)
        + "/../../../azure-storage-blobs/test/ut/recordings";
    m_testContext.RenameTest(
        "DataLocalityRecordedTest", "GetLayoutRangeReturnsFullBlobSize_PLAYBACKONLY_");
    auto clientOptions = InitStorageClientOptions<DataLake::DataLakeClientOptions>();
    auto client = DataLake::DataLakeFileClient(
        "https://" + StandardStorageAccountName()
            + ".dfs.core.windows.net/data-locality-test/data-locality-multi-endpoint.bin",
        GetTestCredential(),
        clientOptions);
    constexpr int64_t fileSize = 64 * 1024 * 1024;
    for (bool bounded : {true, false})
    {
      SCOPED_TRACE(bounded);
      DataLake::GetFileLayoutOptions options;
      options.Range = Core::Http::HttpRange();
      options.Range.Value().Offset = bounded ? 8 * 1024 * 1024 - 2 : fileSize - 16;
      if (bounded)
      {
        options.Range.Value().Length = 5;
      }
      const auto offset = options.Range.Value().Offset;
      bool coversOffset = false;
      size_t rangeCount = 0;
      for (auto page = client.GetLayout(options); page.HasPage(); page.MoveToNextPage())
      {
        EXPECT_EQ(page.Layout.Properties.FileSize, fileSize);
        EXPECT_GT(page.Layout.Properties.FileSize, options.Range.Value().Length.ValueOr(16));
        EXPECT_TRUE(page.Layout.Properties.ETag.HasValue());
        for (const auto& range : page.Layout.Ranges)
        {
          ASSERT_TRUE(range.Range.Length.HasValue());
          ASSERT_GT(range.Range.Length.Value(), 0);
          ASSERT_GE(range.Range.Offset, 0);
          ASSERT_LE(range.Range.Offset, fileSize);
          ASSERT_LE(range.Range.Length.Value(), fileSize - range.Range.Offset);
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

  TEST(DataLakeLocalityTest, GetLayoutMapsPropertiesAndPages)
  {
    auto state = std::make_shared<LocalityState>();
    auto client = CreateClient(state, 0);
    DataLake::GetFileLayoutOptions options;
    options.Range = Core::Http::HttpRange();
    options.Range.Value().Offset = 2;
    options.Range.Value().Length = 10;
    options.AccessConditions.IfMatch = ETag("\"layout-etag\"");
    auto page = client.GetLayout(options);
    EXPECT_EQ(page.CurrentPageToken, "");
    EXPECT_EQ(page.NextPageToken.Value(), "page2");
    ASSERT_EQ(page.Layout.Ranges.size(), 1U);
    EXPECT_EQ(page.Layout.Ranges[0].Endpoint, "locality0.test:443");
    EXPECT_EQ(page.Layout.Properties.FileSize, 12);
    EXPECT_EQ(page.Layout.Properties.Metadata.at("layout"), "metadata");
    EXPECT_EQ(page.Layout.Properties.Owner.Value(), "owner");
    EXPECT_EQ(page.Layout.Properties.Group.Value(), "group");
    EXPECT_EQ(page.Layout.Properties.Permissions.Value(), "rw-r-----");
    EXPECT_EQ(page.Layout.Properties.EncryptionContext.Value(), "context");
    ASSERT_TRUE(page.Layout.Properties.Acls.HasValue());
    EXPECT_EQ(page.Layout.Properties.Acls.Value().size(), 3U);
    page.MoveToNextPage();
    EXPECT_EQ(page.CurrentPageToken, "page2");
    ASSERT_EQ(page.Layout.Ranges.size(), 1U);
    EXPECT_EQ(page.Layout.Ranges[0].Range.Offset, 6);
    EXPECT_EQ(page.Layout.Ranges[0].Range.Length.Value(), 6);
    EXPECT_EQ(page.Layout.Ranges[0].Endpoint, "locality1.test:443");
    page.MoveToNextPage();
    EXPECT_FALSE(page.HasPage());
    ASSERT_EQ(state->Requests.size(), 2U);
    EXPECT_EQ(state->Requests[0].Host, "account.blob.core.windows.net");
    EXPECT_EQ(state->Requests[1].Range, "bytes=2-11");
    EXPECT_EQ(state->Requests[1].IfMatch, "\"layout-etag\"");
    EXPECT_EQ(state->Requests[1].Marker, "page2");
  }

  TEST(DataLakeLocalityTest, GetLayoutNoContent)
  {
    auto state = std::make_shared<LocalityState>();
    state->NoLayout = true;
    auto page = CreateClient(state, 0).GetLayout();
    EXPECT_TRUE(page.Layout.Ranges.empty());
    EXPECT_EQ(page.Layout.Properties.FileSize, 12);
    EXPECT_EQ(page.RawResponse->GetStatusCode(), Core::Http::HttpStatusCode::NoContent);
    page.MoveToNextPage();
    EXPECT_FALSE(page.HasPage());
  }

  TEST(DataLakeLocalityTest, RoutesOneShotDownloadForAllClientOrigins)
  {
    for (int origin = 0; origin != 4; ++origin)
    {
      SCOPED_TRACE(origin);
      auto state = std::make_shared<LocalityState>();
      auto client = CreateClient(state, origin);
      DataLake::DownloadFileOptions options;
      options.Range = Core::Http::HttpRange();
      options.Range.Value().Offset = 6;
      options.Range.Value().Length = 1;
      options.LayoutEndpoint = "locality1.test:443";
      auto response = client.Download(options);
      EXPECT_EQ(response.Value.Body->ReadToEnd(), std::vector<uint8_t>{'g'});
      ASSERT_EQ(state->Requests.size(), 1U);
      EXPECT_EQ(state->Requests[0].Host, "locality1.test");
      EXPECT_EQ(state->Requests[0].HostHeader, "account.blob.core.windows.net");
      ASSERT_TRUE(response.Value.Details.DownloadHint.HasValue());
      EXPECT_EQ(response.Value.Details.DownloadHint.Value(), Blobs::Models::DownloadHint::Layout);
    }
  }

  TEST(DataLakeLocalityTest, RoutesManagedDownloadsForAllClientOrigins)
  {
    using Routing = DataLake::LayoutAwareRouting;
    for (int origin = 0; origin != 4; ++origin)
    {
      for (const auto& routing :
           {Azure::Nullable<Routing>(),
            Azure::Nullable<Routing>(Routing::Auto),
            Azure::Nullable<Routing>(Routing::Enabled),
            Azure::Nullable<Routing>(Routing::Disabled)})
      {
        for (bool toFile : {false, true})
        {
          SCOPED_TRACE(origin);
          SCOPED_TRACE(toFile);
          SCOPED_TRACE(routing.HasValue() ? static_cast<int>(routing.Value()) : -1);
          auto state = std::make_shared<LocalityState>();
          auto client = CreateClient(state, origin);
          DataLake::DownloadFileToOptions options;
          options.Range = Core::Http::HttpRange();
          options.Range.Value().Offset = 5;
          options.Range.Value().Length = 7;
          EXPECT_EQ(options.LayoutAwareRouting, DataLake::LayoutAwareRouting::Disabled);
          if (routing.HasValue())
          {
            options.LayoutAwareRouting = routing.Value();
          }
          options.TransferOptions.InitialChunkSize = 1;
          options.TransferOptions.ChunkSize = 6;
          options.TransferOptions.Concurrency = 1;
          std::vector<uint8_t> content(7);
          if (toFile)
          {
            const auto fileName = Core::Uuid::CreateUuid().ToString() + ".tmp";
            auto response = client.DownloadTo(fileName, options);
            std::ifstream file(fileName, std::ios::binary);
            content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
            file.close();
            std::remove(fileName.c_str());
            EXPECT_TRUE(response.Value.Details.DownloadHint.HasValue());
          }
          else
          {
            auto response = client.DownloadTo(content.data(), content.size(), options);
            EXPECT_TRUE(response.Value.Details.DownloadHint.HasValue());
          }
          EXPECT_EQ(content, (std::vector<uint8_t>{'f', 'g', 'h', 'i', 'j', 'k', 'l'}));
          if (options.LayoutAwareRouting == DataLake::LayoutAwareRouting::Disabled)
          {
            ASSERT_EQ(state->Requests.size(), 2U);
            EXPECT_EQ(state->Requests.back().Host, "account.blob.core.windows.net");
            EXPECT_FALSE(state->Requests.back().IsLayout);
          }
          else
          {
            ASSERT_EQ(state->Requests.size(), 4U);
            EXPECT_TRUE(state->Requests[1].IsLayout);
            EXPECT_TRUE(state->Requests[2].IsLayout);
            EXPECT_EQ(state->Requests.back().Host, "locality1.test");
            EXPECT_EQ(state->Requests.back().HostHeader, "account.blob.core.windows.net");
            EXPECT_EQ(state->Requests.back().IfMatch, "\"layout-etag\"");
          }
        }
      }
    }
  }

}}} // namespace Azure::Storage::Test
