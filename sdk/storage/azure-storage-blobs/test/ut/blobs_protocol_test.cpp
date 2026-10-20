// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/exception.hpp>
#include <azure/core/http/transport.hpp>
#include <azure/storage/blobs.hpp>
#include <azure/storage/common/storage_exception.hpp>

#include <chrono>
#include <deque>
#include <stdexcept>

#include <gtest/gtest.h>

namespace Azure { namespace Storage { namespace Test { namespace {
  namespace Http = Core::Http;
  using namespace Blobs;

  char const* const Date = "Tue, 20 Oct 2026 01:02:03 GMT";
  char const* const Endpoint
      = "https://account.blob.example/container/blob?sig=sample%2Bsignature%3D";
  Core::Context::Key const ContextKey;

  struct Reply
  {
    Http::HttpStatusCode Status;
    std::string Body;
    Core::CaseInsensitiveMap Headers;
  };

  struct Request
  {
    Core::Url Url;
    Core::CaseInsensitiveMap Headers;
    std::string Body;
    Core::IO::BodyStream* Stream;
    std::string ContextValue;
  };

  class ReadCountingStream final : public Core::IO::BodyStream {
    std::string m_bytes;
    Core::IO::MemoryBodyStream m_stream;
    std::shared_ptr<int> m_reads;

    size_t OnRead(uint8_t* buffer, size_t count, Core::Context const& context) override
    {
      ++*m_reads;
      return m_stream.Read(buffer, count, context);
    }

  public:
    ReadCountingStream(std::string bytes, std::shared_ptr<int> reads)
        : m_bytes(std::move(bytes)),
          m_stream(reinterpret_cast<uint8_t const*>(m_bytes.data()), m_bytes.size()),
          m_reads(std::move(reads))
    {
    }
    int64_t Length() const override { return m_stream.Length(); }
    void Rewind() override { m_stream.Rewind(); }
  };

  class ProtocolTransport final : public Http::HttpTransport {
  public:
    std::deque<Reply> Replies;
    std::vector<Request> Requests;
    std::shared_ptr<int> ResponseReads = std::make_shared<int>(0);

    std::unique_ptr<Http::RawResponse> Send(Http::Request& request, Core::Context const& context)
        override
    {
      context.ThrowIfCancelled();
      if (Replies.empty())
      {
        throw std::runtime_error("Unexpected request in Blob protocol test.");
      }
      std::string body;
      if (request.GetBodyStream())
      {
        auto bytes = request.GetBodyStream()->ReadToEnd(context);
        body.assign(bytes.begin(), bytes.end());
      }
      std::string contextValue;
      context.TryGetValue(ContextKey, contextValue);
      Requests.push_back(
          {request.GetUrl(),
           request.GetHeaders(),
           std::move(body),
           request.GetBodyStream(),
           std::move(contextValue)});
      auto reply = std::move(Replies.front());
      Replies.pop_front();
      auto response = std::make_unique<Http::RawResponse>(1, 1, reply.Status, "Test response");
      response->SetHeader("date", Date);
      response->SetHeader("x-ms-version", "2026-10-06");
      response->SetHeader("x-ms-request-id", "request-id");
      if (!reply.Body.empty() && reply.Body.front() == '<')
      {
        response->SetHeader("content-type", "application/xml");
      }
      for (auto const& header : reply.Headers)
      {
        response->SetHeader(header.first, header.second);
      }
      response->SetBodyStream(
          std::make_unique<ReadCountingStream>(std::move(reply.Body), ResponseReads));
      return response;
    }
  };

  class UserPolicy final : public Http::Policies::HttpPolicy {
  public:
    std::unique_ptr<Http::Policies::HttpPolicy> Clone() const override
    {
      return std::make_unique<UserPolicy>();
    }
    std::unique_ptr<Http::RawResponse> Send(
        Http::Request& request,
        Http::Policies::NextHttpPolicy next,
        Core::Context const& context) const override
    {
      request.SetHeader("Accept", "application/custom");
      request.SetHeader("Content-Type", "application/custom");
      return next.Send(request, context);
    }
  };

  class BatchPolicy final : public Http::Policies::HttpPolicy {
    std::string m_header;
    std::shared_ptr<int> m_calls;

  public:
    BatchPolicy(std::string header, std::shared_ptr<int> calls)
        : m_header(std::move(header)), m_calls(std::move(calls))
    {
    }
    std::unique_ptr<Http::Policies::HttpPolicy> Clone() const override
    {
      return std::make_unique<BatchPolicy>(*this);
    }
    std::unique_ptr<Http::RawResponse> Send(
        Http::Request& request,
        Http::Policies::NextHttpPolicy next,
        Core::Context const& context) const override
    {
      if (request.GetMethod() != Http::HttpMethod::Post)
      {
        ++*m_calls;
        request.SetHeader(m_header, "applied");
      }
      return next.Send(request, context);
    }
  };

  class BatchTokenCredential final : public Core::Credentials::TokenCredential {
  public:
    BatchTokenCredential() : TokenCredential("BatchTestCredential") {}
    Core::Credentials::AccessToken GetToken(
        Core::Credentials::TokenRequestContext const&,
        Core::Context const& context) const override
    {
      context.ThrowIfCancelled();
      return {"batch-token", DateTime::clock::now() + std::chrono::hours(1)};
    }
  };

  class BlobsProtocolTest : public ::testing::Test {
  protected:
    std::shared_ptr<ProtocolTransport> Transport = std::make_shared<ProtocolTransport>();

    BlobClientOptions Options()
    {
      BlobClientOptions options;
      options.Transport.Transport = Transport;
      options.Retry.MaxRetries = 0;
      return options;
    }

    static Core::CaseInsensitiveMap EntityHeaders()
    {
      return {
          {"etag", "\"entity\""},
          {"last-modified", Date},
          {"x-ms-request-server-encrypted", "true"},
          {"x-ms-server-encrypted", "true"}};
    }

    static Core::CaseInsensitiveMap DownloadHeaders(std::string length = "3")
    {
      auto headers = EntityHeaders();
      headers.emplace("x-ms-creation-time", Date);
      headers.emplace("content-length", std::move(length));
      headers.emplace("content-type", "text/plain");
      headers.emplace("x-ms-blob-type", "BlockBlob");
      return headers;
    }

    static std::string Listing()
    {
      return "<EnumerationResults ServiceEndpoint=\"https://account.blob.example/\" "
             "ContainerName=\"container\"><Prefix/><Marker/><NextMarker>next&amp;page</NextMarker>"
             "<Delimiter>/</Delimiter><Blobs><Blob><Name>a&amp;b</Name><Properties>"
             "<Last-Modified>"
          + std::string(Date)
          + "</Last-Modified><Etag>&quot;entity&quot;</Etag><Content-Length>3</Content-Length>"
            "<BlobType>BlockBlob</BlobType></Properties></Blob><BlobPrefix>"
            "<Name Encoded=\"true\">folder%2F</Name></BlobPrefix></Blobs></EnumerationResults>";
    }

    static Reply BatchReply(std::vector<std::string> const& responses)
    {
      std::string body;
      for (size_t i = 0; i < responses.size(); ++i)
      {
        body += "--reply\r\nContent-Type: application/http\r\nContent-ID: " + std::to_string(i)
            + "\r\n\r\n" + responses[i] + "\r\n";
      }
      body += "--reply--";
      return {
          Http::HttpStatusCode::Accepted,
          std::move(body),
          {{"content-type", "multipart/mixed; boundary=reply"}}};
    }

    void TearDown() override
    {
      EXPECT_TRUE(Transport->Replies.empty());
      for (auto const& request : Transport->Requests)
      {
        EXPECT_EQ(request.Url.GetQueryParameters().at("sig"), "sample%2Bsignature%3D");
      }
    }
  };

  TEST_F(BlobsProtocolTest, UploadBorrowsStreamAndPreservesHeadersAndContext)
  {
    auto options = Options();
    options.ApiVersion = "2023-11-03";
    EncryptionKey key;
    key.Key = "c2FtcGxl";
    key.KeyHash = {1, 2, 3};
    key.Algorithm = Models::EncryptionAlgorithmType::Aes256;
    options.CustomerProvidedKey = key;
    BlockBlobClient client(Endpoint, options);
    Transport->Replies.push_back({Http::HttpStatusCode::Created, "", EntityHeaders()});
    std::string bytes = "hello";
    Core::IO::MemoryBodyStream stream(reinterpret_cast<uint8_t const*>(bytes.data()), bytes.size());
    UploadBlockBlobOptions upload;
    upload.Metadata = {{"Name", "value"}, {"empty", ""}};
    upload.HttpHeaders.ContentType = "text/plain";
    upload.Tags = {{"a&b", "c<d"}};
    upload.AccessConditions.IfMatch = ETag("\"match\"");
    upload.AccessConditions.IfNoneMatch = ETag("\"other\"");
    ContentHash hash;
    hash.Algorithm = HashAlgorithm::Md5;
    hash.Value = {1, 2, 3};
    upload.TransactionalContentHash = hash;
    auto response = client.Upload(
        stream, upload, Core::Context().WithValue(ContextKey, std::string("forwarded")));
    EXPECT_EQ(response.Value.ETag.ToString(), "\"entity\"");
    auto const& request = Transport->Requests.at(0);
    EXPECT_EQ(request.Stream, &stream);
    EXPECT_EQ(request.Body, "hello");
    EXPECT_EQ(request.ContextValue, "forwarded");
    EXPECT_EQ(request.Headers.at("x-ms-version"), "2023-11-03");
    EXPECT_EQ(request.Headers.at("x-ms-encryption-key-sha256"), "AQID");
    EXPECT_EQ(request.Headers.at("x-ms-encryption-algorithm"), "AES256");
    EXPECT_EQ(request.Headers.at("content-md5"), "AQID");
    EXPECT_EQ(request.Headers.at("if-match"), "\"match\"");
    EXPECT_EQ(request.Headers.at("if-none-match"), "\"other\"");
    EXPECT_EQ(request.Headers.at("x-ms-meta-empty"), "");
    EXPECT_EQ(request.Headers.at("x-ms-meta-name"), "value");
    EXPECT_EQ(request.Headers.count("accept"), 0U);
    EXPECT_EQ(request.Headers.count("content-type"), 0U);
  }

  TEST_F(BlobsProtocolTest, RetryRewindsTheBorrowedUploadStream)
  {
    auto options = Options();
    options.Retry.MaxRetries = 1;
    options.Retry.RetryDelay = std::chrono::milliseconds(0);
    options.Retry.MaxRetryDelay = std::chrono::milliseconds(0);
    BlockBlobClient client(Endpoint, options);
    Transport->Replies.push_back({Http::HttpStatusCode::InternalServerError, "", {}});
    Transport->Replies.push_back({Http::HttpStatusCode::Created, "", EntityHeaders()});
    std::string bytes = "hello";
    Core::IO::MemoryBodyStream stream(reinterpret_cast<uint8_t const*>(bytes.data()), bytes.size());
    client.Upload(stream);
    ASSERT_EQ(Transport->Requests.size(), 2U);
    for (auto const& request : Transport->Requests)
    {
      EXPECT_EQ(request.Stream, &stream);
      EXPECT_EQ(request.Body, bytes);
    }
  }

  TEST_F(BlobsProtocolTest, DownloadKeepsStreamAliveAndDecodesChecksums)
  {
    auto headers = DownloadHeaders();
    headers.emplace("content-range", "bytes 2-4/10");
    headers.emplace("x-ms-content-crc64", "AQIDBAUGBwg=");
    Transport->Replies.push_back({Http::HttpStatusCode::PartialContent, "abc", headers});
    DownloadBlobOptions download;
    Core::Http::HttpRange range;
    range.Offset = 2;
    range.Length = 3;
    download.Range = range;
    download.RangeHashAlgorithm = HashAlgorithm::Crc64;
    auto response = BlobClient(Endpoint, Options()).WithSnapshot("a+b=").Download(download);
    EXPECT_EQ(*Transport->ResponseReads, 0);
    EXPECT_EQ(response.Value.BlobSize, 10);
    ASSERT_TRUE(response.Value.TransactionalContentHash.HasValue());
    EXPECT_EQ(response.Value.TransactionalContentHash.Value().Algorithm, HashAlgorithm::Crc64);
    EXPECT_EQ(
        response.Value.TransactionalContentHash.Value().Value,
        (std::vector<uint8_t>{1, 2, 3, 4, 5, 6, 7, 8}));
    auto bytes = response.Value.BodyStream->ReadToEnd();
    EXPECT_EQ(std::string(bytes.begin(), bytes.end()), "abc");
    EXPECT_EQ(response.RawResponse->GetHeaders().at("x-ms-content-crc64"), "AQIDBAUGBwg=");
    auto const& request = Transport->Requests.at(0);
    EXPECT_EQ(request.Headers.at("x-ms-range"), "bytes=2-4");
    EXPECT_EQ(request.Headers.count("range"), 0U);
    EXPECT_EQ(request.Url.GetQueryParameters().at("snapshot"), "a%2Bb%3D");
  }

  TEST_F(BlobsProtocolTest, MissingConditionalDownloadHeadersRemainOptional)
  {
    auto headers = DownloadHeaders();
    headers.erase("etag");
    headers.erase("last-modified");
    Transport->Replies.push_back({Http::HttpStatusCode::Ok, "abc", headers});
    auto response = BlobClient(Endpoint, Options()).Download();
    EXPECT_FALSE(response.Value.Details.ETag.HasValue());
    EXPECT_FALSE(response.Value.Details.SequenceNumber.HasValue());
    EXPECT_FALSE(response.Value.TransactionalContentHash.HasValue());
    EXPECT_EQ(response.Value.Details.HttpHeaders.ContentType, "text/plain");
  }

  TEST_F(BlobsProtocolTest, StageBlockToleratesOmittedEntityHeaders)
  {
    Transport->Replies.push_back(
        {Http::HttpStatusCode::Created, "", {{"x-ms-request-server-encrypted", "true"}}});
    std::string bytes = "abc";
    Core::IO::MemoryBodyStream stream(reinterpret_cast<uint8_t const*>(bytes.data()), bytes.size());
    auto response = BlockBlobClient(Endpoint, Options()).StageBlock("YQ==", stream);
    EXPECT_TRUE(response.Value.IsServerEncrypted);
    EXPECT_EQ(Transport->Requests.at(0).Url.GetQueryParameters().at("blockid"), "YQ%3D%3D");
    EXPECT_EQ(Transport->Requests.at(0).Stream, &stream);
  }

  TEST_F(BlobsProtocolTest, DataLakePrincipalNameUsesTheOriginalHeader)
  {
    BlobClient client(Endpoint, Options());
    for (bool upn : {true, false})
    {
      auto context
          = Core::Context().WithValue(Blobs::_detail::DataLakeInteroperabilityExtraOptionsKey, upn);
      Transport->Replies.push_back({Http::HttpStatusCode::Ok, "abc", DownloadHeaders()});
      client.Download({}, context);
      Transport->Replies.push_back({Http::HttpStatusCode::Ok, "", DownloadHeaders()});
      client.GetProperties({}, context);
    }
    ASSERT_EQ(Transport->Requests.size(), 4U);
    for (std::size_t i = 0; i < Transport->Requests.size(); ++i)
    {
      auto const& request = Transport->Requests[i];
      EXPECT_EQ(request.Headers.at("x-ms-upn"), i < 2 ? "true" : "false");
      EXPECT_EQ(request.Url.GetQueryParameters().count("upn"), 0U);
    }
  }

  TEST_F(BlobsProtocolTest, CommitBlockListPreservesAlreadyEncodedIds)
  {
    Transport->Replies.push_back({Http::HttpStatusCode::Created, "", EntityHeaders()});
    BlockBlobClient(Endpoint, Options()).CommitBlockList({"YQ==", "Yg=="});
    auto const& request = Transport->Requests.at(0);
    EXPECT_NE(request.Body.find("<Latest>YQ==</Latest>"), std::string::npos);
    EXPECT_NE(request.Body.find("<Latest>Yg==</Latest>"), std::string::npos);
    EXPECT_EQ(request.Headers.at("content-type"), "application/xml; charset=UTF-8");
    EXPECT_EQ(request.Headers.count("accept"), 0U);
  }

  TEST_F(BlobsProtocolTest, GetBlockListPreservesNamesAndSizes)
  {
    auto headers = EntityHeaders();
    headers.emplace("x-ms-blob-content-length", "4096");
    Transport->Replies.push_back(
        {Http::HttpStatusCode::Ok,
         "<BlockList><CommittedBlocks><Block><Name>YQ==</Name><Size>4096</Size></Block>"
         "</CommittedBlocks><UncommittedBlocks><Block><Name>Yg==</Name><Size>7</Size>"
         "</Block></UncommittedBlocks></BlockList>",
         headers});
    GetBlockListOptions options;
    options.ListType = Models::BlockListType::All;
    auto response = BlockBlobClient(Endpoint, Options()).GetBlockList(options);
    ASSERT_EQ(response.Value.CommittedBlocks.size(), 1U);
    EXPECT_EQ(response.Value.CommittedBlocks[0].Name, "YQ==");
    EXPECT_EQ(response.Value.CommittedBlocks[0].Size, 4096);
    ASSERT_EQ(response.Value.UncommittedBlocks.size(), 1U);
    EXPECT_EQ(response.Value.UncommittedBlocks[0].Name, "Yg==");
    EXPECT_EQ(response.Value.BlobSize, 4096);
  }

  TEST_F(BlobsProtocolTest, TagsPreserveXmlEscapingAndEmptyValues)
  {
    Transport->Replies.push_back({Http::HttpStatusCode::NoContent, "", {}});
    BlobClient client(Endpoint, Options());
    client.SetTags({{"a&b", "c<d"}, {"empty", ""}});
    EXPECT_NE(Transport->Requests.at(0).Body.find("<Key>a&amp;b</Key>"), std::string::npos);
    EXPECT_NE(Transport->Requests.at(0).Body.find("<Value>c&lt;d</Value>"), std::string::npos);
    Transport->Replies.push_back(
        {Http::HttpStatusCode::Ok,
         "<Tags><TagSet><Tag><Key>a&amp;b</Key><Value>c&lt;d</Value></Tag>"
         "<Tag><Key>empty</Key><Value/></Tag></TagSet></Tags>",
         {}});
    auto tags = client.GetTags().Value;
    EXPECT_EQ(tags.at("a&b"), "c<d");
    EXPECT_EQ(tags.at("empty"), "");
  }

  TEST_F(BlobsProtocolTest, ListingPreservesArrowNegotiationAndXmlFallback)
  {
    Transport->Replies.push_back({Http::HttpStatusCode::Ok, Listing(), {}});
    ListBlobsOptions list;
    list.ResponseFormat = StorageResponseFormat::Arrow;
    list.Prefix = "a&b";
    list.StartFrom = "begin+";
    list.EndBefore = "end+";
    list.ContinuationToken = "marker+";
    list.Include = Models::ListBlobsIncludeFlags::Metadata | Models::ListBlobsIncludeFlags::Tags;
    auto response = BlobContainerClient(Endpoint, Options()).ListBlobs(list);
    ASSERT_EQ(response.Blobs.size(), 1U);
    EXPECT_EQ(response.Blobs[0].Name, "a&b");
    ASSERT_TRUE(response.NextPageToken.HasValue());
    EXPECT_EQ(response.NextPageToken.Value(), "next&page");
    auto const& request = Transport->Requests.at(0);
    EXPECT_EQ(request.Headers.at("accept"), "application/vnd.apache.arrow.stream,application/xml");
    auto const& query = request.Url.GetQueryParameters();
    EXPECT_EQ(query.at("startFrom"), "begin%2B");
    EXPECT_EQ(query.at("endBefore"), "end%2B");
    EXPECT_EQ(query.at("marker"), "marker%2B");
    EXPECT_EQ(query.at("prefix"), "a%26b");
    EXPECT_EQ(query.at("include"), "metadata,tags");
  }

  TEST_F(BlobsProtocolTest, HierarchicalListingPreservesEncodedPrefixes)
  {
    Transport->Replies.push_back({Http::HttpStatusCode::Ok, Listing(), {}});
    auto response = BlobContainerClient(Endpoint, Options()).ListBlobsByHierarchy("/");
    EXPECT_EQ(response.Delimiter, "/");
    EXPECT_EQ(response.BlobPrefixes, (std::vector<std::string>{"folder/"}));
    EXPECT_EQ(
        Core::Url::Decode(Transport->Requests.at(0).Url.GetQueryParameters().at("delimiter")), "/");
    EXPECT_EQ(Transport->Requests.at(0).Headers.count("accept"), 0U);
  }

  TEST_F(BlobsProtocolTest, FindTagsPreservesAttributesAndContinuation)
  {
    Transport->Replies.push_back(
        {Http::HttpStatusCode::Ok,
         "<EnumerationResults ServiceEndpoint=\"https://account.blob.example/\">"
         "<Blobs><Blob><Name>a&amp;b</Name><ContainerName>container</ContainerName>"
         "<Tags><TagSet><Tag><Key>key</Key><Value>value</Value></Tag></TagSet></Tags>"
         "</Blob></Blobs><NextMarker>next&amp;page</NextMarker></EnumerationResults>",
         {}});
    FindBlobsByTagsOptions find;
    find.PageSizeHint = 17;
    auto response = BlobServiceClient(Endpoint, Options()).FindBlobsByTags("\"key\"='value'", find);
    ASSERT_EQ(response.TaggedBlobs.size(), 1U);
    EXPECT_EQ(response.TaggedBlobs[0].BlobName, "a&b");
    EXPECT_EQ(response.TaggedBlobs[0].BlobContainerName, "container");
    EXPECT_EQ(response.TaggedBlobs[0].Tags.at("key"), "value");
    ASSERT_TRUE(response.NextPageToken.HasValue());
    EXPECT_EQ(response.NextPageToken.Value(), "next&page");
  }

  TEST_F(BlobsProtocolTest, ServicePropertiesPreserveFalseAndMissingOptionalValues)
  {
    Transport->Replies.push_back(
        {Http::HttpStatusCode::Ok,
         "<StorageServiceProperties><Logging><Version>1.0</Version><Delete>false</Delete>"
         "<Read>true</Read><Write>false</Write><RetentionPolicy><Enabled>false</Enabled>"
         "</RetentionPolicy></Logging><HourMetrics><Version>1.0</Version><Enabled>false</Enabled>"
         "<IncludeAPIs>false</IncludeAPIs></HourMetrics><Cors/><StaticWebsite><Enabled>false"
         "</Enabled></StaticWebsite></StorageServiceProperties>",
         {}});
    auto response = BlobServiceClient(Endpoint, Options()).GetProperties();
    EXPECT_EQ(response.Value.Logging.Version, "1.0");
    EXPECT_TRUE(response.Value.Logging.Read);
    EXPECT_FALSE(response.Value.Logging.Write);
    EXPECT_FALSE(response.Value.HourMetrics.IsEnabled);
    ASSERT_TRUE(response.Value.HourMetrics.IncludeApis.HasValue());
    EXPECT_FALSE(response.Value.HourMetrics.IncludeApis.Value());
    EXPECT_FALSE(response.Value.StaticWebsite.IsEnabled);
    EXPECT_TRUE(response.Value.Cors.empty());
  }

  TEST_F(BlobsProtocolTest, StatisticsTolerateEmptyLastSyncTime)
  {
    Transport->Replies.push_back(
        {Http::HttpStatusCode::Ok,
         "<StorageServiceStats><GeoReplication><Status>unavailable</Status><LastSyncTime/>"
         "</GeoReplication></StorageServiceStats>",
         {}});
    auto response = BlobServiceClient(Endpoint, Options()).GetStatistics();
    EXPECT_EQ(response.Value.GeoReplication.Status, Models::GeoReplicationStatus::Unavailable);
    EXPECT_FALSE(response.Value.GeoReplication.LastSyncedOn.HasValue());
  }

  TEST_F(BlobsProtocolTest, CopyPreservesLegacySourceLease)
  {
    auto headers = EntityHeaders();
    headers.emplace("x-ms-copy-id", "copy-id");
    headers.emplace("x-ms-copy-status", "success");
    Transport->Replies.push_back({Http::HttpStatusCode::Accepted, "", headers});
    StartBlobCopyFromUriOptions copy;
    copy.SourceAccessConditions.LeaseId = "source-lease";
    BlobClient(Endpoint, Options()).StartCopyFromUri("https://source.example/blob?sig=a%2Bb", copy);
    EXPECT_EQ(Transport->Requests.at(0).Headers.at("x-ms-source-lease-id"), "source-lease");
    EXPECT_EQ(
        Transport->Requests.at(0).Headers.at("x-ms-copy-source"),
        "https://source.example/blob?sig=a%2Bb");
  }

  TEST_F(BlobsProtocolTest, CopyFromUriPreservesCrc64AndEncryptionScope)
  {
    auto options = Options();
    options.EncryptionScope = "scope";
    Transport->Replies.push_back({Http::HttpStatusCode::Created, "", EntityHeaders()});
    UploadBlockBlobFromUriOptions copy;
    ContentHash hash;
    hash.Algorithm = HashAlgorithm::Crc64;
    hash.Value = {1, 2, 3, 4, 5, 6, 7, 8};
    copy.TransactionalContentHash = hash;
    BlockBlobClient(Endpoint, options).UploadFromUri("https://source.example/blob", copy);
    EXPECT_EQ(Transport->Requests.at(0).Headers.at("x-ms-source-content-crc64"), "AQIDBAUGBwg=");
    EXPECT_EQ(Transport->Requests.at(0).Headers.at("x-ms-encryption-scope"), "scope");
  }

  TEST_F(BlobsProtocolTest, QueryPreservesEmptyTextSettingsAndStreaming)
  {
    auto options = Options();
    options.EncryptionScope = "query-scope";
    Transport->Replies.push_back({Http::HttpStatusCode::Ok, "", DownloadHeaders("0")});
    QueryBlobOptions query;
    query.InputTextConfiguration
        = BlobQueryInputTextOptions::CreateCsvTextOptions("", "", "", "", false);
    query.OutputTextConfiguration = BlobQueryOutputTextOptions::CreateJsonTextOptions("");
    auto response = BlockBlobClient(Endpoint, options)
                        .Query("SELECT * FROM BlobStorage WHERE _1 < 2;", query);
    ASSERT_NE(response.Value.BodyStream, nullptr);
    EXPECT_EQ(*Transport->ResponseReads, 0);
    auto const& request = Transport->Requests.at(0);
    EXPECT_EQ(request.Headers.at("x-ms-encryption-scope"), "query-scope");
    EXPECT_NE(request.Body.find("&lt; 2"), std::string::npos);
    EXPECT_NE(request.Body.find("<ColumnSeparator"), std::string::npos);
    EXPECT_NE(request.Body.find("<RecordSeparator"), std::string::npos);
    EXPECT_NE(request.Body.find("<HasHeaders>false</HasHeaders>"), std::string::npos);
  }

  TEST_F(BlobsProtocolTest, QueryPreservesEmptyParquetConfiguration)
  {
    Transport->Replies.push_back({Http::HttpStatusCode::Ok, "", DownloadHeaders("0")});
    QueryBlobOptions query;
    query.InputTextConfiguration = BlobQueryInputTextOptions::CreateParquetTextOptions();
    query.OutputTextConfiguration = BlobQueryOutputTextOptions::CreateJsonTextOptions();
    auto response = BlockBlobClient(Endpoint, Options()).Query("SELECT * FROM BlobStorage;", query);
    ASSERT_NE(response.Value.BodyStream, nullptr);
    EXPECT_EQ(*Transport->ResponseReads, 0);
    auto const& body = Transport->Requests.at(0).Body;
    EXPECT_NE(body.find("<ParquetTextConfiguration"), std::string::npos);
    EXPECT_EQ(body.find("<>"), std::string::npos);
  }

  TEST_F(BlobsProtocolTest, UserPoliciesRunAfterProtocolNormalization)
  {
    auto options = Options();
    options.PerOperationPolicies.push_back(std::make_unique<UserPolicy>());
    Transport->Replies.push_back({Http::HttpStatusCode::Created, "", EntityHeaders()});
    BlockBlobClient(Endpoint, options).CommitBlockList({"YQ=="});
    EXPECT_EQ(Transport->Requests.at(0).Headers.at("accept"), "application/custom");
    EXPECT_EQ(Transport->Requests.at(0).Headers.at("content-type"), "application/custom");
  }

  TEST_F(BlobsProtocolTest, ErrorsRemainStorageExceptionsAndCancellationIsForwarded)
  {
    Transport->Replies.push_back(
        {Http::HttpStatusCode::PreconditionFailed,
         "<Error><Code>ConditionNotMet</Code><Message>condition failed</Message></Error>",
         {{"x-ms-error-code", "ConditionNotMet"}}});
    BlobClient client(Endpoint, Options());
    try
    {
      client.Delete();
      FAIL() << "Expected StorageException.";
    }
    catch (StorageException const& error)
    {
      EXPECT_EQ(error.ErrorCode, "ConditionNotMet");
      EXPECT_EQ(error.StatusCode, Http::HttpStatusCode::PreconditionFailed);
      ASSERT_NE(error.RawResponse, nullptr);
    }
    Core::Context context;
    context.Cancel();
    EXPECT_THROW(client.Delete({}, context), Core::OperationCancelledException);
    EXPECT_EQ(Transport->Requests.size(), 1U);
  }

  TEST_F(BlobsProtocolTest, ServiceBatchPreservesMultipartAndDeferredResponses)
  {
    auto client = BlobServiceClient(Endpoint, Options());
    auto batch = client.CreateBatch();
    auto deleted = batch.DeleteBlob("container", "blob");
    auto tiered = batch.SetBlobAccessTier("container", "other", Models::AccessTier::Cool);
    Transport->Replies.push_back(
        {Http::HttpStatusCode::Accepted,
         "--reply\r\nContent-Type: application/http\r\nContent-ID: 0\r\n\r\n"
         "HTTP/1.1 202 Accepted\r\nx-ms-request-id: delete-id\r\n\r\n\r\n"
         "--reply\r\nContent-Type: application/http\r\nContent-ID: 1\r\n\r\n"
         "HTTP/1.1 200 OK\r\nx-ms-request-id: tier-id\r\n\r\n\r\n--reply--",
         {{"content-type", "multipart/mixed; boundary=reply"}}});
    client.SubmitBatch(batch);
    EXPECT_TRUE(deleted.GetResponse().Value.Deleted);
    EXPECT_EQ(tiered.GetResponse().RawResponse->GetStatusCode(), Http::HttpStatusCode::Ok);
    auto const& request = Transport->Requests.at(0);
    EXPECT_NE(
        request.Headers.at("content-type").find("multipart/mixed; boundary="), std::string::npos);
    EXPECT_NE(request.Body.find("DELETE "), std::string::npos);
    EXPECT_NE(request.Body.find("PUT "), std::string::npos);
    EXPECT_NE(request.Body.find("x-ms-access-tier: Cool"), std::string::npos);
  }

  TEST_F(BlobsProtocolTest, ContainerBatchPreservesDeferredDeletion)
  {
    auto client = BlobContainerClient(Endpoint, Options());
    auto batch = client.CreateBatch();
    auto deleted = batch.DeleteBlob("blob");
    Transport->Replies.push_back(
        {Http::HttpStatusCode::Accepted,
         "--reply\r\nContent-Type: application/http\r\nContent-ID: 0\r\n\r\n"
         "HTTP/1.1 202 Accepted\r\nx-ms-request-id: delete-id\r\n\r\n\r\n--reply--",
         {{"content-type", "multipart/mixed; boundary=reply"}}});
    client.SubmitBatch(batch);
    EXPECT_TRUE(deleted.GetResponse().Value.Deleted);
    EXPECT_EQ(Transport->Requests.at(0).Url.GetQueryParameters().at("restype"), "container");
  }

  TEST_F(BlobsProtocolTest, ServiceBatchPreservesConditionsPoliciesAndSharedKeyAuthentication)
  {
    auto options = Options();
    auto operationCalls = std::make_shared<int>(0);
    auto retryCalls = std::make_shared<int>(0);
    options.PerOperationPolicies.push_back(
        std::make_unique<BatchPolicy>("x-test-operation", operationCalls));
    options.PerRetryPolicies.push_back(std::make_unique<BatchPolicy>("x-test-retry", retryCalls));
    auto credential = std::make_shared<StorageSharedKeyCredential>("account", "ZmFrZQ==");
    BlobServiceClient client(Endpoint, credential, options);
    auto batch = client.CreateBatch();
    DeleteBlobOptions deletion;
    deletion.DeleteSnapshots = Models::DeleteSnapshotsOption::IncludeSnapshots;
    auto& conditions = deletion.AccessConditions;
    conditions.LeaseId = "delete-lease";
    conditions.TagConditions = "\"key\"='value'";
    conditions.IfMatch = ETag("\"match\"");
    conditions.IfNoneMatch = ETag("\"other\"");
    conditions.IfModifiedSince = DateTime::Parse(Date, DateTime::DateFormat::Rfc1123);
    conditions.IfUnmodifiedSince = conditions.IfModifiedSince;
    conditions.AccessTierIfModifiedSince = conditions.IfModifiedSince;
    conditions.AccessTierIfUnmodifiedSince = conditions.IfModifiedSince;
    auto deleted = batch.DeleteBlobUrl(
        std::string(Endpoint) + "&snapshot=snap%2Bvalue&versionid=version%2Fone", deletion);
    SetBlobAccessTierOptions tierOptions;
    tierOptions.RehydratePriority = Models::RehydratePriority::High;
    tierOptions.AccessConditions.LeaseId = "tier-lease";
    tierOptions.AccessConditions.TagConditions = "\"tier\"='archive'";
    auto tiered = batch.SetBlobAccessTierUrl(
        std::string(Endpoint) + "&versionid=tier%2Bversion", Models::AccessTier::Cool, tierOptions);
    Transport->Replies.push_back(BatchReply({
        "HTTP/1.1 202 Accepted\r\n\r\n",
        "HTTP/1.1 200 OK\r\n\r\n",
    }));
    client.SubmitBatch(batch);
    EXPECT_TRUE(deleted.GetResponse().Value.Deleted);
    EXPECT_EQ(tiered.GetResponse().RawResponse->GetStatusCode(), Http::HttpStatusCode::Ok);
    EXPECT_EQ(*operationCalls, 4);
    EXPECT_EQ(*retryCalls, 4);
    auto const& body = Transport->Requests.at(0).Body;
    for (auto const& field : {
             "snapshot=snap%2Bvalue",
             "versionid=version%2Fone",
             "versionid=tier%2Bversion",
             "x-ms-delete-snapshots: include",
             "x-ms-lease-id: delete-lease",
             "x-ms-lease-id: tier-lease",
             "if-match: \"match\"",
             "if-none-match: \"other\"",
             "if-modified-since: Tue, 20 Oct 2026 01:02:03 GMT",
             "if-unmodified-since: Tue, 20 Oct 2026 01:02:03 GMT",
             "x-ms-access-tier-if-modified-since: Tue, 20 Oct 2026 01:02:03 GMT",
             "x-ms-access-tier-if-unmodified-since: Tue, 20 Oct 2026 01:02:03 GMT",
             "x-ms-if-tags: \"key\"='value'",
             "x-ms-if-tags: \"tier\"='archive'",
             "x-ms-access-tier: Cool",
             "x-ms-rehydrate-priority: High",
             "x-test-operation: applied",
             "x-test-retry: applied",
             "SharedKey account:",
         })
    {
      EXPECT_NE(body.find(field), std::string::npos) << field;
    }
    EXPECT_EQ(body.find("x-ms-version:"), std::string::npos);
  }

  TEST_F(BlobsProtocolTest, ServiceBatchPreservesTokenAuthentication)
  {
    BlobServiceClient client(Endpoint, std::make_shared<BatchTokenCredential>(), Options());
    auto batch = client.CreateBatch();
    auto deleted = batch.DeleteBlob("container", "blob");
    auto tiered = batch.SetBlobAccessTier("container", "other", Models::AccessTier::Cool);
    Transport->Replies.push_back(BatchReply({
        "HTTP/1.1 202 Accepted\r\n\r\n",
        "HTTP/1.1 200 OK\r\n\r\n",
    }));
    client.SubmitBatch(batch);
    EXPECT_TRUE(deleted.GetResponse().Value.Deleted);
    EXPECT_EQ(tiered.GetResponse().RawResponse->GetStatusCode(), Http::HttpStatusCode::Ok);
    auto const& request = Transport->Requests.at(0);
    EXPECT_EQ(request.Headers.at("authorization"), "Bearer batch-token");
    auto first = request.Body.find("Bearer batch-token");
    ASSERT_NE(first, std::string::npos);
    EXPECT_NE(request.Body.find("Bearer batch-token", first + 1), std::string::npos);
  }

  TEST_F(BlobsProtocolTest, ContainerBatchTierAcceptsAsyncResponseAndOmitsEmptyHeaders)
  {
    BlobContainerClient client(Endpoint, Options());
    auto batch = client.CreateBatch();
    SetBlobAccessTierOptions options;
    options.AccessConditions.LeaseId = "";
    options.AccessConditions.TagConditions = "";
    options.RehydratePriority = Models::RehydratePriority("");
    auto tiered = batch.SetBlobAccessTierUrl(
        std::string(Endpoint) + "&versionid=tier%2Bversion", Models::AccessTier::Archive, options);
    Transport->Replies.push_back(BatchReply({"HTTP/1.1 202 Accepted\r\n\r\n"}));
    client.SubmitBatch(batch);
    EXPECT_EQ(tiered.GetResponse().RawResponse->GetStatusCode(), Http::HttpStatusCode::Accepted);
    auto const& body = Transport->Requests.at(0).Body;
    EXPECT_NE(body.find("versionid=tier%2Bversion"), std::string::npos);
    EXPECT_NE(body.find("comp=tier"), std::string::npos);
    EXPECT_NE(body.find("x-ms-access-tier: Archive"), std::string::npos);
    EXPECT_EQ(body.find("x-ms-lease-id:"), std::string::npos);
    EXPECT_EQ(body.find("x-ms-if-tags:"), std::string::npos);
    EXPECT_EQ(body.find("x-ms-rehydrate-priority:"), std::string::npos);
    EXPECT_EQ(body.find("x-ms-version:"), std::string::npos);
  }

  TEST_F(BlobsProtocolTest, BatchDefersErrorsAndPreservesOtherSubresponses)
  {
    BlobContainerClient client(Endpoint, Options());
    auto batch = client.CreateBatch();
    auto deleted = batch.DeleteBlobUrl(Endpoint);
    auto tiered = batch.SetBlobAccessTier("other", Models::AccessTier::Cool);
    Transport->Replies.push_back(BatchReply({
        "HTTP/1.1 404 Not Found\r\nx-ms-error-code: BlobNotFound\r\n\r\n"
        "<Error><Code>BlobNotFound</Code><Message>missing blob</Message></Error>",
        "HTTP/1.1 200 OK\r\n\r\n",
    }));
    EXPECT_NO_THROW(client.SubmitBatch(batch));
    try
    {
      deleted.GetResponse();
      FAIL() << "Expected deferred StorageException.";
    }
    catch (StorageException const& error)
    {
      EXPECT_EQ(error.StatusCode, Http::HttpStatusCode::NotFound);
      EXPECT_EQ(error.ErrorCode, "BlobNotFound");
      ASSERT_NE(error.RawResponse, nullptr);
    }
    EXPECT_EQ(tiered.GetResponse().RawResponse->GetStatusCode(), Http::HttpStatusCode::Ok);
  }

  TEST_F(BlobsProtocolTest, BatchRejectsUnexpectedSuccessStatusesPerOperation)
  {
    BlobServiceClient client(Endpoint, Options());
    auto batch = client.CreateBatch();
    auto deleted = batch.DeleteBlob("container", "blob");
    auto tiered = batch.SetBlobAccessTier("container", "other", Models::AccessTier::Cool);
    Transport->Replies.push_back(BatchReply({
        "HTTP/1.1 200 OK\r\n\r\n",
        "HTTP/1.1 204 No Content\r\n\r\n",
    }));
    EXPECT_NO_THROW(client.SubmitBatch(batch));
    EXPECT_THROW(deleted.GetResponse(), StorageException);
    EXPECT_THROW(tiered.GetResponse(), StorageException);
  }
}}}} // namespace Azure::Storage::Test::
