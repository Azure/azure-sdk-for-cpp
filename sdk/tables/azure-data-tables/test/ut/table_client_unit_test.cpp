// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/data/tables/table_client.hpp"
#include "test/ut/test_base.hpp"

#include <azure/core/internal/json/json.hpp>

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using namespace Azure::Data::Tables;
using namespace Azure::Data::Tables::Models;
using Azure::Core::Http::HttpStatusCode;
using Azure::Core::Http::RawResponse;
using Azure::Core::Json::_internal::json;

namespace Azure { namespace Data { namespace Test {
  namespace {
    struct RecordedRequest
    {
      std::string Method;
      Core::Url Url;
      Core::CaseInsensitiveMap Headers;
      std::string Body;
    };

    struct BatchRequest
    {
      std::string RequestLine;
      Core::CaseInsensitiveMap Headers;
      std::string Body;
    };

    std::string ETagTestName(std::string const& etag)
    {
      return etag.empty() ? "EmptyETag" : etag == "*" ? "WildcardETag" : "ConditionalETag";
    }

    std::string BatchTestName(
        testing::TestParamInfo<std::tuple<TransactionActionType, std::string>> const& info)
    {
      std::map<TransactionActionType, std::string> const names{
          {TransactionActionType::Add, "Add"},
          {TransactionActionType::UpdateMerge, "UpdateMerge"},
          {TransactionActionType::UpdateReplace, "UpdateReplace"},
          {TransactionActionType::Delete, "Delete"},
          {TransactionActionType::InsertMerge, "InsertMerge"},
          {TransactionActionType::InsertReplace, "InsertReplace"}};
      return names.at(std::get<0>(info.param)) + ETagTestName(std::get<1>(info.param));
    }

    TableEntity MakeEntity(
        std::string const& partitionKey = "partition",
        std::string const& rowKey = "row")
    {
      TableEntity entity;
      entity.SetPartitionKey(partitionKey);
      entity.SetRowKey(rowKey);
      entity.Properties["Name"] = TableEntityProperty("caf\xc3\xa9 \xe6\xb0\xb4");
      return entity;
    }

    void ParseBatchRequest(RecordedRequest const& request, std::vector<BatchRequest>& parts)
    {
      EXPECT_EQ(request.Method, "POST");
      EXPECT_EQ(request.Url.GetPath(), "$batch");
      ASSERT_EQ(request.Headers.count("Content-Length"), 1U);
      EXPECT_EQ(request.Headers.at("Content-Length"), std::to_string(request.Body.size()));
      std::string const contentTypePrefix = "multipart/mixed; boundary=";
      ASSERT_EQ(request.Headers.count("Content-Type"), 1U);
      auto const& contentType = request.Headers.at("Content-Type");
      ASSERT_EQ(contentType.substr(0, contentTypePrefix.size()), contentTypePrefix);
      auto const batch = contentType.substr(contentTypePrefix.size());
      auto const& body = request.Body;
      for (size_t i = 0; i < body.size(); ++i)
      {
        if (body[i] == '\n')
        {
          ASSERT_GT(i, 0U);
          ASSERT_EQ(body[i - 1], '\r');
        }
        if (body[i] == '\r')
        {
          ASSERT_LT(i + 1, body.size());
          ASSERT_EQ(body[i + 1], '\n');
        }
      }

      auto const prefix = "--" + batch + "\r\nContent-Type: " + contentTypePrefix;
      ASSERT_EQ(body.substr(0, prefix.size()), prefix);
      auto const changesetEnd = body.find("\r\n\r\n", prefix.size());
      ASSERT_NE(changesetEnd, std::string::npos);
      auto const changeset = body.substr(prefix.size(), changesetEnd - prefix.size());
      auto const partPrefix = "--" + changeset
          + "\r\nContent-Type: application/http\r\nContent-Transfer-Encoding: binary\r\n\r\n";
      size_t position = changesetEnd + 4;
      while (body.compare(position, partPrefix.size(), partPrefix) == 0)
      {
        position += partPrefix.size();
        auto const requestLineEnd = body.find("\r\n", position);
        ASSERT_NE(requestLineEnd, std::string::npos);
        BatchRequest part;
        part.RequestLine = body.substr(position, requestLineEnd - position);
        position = requestLineEnd + 2;
        while (body.compare(position, 2, "\r\n") != 0)
        {
          auto const headerEnd = body.find("\r\n", position);
          ASSERT_NE(headerEnd, std::string::npos);
          auto const separator = body.find(": ", position);
          ASSERT_LT(separator, headerEnd);
          ASSERT_TRUE(part.Headers
                          .emplace(
                              body.substr(position, separator - position),
                              body.substr(separator + 2, headerEnd - separator - 2))
                          .second);
          position = headerEnd + 2;
        }
        position += 2;
        auto const nextBoundary = body.find("\r\n--" + changeset, position);
        ASSERT_NE(nextBoundary, std::string::npos);
        part.Body = body.substr(position, nextBoundary - position);
        parts.emplace_back(std::move(part));
        position = nextBoundary + 2;
      }
      EXPECT_EQ(body.substr(position), "--" + changeset + "--\r\n--" + batch + "--\r\n");
    }
  } // namespace

  class TablesRequestTest : public ::testing::Test {
  protected:
    std::vector<RecordedRequest> m_requests;
    std::vector<std::unique_ptr<RawResponse>> m_responses;

    void QueueResponse(
        HttpStatusCode status,
        std::string const& body = {},
        Core::CaseInsensitiveMap const& headers = {})
    {
      auto response = std::make_unique<RawResponse>(1, 1, status, "");
      response->SetBody(std::vector<uint8_t>(body.begin(), body.end()));
      response->SetHeader("Content-Length", std::to_string(body.size()));
      for (auto const& header : headers)
      {
        response->SetHeader(header.first, header.second);
      }
      m_responses.emplace_back(std::move(response));
    }

    TableClient CreateClient(TableClientOptions options = {})
    {
      options.Retry.MaxRetries = 0;
      options.PerRetryPolicies.emplace_back(
          std::make_unique<Storage::Test::MockResponsePolicy>(
              [this](Core::Http::Request& request, Core::Context const& context) {
                std::vector<uint8_t> body;
                if (request.GetBodyStream() != nullptr)
                {
                  body = request.GetBodyStream()->ReadToEnd(context);
                }
                m_requests.push_back(
                    {request.GetMethod().ToString(),
                     request.GetUrl(),
                     request.GetHeaders(),
                     std::string(body.begin(), body.end())});
                return std::move(m_responses.at(m_requests.size() - 1));
              }));
      return TableClient("https://example.com", "table", options);
    }
  };

  class TablesWriteTest : public TablesRequestTest,
                          public ::testing::WithParamInterface<std::string> {
  protected:
    TableEntity EntityWithETag()
    {
      auto entity = MakeEntity();
      entity.SetETag(GetParam());
      QueueResponse(HttpStatusCode::NoContent, "", {{"ETag", "W/\"response-etag\""}});
      return entity;
    }

    void CheckWrite(std::string const& method, bool hasBody, std::string const& ifMatch)
    {
      ASSERT_EQ(m_requests.size(), 1U);
      auto const& request = m_requests[0];
      EXPECT_EQ(request.Method, method);
      if (ifMatch.empty())
      {
        EXPECT_EQ(request.Headers.count("If-Match"), 0U);
      }
      else
      {
        ASSERT_EQ(request.Headers.count("If-Match"), 1U);
        EXPECT_EQ(request.Headers.at("If-Match"), ifMatch);
      }
      if (hasBody)
      {
        EXPECT_EQ(request.Headers.at("Content-Type"), "application/json");
        EXPECT_EQ(request.Headers.at("Content-Length"), std::to_string(request.Body.size()));
        auto const body = json::parse(request.Body);
        EXPECT_EQ(body.at("PartitionKey"), "partition");
        EXPECT_EQ(body.at("RowKey"), "row");
        EXPECT_EQ(body.at("Name"), "caf\xc3\xa9 \xe6\xb0\xb4");
        EXPECT_EQ(body.at("odata.etag"), GetParam());
      }
      else
      {
        EXPECT_TRUE(request.Body.empty());
      }
    }
  };

  TEST_P(TablesWriteTest, AddDoesNotSendIfMatch)
  {
    auto entity = EntityWithETag();
    auto response = CreateClient().AddEntity(entity);
    CheckWrite("POST", true, "");
    EXPECT_EQ(response.Value.ETag, "W/\"response-etag\"");
  }

  TEST_P(TablesWriteTest, UpdateSendsIfMatch)
  {
    auto entity = EntityWithETag();
    auto response = CreateClient().UpdateEntity(entity);
    CheckWrite("PUT", true, GetParam().empty() ? "*" : GetParam());
    EXPECT_EQ(response.Value.ETag, "W/\"response-etag\"");
  }

  TEST_P(TablesWriteTest, MergeSendsIfMatch)
  {
    auto entity = EntityWithETag();
    auto response = CreateClient().MergeEntity(entity);
    CheckWrite("PATCH", true, GetParam().empty() ? "*" : GetParam());
    EXPECT_EQ(response.Value.ETag, "W/\"response-etag\"");
  }

  TEST_P(TablesWriteTest, DeleteSendsIfMatch)
  {
    auto entity = EntityWithETag();
    auto response = CreateClient().DeleteEntity(entity);
    CheckWrite("DELETE", false, GetParam().empty() ? "*" : GetParam());
    EXPECT_EQ(response.RawResponse->GetStatusCode(), HttpStatusCode::NoContent);
  }

  TEST_P(TablesWriteTest, InsertReplaceDoesNotSendIfMatch)
  {
    auto entity = EntityWithETag();
    auto response = CreateClient().UpdateOrInsertEntity(entity);
    CheckWrite("PUT", true, "");
    EXPECT_EQ(response.Value.ETag, "W/\"response-etag\"");
  }

  TEST_P(TablesWriteTest, InsertMergeDoesNotSendIfMatch)
  {
    auto entity = EntityWithETag();
    auto response = CreateClient().MergeOrInsertEntity(entity);
    CheckWrite("PATCH", true, "");
    EXPECT_EQ(response.Value.ETag, "W/\"response-etag\"");
  }

  INSTANTIATE_TEST_SUITE_P(
      ETags,
      TablesWriteTest,
      ::testing::Values("", "*", "W/\"datetime'2026-01-02T03%3A04%3A05.1234567Z'\""),
      [](testing::TestParamInfo<std::string> const& info) { return ETagTestName(info.param); });

  class TablesBatchTest
      : public TablesRequestTest,
        public ::testing::WithParamInterface<std::tuple<TransactionActionType, std::string>> {};

  TEST_P(TablesBatchTest, ActionSemantics)
  {
    auto const action = std::get<0>(GetParam());
    auto const& etag = std::get<1>(GetParam());
    auto entity = MakeEntity();
    entity.SetETag(etag);
    QueueResponse(HttpStatusCode::Accepted);
    auto response = CreateClient().SubmitTransaction({{action, entity}});
    EXPECT_EQ(response.RawResponse->GetStatusCode(), HttpStatusCode::Accepted);
    ASSERT_EQ(m_requests.size(), 1U);
    std::vector<BatchRequest> parts;
    ParseBatchRequest(m_requests[0], parts);
    ASSERT_EQ(parts.size(), 1U);
    auto const& part = parts[0];
    bool const isUpdate = action == TransactionActionType::UpdateMerge
        || action == TransactionActionType::UpdateReplace
        || action == TransactionActionType::Delete;
    if (isUpdate)
    {
      ASSERT_EQ(part.Headers.count("If-Match"), 1U);
      EXPECT_EQ(part.Headers.at("If-Match"), etag.empty() ? "*" : etag);
    }
    else
    {
      EXPECT_EQ(part.Headers.count("If-Match"), 0U);
    }
    std::string method;
    switch (action)
    {
      case TransactionActionType::Add:
        method = "POST";
        break;
      case TransactionActionType::Delete:
        method = "DELETE";
        break;
      case TransactionActionType::InsertMerge:
      case TransactionActionType::UpdateMerge:
        method = "MERGE";
        break;
      case TransactionActionType::InsertReplace:
      case TransactionActionType::UpdateReplace:
        method = "PUT";
        break;
    }
    std::string const path = action == TransactionActionType::Add
        ? "table"
        : "table(PartitionKey='partition',RowKey='row')";
    EXPECT_EQ(
        Core::Url::Decode(part.RequestLine), method + " https://example.com/" + path + " HTTP/1.1");
    if (action == TransactionActionType::Delete)
    {
      EXPECT_TRUE(part.Body.empty());
    }
    else
    {
      EXPECT_EQ(part.Headers.at("Content-Type"), "application/json");
      auto const body = json::parse(part.Body);
      EXPECT_EQ(body.at("PartitionKey"), "partition");
      EXPECT_EQ(body.at("RowKey"), "row");
      EXPECT_EQ(body.at("Name"), "caf\xc3\xa9 \xe6\xb0\xb4");
      EXPECT_EQ(body.at("odata.etag"), etag);
      if (part.Headers.count("Content-Length") != 0)
      {
        EXPECT_EQ(part.Headers.at("Content-Length"), std::to_string(part.Body.size()));
      }
    }
  }

  INSTANTIATE_TEST_SUITE_P(
      ActionsAndETags,
      TablesBatchTest,
      ::testing::Combine(
          ::testing::Values(
              TransactionActionType::Add,
              TransactionActionType::UpdateMerge,
              TransactionActionType::UpdateReplace,
              TransactionActionType::Delete,
              TransactionActionType::InsertMerge,
              TransactionActionType::InsertReplace),
          ::testing::Values("", "*", "W/\"datetime'2026-01-02T03%3A04%3A05.1234567Z'\"")),
      BatchTestName);

  TEST_F(TablesRequestTest, MultipleDeletesTerminateEachEmbeddedRequest)
  {
    QueueResponse(HttpStatusCode::Accepted);
    CreateClient().SubmitTransaction(
        {{TransactionActionType::Delete, MakeEntity("partition", "0")},
         {TransactionActionType::Delete, MakeEntity("partition", "1")}});
    ASSERT_EQ(m_requests.size(), 1U);
    std::vector<BatchRequest> parts;
    ParseBatchRequest(m_requests[0], parts);
    ASSERT_EQ(parts.size(), 2U);
    for (size_t i = 0; i < parts.size(); ++i)
    {
      EXPECT_EQ(
          Core::Url::Decode(parts[i].RequestLine),
          "DELETE https://example.com/table(PartitionKey='partition',RowKey='" + std::to_string(i)
              + "') HTTP/1.1");
      EXPECT_EQ(parts[i].Headers.at("If-Match"), "*");
      EXPECT_TRUE(parts[i].Body.empty());
    }
  }

  TEST_F(TablesRequestTest, MixedBatchPreservesOrderAndResponse)
  {
    std::string const responseBody
        = "--batchresponse\r\nContent-Type: multipart/mixed; boundary=changesetresponse\r\n\r\n"
          "--changesetresponse\r\nContent-Type: application/http\r\n\r\n"
          "HTTP/1.1 204 No Content\r\nETag: W/\"first\"\r\n\r\n"
          "--changesetresponse\r\nContent-Type: application/http\r\n\r\n"
          "HTTP/1.1 204 No Content\r\n\r\n"
          "--changesetresponse\r\nContent-Type: application/http\r\n\r\n"
          "HTTP/1.1 204 No Content\r\nETag: W/\"last\"\r\n\r\n"
          "--changesetresponse--\r\n--batchresponse--\r\n";
    QueueResponse(HttpStatusCode::Accepted, responseBody);
    auto response = CreateClient().SubmitTransaction(
        {{TransactionActionType::Add, MakeEntity("partition", "0")},
         {TransactionActionType::Delete, MakeEntity("partition", "1")},
         {TransactionActionType::InsertReplace, MakeEntity("partition", "2")}});
    ASSERT_EQ(m_requests.size(), 1U);
    std::vector<BatchRequest> parts;
    ParseBatchRequest(m_requests[0], parts);
    ASSERT_EQ(parts.size(), 3U);
    EXPECT_EQ(parts[0].RequestLine, "POST https://example.com/table HTTP/1.1");
    EXPECT_EQ(json::parse(parts[0].Body).at("RowKey"), "0");
    EXPECT_EQ(
        Core::Url::Decode(parts[1].RequestLine),
        "DELETE https://example.com/table(PartitionKey='partition',RowKey='1') HTTP/1.1");
    EXPECT_TRUE(parts[1].Body.empty());
    EXPECT_EQ(
        Core::Url::Decode(parts[2].RequestLine),
        "PUT https://example.com/table(PartitionKey='partition',RowKey='2') HTTP/1.1");
    EXPECT_EQ(json::parse(parts[2].Body).at("RowKey"), "2");
    EXPECT_EQ(response.Value.StatusCode, "204");
    EXPECT_FALSE(response.Value.Error.HasValue());
    EXPECT_EQ(
        std::string(response.RawResponse->GetBody().begin(), response.RawResponse->GetBody().end()),
        responseBody);
  }

  TEST_F(TablesRequestTest, BatchPreservesServiceErrors)
  {
    std::string const responseBody
        = "--batchresponse\r\nContent-Type: multipart/mixed; boundary=changesetresponse\r\n\r\n"
          "--changesetresponse\r\nContent-Type: application/http\r\n\r\n"
          "HTTP/1.1 412 Precondition Failed\r\nContent-Type: application/json\r\n\r\n"
          "{\"odata.error\":{\"code\":\"UpdateConditionNotSatisfied\","
          "\"message\":{\"lang\":\"en-US\",\"value\":\"The condition was not satisfied.\"}}}\r\n"
          "--changesetresponse--\r\n--batchresponse--\r\n";
    QueueResponse(HttpStatusCode::Accepted, responseBody);
    auto response
        = CreateClient().SubmitTransaction({{TransactionActionType::UpdateMerge, MakeEntity()}});
    EXPECT_EQ(response.Value.StatusCode, "412");
    ASSERT_TRUE(response.Value.Error.HasValue());
    EXPECT_EQ(response.Value.Error.Value().Code, "UpdateConditionNotSatisfied");
    EXPECT_EQ(response.Value.Error.Value().Message, "The condition was not satisfied.");
    EXPECT_EQ(
        std::string(response.RawResponse->GetBody().begin(), response.RawResponse->GetBody().end()),
        responseBody);
  }

  TEST_F(TablesRequestTest, GetUpdateRoundTripWritesNativeJson)
  {
    QueueResponse(
        HttpStatusCode::Ok,
        R"({"PartitionKey":"partition","RowKey":"row","Enabled":true,"Count":42,
          "Ratio":1.2345678901234567,"Text":"42","Large":"9223372036854775807",
          "Large@odata.type":"Edm.Int64","Special":"Infinity",
          "Special@odata.type":"Edm.Double"})");
    QueueResponse(HttpStatusCode::NoContent, "", {{"ETag", "W/\"updated\""}});
    QueueResponse(HttpStatusCode::Accepted);
    auto client = CreateClient();
    auto const entity = client.GetEntity("partition", "row").Value;
    client.UpdateEntity(entity);
    client.SubmitTransaction({{TransactionActionType::InsertReplace, entity}});
    ASSERT_EQ(m_requests.size(), 3U);
    std::vector<BatchRequest> parts;
    ParseBatchRequest(m_requests[2], parts);
    ASSERT_EQ(parts.size(), 1U);
    for (auto const& body : {m_requests[1].Body, parts[0].Body})
    {
      auto const payload = json::parse(body);
      ASSERT_TRUE(payload.at("Enabled").is_boolean());
      EXPECT_TRUE(payload.at("Enabled").get<bool>());
      ASSERT_TRUE(payload.at("Count").is_number_integer());
      EXPECT_EQ(payload.at("Count").get<int>(), 42);
      ASSERT_TRUE(payload.at("Ratio").is_number_float());
      EXPECT_DOUBLE_EQ(payload.at("Ratio").get<double>(), 1.2345678901234567);
      EXPECT_EQ(payload.at("Text"), "42");
      EXPECT_EQ(payload.at("Large"), "9223372036854775807");
      EXPECT_EQ(payload.at("Large@odata.type"), "Edm.Int64");
      EXPECT_EQ(payload.at("Special"), "Infinity");
      EXPECT_EQ(payload.at("Special@odata.type"), "Edm.Double");
      EXPECT_EQ(payload.at("Enabled@odata.type"), "Edm.Boolean");
      EXPECT_EQ(payload.at("Count@odata.type"), "Edm.Int32");
      EXPECT_EQ(payload.at("Ratio@odata.type"), "Edm.Double");
    }
  }

  TEST_F(TablesRequestTest, PointOperationsEscapeODataKeys)
  {
    struct KeyCase
    {
      std::string Partition;
      std::string Row;
      std::string EscapedPartition;
      std::string EscapedRow;
    };
    std::vector<KeyCase> const cases{
        {"", "", "", ""},
        {"o'clock", "a''b", "o''clock", "a''''b"},
        {"'", "'", "''", "''"},
        {"part \xc3\xa9%+'", "row \xe6\xb0\xb4'", "part \xc3\xa9%+''", "row \xe6\xb0\xb4''"}};
    auto client = CreateClient();
    for (auto const& testCase : cases)
    {
      SCOPED_TRACE(testCase.Partition + ":" + testCase.Row);
      auto const start = m_requests.size();
      auto const entity = MakeEntity(testCase.Partition, testCase.Row);
      QueueResponse(HttpStatusCode::Ok, "{}");
      for (int i = 0; i < 5; ++i)
      {
        QueueResponse(HttpStatusCode::NoContent, "", {{"ETag", "W/\"response\""}});
      }
      client.GetEntity(testCase.Partition, testCase.Row);
      client.UpdateEntity(entity);
      client.MergeEntity(entity);
      client.DeleteEntity(entity);
      client.UpdateOrInsertEntity(entity);
      client.MergeOrInsertEntity(entity);
      ASSERT_EQ(m_requests.size(), start + 6);
      auto const path = Core::Url::Encode(
          "table(PartitionKey='" + testCase.EscapedPartition + "',RowKey='" + testCase.EscapedRow
          + "')");
      for (auto i = start; i < m_requests.size(); ++i)
      {
        EXPECT_EQ(m_requests[i].Url.GetPath(), path);
      }
    }
  }

  TEST_F(TablesRequestTest, BatchOperationsEscapeODataKeys)
  {
    std::vector<TransactionStep> steps;
    std::vector<TransactionActionType> const actions{
        TransactionActionType::UpdateMerge,
        TransactionActionType::UpdateReplace,
        TransactionActionType::Delete,
        TransactionActionType::InsertMerge,
        TransactionActionType::InsertReplace};
    for (size_t i = 0; i < actions.size(); ++i)
    {
      steps.push_back(
          {actions[i], MakeEntity("part' \xc3\xa9", "row' \xe6\xb0\xb4" + std::to_string(i))});
    }
    QueueResponse(HttpStatusCode::Accepted);
    CreateClient().SubmitTransaction(steps);
    ASSERT_EQ(m_requests.size(), 1U);
    std::vector<BatchRequest> parts;
    ParseBatchRequest(m_requests[0], parts);
    ASSERT_EQ(parts.size(), actions.size());
    for (size_t i = 0; i < parts.size(); ++i)
    {
      auto const path = Core::Url::Encode(
          "table(PartitionKey='part'' \xc3\xa9',RowKey='row'' \xe6\xb0\xb4" + std::to_string(i)
          + "')");
      auto const methodEnd = parts[i].RequestLine.find(' ');
      ASSERT_NE(methodEnd, std::string::npos);
      EXPECT_EQ(
          parts[i].RequestLine.substr(methodEnd + 1), "https://example.com/" + path + " HTTP/1.1");
    }
  }

  TEST_F(TablesRequestTest, QueryPreservesOptionsAcrossPages)
  {
    for (bool const hasKeySelector : {false, true})
    {
      SCOPED_TRACE(hasKeySelector);
      auto const start = m_requests.size();
      QueueResponse(
          HttpStatusCode::Ok,
          R"({"value":[{"PartitionKey":"partition","RowKey":"0"}]})",
          {{"x-ms-continuation-NextPartitionKey", "partition+token="},
           {"x-ms-continuation-NextRowKey", "row+token="}});
      QueueResponse(
          HttpStatusCode::Ok, R"({"value":[]})", {{"x-ms-continuation-NextPartitionKey", "next"}});
      QueueResponse(HttpStatusCode::Ok, R"({"value":[{"PartitionKey":"partition","RowKey":"2"}]})");
      TableClientOptions clientOptions;
      clientOptions.PerRetryPolicies.emplace_back(
          std::make_unique<Storage::Test::PeekHttpRequestPolicy>([](Core::Http::Request& request) {
            request.GetUrl().AppendQueryParameter("$top", "2");
          }));
      auto client = CreateClient(clientOptions);
      QueryEntitiesOptions options;
      options.Filter = "Name eq 'caf\xc3\xa9'";
      options.SelectColumns = "PartitionKey,RowKey,Name";
      if (hasKeySelector)
      {
        options.PartitionKey = "part' \xc3\xa9";
        options.RowKey = "row' \xe6\xb0\xb4";
      }
      auto page = client.QueryEntities(options);
      ASSERT_TRUE(page.HasPage());
      ASSERT_EQ(page.TableEntities.size(), 1U);
      EXPECT_EQ(page.TableEntities[0].GetRowKey().Value, "0");
      page.MoveToNextPage();
      ASSERT_TRUE(page.HasPage());
      EXPECT_TRUE(page.TableEntities.empty());
      page.MoveToNextPage();
      ASSERT_TRUE(page.HasPage());
      ASSERT_EQ(page.TableEntities.size(), 1U);
      EXPECT_EQ(page.TableEntities[0].GetRowKey().Value, "2");
      page.MoveToNextPage();
      EXPECT_FALSE(page.HasPage());
      ASSERT_EQ(m_requests.size(), start + 3);
      for (size_t i = start; i < m_requests.size(); ++i)
      {
        auto const& request = m_requests[i];
        EXPECT_EQ(request.Method, "GET");
        EXPECT_EQ(
            Core::Url::Decode(request.Url.GetPath()),
            hasKeySelector ? "table(PartitionKey='part'' \xc3\xa9',RowKey='row'' \xe6\xb0\xb4')"
                           : "table()");
        auto const& query = request.Url.GetQueryParameters();
        ASSERT_EQ(query.count("$filter"), 1U);
        ASSERT_EQ(query.count("$select"), 1U);
        EXPECT_EQ(query.at("$filter"), Core::Url::Encode(options.Filter.Value()));
        EXPECT_EQ(query.at("$select"), Core::Url::Encode(options.SelectColumns));
        EXPECT_EQ(query.at("$top"), "2");
      }
      auto const& first = m_requests[start].Url.GetQueryParameters();
      EXPECT_EQ(first.count("NextPartitionKey"), 0U);
      EXPECT_EQ(first.count("NextRowKey"), 0U);
      auto const& second = m_requests[start + 1].Url.GetQueryParameters();
      ASSERT_EQ(second.count("NextPartitionKey"), 1U);
      ASSERT_EQ(second.count("NextRowKey"), 1U);
      EXPECT_EQ(second.at("NextPartitionKey"), "partition%2Btoken%3D");
      EXPECT_EQ(second.at("NextRowKey"), "row%2Btoken%3D");
      auto const& third = m_requests[start + 2].Url.GetQueryParameters();
      ASSERT_EQ(third.count("NextPartitionKey"), 1U);
      EXPECT_EQ(third.at("NextPartitionKey"), "next");
      EXPECT_EQ(third.count("NextRowKey"), 0U);
    }
  }

  TEST_F(TablesRequestTest, QueryResumesWithPartitionOnlyContinuation)
  {
    for (bool const emptyRowHeader : {false, true})
    {
      SCOPED_TRACE(emptyRowHeader);
      Core::CaseInsensitiveMap headers{{"x-ms-continuation-NextPartitionKey", "partition+token="}};
      if (emptyRowHeader)
      {
        headers.emplace("x-ms-continuation-NextRowKey", "");
      }
      QueueResponse(HttpStatusCode::Ok, R"({"value":[]})", headers);
      QueueResponse(HttpStatusCode::Ok, R"({"value":[]})");
      auto client = CreateClient();
      auto page = client.QueryEntities();
      ASSERT_TRUE(page.HasPage());
      EXPECT_TRUE(page.TableEntities.empty());
      page.MoveToNextPage();
      auto const& query = m_requests.back().Url.GetQueryParameters();
      ASSERT_EQ(query.count("NextPartitionKey"), 1U);
      EXPECT_EQ(query.at("NextPartitionKey"), "partition%2Btoken%3D");
      EXPECT_EQ(query.count("NextRowKey"), 0U);
      page.MoveToNextPage();
      EXPECT_FALSE(page.HasPage());
    }
    QueueResponse(HttpStatusCode::Ok, R"({"value":[]})");
    QueryEntitiesOptions options;
    options.NextPartitionKey = "manual+token=";
    CreateClient().QueryEntities(options);
    auto const& query = m_requests.back().Url.GetQueryParameters();
    ASSERT_EQ(query.count("NextPartitionKey"), 1U);
    EXPECT_EQ(query.at("NextPartitionKey"), "manual%2Btoken%3D");
    EXPECT_EQ(query.count("NextRowKey"), 0U);
  }
}}} // namespace Azure::Data::Test
