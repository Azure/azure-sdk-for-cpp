// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../src/private/queue_protocol.hpp"

#include <azure/core/exception.hpp>
#include <azure/core/http/transport.hpp>
#include <azure/storage/common/internal/storage_switch_to_secondary_policy.hpp>
#include <azure/storage/queues.hpp>

#include <deque>
#include <functional>
#include <limits>
#include <stdexcept>

#include <gtest/gtest.h>

namespace Azure { namespace Storage { namespace Test { namespace {
  namespace Http = Core::Http;
  using namespace Queues;

  Core::Context::Key const ContextKey;
  char const* const Date = "Sun, 18 Oct 2026 10:20:30 GMT";
  char const* const Endpoint
      = "https://account.queue.example/queue?sv=2026-04-06&sig=not-a-secret%2Bsample%3D";

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
    std::string ContextValue;
    bool HasReplicaStatus;
  };

  class ProtocolTransport final : public Http::HttpTransport {
  public:
    std::deque<Reply> Replies;
    std::vector<RecordedRequest> Requests;

    std::unique_ptr<Http::RawResponse> Send(Http::Request& request, Core::Context const& context)
        override
    {
      context.ThrowIfCancelled();
      if (Replies.empty())
      {
        throw std::runtime_error("Unexpected request in Queue protocol test.");
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
           std::move(contextValue),
           hasReplica});
      auto reply = std::move(Replies.front());
      Replies.pop_front();
      auto response = std::make_unique<Http::RawResponse>(1, 1, reply.Status, "Test response");
      response->SetHeader("date", Date);
      response->SetHeader("x-ms-version", "2026-04-06");
      response->SetHeader("x-ms-request-id", "test-request");
      if (!reply.Body.empty())
      {
        response->SetHeader("content-type", "application/xml");
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

  class CountingPolicy final : public Http::Policies::HttpPolicy {
  private:
    std::shared_ptr<int> m_count;

  public:
    explicit CountingPolicy(std::shared_ptr<int> count) : m_count(std::move(count)) {}

    std::unique_ptr<Http::Policies::HttpPolicy> Clone() const override
    {
      return std::make_unique<CountingPolicy>(m_count);
    }

    std::unique_ptr<Http::RawResponse> Send(
        Http::Request& request,
        Http::Policies::NextHttpPolicy nextPolicy,
        Core::Context const& context) const override
    {
      ++*m_count;
      request.SetHeader("x-test-policy", "once");
      return nextPolicy.Send(request, context);
    }
  };

  class QueueProtocolTest : public ::testing::Test {
  protected:
    std::shared_ptr<ProtocolTransport> Transport = std::make_shared<ProtocolTransport>();

    QueueClientOptions Options()
    {
      QueueClientOptions options;
      options.Transport.Transport = Transport;
      options.Retry.MaxRetries = 0;
      return options;
    }
  };

  TEST_F(QueueProtocolTest, MetadataVersionSasAndIdempotency)
  {
    auto options = Options();
    options.ApiVersion = ServiceVersion::V2019_12_12;
    auto count = std::make_shared<int>(0);
    options.PerOperationPolicies.emplace_back(std::make_unique<CountingPolicy>(count));
    QueueClient client(Endpoint, options);
    Transport->Replies
        = {{Http::HttpStatusCode::Created, "", {}},
           {Http::HttpStatusCode::NoContent, "", {}},
           {Http::HttpStatusCode::Conflict,
            "<Error><Code>QueueAlreadyExists</Code><Message>Exists</Message></Error>",
            {}},
           {Http::HttpStatusCode::NoContent, "", {}},
           {Http::HttpStatusCode::NotFound,
            "<Error><Code>QueueNotFound</Code><Message>Missing</Message></Error>",
            {}},
           {Http::HttpStatusCode::NoContent, "", {}},
           {Http::HttpStatusCode::BadRequest,
            "<Error><Code>InvalidMetadata</Code><Message>Invalid</Message></Error>",
            {}}};

    CreateQueueOptions create;
    create.Metadata = {{"Name", "value"}, {"Empty", ""}};
    auto context = Core::Context().WithValue(ContextKey, std::string("forwarded"));
    EXPECT_TRUE(client.Create(create, context).Value.Created);
    EXPECT_FALSE(client.Create().Value.Created);
    EXPECT_FALSE(client.Create().Value.Created);
    EXPECT_TRUE(client.Delete().Value.Deleted);
    EXPECT_FALSE(client.Delete().Value.Deleted);
    client.SetMetadata({{"Other", "value"}});
    try
    {
      client.SetMetadata({});
      FAIL() << "Expected StorageException";
    }
    catch (StorageException const& error)
    {
      EXPECT_EQ(error.ErrorCode, "InvalidMetadata");
      EXPECT_EQ(error.StatusCode, Http::HttpStatusCode::BadRequest);
      ASSERT_NE(error.RawResponse, nullptr);
      EXPECT_EQ(error.RawResponse->GetHeaders().at("x-ms-request-id"), "test-request");
    }
    ASSERT_EQ(Transport->Requests.size(), 7U);
    EXPECT_EQ(*count, 7);
    EXPECT_EQ(Transport->Requests[0].ContextValue, "forwarded");
    EXPECT_EQ(Transport->Requests[0].Headers.at("x-ms-meta-name"), "value");
    EXPECT_EQ(Transport->Requests[0].Headers.at("x-ms-meta-empty"), "");
    EXPECT_EQ(Transport->Requests[5].Headers.at("x-ms-meta-other"), "value");
    EXPECT_EQ(Transport->Requests[5].Url.GetQueryParameters().at("comp"), "metadata");
    for (auto const& request : Transport->Requests)
    {
      EXPECT_EQ(request.Headers.at("x-ms-version"), "2019-12-12");
      EXPECT_EQ(request.Headers.at("x-test-policy"), "once");
      EXPECT_EQ(request.Headers.count("Accept"), 0U);
      EXPECT_EQ(request.Url.GetQueryParameters().at("sig"), "not-a-secret%2Bsample%3D");
    }
  }

  TEST_F(QueueProtocolTest, ApproximateMessageCountsAndMetadata)
  {
    QueueClient client(Endpoint, Options());
    Transport->Replies
        = {{Http::HttpStatusCode::Ok,
            "",
            {{"x-ms-approximate-messages-count", "4294967296"}, {"x-ms-meta-name", "value"}}},
           {Http::HttpStatusCode::Ok, "", {{"x-ms-approximate-messages-count", "0"}}},
           {Http::HttpStatusCode::Ok, "", {}}};
    auto response = client.GetProperties();
    EXPECT_EQ(response.Value.ApproximateMessageCount, -1);
    EXPECT_EQ(response.Value.ApproximateMessageCountLong, INT64_C(4294967296));
    EXPECT_EQ(response.Value.Metadata.at("name"), "value");
    EXPECT_EQ(client.GetProperties().Value.ApproximateMessageCount, 0);
    EXPECT_THROW(client.GetProperties(), std::out_of_range);
  }

  TEST_F(QueueProtocolTest, UserHeaderPoliciesTakePrecedence)
  {
    class HeaderPolicy final : public Http::Policies::HttpPolicy {
    public:
      std::unique_ptr<Http::Policies::HttpPolicy> Clone() const override
      {
        return std::make_unique<HeaderPolicy>();
      }

      std::unique_ptr<Http::RawResponse> Send(
          Http::Request& request,
          Http::Policies::NextHttpPolicy nextPolicy,
          Core::Context const& context) const override
      {
        request.SetHeader("Accept", "application/custom");
        request.SetHeader("Content-Type", "application/custom");
        return nextPolicy.Send(request, context);
      }
    };
    auto options = Options();
    options.PerOperationPolicies.emplace_back(std::make_unique<HeaderPolicy>());
    QueueClient client(Endpoint, options);
    QueueServiceClient service("https://account.queue.example", options);
    Transport->Replies
        = {{Http::HttpStatusCode::Created, "<QueueMessagesList/>", {}},
           {Http::HttpStatusCode::Created, "<QueueMessagesList/>", {}}};
    client.EnqueueMessage("message");
    service.GetQueueClient("queue").EnqueueMessage("message");
    ASSERT_EQ(Transport->Requests.size(), 2U);
    for (auto const& request : Transport->Requests)
    {
      EXPECT_EQ(request.Headers.at("Accept"), "application/custom");
      EXPECT_EQ(request.Headers.at("Content-Type"), "application/custom");
    }
  }

  TEST_F(QueueProtocolTest, StorageErrorsAcrossOperations)
  {
    QueueClient client(Endpoint, Options());
    QueueServiceClient service("https://account.queue.example", Options());
    auto const expiresOn = DateTime::Parse("2026-10-19T10:20:30Z", DateTime::DateFormat::Rfc3339);
    std::vector<std::pair<std::string, std::function<void()>>> const operations{
        {"Create", [&] { client.Create(); }},
        {"Delete", [&] { client.Delete(); }},
        {"GetProperties", [&] { client.GetProperties(); }},
        {"SetMetadata", [&] { client.SetMetadata({}); }},
        {"GetAccessPolicy", [&] { client.GetAccessPolicy(); }},
        {"SetAccessPolicy", [&] { client.SetAccessPolicy({}); }},
        {"EnqueueMessage", [&] { client.EnqueueMessage("message"); }},
        {"ReceiveMessages", [&] { client.ReceiveMessages(); }},
        {"PeekMessages", [&] { client.PeekMessages(); }},
        {"UpdateMessage", [&] { client.UpdateMessage("id", "receipt", std::chrono::seconds(60)); }},
        {"DeleteMessage", [&] { client.DeleteMessage("id", "receipt"); }},
        {"ClearMessages", [&] { client.ClearMessages(); }},
        {"ListQueues", [&] { service.ListQueues(); }},
        {"SetServiceProperties", [&] { service.SetProperties({}); }},
        {"GetServiceProperties", [&] { service.GetProperties(); }},
        {"GetStatistics", [&] { service.GetStatistics(); }},
        {"GetUserDelegationKey", [&] { service.GetUserDelegationKey(expiresOn); }},
        {"CreateQueue", [&] { service.CreateQueue("queue"); }},
        {"DeleteQueue", [&] { service.DeleteQueue("queue"); }}};
    for (auto const& operation : operations)
    {
      SCOPED_TRACE(operation.first);
      Transport->Replies.push_back(
          {Http::HttpStatusCode::BadRequest,
           "<Error><Code>InvalidMetadata</Code><Message>Invalid metadata</Message>"
           "<Detail>extra detail</Detail></Error>",
           {{"x-ms-client-request-id", "client-request"}}});
      try
      {
        operation.second();
        FAIL() << "Expected StorageException";
      }
      catch (StorageException const& error)
      {
        EXPECT_EQ(error.ErrorCode, "InvalidMetadata");
        EXPECT_EQ(error.Message, "Invalid metadata");
        EXPECT_EQ(error.StatusCode, Http::HttpStatusCode::BadRequest);
        EXPECT_EQ(error.RequestId, "test-request");
        EXPECT_EQ(error.ClientRequestId, "client-request");
        EXPECT_EQ(error.AdditionalInformation.at("Detail"), "extra detail");
        ASSERT_NE(error.RawResponse, nullptr);
        EXPECT_EQ(error.RawResponse->GetStatusCode(), Http::HttpStatusCode::BadRequest);
      }
    }
    EXPECT_EQ(Transport->Requests.size(), operations.size());
    EXPECT_TRUE(Transport->Replies.empty());
  }

  TEST_F(QueueProtocolTest, JsonTextAndEmptyErrors)
  {
    QueueClient client(Endpoint, Options());
    Transport->Replies
        = {{Http::HttpStatusCode::BadRequest,
            R"({"error":{"code":"InvalidMetadata","message":"JSON error"}})",
            {{"content-type", "application/json"}}},
           {Http::HttpStatusCode::BadRequest,
            "Text error",
            {{"content-type", "text/plain"}, {"x-ms-error-code", "InvalidMetadata"}}},
           {Http::HttpStatusCode::BadRequest, "", {{"x-ms-error-code", "InvalidMetadata"}}}};
    for (auto const& message : {"JSON error", "Text error", ""})
    {
      try
      {
        client.SetMetadata({});
        FAIL() << "Expected StorageException";
      }
      catch (StorageException const& error)
      {
        EXPECT_EQ(error.ErrorCode, "InvalidMetadata");
        EXPECT_EQ(error.Message, message);
        ASSERT_NE(error.RawResponse, nullptr);
        EXPECT_EQ(error.RawResponse->GetStatusCode(), Http::HttpStatusCode::BadRequest);
      }
    }
    EXPECT_TRUE(Transport->Replies.empty());
  }

  TEST_F(QueueProtocolTest, ContinuationErrors)
  {
    QueueServiceClient client("https://account.queue.example", Options());
    Transport->Replies
        = {{Http::HttpStatusCode::Ok,
            "<EnumerationResults ServiceEndpoint=\"https://account.queue.example\">"
            "<Queues/><NextMarker>next</NextMarker></EnumerationResults>",
            {}},
           {Http::HttpStatusCode::BadRequest,
            "<Error><Code>InvalidMarker</Code><Message>Invalid marker</Message></Error>",
            {}}};
    auto page = client.ListQueues();
    try
    {
      page.MoveToNextPage();
      FAIL() << "Expected StorageException";
    }
    catch (StorageException const& error)
    {
      EXPECT_EQ(error.ErrorCode, "InvalidMarker");
      EXPECT_EQ(error.Message, "Invalid marker");
      ASSERT_NE(error.RawResponse, nullptr);
    }
    ASSERT_EQ(Transport->Requests.size(), 2U);
    EXPECT_EQ(Transport->Requests[1].Url.GetQueryParameters().at("marker"), "next");
  }

  TEST(QueueProtocolAdapterTest, PreservesExistingExceptions)
  {
    try
    {
      Queues::_detail::InvokeStorageProtocol([] {
        StorageException error("existing Storage exception");
        error.ErrorCode = "Existing";
        error.AdditionalInformation["Detail"] = "retained";
        error.RawResponse
            = std::make_unique<Http::RawResponse>(1, 1, Http::HttpStatusCode::Conflict, "Conflict");
        throw error;
      });
      FAIL() << "Expected StorageException";
    }
    catch (StorageException const& error)
    {
      EXPECT_STREQ(error.what(), "existing Storage exception");
      EXPECT_EQ(error.ErrorCode, "Existing");
      EXPECT_EQ(error.AdditionalInformation.at("Detail"), "retained");
      ASSERT_NE(error.RawResponse, nullptr);
    }
    try
    {
      Queues::_detail::InvokeStorageProtocol(
          [] { throw Core::RequestFailedException("response parsing failed"); });
      FAIL() << "Expected Core::RequestFailedException";
    }
    catch (StorageException const&)
    {
      FAIL() << "An exception without an HTTP response must not be translated";
    }
    catch (Core::RequestFailedException const& error)
    {
      EXPECT_STREQ(error.what(), "response parsing failed");
      EXPECT_EQ(error.RawResponse, nullptr);
    }
    EXPECT_THROW(
        Queues::_detail::InvokeStorageProtocol([] { throw std::logic_error("unrelated"); }),
        std::logic_error);
    auto value = Queues::_detail::InvokeStorageProtocol([] { return std::make_unique<int>(42); });
    EXPECT_EQ(*value, 42);
  }

  TEST_F(QueueProtocolTest, CancellationAndTransportFailures)
  {
    QueueClient client(Endpoint, Options());
    auto const cancelled = Core::Context::ApplicationContext.WithDeadline(
        std::chrono::system_clock::now() - std::chrono::seconds(1));
    EXPECT_THROW(client.GetProperties({}, cancelled), Core::OperationCancelledException);
    EXPECT_TRUE(Transport->Requests.empty());
    EXPECT_THROW(client.GetProperties(), std::runtime_error);
    EXPECT_TRUE(Transport->Requests.empty());
  }

  TEST_F(QueueProtocolTest, MessageModelsAndVisibilityOnlyUpdates)
  {
    QueueClient client(Endpoint, Options());
    std::string const message
        = "<MessageId>id</MessageId><InsertionTime>Sun, 18 Oct 2026 10:20:30 GMT</InsertionTime>"
          "<ExpirationTime>Sun, 25 Oct 2026 10:20:30 GMT</ExpirationTime>"
          "<DequeueCount>4294967296</DequeueCount><MessageText>a&amp;b</MessageText>";
    std::string const received = "<QueueMessagesList><QueueMessage>" + message
        + "<PopReceipt>receipt</PopReceipt>"
          "<TimeNextVisible>Sun, 18 Oct 2026 10:21:30 GMT</TimeNextVisible>"
          "</QueueMessage></QueueMessagesList>";
    Core::CaseInsensitiveMap const updatedHeaders{
        {"x-ms-popreceipt", "updated"},
        {"x-ms-time-next-visible", "Sun, 18 Oct 2026 10:21:30 GMT"}};
    Transport->Replies
        = {{Http::HttpStatusCode::Ok, received, {}},
           {Http::HttpStatusCode::Ok,
            "<QueueMessagesList><QueueMessage>" + message + "</QueueMessage></QueueMessagesList>",
            {}},
           {Http::HttpStatusCode::Created, received, {}},
           {Http::HttpStatusCode::NoContent, "", updatedHeaders},
           {Http::HttpStatusCode::NoContent, "", updatedHeaders},
           {Http::HttpStatusCode::NoContent, "", updatedHeaders},
           {Http::HttpStatusCode::NoContent, "", {}},
           {Http::HttpStatusCode::NoContent, "", {}}};
    ReceiveMessagesOptions receive;
    receive.MaxMessages = INT64_C(2147483648);
    auto messages = client.ReceiveMessages(receive);
    ASSERT_EQ(messages.Value.Messages.size(), 1U);
    EXPECT_EQ(messages.Value.Messages[0].DequeueCount, INT64_C(4294967296));
    EXPECT_EQ(messages.Value.Messages[0].MessageText, "a&b");
    EXPECT_EQ(messages.Value.Messages[0].PopReceipt, "receipt");
    PeekMessagesOptions peek;
    peek.MaxMessages = INT64_C(2147483648);
    auto peeked = client.PeekMessages(peek);
    ASSERT_EQ(peeked.Value.Messages.size(), 1U);
    EXPECT_EQ(peeked.Value.Messages[0].InsertedOn, messages.Value.Messages[0].InsertedOn);
    EXPECT_EQ(peeked.Value.Messages[0].DequeueCount, INT64_C(4294967296));
    auto sent = client.EnqueueMessage("a&b");
    EXPECT_EQ(sent.Value.MessageId, "id");
    auto updated = client.UpdateMessage("id", "receipt", std::chrono::seconds(60));
    EXPECT_EQ(updated.Value.PopReceipt, "updated");
    UpdateMessageOptions update;
    update.MessageText = "";
    client.UpdateMessage("id", "receipt", std::chrono::seconds(60), update);
    update.MessageText = "a&b";
    client.UpdateMessage("id", "receipt", std::chrono::seconds(60), update);
    client.DeleteMessage("id", "receipt");
    client.ClearMessages();
    ASSERT_EQ(Transport->Requests.size(), 8U);
    EXPECT_EQ(Transport->Requests[0].Url.GetQueryParameters().at("numofmessages"), "2147483648");
    EXPECT_EQ(Transport->Requests[1].Url.GetQueryParameters().at("numofmessages"), "2147483648");
    EXPECT_EQ(Transport->Requests[1].Url.GetQueryParameters().at("peekonly"), "true");
    EXPECT_EQ(Transport->Requests[3].Body, "");
    EXPECT_EQ(Transport->Requests[3].Headers.count("Content-Type"), 0U);
    auto const length = Transport->Requests[3].Headers.find("content-length");
    if (length != Transport->Requests[3].Headers.end())
    {
      EXPECT_EQ(length->second, "0");
    }
    EXPECT_NE(Transport->Requests[4].Body.find("MessageText"), std::string::npos);
    EXPECT_NE(Transport->Requests[5].Body.find("a&amp;b"), std::string::npos);
    for (auto const index : {0, 1, 2})
    {
      EXPECT_EQ(Transport->Requests[index].Headers.at("Accept"), "application/xml");
    }
    for (auto const index : {2, 4, 5})
    {
      EXPECT_EQ(Transport->Requests[index].Headers.at("Content-Type"), "application/xml");
    }
    for (auto const index : {0, 1, 6, 7})
    {
      EXPECT_EQ(Transport->Requests[index].Headers.count("Content-Type"), 0U);
    }
    for (auto const index : {3, 4, 5, 6, 7})
    {
      EXPECT_EQ(Transport->Requests[index].Headers.count("Accept"), 0U);
    }
    EXPECT_EQ(Transport->Requests[6].Url.GetQueryParameters().at("popreceipt"), "receipt");
    for (auto const& request : Transport->Requests)
    {
      EXPECT_EQ(request.Url.GetQueryParameters().at("sig"), "not-a-secret%2Bsample%3D");
    }
  }

  TEST_F(QueueProtocolTest, AccessPolicyDatesAndEmptyLists)
  {
    QueueClient client(Endpoint, Options());
    Transport->Replies
        = {{Http::HttpStatusCode::NoContent, "", {}},
           {Http::HttpStatusCode::Ok,
            "<SignedIdentifiers><SignedIdentifier><Id>id</Id><AccessPolicy>"
            "<Start>2026-10-18T10:20:30Z</Start><Expiry>2026-10-19T10:20:30Z</Expiry>"
            "<Permission>ra</Permission></AccessPolicy></SignedIdentifier></SignedIdentifiers>",
            {}},
           {Http::HttpStatusCode::NoContent, "", {}}};
    Models::SignedIdentifier identifier;
    identifier.Id = "id";
    identifier.StartsOn
        = DateTime::Parse("2026-10-18T10:20:30.1234567Z", DateTime::DateFormat::Rfc3339);
    identifier.ExpiresOn
        = DateTime::Parse("2026-10-19T10:20:30.1234567Z", DateTime::DateFormat::Rfc3339);
    identifier.Permissions = "ra";
    Models::QueueAccessPolicy policy;
    policy.SignedIdentifiers.emplace_back(std::move(identifier));
    client.SetAccessPolicy(policy);
    auto response = client.GetAccessPolicy();
    ASSERT_EQ(response.Value.SignedIdentifiers.size(), 1U);
    EXPECT_EQ(response.Value.SignedIdentifiers[0].Permissions, "ra");
    EXPECT_EQ(
        response.Value.SignedIdentifiers[0].StartsOn.Value(),
        DateTime::Parse("2026-10-18T10:20:30Z", DateTime::DateFormat::Rfc3339));
    client.SetAccessPolicy({});
    ASSERT_EQ(Transport->Requests.size(), 3U);
    EXPECT_NE(
        Transport->Requests[0].Body.find("<Start>2026-10-18T10:20:30.1234567Z</Start>"),
        std::string::npos);
    EXPECT_EQ(
        Transport->Requests[0].Body.find("<SignedIdentifier><SignedIdentifier>"),
        std::string::npos);
    EXPECT_NE(Transport->Requests[2].Body.find("SignedIdentifiers"), std::string::npos);
  }

  TEST_F(QueueProtocolTest, AccessPolicyPreservesFractionalSecondZeros)
  {
    QueueClient client(Endpoint, Options());
    for (auto const* fraction : {"0000000", "1230000", "1234567"})
    {
      SCOPED_TRACE(fraction);
      auto const timestamp = std::string("2026-10-18T10:20:30.") + fraction + "Z";
      Models::SignedIdentifier identifier;
      identifier.Id = "id";
      identifier.StartsOn = DateTime::Parse(timestamp, DateTime::DateFormat::Rfc3339);
      identifier.ExpiresOn = identifier.StartsOn;
      identifier.Permissions = "ra";
      Models::QueueAccessPolicy policy;
      policy.SignedIdentifiers.emplace_back(std::move(identifier));
      Transport->Replies.push_back({Http::HttpStatusCode::NoContent, "", {}});
      client.SetAccessPolicy(policy);
      auto const& body = Transport->Requests.back().Body;
      EXPECT_NE(body.find("<Start>" + timestamp + "</Start>"), std::string::npos);
      EXPECT_NE(body.find("<Expiry>" + timestamp + "</Expiry>"), std::string::npos);
    }
  }

  TEST_F(QueueProtocolTest, ServicePropertiesAndStatistics)
  {
    QueueServiceClient client("https://account.queue.example?sig=not-a-secret", Options());
    Transport->Replies
        = {{Http::HttpStatusCode::Accepted, "", {}},
           {Http::HttpStatusCode::Ok,
            "<StorageServiceProperties><Logging><Version>1.0</Version><Delete>true</Delete>"
            "<Read>false</Read><Write>true</Write><RetentionPolicy><Enabled>true</Enabled>"
            "<Days>3</Days></RetentionPolicy></Logging><HourMetrics><Version>1.0</Version>"
            "<Enabled>false</Enabled><RetentionPolicy><Enabled>false</Enabled></RetentionPolicy>"
            "</HourMetrics><Cors/></StorageServiceProperties>",
            {}},
           {Http::HttpStatusCode::Ok,
            "<StorageServiceStats><GeoReplication><Status>live</Status>"
            "<LastSyncTime>Sun, 18 Oct 2026 10:20:30 GMT</LastSyncTime>"
            "</GeoReplication></StorageServiceStats>",
            {}},
           {Http::HttpStatusCode::Ok,
            "<StorageServiceStats><GeoReplication><Status>unavailable</Status>"
            "<LastSyncTime/></GeoReplication></StorageServiceStats>",
            {}}};
    Models::QueueServiceProperties properties;
    properties.Logging.Version = "1.0";
    properties.Logging.RetentionPolicy.IsEnabled = true;
    properties.Logging.RetentionPolicy.Days = 3;
    properties.HourMetrics.Version = "1.0";
    properties.MinuteMetrics.Version = "1.0";
    client.SetProperties(properties);
    auto response = client.GetProperties();
    EXPECT_EQ(response.Value.Logging.Version, "1.0");
    EXPECT_TRUE(response.Value.Logging.Delete);
    EXPECT_TRUE(response.Value.Logging.Write);
    EXPECT_TRUE(response.Value.Logging.RetentionPolicy.IsEnabled);
    EXPECT_EQ(response.Value.Logging.RetentionPolicy.Days.Value(), 3);
    EXPECT_FALSE(response.Value.HourMetrics.IsEnabled);
    EXPECT_FALSE(response.Value.HourMetrics.IncludeApis.HasValue());
    EXPECT_TRUE(response.Value.Cors.empty());
    auto stats = client.GetStatistics();
    EXPECT_EQ(stats.Value.GeoReplication.Status, Models::GeoReplicationStatus::Live);
    ASSERT_TRUE(stats.Value.GeoReplication.LastSyncedOn.HasValue());
    EXPECT_EQ(
        stats.Value.GeoReplication.LastSyncedOn.Value(),
        DateTime::Parse("2026-10-18T10:20:30Z", DateTime::DateFormat::Rfc3339));
    EXPECT_FALSE(client.GetStatistics().Value.GeoReplication.LastSyncedOn.HasValue());
    ASSERT_EQ(Transport->Requests.size(), 4U);
    EXPECT_NE(Transport->Requests[0].Body.find("<StorageServiceProperties>"), std::string::npos);
    EXPECT_EQ(Transport->Requests[0].Headers.at("Content-Type"), "application/xml");
    EXPECT_EQ(Transport->Requests[0].Headers.count("Accept"), 0U);
    for (auto const index : {1, 2, 3})
    {
      EXPECT_EQ(Transport->Requests[index].Headers.at("Accept"), "application/xml");
      EXPECT_EQ(Transport->Requests[index].Headers.count("Content-Type"), 0U);
    }
    EXPECT_EQ(Transport->Requests[0].Body.find("<QueueServiceProperties>"), std::string::npos);
    EXPECT_EQ(Transport->Requests[0].Url.GetQueryParameters().at("restype"), "service");
    EXPECT_TRUE(Transport->Requests[1].HasReplicaStatus);
  }

  TEST_F(QueueProtocolTest, PagingAndDelegationKey)
  {
    QueueServiceClient client("https://account.queue.example?sig=not-a-secret", Options());
    Transport->Replies
        = {{Http::HttpStatusCode::Ok,
            "<EnumerationResults "
            "ServiceEndpoint=\"https://account.queue.example\"><Prefix>pre</Prefix>"
            "<Queues><Queue><Name>first</Name><Metadata><Name>value</Name><Empty/>"
            "</Metadata></Queue></Queues><NextMarker>next marker</NextMarker></EnumerationResults>",
            {}},
           {Http::HttpStatusCode::Ok,
            "<EnumerationResults "
            "ServiceEndpoint=\"https://account.queue.example\"><Prefix>pre</Prefix>"
            "<Queues/><NextMarker/></EnumerationResults>",
            {}},
           {Http::HttpStatusCode::Ok,
            "<UserDelegationKey><SignedOid>object</SignedOid><SignedTid>tenant</SignedTid>"
            "<SignedStart>2026-10-18T10:20:30Z</SignedStart>"
            "<SignedExpiry>2026-10-19T10:20:30Z</SignedExpiry><SignedService>q</SignedService>"
            "<SignedVersion>2026-04-06</SignedVersion><Value>sample-key</Value>"
            "</UserDelegationKey>",
            {}}};
    ListQueuesOptions list;
    list.Prefix = "pre";
    list.Include = Models::ListQueuesIncludeFlags::Metadata;
    auto page = client.ListQueues(list);
    ASSERT_EQ(page.Queues.size(), 1U);
    EXPECT_EQ(page.ServiceEndpoint, "https://account.queue.example");
    EXPECT_EQ(page.Queues[0].Name, "first");
    EXPECT_EQ(page.Queues[0].Metadata.at("name"), "value");
    EXPECT_EQ(page.Queues[0].Metadata.at("empty"), "");
    EXPECT_EQ(page.NextPageToken.Value(), "next marker");
    page.MoveToNextPage();
    EXPECT_TRUE(page.Queues.empty());
    EXPECT_FALSE(page.NextPageToken.HasValue());
    GetUserDelegationKeyOptions keyOptions;
    keyOptions.StartsOn
        = DateTime::Parse("2026-10-18T10:20:30.1234567Z", DateTime::DateFormat::Rfc3339);
    keyOptions.DelegatedUserTid = "delegated";
    auto key = client.GetUserDelegationKey(
        DateTime::Parse("2026-10-19T10:20:30.1234567Z", DateTime::DateFormat::Rfc3339), keyOptions);
    EXPECT_EQ(key.Value.SignedObjectId, "object");
    EXPECT_EQ(key.Value.SignedTenantId, "tenant");
    EXPECT_EQ(
        key.Value.SignedStartsOn,
        DateTime::Parse("2026-10-18T10:20:30Z", DateTime::DateFormat::Rfc3339));
    EXPECT_EQ(
        key.Value.SignedExpiresOn,
        DateTime::Parse("2026-10-19T10:20:30Z", DateTime::DateFormat::Rfc3339));
    EXPECT_EQ(key.Value.Value, "sample-key");
    ASSERT_EQ(Transport->Requests.size(), 3U);
    EXPECT_TRUE(Transport->Requests[0].HasReplicaStatus);
    EXPECT_TRUE(Transport->Requests[1].HasReplicaStatus);
    EXPECT_EQ(Transport->Requests[0].Url.GetQueryParameters().at("include"), "metadata");
    EXPECT_EQ(Transport->Requests[1].Url.GetQueryParameters().at("marker"), "next%20marker");
    EXPECT_NE(
        Transport->Requests[2].Body.find("<Start>2026-10-18T10:20:30Z</Start>"), std::string::npos);
    EXPECT_NE(
        Transport->Requests[2].Body.find("<Expiry>2026-10-19T10:20:30Z</Expiry>"),
        std::string::npos);
    EXPECT_NE(
        Transport->Requests[2].Body.find("<DelegatedUserTid>delegated</DelegatedUserTid>"),
        std::string::npos);
  }
}}}} // namespace Azure::Storage::Test::
