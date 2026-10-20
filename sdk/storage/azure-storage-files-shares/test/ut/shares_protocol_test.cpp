// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/exception.hpp>
#include <azure/core/http/transport.hpp>
#include <azure/storage/common/storage_exception.hpp>
#include <azure/storage/files/shares.hpp>

#include <chrono>
#include <deque>
#include <functional>
#include <stdexcept>

#include <gtest/gtest.h>

namespace Azure { namespace Storage { namespace Test { namespace {
  namespace Http = Core::Http;
  using namespace Files::Shares;

  char const* const Date = "Tue, 20 Oct 2026 01:02:03 GMT";
  char const* const Endpoint
      = "https://account.file.example/share/directory/file?sig=sample%2Bsignature%3D";
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
    std::function<void()> BeforeSend;

    std::unique_ptr<Http::RawResponse> Send(Http::Request& request, Core::Context const& context)
        override
    {
      context.ThrowIfCancelled();
      if (BeforeSend)
      {
        BeforeSend();
      }
      if (Replies.empty())
      {
        throw std::runtime_error("Unexpected request in File Shares protocol test.");
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

  class SharesProtocolTest : public ::testing::Test {
  protected:
    std::shared_ptr<ProtocolTransport> Transport = std::make_shared<ProtocolTransport>();

    ShareClientOptions Options()
    {
      ShareClientOptions options;
      options.Transport.Transport = Transport;
      options.Retry.MaxRetries = 0;
      return options;
    }

    static Core::CaseInsensitiveMap EntityHeaders()
    {
      return {{"etag", "\"entity\""}, {"last-modified", Date}};
    }

    static Core::CaseInsensitiveMap FileHeaders()
    {
      auto headers = EntityHeaders();
      headers.emplace("x-ms-request-server-encrypted", "true");
      headers.emplace("x-ms-server-encrypted", "true");
      headers.emplace("x-ms-file-id", "file-id");
      headers.emplace("x-ms-file-parent-id", "parent-id");
      return headers;
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

  TEST_F(SharesProtocolTest, CreateSharePreservesMetadataQuotaAndVersion)
  {
    auto options = Options();
    options.ApiVersion = "2023-11-03";
    ShareClient client(Endpoint, options);
    auto headers = EntityHeaders();
    headers.emplace("x-ms-share-quota", "4294967296");
    Transport->Replies.push_back({Http::HttpStatusCode::Created, "", headers});
    CreateShareOptions create;
    create.ShareQuotaInGiB = INT64_C(4294967296);
    create.Metadata = {{"Name", "value"}, {"empty", ""}};
    create.AccessTier = Models::AccessTier::Cool;
    auto response
        = client.Create(create, Core::Context().WithValue(ContextKey, std::string("forwarded")));
    EXPECT_EQ(response.Value.Quota.Value(), INT64_C(4294967296));
    EXPECT_EQ(response.Value.ETag.ToString(), "\"entity\"");
    auto const& request = Transport->Requests.at(0);
    EXPECT_EQ(request.Headers.at("x-ms-version"), "2023-11-03");
    EXPECT_EQ(request.Headers.at("x-ms-share-quota"), "4294967296");
    EXPECT_EQ(request.Headers.at("x-ms-meta-name"), "value");
    EXPECT_EQ(request.Headers.at("x-ms-meta-empty"), "");
    EXPECT_EQ(request.Headers.count("Accept"), 0U);
    EXPECT_EQ(request.ContextValue, "forwarded");
  }

  TEST_F(SharesProtocolTest, HierarchyAndUserPoliciesPreserveOverrides)
  {
    auto options = Options();
    options.PerOperationPolicies.emplace_back(std::make_unique<UserPolicy>());
    ShareServiceClient service("https://account.file.example?sig=sample%2Bsignature%3D", options);
    Transport->Replies
        = {{Http::HttpStatusCode::Accepted, "", {}},
           {Http::HttpStatusCode::Accepted, "", {}},
           {Http::HttpStatusCode::Accepted, "", {}}};
    auto share = service.GetShareClient("share");
    share.GetRootDirectoryClient().GetFileClient("file").Delete();
    share.GetRootDirectoryClient().GetSubdirectoryClient("directory").Delete();
    share.Delete();
    ASSERT_EQ(Transport->Requests.size(), 3U);
    for (auto const& request : Transport->Requests)
    {
      EXPECT_EQ(request.Headers.at("Accept"), "application/custom");
      EXPECT_EQ(request.Headers.at("Content-Type"), "application/custom");
    }
  }

  TEST_F(SharesProtocolTest, StorageErrorsAndIdempotentDelete)
  {
    ShareFileClient client(Endpoint, Options());
    Transport->Replies
        = {{Http::HttpStatusCode::NotFound,
            "<Error><Code>ResourceNotFound</Code><Message>missing</Message></Error>",
            {{"x-ms-error-code", "ResourceNotFound"}}},
           {Http::HttpStatusCode::Forbidden,
            "<Error><Code>AuthorizationFailure</Code><Message>denied</Message></Error>",
            {{"x-ms-error-code", "AuthorizationFailure"}}}};
    EXPECT_FALSE(client.DeleteIfExists().Value.Deleted);
    try
    {
      client.Delete();
      FAIL() << "Expected a StorageException.";
    }
    catch (StorageException const& error)
    {
      EXPECT_EQ(error.ErrorCode, "AuthorizationFailure");
      ASSERT_NE(error.RawResponse, nullptr);
      EXPECT_EQ(error.StatusCode, Http::HttpStatusCode::Forbidden);
    }
  }

  TEST_F(SharesProtocolTest, ShareListingPreservesXmlETagsAnd64BitQuota)
  {
    ShareServiceClient client("https://account.file.example?sig=sample%2Bsignature%3D", Options());
    Transport->Replies = {
        {Http::HttpStatusCode::Ok,
         R"(<EnumerationResults ServiceEndpoint="https://account.file.example/"><Shares><Share><Name>share</Name><Properties><Last-Modified>Tue, 20 Oct 2026 01:02:03 GMT</Last-Modified><Etag>W/&quot;etag&amp;value&quot;</Etag><Quota>4294967296</Quota><EnabledProtocols>NFS</EnabledProtocols></Properties><Metadata><Name>value</Name></Metadata></Share></Shares><NextMarker>next+/=</NextMarker></EnumerationResults>)",
         {}},
        {Http::HttpStatusCode::Ok,
         "<EnumerationResults><Shares/><NextMarker/></EnumerationResults>",
         {}}};
    ListSharesOptions options;
    options.ListSharesIncludeFlags
        = Models::ListSharesIncludeFlags::Metadata | Models::ListSharesIncludeFlags::Snapshots;
    auto page = client.ListShares(options);
    ASSERT_EQ(page.Shares.size(), 1U);
    EXPECT_EQ(page.Shares[0].Details.Etag.ToString(), "W/\"etag&value\"");
    EXPECT_EQ(page.Shares[0].Details.Quota, INT64_C(4294967296));
    EXPECT_EQ(page.Shares[0].Metadata.at("name"), "value");
    EXPECT_EQ(page.Shares[0].Details.EnabledProtocols.Value(), Models::ShareProtocols::Nfs);
    page.MoveToNextPage();
    EXPECT_TRUE(page.Shares.empty());
    EXPECT_EQ(
        Core::Url::Decode(Transport->Requests[1].Url.GetQueryParameters().at("marker")), "next+/=");
    EXPECT_EQ(
        Core::Url::Decode(Transport->Requests[0].Url.GetQueryParameters().at("include")),
        "snapshots,metadata");
  }

  TEST_F(SharesProtocolTest, DirectoryListingPreservesTextAttributesAndOptionalProperties)
  {
    ShareDirectoryClient client(Endpoint, Options());
    Transport->Replies = {
        {Http::HttpStatusCode::Ok,
         R"(<EnumerationResults ServiceEndpoint="https://account.file.example/" ShareName="share" DirectoryPath="directory"><Unknown><File><Name>ignored</Name></File></Unknown><Entries><Directory><Name Encoded="true">dir%20one</Name><FileId>dir-id</FileId></Directory><File><Name Encoded="false">file&amp;one</Name><Properties><Content-Length>4294967296</Content-Length><Etag>&quot;file&quot;</Etag></Properties></File></Entries><NextMarker>next+/=</NextMarker></EnumerationResults>)",
         {}},
        {Http::HttpStatusCode::Ok,
         "<EnumerationResults><Entries/><NextMarker/></EnumerationResults>",
         {}}};
    ListFilesAndDirectoriesOptions options;
    options.PageSizeHint = 2;
    options.Include = Models::ListFilesIncludeFlags::ETag;
    auto page = client.ListFilesAndDirectories(options);
    ASSERT_EQ(page.Directories.size(), 1U);
    ASSERT_EQ(page.Files.size(), 1U);
    EXPECT_EQ(page.Directories[0].Name, "dir one");
    EXPECT_EQ(page.Directories[0].Details.SmbProperties.FileId, "dir-id");
    EXPECT_FALSE(page.Directories[0].Details.Etag.HasValue());
    EXPECT_EQ(page.Files[0].Name, "file&one");
    EXPECT_EQ(page.Files[0].Details.FileSize, INT64_C(4294967296));
    EXPECT_EQ(page.Files[0].Details.Etag.ToString(), "\"file\"");
    page.MoveToNextPage();
    EXPECT_TRUE(page.Files.empty());
    EXPECT_EQ(
        Core::Url::Decode(Transport->Requests[1].Url.GetQueryParameters().at("marker")), "next+/=");
  }

  TEST_F(SharesProtocolTest, DownloadStreamsWithoutReadingAndOutlivesClient)
  {
    auto headers = FileHeaders();
    headers.emplace("content-length", "5");
    headers.emplace("content-type", "application/custom");
    headers.emplace("x-ms-meta-name", "value");
    Transport->Replies.push_back({Http::HttpStatusCode::Ok, "hello", headers});
    auto response = [&] {
      ShareFileClient client(Endpoint, Options());
      return client.Download();
    }();
    EXPECT_EQ(*Transport->ResponseReads, 0);
    EXPECT_TRUE(response.RawResponse->GetBody().empty());
    EXPECT_EQ(response.Value.HttpHeaders.ContentType, "application/custom");
    EXPECT_EQ(response.Value.Details.Metadata.at("name"), "value");
    EXPECT_EQ(response.Value.FileSize, 5);
    auto bytes = response.Value.BodyStream->ReadToEnd();
    EXPECT_EQ(std::string(bytes.begin(), bytes.end()), "hello");
    EXPECT_GT(*Transport->ResponseReads, 0);
  }

  TEST_F(SharesProtocolTest, PartialDownloadKeepsTransactionalAndFileHashesSeparate)
  {
    auto headers = FileHeaders();
    headers.emplace("content-length", "3");
    headers.emplace("content-range", "bytes 2-4/10");
    headers.emplace("content-md5", "AQID");
    headers.emplace("x-ms-content-md5", "BAUG");
    Transport->Replies.push_back({Http::HttpStatusCode::PartialContent, "abc", headers});
    ShareFileClient client(Endpoint, Options());
    DownloadFileOptions options;
    Core::Http::HttpRange range;
    range.Offset = 2;
    range.Length = 3;
    options.Range = range;
    options.RangeHashAlgorithm = HashAlgorithm::Md5;
    auto response = client.Download(options);
    EXPECT_EQ(*Transport->ResponseReads, 0);
    EXPECT_EQ(response.Value.ContentRange.Offset, 2);
    EXPECT_EQ(response.Value.ContentRange.Length.Value(), 3);
    EXPECT_EQ(response.Value.FileSize, 10);
    EXPECT_EQ(
        response.Value.TransactionalContentHash.Value().Value, (std::vector<uint8_t>{1, 2, 3}));
    EXPECT_EQ(response.Value.HttpHeaders.ContentHash.Value, (std::vector<uint8_t>{4, 5, 6}));
    EXPECT_EQ(Transport->Requests[0].Headers.at("x-ms-range"), "bytes=2-4");
    EXPECT_EQ(Transport->Requests[0].Headers.at("x-ms-range-get-content-md5"), "true");
  }

  TEST_F(SharesProtocolTest, UploadBorrowsStreamAndPreservesHashAndLease)
  {
    Transport->Replies.push_back({Http::HttpStatusCode::Created, "", EntityHeaders()});
    ShareFileClient client(Endpoint, Options());
    std::vector<uint8_t> bytes{1, 2, 3};
    Core::IO::MemoryBodyStream stream(bytes);
    UploadFileRangeOptions options;
    ContentHash hash;
    hash.Algorithm = HashAlgorithm::Md5;
    hash.Value = bytes;
    options.TransactionalContentHash = hash;
    options.AccessConditions.LeaseId = "lease";
    options.FileLastWrittenMode = Models::FileLastWrittenMode::Preserve;
    auto response = client.UploadRange(7, stream, options);
    auto const& request = Transport->Requests.at(0);
    EXPECT_EQ(request.Stream, &stream);
    EXPECT_EQ(request.Headers.at("content-md5"), "AQID");
    EXPECT_EQ(request.Headers.at("x-ms-range"), "bytes=7-9");
    EXPECT_EQ(request.Headers.at("x-ms-lease-id"), "lease");
    EXPECT_EQ(request.Headers.at("x-ms-file-last-write-time"), "preserve");
    EXPECT_EQ(request.Headers.count("Content-Type"), 0U);
    EXPECT_FALSE(response.Value.IsServerEncrypted);
  }

  TEST_F(SharesProtocolTest, ClearRangeAcceptsAbsentEncryptionHeader)
  {
    Transport->Replies.push_back({Http::HttpStatusCode::Created, "", EntityHeaders()});
    ShareFileClient client(Endpoint, Options());
    auto response = client.ClearRange(512, 512);
    EXPECT_FALSE(response.Value.IsServerEncrypted);
    EXPECT_EQ(Transport->Requests[0].Headers.at("x-ms-range"), "bytes=512-1023");
    EXPECT_EQ(Transport->Requests[0].Headers.at("x-ms-write"), "clear");
    EXPECT_EQ(Transport->Requests[0].Headers.at("Content-Length"), "0");
    EXPECT_TRUE(Transport->Requests[0].Body.empty());
  }

  TEST_F(SharesProtocolTest, RangeListPreservesInclusiveRangesAndContinuation)
  {
    auto headers = EntityHeaders();
    headers.emplace("x-ms-content-length", "4294967296");
    Transport->Replies
        = {{Http::HttpStatusCode::Ok,
            "<Ranges><Range><Start>1</Start><End>3</End></Range><ClearRange><Start>8</"
            "Start><End>8</End></ClearRange><NextMarker>next+/=</NextMarker></Ranges>",
            headers},
           {Http::HttpStatusCode::Ok, "<Ranges/>", headers}};
    ShareFileClient client(Endpoint, Options());
    GetFileRangeListOptions options;
    options.PageSizeHint = 1;
    auto page = client.GetAllRangeList(options);
    ASSERT_EQ(page.Ranges.size(), 1U);
    EXPECT_EQ(page.Ranges[0].Offset, 1);
    EXPECT_EQ(page.Ranges[0].Length.Value(), 3);
    page.MoveToNextPage();
    EXPECT_TRUE(page.Ranges.empty());
    EXPECT_EQ(
        Core::Url::Decode(Transport->Requests[1].Url.GetQueryParameters().at("marker")), "next+/=");
    EXPECT_EQ(Transport->Requests[0].Url.GetQueryParameters().at("maxresults"), "1");
  }

  TEST_F(SharesProtocolTest, ShareStatisticsAre64BitAndEntityHeadersOptional)
  {
    Transport->Replies.push_back(
        {Http::HttpStatusCode::Ok,
         "<ShareStats><ShareUsageBytes>4294967296</ShareUsageBytes></ShareStats>",
         {}});
    ShareClient client(Endpoint, Options());
    auto response = client.GetStatistics();
    EXPECT_EQ(response.Value.ShareUsageInBytes, INT64_C(4294967296));
    EXPECT_FALSE(response.Value.ETag.HasValue());
  }

  TEST_F(SharesProtocolTest, AccessPolicyAllowsMissingDatesAndUsesLegacyXmlContentType)
  {
    Transport->Replies
        = {{Http::HttpStatusCode::Ok,
            "<SignedIdentifiers><SignedIdentifier><Id>id</Id><AccessPolicy><Permission>r</"
            "Permission></AccessPolicy></SignedIdentifier></SignedIdentifiers>",
            {}},
           {Http::HttpStatusCode::Ok, "", EntityHeaders()}};
    ShareClient client(Endpoint, Options());
    auto response = client.GetAccessPolicy();
    ASSERT_EQ(response.Value.SignedIdentifiers.size(), 1U);
    EXPECT_FALSE(response.Value.SignedIdentifiers[0].Policy.StartsOn.HasValue());
    EXPECT_FALSE(response.Value.SignedIdentifiers[0].Policy.ExpiresOn.HasValue());
    EXPECT_EQ(response.Value.SignedIdentifiers[0].Policy.Permission, "r");
    client.SetAccessPolicy(response.Value.SignedIdentifiers);
    auto const& request = Transport->Requests[1];
    EXPECT_EQ(request.Headers.at("Content-Type"), "application/xml; charset=UTF-8");
    EXPECT_NE(request.Body.find("<SignedIdentifier>"), std::string::npos);
    EXPECT_NE(request.Body.find("<Permission>r</Permission>"), std::string::npos);
    EXPECT_EQ(request.Body.find("<Start>"), std::string::npos);
  }

  TEST_F(SharesProtocolTest, ServiceXmlPropertiesPreserveNestedSettings)
  {
    Transport->Replies = {
        {Http::HttpStatusCode::Ok,
         "<StorageServiceProperties><HourMetrics><Version>1.0</Version><Enabled>true</"
         "Enabled><IncludeAPIs>false</IncludeAPIs><RetentionPolicy><Enabled>true</Enabled><Days>7</"
         "Days></RetentionPolicy></HourMetrics><ProtocolSettings><SMB><Multichannel><Enabled>true</"
         "Enabled></Multichannel><EncryptionInTransit><Required>false</Required></"
         "EncryptionInTransit></SMB><NFS><EncryptionInTransit><Required>true</Required></"
         "EncryptionInTransit></NFS></ProtocolSettings><Cors/></StorageServiceProperties>",
         {}},
        {Http::HttpStatusCode::Accepted, "", {}}};
    ShareServiceClient client("https://account.file.example?sig=sample%2Bsignature%3D", Options());
    auto response = client.GetProperties();
    EXPECT_TRUE(response.Value.HourMetrics.Enabled);
    EXPECT_EQ(response.Value.HourMetrics.RetentionPolicy.Days.Value(), 7);
    EXPECT_TRUE(response.Value.Protocol.Value().Settings.Value().Multichannel.Value().Enabled);
    EXPECT_TRUE(
        response.Value.Protocol.Value().NfsSettings.Value().EncryptionInTransit.Value().Required);
    client.SetProperties(response.Value);
    EXPECT_EQ(Transport->Requests[1].Headers.at("Content-Type"), "application/xml; charset=UTF-8");
    EXPECT_NE(
        Transport->Requests[1].Body.find("<IncludeAPIs>false</IncludeAPIs>"), std::string::npos);
    EXPECT_NE(Transport->Requests[1].Body.find("<Required>true</Required>"), std::string::npos);
  }

  TEST_F(SharesProtocolTest, NfsLinkOperationsPreserve64BitLinkCount)
  {
    auto headers = FileHeaders();
    headers.emplace("x-ms-mode", "0755");
    headers.emplace("x-ms-owner", "123");
    headers.emplace("x-ms-group", "456");
    headers.emplace("x-ms-file-file-type", "SymLink");
    headers.emplace("x-ms-link-count", "4294967296");
    headers.emplace("x-ms-link-text", "../target");
    Transport->Replies
        = {{Http::HttpStatusCode::Created, "", headers},
           {Http::HttpStatusCode::Ok, "", headers},
           {Http::HttpStatusCode::Created, "", headers}};
    ShareFileClient client(Endpoint, Options());
    auto symbolic = client.CreateSymbolicLink("../target");
    EXPECT_EQ(symbolic.Value.SmbProperties.FileId, "file-id");
    EXPECT_EQ(client.GetSymbolicLink().Value.LinkText, "../target");
    auto hard = client.CreateHardLink("share/target");
    EXPECT_EQ(hard.Value.PosixProperties.LinkCount.Value(), INT64_C(4294967296));
    EXPECT_EQ(Transport->Requests[0].Headers.at("x-ms-link-text"), "../target");
    EXPECT_EQ(Transport->Requests[2].Headers.at("x-ms-file-target-file"), "share/target");
  }

  TEST_F(SharesProtocolTest, MissingNestedServiceBooleansRetainLegacyDefaults)
  {
    ShareServiceClient client(Endpoint, Options());
    Transport->Replies.push_back(
        {Http::HttpStatusCode::Ok,
         "<StorageServiceProperties><ProtocolSettings>"
         "<SMB><Multichannel/><EncryptionInTransit/></SMB>"
         "<NFS><EncryptionInTransit/></NFS>"
         "</ProtocolSettings></StorageServiceProperties>",
         {}});
    auto properties = client.GetProperties().Value;
    ASSERT_TRUE(properties.Protocol.HasValue());
    auto const& protocol = properties.Protocol.Value();
    ASSERT_TRUE(protocol.Settings.HasValue());
    ASSERT_TRUE(protocol.Settings.Value().Multichannel.HasValue());
    EXPECT_FALSE(protocol.Settings.Value().Multichannel.Value().Enabled);
    ASSERT_TRUE(protocol.Settings.Value().EncryptionInTransit.HasValue());
    EXPECT_FALSE(protocol.Settings.Value().EncryptionInTransit.Value().Required);
    ASSERT_TRUE(protocol.NfsSettings.HasValue());
    ASSERT_TRUE(protocol.NfsSettings.Value().EncryptionInTransit.HasValue());
    EXPECT_FALSE(protocol.NfsSettings.Value().EncryptionInTransit.Value().Required);
  }

  TEST_F(SharesProtocolTest, TransportFailuresAreNotTranslated)
  {
    ShareClient client(Endpoint, Options());
    Transport->BeforeSend = [] { throw std::logic_error("transport failure"); };
    EXPECT_THROW(client.Delete(), std::logic_error);
  }

  TEST_F(SharesProtocolTest, UploadFromUriPreservesCrc64AndSourceConditions)
  {
    auto headers = EntityHeaders();
    headers.emplace("x-ms-content-crc64", "AQIDBAUGBwg=");
    Transport->Replies.push_back({Http::HttpStatusCode::Created, "", headers});
    auto clientOptions = Options();
    clientOptions.AllowTrailingDot = true;
    clientOptions.AllowSourceTrailingDot = false;
    ShareFileClient client(Endpoint, clientOptions);
    UploadFileRangeFromUriOptions options;
    ContentHash hash;
    hash.Algorithm = HashAlgorithm::Crc64;
    hash.Value = {1, 2, 3, 4, 5, 6, 7, 8};
    options.TransactionalContentHash = hash;
    options.SourceAccessCondition.IfMatchContentHash = hash;
    options.AccessConditions.LeaseId = "lease";
    options.SourceAuthorization = "Bearer sample-token";
    Core::Http::HttpRange range;
    range.Offset = 4;
    range.Length = 8;
    auto response = client.UploadRangeFromUri(16, "https://source.example/file", range, options);
    EXPECT_EQ(response.Value.TransactionalContentHash.Value, hash.Value);
    EXPECT_EQ(response.Value.TransactionalContentHash.Algorithm, HashAlgorithm::Crc64);
    auto const& request = Transport->Requests.at(0);
    EXPECT_EQ(request.Headers.at("x-ms-range"), "bytes=16-23");
    EXPECT_EQ(request.Headers.at("x-ms-source-range"), "bytes=4-11");
    EXPECT_EQ(request.Headers.at("x-ms-source-content-crc64"), "AQIDBAUGBwg=");
    EXPECT_EQ(request.Headers.at("x-ms-source-if-match-crc64"), "AQIDBAUGBwg=");
    EXPECT_EQ(request.Headers.at("x-ms-copy-source-authorization"), "Bearer sample-token");
    EXPECT_EQ(request.Headers.at("x-ms-allow-trailing-dot"), "true");
    EXPECT_EQ(request.Headers.at("x-ms-source-allow-trailing-dot"), "false");
  }

  TEST_F(SharesProtocolTest, RetryRewindsCallerOwnedUploadStream)
  {
    auto options = Options();
    options.Retry.MaxRetries = 1;
    options.Retry.RetryDelay = std::chrono::milliseconds(1);
    options.Retry.MaxRetryDelay = std::chrono::milliseconds(1);
    ShareFileClient client(Endpoint, options);
    Transport->Replies
        = {{Http::HttpStatusCode::ServiceUnavailable, "<Error><Code>ServerBusy</Code></Error>", {}},
           {Http::HttpStatusCode::Created, "", EntityHeaders()}};
    std::vector<uint8_t> bytes{'a', 'b', 'c'};
    Core::IO::MemoryBodyStream stream(bytes);
    client.UploadRange(0, stream);
    ASSERT_EQ(Transport->Requests.size(), 2U);
    EXPECT_EQ(Transport->Requests[0].Body, "abc");
    EXPECT_EQ(Transport->Requests[1].Body, "abc");
    EXPECT_EQ(Transport->Requests[1].Stream, &stream);
  }

  TEST_F(SharesProtocolTest, CancellationDoesNotReadOrSendUpload)
  {
    ShareFileClient client(Endpoint, Options());
    auto reads = std::make_shared<int>(0);
    ReadCountingStream stream("abc", reads);
    auto context
        = Core::Context().WithDeadline(std::chrono::system_clock::now() - std::chrono::seconds(1));
    EXPECT_THROW(client.UploadRange(0, stream, {}, context), Core::OperationCancelledException);
    EXPECT_EQ(*reads, 0);
    EXPECT_TRUE(Transport->Requests.empty());
  }

}}}} // namespace Azure::Storage::Test::
