// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/data/tables/tables_clients.hpp"

#include "azure/data/tables/internal/policies/service_version_policy.hpp"
#include "azure/data/tables/internal/policies/shared_key_lite_policy.hpp"
#include "azure/data/tables/internal/policies/tenant_bearer_token_policy.hpp"
#include "azure/data/tables/internal/policies/timeout_policy.hpp"
#include "azure/data/tables/internal/serializers.hpp"

#include <sstream>
#include <string>

using namespace Azure::Data::Tables;
using namespace Azure::Data::Tables::_detail::Policies;
using namespace Azure::Data::Tables::_detail::Xml;
using namespace Azure::Data::Tables::Credentials::_detail;
using namespace Azure::Data::Tables::_detail;

TableServiceClient::TableServiceClient(const TableClientOptions& options)
{
  TableClientOptions newOptions = options;
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
  perRetryPolicies.emplace_back(std::make_unique<TimeoutPolicy>());
  perOperationPolicies.emplace_back(
      std::make_unique<ServiceVersionPolicy>(newOptions.ApiVersion.ToString()));
  m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
      newOptions,
      _detail::TablesServicePackageName,
      _detail::ApiVersion,
      std::move(perRetryPolicies),
      std::move(perOperationPolicies));
}

TableServiceClient::TableServiceClient(
    const std::string& serviceUrl,
    const TableClientOptions& options)
{
  m_url = Azure::Core::Url(serviceUrl);
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
  perRetryPolicies.emplace_back(std::make_unique<TimeoutPolicy>());
  perOperationPolicies.emplace_back(
      std::make_unique<ServiceVersionPolicy>(options.ApiVersion.ToString()));
  m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
      options,
      _detail::TablesServicePackageName,
      _detail::ApiVersion,
      std::move(perRetryPolicies),
      std::move(perOperationPolicies));
}

TableServiceClient::TableServiceClient(
    const std::string& serviceUrl,
    std::shared_ptr<Core::Credentials::TokenCredential> credential,
    const TableClientOptions& options)
    : TableServiceClient(options)

{
  m_tokenCredential = credential;
  TableClientOptions newOptions = options;
  m_url = Azure::Core::Url(serviceUrl);
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
  perRetryPolicies.emplace_back(std::make_unique<TimeoutPolicy>());
  perOperationPolicies.emplace_back(
      std::make_unique<ServiceVersionPolicy>(options.ApiVersion.ToString()));

  perRetryPolicies.emplace_back(std::make_unique<TimeoutPolicy>());
  {
    Azure::Core::Credentials::TokenRequestContext tokenContext;
    tokenContext.Scopes.emplace_back(
        newOptions.Audience.HasValue() ? newOptions.Audience.Value().ToString()
                                       : m_url.GetAbsoluteUrl() + "/.default");

    perRetryPolicies.emplace_back(std::make_unique<TenantBearerTokenAuthenticationPolicy>(
        credential, tokenContext, newOptions.EnableTenantDiscovery));
  }
  perOperationPolicies.emplace_back(
      std::make_unique<ServiceVersionPolicy>(newOptions.ApiVersion.ToString()));
  m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
      newOptions,
      _detail::TablesServicePackageName,
      _detail::ApiVersion,
      std::move(perRetryPolicies),
      std::move(perOperationPolicies));
}

TableServiceClient::TableServiceClient(
    const std::string& serviceUrl,
    std::shared_ptr<Azure::Data::Tables::Credentials::NamedKeyCredential> credential,
    const TableClientOptions& options)
    : m_url(Azure::Core::Url(serviceUrl))
{
  m_namedKeyCredential = credential;
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
  perRetryPolicies.emplace_back(std::make_unique<TimeoutPolicy>());
  perOperationPolicies.emplace_back(
      std::make_unique<ServiceVersionPolicy>(options.ApiVersion.ToString()));
  m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
      options,
      _detail::TablesServicePackageName,
      _detail::ApiVersion,
      std::move(perRetryPolicies),
      std::move(perOperationPolicies));

  TableClientOptions newOptions = options;
  newOptions.PerRetryPolicies.emplace_back(
      std::make_unique<Azure::Data::Tables::_detail::Policies::SharedKeyLitePolicy>(credential));

  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies2;
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies2;
  perRetryPolicies2.emplace_back(std::make_unique<TimeoutPolicy>());
  perOperationPolicies2.emplace_back(
      std::make_unique<ServiceVersionPolicy>(newOptions.ApiVersion.ToString()));
  m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
      newOptions,
      _detail::TablesServicePackageName,
      _detail::ApiVersion,
      std::move(perRetryPolicies2),
      std::move(perOperationPolicies2));
}

TableServiceClient::TableServiceClient(
    const std::string& serviceUrl,
    std::shared_ptr<Azure::Data::Tables::Credentials::AzureSasCredential> credential,
    const TableClientOptions& options)
    : TableServiceClient(std::string{serviceUrl + credential->GetSignature()}, options)
{
}

TableClient TableServiceClient::GetTableClient(
    const std::string& tableName,
    TableClientOptions const& options) const
{
  if (m_namedKeyCredential != nullptr)
  {
    return TableClient(tableName, m_namedKeyCredential, m_url.GetAbsoluteUrl(), options);
  }
  if (m_tokenCredential != nullptr)
  {
    return TableClient(m_url.GetAbsoluteUrl(), tableName, m_tokenCredential, options);
  }
  if (!m_url.GetAbsoluteUrl().empty())
  {
    return TableClient(m_url.GetAbsoluteUrl(), tableName, options);
  }
  throw std::runtime_error("TableServiceClient is not properly initialized.");
}

TableServiceClient TableServiceClient::CreateFromConnectionString(
    const std::string& connectionString,
    const TableClientOptions& options)
{
  auto parsedConnectionString = ParseConnectionString(connectionString);
  auto tablesUrl = std::move(parsedConnectionString.TableServiceUrl);

  if (parsedConnectionString.KeyCredential)
  {
    return TableServiceClient(
        tablesUrl.GetAbsoluteUrl(), std::move(parsedConnectionString.KeyCredential), options);
  }
  else
  {
    return TableServiceClient(options);
  }
}

Azure::Response<Models::PreflightCheckResult> TableServiceClient::PreflightCheck(
    Models::PreflightCheckOptions const& options,
    Core::Context const& context)
{
  auto url = m_url;
  url.AppendPath(options.TableName);
  Core::Http::Request request(Core::Http::HttpMethod::Options, url);
  request.SetHeader(OriginHeader, options.Origin);
  request.SetHeader(AccessControlRequestMethodHeader, Core::Http::HttpMethod::Options.ToString());
  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();

  if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  Models::PreflightCheckResult response;

  return Response<Models::PreflightCheckResult>(std::move(response), std::move(rawResponse));
}

Azure::Response<Models::SetServicePropertiesResult> TableServiceClient::SetServiceProperties(
    Models::SetServicePropertiesOptions const& options,
    Core::Context const& context)
{
  std::string xmlBody = Serializers::SetServiceProperties(options);
  auto url = m_url;

  url.AppendQueryParameter(ResourceTypeHeader, ResrouceTypeService);
  url.AppendQueryParameter(CompHeader, ComponentProperties);
  Core::IO::MemoryBodyStream requestBody(
      reinterpret_cast<std::uint8_t const*>(xmlBody.data()), xmlBody.length());

  Core::Http::Request request(Core::Http::HttpMethod::Put, url, &requestBody);

  request.SetHeader(ContentTypeHeader, ContentTypeXml);
  request.SetHeader(ContentLengthHeader, std::to_string(requestBody.Length()));

  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();

  if (httpStatusCode != Core::Http::HttpStatusCode::Accepted)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  Models::SetServicePropertiesResult response;

  return Response<Models::SetServicePropertiesResult>(std::move(response), std::move(rawResponse));
}

Azure::Response<Models::TableServiceProperties> TableServiceClient::GetServiceProperties(
    Core::Context const& context)
{
  auto url = m_url;
  url.AppendQueryParameter(ResourceTypeHeader, ResrouceTypeService);
  url.AppendQueryParameter(CompHeader, ComponentProperties);

  Core::Http::Request request(Core::Http::HttpMethod::Get, url);

  auto pRawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = pRawResponse->GetStatusCode();

  if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
  {
    throw Core::RequestFailedException(pRawResponse);
  }

  Models::TableServiceProperties response
      = Serializers::ServicePropertiesFromXml(pRawResponse->GetBody());

  return Response<Models::TableServiceProperties>(std::move(response), std::move(pRawResponse));
}

Azure::Response<Models::ServiceStatistics> TableServiceClient::GetStatistics(
    const Core::Context& context)
{
  auto url = m_url;
  std::string host = url.GetHost();
  std::string accountName = host.substr(0, host.find('.'));
  accountName += "-secondary";
  url.SetHost(accountName + "." + host.substr(host.find('.') + 1));
  url.AppendQueryParameter(ResourceTypeHeader, ResrouceTypeService);
  url.AppendQueryParameter(CompHeader, "stats");
  Core::Http::Request request(Core::Http::HttpMethod::Get, url);

  auto pRawResponse = m_pipeline->Send(request, context);
  auto httpStatusCode = pRawResponse->GetStatusCode();
  if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
  {
    throw Core::RequestFailedException(pRawResponse);
  }
  Models::ServiceStatistics response;
  {
    const auto& responseBody = pRawResponse->GetBody();
    Xml::XmlReader reader(reinterpret_cast<const char*>(responseBody.data()), responseBody.size());
    enum class XmlTagEnum
    {
      kUnknown,
      kStorageServiceStats,
      kGeoReplication,
      kStatus,
      kLastSyncTime,
    };
    const std::unordered_map<std::string, XmlTagEnum> XmlTagEnumMap{
        {"StorageServiceStats", XmlTagEnum::kStorageServiceStats},
        {"GeoReplication", XmlTagEnum::kGeoReplication},
        {"Status", XmlTagEnum::kStatus},
        {"LastSyncTime", XmlTagEnum::kLastSyncTime},
    };
    std::vector<XmlTagEnum> xmlPath;

    while (true)
    {
      auto node = reader.Read();
      if (node.Type == XmlNodeType::End)
      {
        break;
      }
      else if (node.Type == XmlNodeType::StartTag)
      {
        auto ite = XmlTagEnumMap.find(node.Name);
        xmlPath.push_back(ite == XmlTagEnumMap.end() ? XmlTagEnum::kUnknown : ite->second);
      }
      else if (node.Type == XmlNodeType::Text)
      {
        if (xmlPath.size() == 3 && xmlPath[0] == XmlTagEnum::kStorageServiceStats
            && xmlPath[1] == XmlTagEnum::kGeoReplication && xmlPath[2] == XmlTagEnum::kStatus)
        {
          response.GeoReplication.Status = Models::GeoReplicationStatus(node.Value);
        }
        else if (
            xmlPath.size() == 3 && xmlPath[0] == XmlTagEnum::kStorageServiceStats
            && xmlPath[1] == XmlTagEnum::kGeoReplication && xmlPath[2] == XmlTagEnum::kLastSyncTime)
        {
          response.GeoReplication.LastSyncedOn
              = DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc1123);
        }
      }
      else if (node.Type == XmlNodeType::Attribute)
      {
      }
      else if (node.Type == XmlNodeType::EndTag)
      {

        xmlPath.pop_back();
      }
    }
  }
  return Response<Models::ServiceStatistics>(std::move(response), std::move(pRawResponse));
}

TableClient::TableClient(
    std::string const& serviceUrl,
    std::string const& tableName,
    const TableClientOptions& options)
    : m_url(serviceUrl), m_tableName(tableName)
{
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
  perRetryPolicies.emplace_back(std::make_unique<TimeoutPolicy>());
  perOperationPolicies.emplace_back(
      std::make_unique<ServiceVersionPolicy>(options.ApiVersion.ToString()));
  m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
      options,
      _detail::TablesServicePackageName,
      _detail::ApiVersion,
      std::move(perRetryPolicies),
      std::move(perOperationPolicies));
}

TableClient::TableClient(
    const std::string& serviceUrl,
    const std::string& tableName,
    std::shared_ptr<Core::Credentials::TokenCredential> credential,
    const TableClientOptions& options)
    : TableClient(serviceUrl, tableName, options)
{
  m_tableName = tableName;
  TableClientOptions newOptions = options;
  m_url = Azure::Core::Url(serviceUrl);
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
  perRetryPolicies.emplace_back(std::make_unique<TimeoutPolicy>());
  perOperationPolicies.emplace_back(
      std::make_unique<ServiceVersionPolicy>(options.ApiVersion.ToString()));

  perRetryPolicies.emplace_back(std::make_unique<TimeoutPolicy>());
  {
    Azure::Core::Credentials::TokenRequestContext tokenContext;
    tokenContext.Scopes.emplace_back(
        newOptions.Audience.HasValue() ? newOptions.Audience.Value().ToString()
                                       : m_url.GetAbsoluteUrl() + "/.default");

    perRetryPolicies.emplace_back(std::make_unique<TenantBearerTokenAuthenticationPolicy>(
        credential, tokenContext, newOptions.EnableTenantDiscovery));
  }
  perOperationPolicies.emplace_back(
      std::make_unique<ServiceVersionPolicy>(newOptions.ApiVersion.ToString()));
  m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
      newOptions,
      _detail::TablesServicePackageName,
      _detail::ApiVersion,
      std::move(perRetryPolicies),
      std::move(perOperationPolicies));
}

TableClient::TableClient(
    const std::string& tableName,
    std::shared_ptr<Azure::Data::Tables::Credentials::NamedKeyCredential> credential,
    std::string url,
    const TableClientOptions& options)
    : m_url(std::move(url)), m_tableName(tableName)

{
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
  perRetryPolicies.emplace_back(std::make_unique<TimeoutPolicy>());
  perOperationPolicies.emplace_back(
      std::make_unique<ServiceVersionPolicy>(options.ApiVersion.ToString()));
  m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
      options,
      _detail::TablesServicePackageName,
      _detail::ApiVersion,
      std::move(perRetryPolicies),
      std::move(perOperationPolicies));

  TableClientOptions newOptions = options;
  newOptions.PerRetryPolicies.emplace_back(
      std::make_unique<Azure::Data::Tables::_detail::Policies::SharedKeyLitePolicy>(credential));

  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies2;
  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies2;
  perRetryPolicies2.emplace_back(std::make_unique<TimeoutPolicy>());
  perOperationPolicies2.emplace_back(
      std::make_unique<ServiceVersionPolicy>(newOptions.ApiVersion.ToString()));
  m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
      newOptions,
      _detail::TablesServicePackageName,
      _detail::ApiVersion,
      std::move(perRetryPolicies2),
      std::move(perOperationPolicies2));
}

TableClient::TableClient(
    const std::string& serviceUrl,
    std::shared_ptr<Azure::Data::Tables::Credentials::AzureSasCredential> credential,
    const std::string& tableName,
    const TableClientOptions& options)
    : TableClient(
        std::string{
            Azure::Core::Url(serviceUrl).GetAbsoluteUrl() + "/" + credential->GetSignature()},
        tableName,
        options)
{
}

TableClient TableClient::CreateFromConnectionString(
    const std::string& connectionString,
    const std::string& tableName,
    const TableClientOptions& options)
{
  auto parsedConnectionString = ParseConnectionString(connectionString);
  auto tablesUrl = std::move(parsedConnectionString.TableServiceUrl);

  if (parsedConnectionString.KeyCredential)
  {
    return TableClient(
        tableName,
        std::move(parsedConnectionString.KeyCredential),
        tablesUrl.GetAbsoluteUrl(),
        options);
  }
  else
  {
    return TableClient(tablesUrl.GetAbsoluteUrl(), tableName, options);
  }
}

Azure::Response<Models::Table> TableServiceClient::CreateTable(
    std::string const& tableName,
    Core::Context const& context)
{
  auto url = m_url;
  url.AppendPath("Tables");

  std::string jsonBody = Serializers::Create(tableName);

  Core::IO::MemoryBodyStream requestBody(
      reinterpret_cast<std::uint8_t const*>(jsonBody.data()), jsonBody.length());

  Core::Http::Request request(Core::Http::HttpMethod::Post, url, &requestBody);

  request.SetHeader(ContentTypeHeader, ContentTypeJson);
  request.SetHeader(ContentLengthHeader, std::to_string(requestBody.Length()));
  request.SetHeader(AcceptHeader, AcceptFullMeta);
  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();

  if (httpStatusCode != Core::Http::HttpStatusCode::Created)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  Models::Table response{};
  {
    auto const& responseBody = rawResponse->GetBody();
    std::string responseString = std::string(responseBody.begin(), responseBody.end());
    if (responseBody.size() > 0)
    {
      auto const jsonRoot
          = Core::Json::_internal::json::parse(responseBody.begin(), responseBody.end());

      response.TableName = jsonRoot[TableName].get<std::string>();
      response.EditLink = jsonRoot[ODataEditLink].get<std::string>();
      response.Id = jsonRoot[ODataId].get<std::string>();
      response.Metadata = jsonRoot[ODataMeta].get<std::string>();
      response.Type = jsonRoot[ODataType].get<std::string>();
    }
  }

  return Response<Models::Table>(std::move(response), std::move(rawResponse));
}

void Models::QueryTablesPagedResponse::OnNextPage(const Azure::Core::Context& context)
{
  m_operationOptions.ContinuationToken = NextPageToken;
  *this = m_tableServiceClient->QueryTables(m_operationOptions, context);
}

Models::QueryTablesPagedResponse TableServiceClient::QueryTables(
    Models::QueryTablesOptions const& options,
    Azure::Core::Context const& context) const
{
  auto url = m_url;
  url.AppendPath("Tables");
  Core::Http::Request request(Core::Http::HttpMethod::Get, url);
  request.SetHeader(AcceptHeader, AcceptFullMeta);
  if (options.Prefix.HasValue())
  {
    request.GetUrl().AppendQueryParameter(IfMatch, options.Prefix.Value());
  }
  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();

  if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  Models::QueryTablesPagedResponse response;
  {
    auto const& responseBody = rawResponse->GetBody();
    std::string responseString = std::string(responseBody.begin(), responseBody.end());
    if (responseBody.size() > 0)
    {
      auto const jsonRoot
          = Core::Json::_internal::json::parse(responseBody.begin(), responseBody.end());
      std::string metadataLink = jsonRoot[ODataMeta].get<std::string>();
      for (auto value : jsonRoot[Value])
      {
        Models::Table table;
        table.TableName = value[TableName].get<std::string>();
        table.EditLink = value[ODataEditLink].get<std::string>();
        table.Id = value[ODataId].get<std::string>();
        table.Type = value[ODataType].get<std::string>();
        table.Metadata = metadataLink;
        response.Tables.emplace_back(std::move(table));
      }
    }

    response.ServiceEndpoint = url.GetAbsoluteUrl();
    response.Prefix = options.Prefix;
    response.m_tableServiceClient = std::make_shared<TableServiceClient>(*this);
    response.m_operationOptions = options;
    response.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    response.RawResponse = std::move(response.RawResponse);
    auto headers = rawResponse->GetHeaders();

    if (headers.find("x-ms-continuation-NextTableName") != headers.end())
    {
      response.NextPageToken = headers.at("x-ms-continuation-NextTableName");
    }
  }

  return response;
}

Azure::Response<Models::SetTableAccessPolicyResult> TableClient::SetAccessPolicy(
    Models::TableAccessPolicy const& tableAccessPolicy,
    Core::Context const& context)

{
  auto url = m_url;
  url.AppendPath(m_tableName);
  url.AppendQueryParameter(CompHeader, "acl");
  std::string xmlBody = Serializers::SetAccessPolicy(tableAccessPolicy);
  Core::IO::MemoryBodyStream requestBody(
      reinterpret_cast<const uint8_t*>(xmlBody.data()), xmlBody.length());

  auto request = Core::Http::Request(Core::Http::HttpMethod::Put, url, &requestBody);
  request.SetHeader(ContentTypeHeader, "application/xml; charset=UTF-8");
  request.SetHeader(ContentLengthHeader, std::to_string(requestBody.Length()));
  request.GetUrl().AppendQueryParameter(CompHeader, "acl");
  request.SetHeader("x-ms-version", "2019-12-12");
  auto pRawResponse = m_pipeline->Send(request, context);
  auto httpStatusCode = pRawResponse->GetStatusCode();
  if (httpStatusCode != Core::Http::HttpStatusCode::NoContent)
  {
    throw Core::RequestFailedException(pRawResponse);
  }
  Models::SetTableAccessPolicyResult response;
  return Response<Models::SetTableAccessPolicyResult>(std::move(response), std::move(pRawResponse));
}

Azure::Response<Models::TableAccessPolicy> TableClient::GetAccessPolicy(
    Core::Context const& context)
{
  auto url = m_url;
  url.SetPath("");
  url.AppendPath(m_tableName);
  url.AppendQueryParameter(CompHeader, "acl");
  Core::Http::Request request(Core::Http::HttpMethod::Get, url);

  auto pRawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = pRawResponse->GetStatusCode();
  if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
  {
    throw Core::RequestFailedException(pRawResponse);
  }
  const auto& responseBody = pRawResponse->GetBody();

  Models::TableAccessPolicy response = Serializers::TableAccessPolicyFromXml(responseBody);

  return Response<Models::TableAccessPolicy>(std::move(response), std::move(pRawResponse));
}

Azure::Response<Models::DeleteTableResult> TableServiceClient::DeleteTable(
    std::string const& tableName,
    Core::Context const& context)
{
  auto url = m_url;
  url.AppendPath("Tables('" + tableName + ClosingFragment);

  Core::Http::Request request(Core::Http::HttpMethod::Delete, url);

  request.SetHeader(ContentTypeHeader, ContentTypeJson);
  request.SetHeader(AcceptHeader, AcceptFullMeta);
  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();

  if (httpStatusCode != Core::Http::HttpStatusCode::NoContent)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  Models::DeleteTableResult response{};

  return Response<Models::DeleteTableResult>(std::move(response), std::move(rawResponse));
}

Azure::Response<Models::AddEntityResult> TableClient::AddEntity(
    Models::TableEntity const& tableEntity,
    Models::AddEntityOptions const& options,
    Core::Context const& context)
{
  (void)options;
  auto url = m_url;
  url.AppendPath(m_tableName);

  std::string jsonBody = Serializers::CreateEntity(tableEntity);

  Core::IO::MemoryBodyStream requestBody(
      reinterpret_cast<std::uint8_t const*>(jsonBody.data()), jsonBody.length());

  Core::Http::Request request(Core::Http::HttpMethod::Post, url, &requestBody);

  request.SetHeader(ContentTypeHeader, ContentTypeJson);
  request.SetHeader(ContentLengthHeader, std::to_string(requestBody.Length()));
  request.SetHeader(AcceptHeader, AcceptFullMeta);
  request.SetHeader(PreferHeader, PreferNoContent);
  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();
  if (httpStatusCode != Core::Http::HttpStatusCode::NoContent)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  Models::AddEntityResult response{};
  response.ETag = rawResponse->GetHeaders().at("ETag");
  return Response<Models::AddEntityResult>(std::move(response), std::move(rawResponse));
}

Azure::Response<Models::UpdateEntityResult> TableClient::UpdateEntity(
    Models::TableEntity const& tableEntity,
    Models::UpdateEntityOptions const& options,
    Core::Context const& context)
{
  (void)options;
  auto url = m_url;
  url.AppendPath(
      m_tableName + PartitionKeyFragment
      + Azure::Core::Url::Encode(tableEntity.GetPartitionKey().Value) + RowKeyFragment
      + Azure::Core::Url::Encode(tableEntity.GetRowKey().Value) + ClosingFragment);

  std::string jsonBody = Serializers::UpdateEntity(tableEntity);

  Core::IO::MemoryBodyStream requestBody(
      reinterpret_cast<std::uint8_t const*>(jsonBody.data()), jsonBody.length());

  Core::Http::Request request(Core::Http::HttpMethod::Put, url, &requestBody);

  request.SetHeader(ContentTypeHeader, ContentTypeJson);
  request.SetHeader(ContentLengthHeader, std::to_string(requestBody.Length()));
  request.SetHeader(AcceptHeader, AcceptFullMeta);
  request.SetHeader(PreferHeader, PreferNoContent);
  if (!tableEntity.GetETag().Value.empty())
  {
    request.SetHeader(IfMatch, tableEntity.GetETag().Value);
  }
  else
  {
    request.SetHeader(IfMatch, "*");
  }

  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();
  if (httpStatusCode != Core::Http::HttpStatusCode::NoContent)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  Models::UpdateEntityResult response{};

  response.ETag = rawResponse->GetHeaders().at("ETag");
  return Response<Models::UpdateEntityResult>(std::move(response), std::move(rawResponse));
}

Azure::Response<Models::MergeEntityResult> TableClient::MergeEntity(
    Models::TableEntity const& tableEntity,
    Models::MergeEntityOptions const& options,
    Core::Context const& context)
{
  (void)options;
  auto url = m_url;
  url.AppendPath(
      m_tableName + PartitionKeyFragment
      + Azure::Core::Url::Encode(tableEntity.GetPartitionKey().Value) + RowKeyFragment
      + Azure::Core::Url::Encode(tableEntity.GetRowKey().Value) + ClosingFragment);

  std::string jsonBody = Serializers::MergeEntity(tableEntity);

  Core::IO::MemoryBodyStream requestBody(
      reinterpret_cast<std::uint8_t const*>(jsonBody.data()), jsonBody.length());

  Core::Http::Request request(Core::Http::HttpMethod::Patch, url, &requestBody);

  request.SetHeader(ContentTypeHeader, ContentTypeJson);
  request.SetHeader(ContentLengthHeader, std::to_string(requestBody.Length()));
  request.SetHeader(AcceptHeader, AcceptFullMeta);
  request.SetHeader(PreferHeader, PreferNoContent);
  if (!tableEntity.GetETag().Value.empty())
  {
    request.SetHeader(IfMatch, tableEntity.GetETag().Value);
  }
  else
  {
    request.SetHeader(IfMatch, "*");
  }

  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();
  if (httpStatusCode != Core::Http::HttpStatusCode::NoContent)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  Models::MergeEntityResult response{};

  response.ETag = rawResponse->GetHeaders().at("ETag");
  return Response<Models::MergeEntityResult>(std::move(response), std::move(rawResponse));
}

Azure::Response<Models::DeleteEntityResult> TableClient::DeleteEntity(
    Models::TableEntity const& tableEntity,
    Core::Context const& context)
{
  auto url = m_url;
  url.AppendPath(
      m_tableName + PartitionKeyFragment
      + Azure::Core::Url::Encode(tableEntity.GetPartitionKey().Value) + RowKeyFragment
      + Azure::Core::Url::Encode(tableEntity.GetRowKey().Value) + ClosingFragment);

  Core::Http::Request request(Core::Http::HttpMethod::Delete, url);

  if (!tableEntity.GetETag().Value.empty())
  {
    request.SetHeader(IfMatch, tableEntity.GetETag().Value);
  }
  else
  {
    request.SetHeader(IfMatch, "*");
  }
  request.SetHeader(AcceptHeader, AcceptFullMeta);
  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();
  if (httpStatusCode != Core::Http::HttpStatusCode::NoContent)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  Models::DeleteEntityResult response{};
  return Response<Models::DeleteEntityResult>(std::move(response), std::move(rawResponse));
}

Azure::Response<Models::UpsertEntityResult> TableClient::UpsertEntity(
    Models::TableEntity const& tableEntity,
    Models::UpsertEntityOptions const& options,
    Core::Context const& context)
{
  (void)options;
  try
  {
    switch (options.UpsertType)
    {
      case Models::UpsertKind::Merge: {
        auto response = MergeEntity(tableEntity, Models::MergeEntityOptions(options), context);
        return Azure::Response<Models::UpsertEntityResult>(
            Models::UpsertEntityResult(response.Value), std::move(response.RawResponse));
      }
      default: {
        auto response = UpdateEntity(tableEntity, Models::UpdateEntityOptions(options), context);
        return Azure::Response<Models::UpsertEntityResult>(
            Models::UpsertEntityResult(response.Value), std::move(response.RawResponse));
      }
    }
  }
  catch (const Azure::Core::RequestFailedException&)
  {
    auto response = AddEntity(tableEntity, Models::AddEntityOptions(options), context);
    return Azure::Response<Models::UpsertEntityResult>(
        Models::UpsertEntityResult(response.Value), std::move(response.RawResponse));
  }
}

void Models::QueryEntitiesPagedResponse::OnNextPage(const Azure::Core::Context& context)
{
  m_operationOptions.PartitionKey = NextPartitionKey;
  m_operationOptions.RowKey = NextRowKey;
  *this = m_tableClient->QueryEntities(m_operationOptions, context);
}

Azure::Response<Models::TableEntity> TableClient::GetEntity(
    const std::string& partitionKey,
    const std::string& rowKey,
    Core::Context const& context)
{
  auto url = m_url;
  url.AppendPath(
      m_tableName + PartitionKeyFragment + Azure::Core::Url::Encode(partitionKey) + RowKeyFragment
      + Azure::Core::Url::Encode(rowKey) + ClosingFragment);

  Core::Http::Request request(Core::Http::HttpMethod::Get, url);
  request.SetHeader(AcceptHeader, AcceptFullMeta);

  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();
  if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  Models::TableEntity response{};
  {
    const auto& responseBody = rawResponse->GetBody();
    std::string responseString = std::string(responseBody.begin(), responseBody.end());

    auto const jsonRoot
        = Azure::Core::Json::_internal::json::parse(responseBody.begin(), responseBody.end());

    response = Serializers::DeserializeEntity(jsonRoot);
  }
  return Response<Models::TableEntity>(std::move(response), std::move(rawResponse));
}

Models::QueryEntitiesPagedResponse TableClient::QueryEntities(
    Models::QueryEntitiesOptions const& options,
    Core::Context const& context)
{
  auto url = m_url;
  std::string appendPath = m_tableName + "(";
  if (!options.PartitionKey.empty())
  {
    appendPath += "PartitionKey='" + Azure::Core::Url::Encode(options.PartitionKey) + "'";
  }
  if (!options.RowKey.empty())
  {
    appendPath += ",RowKey='" + Azure::Core::Url::Encode(options.RowKey) + "'";
  }
  appendPath += ")";

  url.AppendPath(appendPath);

  if (options.Filter.HasValue())
  {
    url.AppendQueryParameter("$filter", Azure::Core::Url::Encode(options.Filter.Value()));
  }
  if (!options.SelectColumns.empty())
  {
    url.AppendQueryParameter("$select", Azure::Core::Url::Encode(options.SelectColumns));
  }

  Core::Http::Request request(Core::Http::HttpMethod::Get, url);
  request.SetHeader(AcceptHeader, AcceptFullMeta);

  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();
  if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  Models::QueryEntitiesPagedResponse response{};
  {
    const auto& responseBody = rawResponse->GetBody();
    std::string responseString = std::string(responseBody.begin(), responseBody.end());

    auto headers = rawResponse->GetHeaders();
    if (headers.find("x-ms-continuation-NextPartitionKey") != headers.end())
    {
      response.NextPartitionKey = headers.at("x-ms-continuation-NextPartitionKey");
    }

    if (headers.find("x-ms-continuation-NextRowKey") != headers.end())
    {
      response.NextRowKey = headers.at("x-ms-continuation-NextRowKey");
    }

    auto const jsonRoot
        = Core::Json::_internal::json::parse(responseBody.begin(), responseBody.end());

    if (!jsonRoot.contains(Value))
    {
      response.TableEntities.emplace_back(Serializers::DeserializeEntity(jsonRoot));
    }
    else
    {
      for (auto value : jsonRoot[Value])
      {
        response.TableEntities.emplace_back(Serializers::DeserializeEntity(value));
      }
    }
  }
  return response;
}

Azure::Response<Models::SubmitTransactionResult> TableClient::SubmitTransaction(
    std::vector<Models::TransactionStep> const& steps,
    Core::Context const& context)
{
  auto url = m_url;
  url.AppendPath("$batch");
  std::string batchId = "batch_" + Azure::Core::Uuid::CreateUuid().ToString();
  std::string changesetId = "changeset_" + Azure::Core::Uuid::CreateUuid().ToString();

  std::string body = PreparePayload(batchId, changesetId, steps);
  Core::IO::MemoryBodyStream requestBody(
      reinterpret_cast<std::uint8_t const*>(body.data()), body.length());

  Core::Http::Request request(Core::Http::HttpMethod::Post, url, &requestBody);

  request.SetHeader(ContentTypeHeader, "multipart/mixed; boundary=" + batchId);
  request.SetHeader(AcceptHeader, AcceptFullMeta);
  request.SetHeader(ContentLengthHeader, std::to_string(requestBody.Length()));
  request.SetHeader("Connection", "Keep-Alive");
  request.SetHeader("DataServiceVersion", "3.0");
  request.SetHeader("Accept-Charset", "UTF-8");
  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();
  if (httpStatusCode != Core::Http::HttpStatusCode::Accepted)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  Models::SubmitTransactionResult response{};
  {
    const auto& responseBody = rawResponse->GetBody();
    std::string responseString = std::string(responseBody.begin(), responseBody.end());
    std::stringstream ss(responseString);

    std::string line;
    std::getline(ss, line, '\n');
    Models::TransactionError error;
    while (line.size() != 0)
    {
      std::getline(ss, line, '\n');
      if (line.find("HTTP") != std::string::npos)
      {
        std::string status = line.substr(line.find(" ") + 1, 3);
        response.StatusCode = status;
      }

      if (line.find(ODataError) != std::string::npos)
      {
        auto const jsonRoot = Core::Json::_internal::json::parse(line.begin(), line.end());
        if (jsonRoot[ODataError].contains("code"))
        {
          error.Code = jsonRoot[ODataError]["code"].get<std::string>();
        }
        if (jsonRoot[ODataError].contains("message")
            && jsonRoot[ODataError]["message"].contains(Value))
        {
          error.Message = jsonRoot[ODataError]["message"][Value].get<std::string>();
        }
      }
    }
    if (error.Message.size() != 0)
    {
      response.Error = error;
    }
  }
  return Response<Models::SubmitTransactionResult>(std::move(response), std::move(rawResponse));
}

std::string TableClient::PreparePayload(
    std::string const& batchId,
    std::string const& changesetId,
    std::vector<Models::TransactionStep> const& steps)
{
  std::string accumulator
      = "--" + batchId + "\nContent-Type: multipart/mixed; boundary=" + changesetId + "\n\n";

  for (auto step : steps)
  {
    switch (step.Action)
    {
      case Models::TransactionActionType::Add:
        accumulator += PrepAddEntity(changesetId, step.Entity);
        break;
      case Models::TransactionActionType::Delete:
        accumulator += PrepDeleteEntity(changesetId, step.Entity);
        break;
      case Models::TransactionActionType::InsertMerge:
      case Models::TransactionActionType::UpdateMerge:
        accumulator += PrepMergeEntity(changesetId, step.Entity);
        break;
      case Models::TransactionActionType::InsertReplace:
      case Models::TransactionActionType::UpdateReplace:
        accumulator += PrepUpdateEntity(changesetId, step.Entity);
        break;
    }
  }

  accumulator += "\n\n--" + changesetId + "--\n";
  accumulator += "--" + batchId + "\n";
  return accumulator;
}
std::string TableClient::PrepAddEntity(std::string const& changesetId, Models::TableEntity entity)
{
  std::string returnValue = "--" + changesetId + "\n";
  returnValue += "Content-Type: application/http\n";
  returnValue += "Content-Transfer-Encoding: binary\n\n";

  returnValue += "POST " + m_url.GetAbsoluteUrl() + "/" + m_tableName + " HTTP/1.1\n";
  returnValue += "Content-Type: application/json\n";
  returnValue += "Accept: application/json;odata=minimalmetadata\n";
  returnValue += "Prefer: return-no-content\n";
  returnValue += "DataServiceVersion: 3.0;\n\n";
  returnValue += Serializers::CreateEntity(entity);
  return returnValue;
}
std::string TableClient::PrepDeleteEntity(
    std::string const& changesetId,
    Models::TableEntity entity)
{
  std::string returnValue = "--" + changesetId + "\n";
  returnValue += "Content-Type: application/http\n";
  returnValue += "Content-Transfer-Encoding: binary\n\n";

  returnValue += "DELETE " + m_url.GetAbsoluteUrl() + "/" + m_tableName + PartitionKeyFragment
      + entity.GetPartitionKey().Value + RowKeyFragment + entity.GetRowKey().Value + ClosingFragment
      + " HTTP/1.1\n";
  returnValue += "Accept: application/json;odata=minimalmetadata\n";
  // returnValue += "Prefer: return-no-content\n";
  returnValue += "DataServiceVersion: 3.0;\n";
  if (!entity.GetETag().Value.empty())
  {
    returnValue += "If-Match: " + entity.GetETag().Value;
  }
  else
  {
    returnValue += "If-Match: *";
  }
  returnValue += "\n";
  return returnValue;
}

std::string TableClient::PrepMergeEntity(std::string const& changesetId, Models::TableEntity entity)
{
  std::string returnValue = "--" + changesetId + "\n";
  returnValue += "Content-Type: application/http\n";
  returnValue += "Content-Transfer-Encoding: binary\n\n";

  returnValue += "MERGE " + m_url.GetAbsoluteUrl() + "/" + m_tableName + PartitionKeyFragment
      + entity.GetPartitionKey().Value + RowKeyFragment + entity.GetRowKey().Value + ClosingFragment
      + " HTTP/1.1\n";
  returnValue += "Content-Type: application/json\n";
  returnValue += "Accept: application/json;odata=minimalmetadata\n";
  returnValue += "DataServiceVersion: 3.0;\n\n";
  returnValue += Serializers::MergeEntity(entity);

  return returnValue;
}

std::string TableClient::PrepUpdateEntity(
    std::string const& changesetId,
    Models::TableEntity entity)
{
  std::string returnValue = "--" + changesetId + "\n";
  returnValue += "Content-Type: application/http\n";
  returnValue += "Content-Transfer-Encoding: binary\n\n";

  returnValue += "PUT " + m_url.GetAbsoluteUrl() + "/" + m_tableName + PartitionKeyFragment
      + entity.GetPartitionKey().Value + RowKeyFragment + entity.GetRowKey().Value + ClosingFragment
      + " HTTP/1.1\n";
  returnValue += "Content-Type: application/json\n";
  returnValue += "Accept: application/json;odata=minimalmetadata\n";
  returnValue += "Prefer: return-no-content\n";
  returnValue += "DataServiceVersion: 3.0;\n";
  if (!entity.GetETag().Value.empty())
  {
    returnValue += "If-Match: " + entity.GetETag().Value;
  }
  else
  {
    returnValue += "If-Match: *";
  }
  returnValue += "\n\n";
  returnValue += Serializers::UpdateEntity(entity);
  return returnValue;
}
