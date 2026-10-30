// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "test/ut/test_base.hpp"

#include <azure/storage/files/datalake.hpp>

#include <cstdio>
#include <fstream>
#include <memory>
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
        m_state->Requests.push_back(
            LocalityRequest{
                request.GetUrl().GetHost(),
                header("host"),
                header("x-ms-range"),
                header("if-match"),
                marker,
                isLayout});

        auto response = std::make_unique<Core::Http::RawResponse>(
            1,
            1,
            isLayout ? (m_state->NoLayout ? Core::Http::HttpStatusCode::NoContent
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
        response->SetBodyStream(
            std::make_unique<Core::IO::MemoryBodyStream>(
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

  TEST(DataLakeLocalityTest, ManagedDownloadFallsBackWithoutLayout)
  {
    auto state = std::make_shared<LocalityState>();
    state->NoLayout = true;
    DataLake::DownloadFileToOptions options;
    options.Range = Core::Http::HttpRange();
    options.Range.Value().Length = 12;
    options.LayoutAwareRouting = DataLake::LayoutAwareRouting::Enabled;
    options.TransferOptions.InitialChunkSize = 1;
    options.TransferOptions.ChunkSize = 6;
    options.TransferOptions.Concurrency = 1;
    std::vector<uint8_t> content(12);
    CreateClient(state, 0).DownloadTo(content.data(), content.size(), options);
    EXPECT_EQ(std::string(content.begin(), content.end()), "abcdefghijkl");
    ASSERT_EQ(state->Requests.size(), 4U);
    EXPECT_TRUE(state->Requests[1].IsLayout);
    for (const auto& request : state->Requests)
    {
      EXPECT_EQ(request.Host, "account.blob.core.windows.net");
    }
  }

  TEST(DataLakeLocalityTest, OneShotDownloadUsesClientEndpointForAllClientOrigins)
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
      auto response = client.Download(options);
      EXPECT_EQ(response.Value.Body->ReadToEnd(), std::vector<uint8_t>{'g'});
      ASSERT_EQ(state->Requests.size(), 1U);
      EXPECT_EQ(state->Requests[0].Host, "account.blob.core.windows.net");
      EXPECT_TRUE(state->Requests[0].HostHeader.empty());
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
