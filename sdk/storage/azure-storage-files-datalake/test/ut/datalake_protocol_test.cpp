// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/exception.hpp>
#include <azure/core/http/transport.hpp>
#include <azure/storage/common/internal/storage_switch_to_secondary_policy.hpp>
#include <azure/storage/common/storage_exception.hpp>
#include <azure/storage/files/datalake.hpp>

#include <deque>
#include <functional>
#include <stdexcept>

#include <gtest/gtest.h>

namespace Azure { namespace Storage { namespace Test { namespace {
  namespace Http = Core::Http;
  using namespace Files::DataLake;

  Core::Context::Key const ContextKey;
  char const* const Date = "Sun, 18 Oct 2026 10:20:30 GMT";
  char const* const Endpoint
      = "https://account.dfs.example/filesystem/path?sv=2026-06-06&sig=not-a-secret%2Bsample%3D";

  struct Reply
  {
    Http::HttpStatusCode Status;
    std::string Body;
    Core::CaseInsensitiveMap Headers;
  };

  struct RecordedRequest
  {
    Core::Url Url;
    Core::CaseInsensitiveMap Headers;
    std::string Body;
    Core::IO::BodyStream* Stream;
    std::string ContextValue;
    bool HasReplicaStatus;
  };

  class TrackingStream final : public Core::IO::BodyStream {
    Core::IO::MemoryBodyStream m_stream;

    size_t OnRead(uint8_t* buffer, size_t count, Core::Context const& context) override
    {
      ++ReadsSinceRewind;
      return m_stream.Read(buffer, count, context);
    }

  public:
    int ReadsSinceRewind = 0;

    explicit TrackingStream(std::vector<uint8_t> const& bytes) : m_stream(bytes) {}
    int64_t Length() const override { return m_stream.Length(); }
    void Rewind() override
    {
      ReadsSinceRewind = 0;
      m_stream.Rewind();
    }
  };

  class UserPolicy final : public Http::Policies::HttpPolicy {
    std::shared_ptr<int> m_count;

  public:
    explicit UserPolicy(std::shared_ptr<int> count) : m_count(std::move(count)) {}
    std::unique_ptr<Http::Policies::HttpPolicy> Clone() const override
    {
      return std::make_unique<UserPolicy>(m_count);
    }
    std::unique_ptr<Http::RawResponse> Send(
        Http::Request& request,
        Http::Policies::NextHttpPolicy nextPolicy,
        Core::Context const& context) const override
    {
      ++*m_count;
      request.SetHeader("Accept", "application/custom");
      request.SetHeader("Content-Type", "application/custom");
      return nextPolicy.Send(request, context);
    }
  };

  class ProtocolTransport final : public Http::HttpTransport {
  public:
    std::deque<Reply> Replies;
    std::vector<RecordedRequest> Requests;
    std::function<void(Http::Request&)> BeforeSend;

    std::unique_ptr<Http::RawResponse> Send(Http::Request& request, Core::Context const& context)
        override
    {
      context.ThrowIfCancelled();
      if (Replies.empty())
      {
        throw std::runtime_error("Unexpected request in Data Lake protocol test.");
      }
      if (BeforeSend)
      {
        BeforeSend(request);
      }
      std::string contextValue;
      context.TryGetValue(ContextKey, contextValue);
      std::shared_ptr<bool> replica;
      auto const hasReplica
          = context.TryGetValue(_internal::SecondaryHostReplicaStatusKey, replica);
      std::string body;
      if (auto stream = request.GetBodyStream())
      {
        auto bytes = stream->ReadToEnd(context);
        body.assign(bytes.begin(), bytes.end());
      }
      Requests.push_back(
          {request.GetUrl(),
           request.GetHeaders(),
           std::move(body),
           request.GetBodyStream(),
           std::move(contextValue),
           hasReplica});
      auto reply = std::move(Replies.front());
      Replies.pop_front();
      auto response = std::make_unique<Http::RawResponse>(1, 1, reply.Status, "Test response");
      response->SetHeader("date", Date);
      response->SetHeader("x-ms-version", "2026-06-06");
      response->SetHeader("x-ms-request-id", "test-request");
      if (!reply.Body.empty())
      {
        response->SetHeader(
            "content-type", reply.Body[0] == '<' ? "application/xml" : "application/json");
      }
      for (auto const& header : reply.Headers)
      {
        response->SetHeader(header.first, header.second);
      }
      response->SetBody(std::vector<uint8_t>(reply.Body.begin(), reply.Body.end()));
      response->SetBodyStream(std::make_unique<Core::IO::MemoryBodyStream>(response->GetBody()));
      return response;
    }
  };

  class DataLakeProtocolTest : public ::testing::Test {
  protected:
    std::shared_ptr<ProtocolTransport> Transport = std::make_shared<ProtocolTransport>();

    DataLakeClientOptions Options()
    {
      DataLakeClientOptions options;
      options.Transport.Transport = Transport;
      options.Retry.MaxRetries = 0;
      return options;
    }

    static Core::CaseInsensitiveMap EntityHeaders()
    {
      return {{"etag", "\"entity\""}, {"last-modified", Date}};
    }
  };

  TEST_F(DataLakeProtocolTest, ListPathsWithoutEntityHeadersAndPaging)
  {
    DataLakeDirectoryClient client(Endpoint, Options());
    Transport->Replies = {
        {Http::HttpStatusCode::Ok,
         R"({"paths":[{"name":"path/file","isDirectory":"true","lastModified":"Sun, 18 Oct 2026 10:20:30 GMT","contentLength":"4294967296","owner":"owner","group":"group","permissions":"rwx------","etag":"\"item\"","EncryptionScope":"scope","EncryptionContext":"context","creationTime":"132223104000000000","expiryTime":"0"}]})",
         {{"x-ms-continuation", "next+/="}}},
        {Http::HttpStatusCode::Ok, R"({"paths":[]})", {}}};
    ListPathsOptions options;
    options.PageSizeHint = 2;
    options.StartFrom = "a b";
    options.UserPrincipalName = true;
    auto context = Core::Context().WithValue(ContextKey, std::string("forwarded"));
    auto page = client.ListPaths(true, options, context);
    ASSERT_EQ(page.Paths.size(), 1U);
    auto const& item = page.Paths[0];
    EXPECT_TRUE(item.IsDirectory);
    EXPECT_EQ(item.FileSize, INT64_C(4294967296));
    EXPECT_EQ(item.ETag, "\"item\"");
    EXPECT_EQ(item.EncryptionScope.Value(), "scope");
    EXPECT_EQ(item.EncryptionContext.Value(), "context");
    EXPECT_EQ(item.CreatedOn.Value(), DateTime(2020, 1, 1));
    EXPECT_FALSE(item.ExpiresOn.HasValue());
    EXPECT_EQ(item.LastModified, DateTime::Parse(Date, DateTime::DateFormat::Rfc1123));
    EXPECT_EQ(page.NextPageToken.Value(), "next+/=");
    EXPECT_EQ(page.RawResponse->GetHeaders().count("etag"), 0U);
    page.MoveToNextPage(context);
    EXPECT_TRUE(page.Paths.empty());
    ASSERT_EQ(Transport->Requests.size(), 2U);
    auto const& first = Transport->Requests[0];
    EXPECT_EQ(first.Url.GetPath(), "filesystem");
    EXPECT_EQ(Core::Url::Decode(first.Url.GetQueryParameters().at("directory")), "path");
    EXPECT_EQ(Core::Url::Decode(first.Url.GetQueryParameters().at("beginFrom")), "a b");
    EXPECT_EQ(first.Url.GetQueryParameters().at("recursive"), "true");
    EXPECT_EQ(first.Url.GetQueryParameters().at("upn"), "true");
    EXPECT_EQ(first.Url.GetQueryParameters().at("maxResults"), "2");
    EXPECT_EQ(first.ContextValue, "forwarded");
    EXPECT_TRUE(first.HasReplicaStatus);
    EXPECT_EQ(
        Core::Url::Decode(Transport->Requests[1].Url.GetQueryParameters().at("continuation")),
        "next+/=");
    for (auto const& request : Transport->Requests)
    {
      EXPECT_EQ(request.Url.GetQueryParameters().at("sig"), "not-a-secret%2Bsample%3D");
      EXPECT_EQ(request.Headers.count("Accept"), 0U);
    }
  }

  TEST_F(DataLakeProtocolTest, FileSystemListingNativeBooleansAndAbsentDirectoryFlag)
  {
    DataLakeFileSystemClient client(
        "https://account.dfs.example/filesystem?sig=not-a-secret", Options());
    Transport->Replies = {
        {Http::HttpStatusCode::Ok,
         R"({"paths":[{"name":"directory","isDirectory":true,"lastModified":"Sun, 18 Oct 2026 10:20:30 GMT","contentLength":0,"owner":"","group":"","permissions":""},{"name":"file","lastModified":"Sun, 18 Oct 2026 10:20:30 GMT","contentLength":12,"owner":"","group":"","permissions":""}]})",
         {}}};
    auto page = client.ListPaths(false);
    ASSERT_EQ(page.Paths.size(), 2U);
    EXPECT_TRUE(page.Paths[0].IsDirectory);
    EXPECT_FALSE(page.Paths[1].IsDirectory);
    EXPECT_TRUE(page.Paths[1].ETag.empty());
    EXPECT_EQ(Transport->Requests[0].Url.GetQueryParameters().at("recursive"), "false");
  }

  TEST_F(DataLakeProtocolTest, LenientQuotedDirectoryFlag)
  {
    DataLakeFileSystemClient client("https://account.dfs.example/filesystem", Options());
    for (auto const& value : {"True", "TRUE", "tRuE"})
    {
      Transport->Replies.push_back(
          {Http::HttpStatusCode::Ok,
           std::string(R"({"paths":[{"name":"directory","isDirectory":")") + value
               + R"(","lastModified":"Sun, 18 Oct 2026 10:20:30 GMT","contentLength":0,"owner":"","group":"","permissions":""}]})",
           {}});
      EXPECT_TRUE(client.ListPaths(false).Paths[0].IsDirectory);
    }
  }

  TEST_F(DataLakeProtocolTest, CreateConditionsVersionAndIdempotency)
  {
    auto options = Options();
    options.ApiVersion = "2019-12-12";
    DataLakeFileClient client(Endpoint, options);
    auto headers = EntityHeaders();
    headers["content-length"] = "4294967296";
    headers["x-ms-request-server-encrypted"] = "true";
    headers["x-ms-encryption-key-sha256"] = "AQID";
    Transport->Replies
        = {{Http::HttpStatusCode::Created, "", headers},
           {Http::HttpStatusCode::Conflict,
            "<Error><Code>PathAlreadyExists</Code><Message>Exists</Message></Error>",
            {}}};
    CreateFileOptions create;
    create.Metadata = {{"name", "value"}};
    create.Permissions = "0640";
    create.Owner = "owner";
    create.Group = "group";
    create.AccessConditions.IfMatch = ETag("\"condition\"");
    create.AccessConditions.LeaseId = "lease";
    auto response = client.Create(create);
    EXPECT_EQ(response.Value.ETag.ToString(), "\"entity\"");
    EXPECT_EQ(response.Value.FileSize.Value(), INT64_C(4294967296));
    EXPECT_TRUE(response.Value.IsServerEncrypted);
    EXPECT_EQ(response.Value.EncryptionKeySha256.Value(), (std::vector<uint8_t>{1, 2, 3}));
    EXPECT_FALSE(client.CreateIfNotExists().Value.Created);
    auto const& request = Transport->Requests[0];
    EXPECT_EQ(request.Headers.at("x-ms-version"), "2019-12-12");
    EXPECT_EQ(request.Headers.at("If-Match"), "\"condition\"");
    EXPECT_EQ(request.Headers.at("x-ms-lease-id"), "lease");
    EXPECT_EQ(request.Headers.at("x-ms-properties"), "name=dmFsdWU=");
    EXPECT_EQ(request.Headers.at("x-ms-permissions"), "0640");
    EXPECT_EQ(request.Url.GetQueryParameters().at("resource"), "file");
    EXPECT_EQ(request.Headers.count("x-ms-content-type"), 0U);
    EXPECT_EQ(request.Headers.count("Accept"), 0U);
    EXPECT_EQ(Transport->Requests[1].Headers.at("If-None-Match"), "*");
  }

  TEST_F(DataLakeProtocolTest, RenamePreservesSourceConditionsAndEscaping)
  {
    DataLakeDirectoryClient client(Endpoint, Options());
    Transport->Replies = {{Http::HttpStatusCode::Created, "", EntityHeaders()}};
    RenameFileOptions options;
    options.SourceAccessConditions.IfMatch = ETag("W/\"source\"");
    options.SourceAccessConditions.IfNoneMatch = ETag("*");
    options.SourceAccessConditions.LeaseId = "source-lease";
    options.AccessConditions.IfMatch = ETag("\"destination\"");
    auto result = client.RenameFile("file name", "other/new name", options);
    auto const& request = Transport->Requests[0];
    EXPECT_EQ(request.Url.GetPath(), "filesystem/other/new%20name");
    auto source = Core::Url("https://source.example" + request.Headers.at("x-ms-rename-source"));
    EXPECT_EQ(source.GetPath(), "filesystem/path/file%20name");
    EXPECT_EQ(source.GetQueryParameters().at("sig"), "not-a-secret%2Bsample%3D");
    EXPECT_EQ(request.Headers.at("x-ms-source-if-match"), "W/\"source\"");
    EXPECT_EQ(request.Headers.at("x-ms-source-if-none-match"), "*");
    EXPECT_EQ(request.Headers.at("x-ms-source-lease-id"), "source-lease");
    EXPECT_EQ(request.Headers.at("If-Match"), "\"destination\"");
    EXPECT_EQ(request.Url.GetQueryParameters().at("mode"), "legacy");
    EXPECT_EQ(request.Url.GetQueryParameters().at("sig"), "not-a-secret%2Bsample%3D");
    EXPECT_EQ(Core::Url(result.Value.GetUrl()).GetPath(), request.Url.GetPath());
    EXPECT_EQ(
        Core::Url(result.Value.GetUrl()).GetQueryParameters().at("sig"),
        "not-a-secret%2Bsample%3D");
  }

  TEST_F(DataLakeProtocolTest, AccessControlAndSystemPropertiesHaveDifferentHeaderRequirements)
  {
    DataLakeFileClient client(Endpoint, Options());
    auto aclHeaders = Core::CaseInsensitiveMap{
        {"x-ms-owner", "owner"},
        {"x-ms-group", "group"},
        {"x-ms-permissions", "rwx------"},
        {"x-ms-acl", "user::rwx,group::---,other::---"}};
    auto systemHeaders = EntityHeaders();
    systemHeaders.insert(aclHeaders.begin(), aclHeaders.end());
    systemHeaders["content-length"] = "4294967296";
    systemHeaders["x-ms-resource-type"] = "file";
    systemHeaders["x-ms-server-encrypted"] = "true";
    Transport->Replies
        = {{Http::HttpStatusCode::Ok, "", aclHeaders},
           {Http::HttpStatusCode::Ok, "", systemHeaders},
           {Http::HttpStatusCode::Ok, "", EntityHeaders()}};
    auto acl = client.GetAccessControlList();
    EXPECT_EQ(acl.Value.Owner, "owner");
    ASSERT_EQ(acl.Value.Acls.size(), 3U);
    EXPECT_EQ(acl.Value.Acls[0].Permissions, "rwx");
    auto properties = client.GetSystemProperties();
    EXPECT_EQ(properties.Value.FileSize, INT64_C(4294967296));
    EXPECT_TRUE(properties.Value.IsServerEncrypted);
    EXPECT_FALSE(properties.Value.IsDirectory);
    EXPECT_FALSE(properties.Value.CreatedOn.HasValue());
    EXPECT_FALSE(properties.Value.ExpiresOn.HasValue());
    auto permissions = client.SetPermissions("0750");
    EXPECT_EQ(permissions.Value.ETag.ToString(), "\"entity\"");
    EXPECT_EQ(Transport->Requests[0].Url.GetQueryParameters().at("action"), "getAccessControl");
    EXPECT_EQ(Transport->Requests[1].Url.GetQueryParameters().at("action"), "getStatus");
    EXPECT_EQ(Transport->Requests[2].Url.GetQueryParameters().at("action"), "setAccessControl");
    EXPECT_EQ(Transport->Requests[2].Headers.at("x-ms-permissions"), "0750");
    EXPECT_TRUE(Transport->Requests[0].HasReplicaStatus);
    EXPECT_TRUE(Transport->Requests[1].HasReplicaStatus);
  }

  TEST_F(DataLakeProtocolTest, RecursiveAclPagination)
  {
    DataLakeDirectoryClient client(Endpoint, Options());
    Transport->Replies = {
        {Http::HttpStatusCode::Ok,
         R"({"directoriesSuccessful":"2","filesSuccessful":"3","failureCount":"1","failedEntries":[{"name":"failed","type":"file","errorMessage":"denied"}]})",
         {{"x-ms-continuation", "next+/="}}},
        {Http::HttpStatusCode::Ok,
         R"({"directoriesSuccessful":0,"filesSuccessful":1,"failureCount":0})",
         {}}};
    SetPathAccessControlListRecursiveOptions options;
    options.PageSizeHint = 2;
    options.ContinueOnFailure = true;
    auto page
        = client.SetAccessControlListRecursive({Models::Acl::FromString("user::rwx")}, options);
    EXPECT_EQ(page.NumberOfSuccessfulFiles, 3);
    EXPECT_EQ(page.NumberOfFailures, 1);
    ASSERT_EQ(page.FailedEntries.size(), 1U);
    EXPECT_EQ(page.FailedEntries[0].ErrorMessage, "denied");
    page.MoveToNextPage();
    EXPECT_EQ(page.NumberOfSuccessfulFiles, 1);
    auto const& request = Transport->Requests[0];
    EXPECT_EQ(request.Url.GetQueryParameters().at("action"), "setAccessControlRecursive");
    EXPECT_EQ(request.Url.GetQueryParameters().at("mode"), "set");
    EXPECT_EQ(request.Url.GetQueryParameters().at("maxRecords"), "2");
    EXPECT_EQ(request.Url.GetQueryParameters().at("forceFlag"), "true");
    EXPECT_EQ(request.Headers.at("x-ms-acl"), "user::rwx");
    EXPECT_EQ(
        Core::Url::Decode(Transport->Requests[1].Url.GetQueryParameters().at("continuation")),
        "next+/=");
  }

  TEST_F(DataLakeProtocolTest, DeletePaginationAndIdempotency)
  {
    DataLakeDirectoryClient client(Endpoint, Options());
    Transport->Replies
        = {{Http::HttpStatusCode::Ok, "", {{"x-ms-continuation", "next+/="}}},
           {Http::HttpStatusCode::Ok, "", {}},
           {Http::HttpStatusCode::NotFound,
            "<Error><Code>PathNotFound</Code><Message>Missing</Message></Error>",
            {}}};
    EXPECT_TRUE(client.DeleteRecursive().Value.Deleted);
    EXPECT_FALSE(client.DeleteRecursiveIfExists().Value.Deleted);
    ASSERT_EQ(Transport->Requests.size(), 3U);
    EXPECT_EQ(Transport->Requests[0].Url.GetQueryParameters().at("recursive"), "true");
    EXPECT_EQ(
        Core::Url::Decode(Transport->Requests[1].Url.GetQueryParameters().at("continuation")),
        "next+/=");
  }

  TEST_F(DataLakeProtocolTest, AppendStreamsAndChecksumResponses)
  {
    DataLakeFileClient client(Endpoint, Options());
    Transport->Replies
        = {{Http::HttpStatusCode::Accepted,
            "",
            {{"content-md5", "AQID"},
             {"x-ms-request-server-encrypted", "true"},
             {"x-ms-encryption-key-sha256", "BAUG"},
             {"x-ms-lease-renewed", "true"}}},
           {Http::HttpStatusCode::Accepted,
            "",
            {{"content-md5", "AQID"},
             {"x-ms-content-crc64", "BAUG"},
             {"x-ms-request-server-encrypted", "true"}}}};
    std::vector<uint8_t> bytes = {'a', 'b', 'c'};
    Core::IO::MemoryBodyStream content(bytes);
    AppendFileOptions options;
    options.TransactionalContentHash = ContentHash{};
    options.TransactionalContentHash.Value().Algorithm = HashAlgorithm::Md5;
    options.TransactionalContentHash.Value().Value = {1, 2, 3};
    options.Flush = true;
    auto response = client.Append(content, INT64_C(4294967296), options);
    EXPECT_EQ(response.Value.TransactionalContentHash.Value().Algorithm, HashAlgorithm::Md5);
    EXPECT_EQ(
        response.Value.TransactionalContentHash.Value().Value, (std::vector<uint8_t>{1, 2, 3}));
    EXPECT_EQ(response.Value.EncryptionKeySha256.Value(), (std::vector<uint8_t>{4, 5, 6}));
    EXPECT_TRUE(response.Value.IsLeaseRenewed.Value());
    ASSERT_EQ(Transport->Requests.size(), 1U);
    auto const& request = Transport->Requests[0];
    EXPECT_EQ(request.Stream, &content);
    EXPECT_EQ(request.Body, "abc");
    EXPECT_EQ(request.Headers.at("Content-Length"), "3");
    EXPECT_EQ(request.Headers.at("Content-MD5"), "AQID");
    EXPECT_EQ(request.Headers.count("Content-Type"), 0U);
    EXPECT_EQ(request.Url.GetQueryParameters().at("position"), "4294967296");
    EXPECT_EQ(request.Url.GetQueryParameters().at("flush"), "true");
    content.Rewind();
    response = client.Append(content, 0, options);
    EXPECT_EQ(response.Value.TransactionalContentHash.Value().Algorithm, HashAlgorithm::Crc64);
    EXPECT_EQ(
        response.Value.TransactionalContentHash.Value().Value, (std::vector<uint8_t>{4, 5, 6}));
  }

  TEST_F(DataLakeProtocolTest, FlushConditionsAndRequiredFields)
  {
    DataLakeFileClient client(Endpoint, Options());
    auto headers = EntityHeaders();
    headers["content-length"] = "4294967296";
    headers["x-ms-request-server-encrypted"] = "true";
    headers["x-ms-lease-renewed"] = "false";
    Transport->Replies
        = {{Http::HttpStatusCode::Ok, "", headers}, {Http::HttpStatusCode::Ok, "", {}}};
    FlushFileOptions options;
    options.Close = true;
    options.RetainUncommittedData = false;
    options.HttpHeaders.ContentType = "text/plain";
    options.AccessConditions.IfMatch = ETag("\"condition\"");
    auto response = client.Flush(INT64_C(4294967296), options);
    EXPECT_EQ(response.Value.FileSize, INT64_C(4294967296));
    EXPECT_FALSE(response.Value.IsLeaseRenewed.Value());
    auto const& request = Transport->Requests[0];
    EXPECT_EQ(request.Headers.at("Content-Length"), "0");
    EXPECT_EQ(request.Headers.at("x-ms-content-type"), "text/plain");
    EXPECT_EQ(request.Headers.at("If-Match"), "\"condition\"");
    EXPECT_EQ(request.Url.GetQueryParameters().at("close"), "true");
    EXPECT_EQ(request.Url.GetQueryParameters().at("retainUncommittedData"), "false");
    EXPECT_THROW(client.Flush(0), std::out_of_range);
  }

  TEST_F(DataLakeProtocolTest, AppendRetryKeepsStreamAndUserPolicyPrecedence)
  {
    auto options = Options();
    options.Retry.MaxRetries = 1;
    options.Retry.RetryDelay = std::chrono::milliseconds(0);
    auto operationCount = std::make_shared<int>(0);
    auto retryCount = std::make_shared<int>(0);
    options.PerOperationPolicies.push_back(std::make_unique<UserPolicy>(operationCount));
    options.PerRetryPolicies.push_back(std::make_unique<UserPolicy>(retryCount));
    DataLakeFileClient client(Endpoint, options);
    Transport->Replies
        = {{Http::HttpStatusCode::ServiceUnavailable,
            "<Error><Code>ServerBusy</Code><Message>Retry</Message></Error>",
            {}},
           {Http::HttpStatusCode::Accepted, "", {{"x-ms-request-server-encrypted", "true"}}}};
    std::vector<uint8_t> bytes = {'a', 'b', 'c'};
    TrackingStream content(bytes);
    Transport->BeforeSend = [&](Http::Request& request) {
      EXPECT_EQ(request.GetBodyStream(), &content);
      EXPECT_EQ(content.ReadsSinceRewind, 0);
    };
    client.Append(content, 0);
    EXPECT_EQ(*operationCount, 1);
    EXPECT_EQ(*retryCount, 2);
    ASSERT_EQ(Transport->Requests.size(), 2U);
    for (auto const& request : Transport->Requests)
    {
      EXPECT_EQ(request.Body, "abc");
      EXPECT_EQ(request.Headers.at("Accept"), "application/custom");
      EXPECT_EQ(request.Headers.at("Content-Type"), "application/custom");
    }
  }

  TEST_F(DataLakeProtocolTest, ServiceChildClientsKeepLegacyWireHeaders)
  {
    DataLakeServiceClient service(
        "https://account.dfs.example?sig=not-a-secret%2Bsample%3D", Options());
    auto fileSystem = service.GetFileSystemClient("filesystem");
    auto directory = fileSystem.GetDirectoryClient("directory");
    auto file = directory.GetFileClient("file");
    Transport->Replies
        = {{Http::HttpStatusCode::Ok, R"({"paths":[]})", {}},
           {Http::HttpStatusCode::Accepted, "", {{"x-ms-request-server-encrypted", "true"}}},
           {Http::HttpStatusCode::Ok,
            R"({"directoriesSuccessful":0,"filesSuccessful":0,"failureCount":0})",
            {}}};
    fileSystem.ListPaths(false);
    std::vector<uint8_t> bytes = {'a', 'b', 'c'};
    Core::IO::MemoryBodyStream content(bytes);
    file.Append(content, 0);
    directory.SetAccessControlListRecursive({Models::Acl::FromString("user::rwx")});
    ASSERT_EQ(Transport->Requests.size(), 3U);
    for (auto const& request : Transport->Requests)
    {
      EXPECT_EQ(request.Headers.count("Accept"), 0U);
      EXPECT_EQ(request.Headers.count("Content-Type"), 0U);
      EXPECT_EQ(request.Url.GetQueryParameters().at("sig"), "not-a-secret%2Bsample%3D");
    }
  }

  TEST_F(DataLakeProtocolTest, CancelledAppendDoesNotReadStream)
  {
    DataLakeFileClient client(Endpoint, Options());
    std::vector<uint8_t> bytes = {'a', 'b', 'c'};
    TrackingStream content(bytes);
    auto context
        = Core::Context().WithDeadline(std::chrono::system_clock::now() - std::chrono::seconds(1));
    EXPECT_THROW(client.Append(content, 0, {}, context), Core::OperationCancelledException);
    EXPECT_EQ(content.ReadsSinceRewind, 0);
    EXPECT_TRUE(Transport->Requests.empty());
  }

  TEST_F(DataLakeProtocolTest, EmptyAndStructuredAppendPreserveWireMetadata)
  {
    DataLakeFileClient client(Endpoint, Options());
    Transport->Replies
        = {{Http::HttpStatusCode::Accepted, "", {{"x-ms-request-server-encrypted", "true"}}},
           {Http::HttpStatusCode::Accepted,
            "",
            {{"x-ms-request-server-encrypted", "true"}, {"x-ms-structured-body", "crc64"}}},
           {Http::HttpStatusCode::Accepted, "", {{"x-ms-request-server-encrypted", "true"}}}};
    std::vector<uint8_t> empty;
    Core::IO::MemoryBodyStream emptyStream(empty);
    client.Append(emptyStream, 0);
    EXPECT_TRUE(Transport->Requests[0].Body.empty());
    EXPECT_EQ(Transport->Requests[0].Headers.at("Content-Length"), "0");
    std::vector<uint8_t> bytes = {'a', 'b', 'c'};
    Core::IO::MemoryBodyStream content(bytes);
    AppendFileOptions options;
    TransferValidationOptions validation;
    validation.Algorithm = StorageChecksumAlgorithm::Crc64;
    options.ValidationOptions = validation;
    client.Append(content, 0, options);
    EXPECT_EQ(
        Transport->Requests[1].Headers.at("x-ms-structured-body"), "XSM/1.0; properties=crc64");
    EXPECT_EQ(Transport->Requests[1].Headers.at("x-ms-structured-content-length"), "3");
    EXPECT_GT(Transport->Requests[1].Body.size(), bytes.size());
    content.Rewind();
    EXPECT_THROW(client.Append(content, 0, options), StorageException);
  }

  TEST_F(DataLakeProtocolTest, UndeleteUsesBlobEndpointAndPreservesSource)
  {
    DataLakeFileSystemClient client(
        "https://account.dfs.example/filesystem?sig=not-a-secret", Options());
    Transport->Replies = {{Http::HttpStatusCode::Ok, "", {{"x-ms-resource-type", "directory"}}}};
    auto response = client.UndeletePath("deleted path", "deletion-id");
    EXPECT_EQ(Transport->Requests[0].Url.GetHost(), "account.blob.example");
    EXPECT_EQ(Transport->Requests[0].Url.GetPath(), "filesystem/deleted%20path");
    EXPECT_EQ(Transport->Requests[0].Url.GetQueryParameters().at("comp"), "undelete");
    EXPECT_EQ(Transport->Requests[0].Headers.at("x-ms-undelete-source"), "?deletionid=deletion-id");
    EXPECT_NE(response.Value.GetUrl().find("deleted%20path"), std::string::npos);
  }

  TEST_F(DataLakeProtocolTest, StorageErrorsKeepOwnedResponses)
  {
    DataLakeFileClient client(Endpoint, Options());
    Transport->Replies
        = {{Http::HttpStatusCode::Forbidden,
            "<Error><Code>AuthorizationFailure</Code><Message>Denied</Message></Error>",
            {}}};
    try
    {
      client.Create();
      FAIL() << "Expected StorageException";
    }
    catch (StorageException const& error)
    {
      EXPECT_EQ(error.ErrorCode, "AuthorizationFailure");
      EXPECT_EQ(error.StatusCode, Http::HttpStatusCode::Forbidden);
      ASSERT_NE(error.RawResponse, nullptr);
      EXPECT_EQ(error.RawResponse->GetHeaders().at("x-ms-request-id"), "test-request");
      EXPECT_EQ(error.Message, "Denied");
    }
  }
}}}} // namespace Azure::Storage::Test::
