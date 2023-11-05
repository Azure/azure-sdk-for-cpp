// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// Code generated by Microsoft (R) AutoRest Code Generator.
// Changes may cause incorrect behavior and will be lost if the code is regenerated.

#include "azure/storage/tables/rest_client.hpp"

#include <azure/core/exception.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/http_status_code.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/io/body_stream.hpp>
#include <azure/storage/common/internal/xml_wrapper.hpp>
using namespace Azure::Storage::Tables;

AllowedMethodsType const AllowedMethodsType::Delete{"DELETE"};
AllowedMethodsType const AllowedMethodsType::Get{"GET"};
AllowedMethodsType const AllowedMethodsType::Head{"HEAD"};
AllowedMethodsType const AllowedMethodsType::Merge{"MERGE"};
AllowedMethodsType const AllowedMethodsType::Post{"POST"};
AllowedMethodsType const AllowedMethodsType::Options{"OPTIONS"};
AllowedMethodsType const AllowedMethodsType::Put{"PUT"};
AllowedMethodsType const AllowedMethodsType::Patch{"PATCH"};
AllowedMethodsType const AllowedMethodsType::Connect{"CONNECT"};
AllowedMethodsType const AllowedMethodsType::Trace{"TRACE"};

const TablesAudience TablesAudience::PublicAudience(
    Azure::Storage::_internal::TablesManagementScope);

Azure::Response<Models::PreflightCheckResult> TableServicesClient::PreflightCheck(
    Models::PreflightCheckOptions const& options,
    Core::Context const& context)
{
  auto url = m_url;
  url.AppendPath(options.TableName);
  Core::Http::Request request(Core::Http::HttpMethod::Options, url);
  request.SetHeader("Origin", options.Origin);
  request.SetHeader("Access-Control-Request-Method", Core::Http::HttpMethod::Options.ToString());
  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();

  if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  Models::PreflightCheckResult response;

  return Response<Models::PreflightCheckResult>(std::move(response), std::move(rawResponse));
}
Azure::Response<Models::SetServicePropertiesResult> TableServicesClient::SetServiceProperties(
    Models::SetServicePropertiesOptions const& options,
    Core::Context const& context)
{
  std::string xmlBody;
  {
    _internal::XmlWriter writer;
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "StorageServiceProperties"});
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Logging"});
    writer.Write(_internal::XmlNode{
        _internal::XmlNodeType::StartTag,
        "Version",
        options.TableServiceProperties.Logging.Version});
    writer.Write(_internal::XmlNode{
        _internal::XmlNodeType::StartTag,
        "Delete",
        options.TableServiceProperties.Logging.Delete ? "true" : "false"});
    writer.Write(_internal::XmlNode{
        _internal::XmlNodeType::StartTag,
        "Read",
        options.TableServiceProperties.Logging.Read ? "true" : "false"});
    writer.Write(_internal::XmlNode{
        _internal::XmlNodeType::StartTag,
        "Write",
        options.TableServiceProperties.Logging.Write ? "true" : "false"});
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "RetentionPolicy"});
    writer.Write(_internal::XmlNode{
        _internal::XmlNodeType::StartTag,
        "Enabled",
        options.TableServiceProperties.Logging.RetentionPolicy.IsEnabled ? "true" : "false"});
    if (options.TableServiceProperties.Logging.RetentionPolicy.Days.HasValue())
    {
      writer.Write(_internal::XmlNode{
          _internal::XmlNodeType::StartTag,
          "Days",
          std::to_string(options.TableServiceProperties.Logging.RetentionPolicy.Days.Value())});
    }
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "HourMetrics"});
    writer.Write(_internal::XmlNode{
        _internal::XmlNodeType::StartTag,
        "Version",
        options.TableServiceProperties.HourMetrics.Version});
    writer.Write(_internal::XmlNode{
        _internal::XmlNodeType::StartTag,
        "Enabled",
        options.TableServiceProperties.HourMetrics.IsEnabled ? "true" : "false"});
    if (options.TableServiceProperties.HourMetrics.IncludeApis.HasValue())
    {
      writer.Write(_internal::XmlNode{
          _internal::XmlNodeType::StartTag,
          "IncludeAPIs",
          options.TableServiceProperties.HourMetrics.IncludeApis.Value() ? "true" : "false"});
    }
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "RetentionPolicy"});
    writer.Write(_internal::XmlNode{
        _internal::XmlNodeType::StartTag,
        "Enabled",
        options.TableServiceProperties.HourMetrics.RetentionPolicy.IsEnabled ? "true" : "false"});
    if (options.TableServiceProperties.HourMetrics.RetentionPolicy.Days.HasValue())
    {
      writer.Write(_internal::XmlNode{
          _internal::XmlNodeType::StartTag,
          "Days",
          std::to_string(options.TableServiceProperties.HourMetrics.RetentionPolicy.Days.Value())});
    }
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "MinuteMetrics"});
    writer.Write(_internal::XmlNode{
        _internal::XmlNodeType::StartTag,
        "Version",
        options.TableServiceProperties.MinuteMetrics.Version});
    writer.Write(_internal::XmlNode{
        _internal::XmlNodeType::StartTag,
        "Enabled",
        options.TableServiceProperties.MinuteMetrics.IsEnabled ? "true" : "false"});
    if (options.TableServiceProperties.MinuteMetrics.IncludeApis.HasValue())
    {
      writer.Write(_internal::XmlNode{
          _internal::XmlNodeType::StartTag,
          "IncludeAPIs",
          options.TableServiceProperties.MinuteMetrics.IncludeApis.Value() ? "true" : "false"});
    }
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "RetentionPolicy"});
    writer.Write(_internal::XmlNode{
        _internal::XmlNodeType::StartTag,
        "Enabled",
        options.TableServiceProperties.MinuteMetrics.RetentionPolicy.IsEnabled ? "true" : "false"});
    if (options.TableServiceProperties.MinuteMetrics.RetentionPolicy.Days.HasValue())
    {
      writer.Write(_internal::XmlNode{
          _internal::XmlNodeType::StartTag,
          "Days",
          std::to_string(
              options.TableServiceProperties.MinuteMetrics.RetentionPolicy.Days.Value())});
    }
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Cors"});
    for (const auto& i1 : options.TableServiceProperties.Cors)
    {
      writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "CorsRule"});
      writer.Write(_internal::XmlNode{
          _internal::XmlNodeType::StartTag, "AllowedOrigins", i1.AllowedOrigins});
      writer.Write(_internal::XmlNode{
          _internal::XmlNodeType::StartTag, "AllowedMethods", i1.AllowedMethods});
      writer.Write(_internal::XmlNode{
          _internal::XmlNodeType::StartTag, "AllowedHeaders", i1.AllowedHeaders});
      writer.Write(_internal::XmlNode{
          _internal::XmlNodeType::StartTag, "ExposedHeaders", i1.ExposedHeaders});
      writer.Write(_internal::XmlNode{
          _internal::XmlNodeType::StartTag, "MaxAgeInSeconds", std::to_string(i1.MaxAgeInSeconds)});
      writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
    }
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::End});
    xmlBody = writer.GetDocument();
  }
  auto url = m_url;

  url.SetQueryParameters({{"restype", "service"}, {"comp", "properties"}});
  Core::IO::MemoryBodyStream requestBody(
      reinterpret_cast<std::uint8_t const*>(xmlBody.data()), xmlBody.length());

  Core::Http::Request request(Core::Http::HttpMethod::Put, url, &requestBody);

  request.SetHeader("Content-Type", "application/xml");
  request.SetHeader("Content-Length", std::to_string(requestBody.Length()));

  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();

  if (httpStatusCode != Core::Http::HttpStatusCode::Accepted)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  Models::SetServicePropertiesResult response;

  return Response<Models::SetServicePropertiesResult>(std::move(response), std::move(rawResponse));
}

Azure::Response<Models::TableServiceProperties> TableServicesClient::GetServiceProperties(
    Models::GetServicePropertiesOptions const& options,
    Core::Context const& context)
{
  (void)options;
  auto url = m_url;
  url.SetQueryParameters({{"restype", "service"}, {"comp", "properties"}});

  Core::Http::Request request(Core::Http::HttpMethod::Get, url);

  auto pRawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = pRawResponse->GetStatusCode();

  if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
  {
    throw Core::RequestFailedException(pRawResponse);
  }

  Models::TableServiceProperties response{};
  {
    const auto& responseBody = pRawResponse->GetBody();
    _internal::XmlReader reader(
        reinterpret_cast<const char*>(responseBody.data()), responseBody.size());
    enum class XmlTagEnum
    {
      kUnknown,
      kStorageServiceProperties,
      kLogging,
      kVersion,
      kDelete,
      kRead,
      kWrite,
      kRetentionPolicy,
      kEnabled,
      kDays,
      kHourMetrics,
      kIncludeAPIs,
      kMinuteMetrics,
      kCors,
      kCorsRule,
      kAllowedOrigins,
      kAllowedMethods,
      kAllowedHeaders,
      kExposedHeaders,
      kMaxAgeInSeconds,
    };
    const std::unordered_map<std::string, XmlTagEnum> XmlTagEnumMap{
        {"StorageServiceProperties", XmlTagEnum::kStorageServiceProperties},
        {"Logging", XmlTagEnum::kLogging},
        {"Version", XmlTagEnum::kVersion},
        {"Delete", XmlTagEnum::kDelete},
        {"Read", XmlTagEnum::kRead},
        {"Write", XmlTagEnum::kWrite},
        {"RetentionPolicy", XmlTagEnum::kRetentionPolicy},
        {"Enabled", XmlTagEnum::kEnabled},
        {"Days", XmlTagEnum::kDays},
        {"HourMetrics", XmlTagEnum::kHourMetrics},
        {"IncludeAPIs", XmlTagEnum::kIncludeAPIs},
        {"MinuteMetrics", XmlTagEnum::kMinuteMetrics},
        {"Cors", XmlTagEnum::kCors},
        {"CorsRule", XmlTagEnum::kCorsRule},
        {"AllowedOrigins", XmlTagEnum::kAllowedOrigins},
        {"AllowedMethods", XmlTagEnum::kAllowedMethods},
        {"AllowedHeaders", XmlTagEnum::kAllowedHeaders},
        {"ExposedHeaders", XmlTagEnum::kExposedHeaders},
        {"MaxAgeInSeconds", XmlTagEnum::kMaxAgeInSeconds},
    };
    std::vector<XmlTagEnum> xmlPath;
    Models::CorsRule vectorElement1;
    while (true)
    {
      auto node = reader.Read();
      if (node.Type == _internal::XmlNodeType::End)
      {
        break;
      }
      else if (node.Type == _internal::XmlNodeType::StartTag)
      {
        auto ite = XmlTagEnumMap.find(node.Name);
        xmlPath.push_back(ite == XmlTagEnumMap.end() ? XmlTagEnum::kUnknown : ite->second);
      }
      else if (node.Type == _internal::XmlNodeType::Text)
      {
        if (xmlPath.size() == 3 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kLogging && xmlPath[2] == XmlTagEnum::kVersion)
        {
          response.Logging.Version = node.Value;
        }
        else if (
            xmlPath.size() == 3 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kLogging && xmlPath[2] == XmlTagEnum::kDelete)
        {
          response.Logging.Delete = node.Value == std::string("true");
        }
        else if (
            xmlPath.size() == 3 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kLogging && xmlPath[2] == XmlTagEnum::kRead)
        {
          response.Logging.Read = node.Value == std::string("true");
        }
        else if (
            xmlPath.size() == 3 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kLogging && xmlPath[2] == XmlTagEnum::kWrite)
        {
          response.Logging.Write = node.Value == std::string("true");
        }
        else if (
            xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kLogging && xmlPath[2] == XmlTagEnum::kRetentionPolicy
            && xmlPath[3] == XmlTagEnum::kEnabled)
        {
          response.Logging.RetentionPolicy.IsEnabled = node.Value == std::string("true");
        }
        else if (
            xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kLogging && xmlPath[2] == XmlTagEnum::kRetentionPolicy
            && xmlPath[3] == XmlTagEnum::kDays)
        {
          response.Logging.RetentionPolicy.Days = std::stoi(node.Value);
        }
        else if (
            xmlPath.size() == 3 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kHourMetrics && xmlPath[2] == XmlTagEnum::kVersion)
        {
          response.HourMetrics.Version = node.Value;
        }
        else if (
            xmlPath.size() == 3 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kHourMetrics && xmlPath[2] == XmlTagEnum::kEnabled)
        {
          response.HourMetrics.IsEnabled = node.Value == std::string("true");
        }
        else if (
            xmlPath.size() == 3 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kHourMetrics && xmlPath[2] == XmlTagEnum::kIncludeAPIs)
        {
          response.HourMetrics.IncludeApis = node.Value == std::string("true");
        }
        else if (
            xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kHourMetrics && xmlPath[2] == XmlTagEnum::kRetentionPolicy
            && xmlPath[3] == XmlTagEnum::kEnabled)
        {
          response.HourMetrics.RetentionPolicy.IsEnabled = node.Value == std::string("true");
        }
        else if (
            xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kHourMetrics && xmlPath[2] == XmlTagEnum::kRetentionPolicy
            && xmlPath[3] == XmlTagEnum::kDays)
        {
          response.HourMetrics.RetentionPolicy.Days = std::stoi(node.Value);
        }
        else if (
            xmlPath.size() == 3 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kMinuteMetrics && xmlPath[2] == XmlTagEnum::kVersion)
        {
          response.MinuteMetrics.Version = node.Value;
        }
        else if (
            xmlPath.size() == 3 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kMinuteMetrics && xmlPath[2] == XmlTagEnum::kEnabled)
        {
          response.MinuteMetrics.IsEnabled = node.Value == std::string("true");
        }
        else if (
            xmlPath.size() == 3 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kMinuteMetrics && xmlPath[2] == XmlTagEnum::kIncludeAPIs)
        {
          response.MinuteMetrics.IncludeApis = node.Value == std::string("true");
        }
        else if (
            xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kMinuteMetrics
            && xmlPath[2] == XmlTagEnum::kRetentionPolicy && xmlPath[3] == XmlTagEnum::kEnabled)
        {
          response.MinuteMetrics.RetentionPolicy.IsEnabled = node.Value == std::string("true");
        }
        else if (
            xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kMinuteMetrics
            && xmlPath[2] == XmlTagEnum::kRetentionPolicy && xmlPath[3] == XmlTagEnum::kDays)
        {
          response.MinuteMetrics.RetentionPolicy.Days = std::stoi(node.Value);
        }
        else if (
            xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kCors && xmlPath[2] == XmlTagEnum::kCorsRule
            && xmlPath[3] == XmlTagEnum::kAllowedOrigins)
        {
          vectorElement1.AllowedOrigins = node.Value;
        }
        else if (
            xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kCors && xmlPath[2] == XmlTagEnum::kCorsRule
            && xmlPath[3] == XmlTagEnum::kAllowedMethods)
        {
          vectorElement1.AllowedMethods = node.Value;
        }
        else if (
            xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kCors && xmlPath[2] == XmlTagEnum::kCorsRule
            && xmlPath[3] == XmlTagEnum::kAllowedHeaders)
        {
          vectorElement1.AllowedHeaders = node.Value;
        }
        else if (
            xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kCors && xmlPath[2] == XmlTagEnum::kCorsRule
            && xmlPath[3] == XmlTagEnum::kExposedHeaders)
        {
          vectorElement1.ExposedHeaders = node.Value;
        }
        else if (
            xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kCors && xmlPath[2] == XmlTagEnum::kCorsRule
            && xmlPath[3] == XmlTagEnum::kMaxAgeInSeconds)
        {
          vectorElement1.MaxAgeInSeconds = std::stoi(node.Value);
        }
      }
      else if (node.Type == _internal::XmlNodeType::Attribute)
      {
      }
      else if (node.Type == _internal::XmlNodeType::EndTag)
      {
        if (xmlPath.size() == 3 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kCors && xmlPath[2] == XmlTagEnum::kCorsRule)
        {
          response.Cors.push_back(std::move(vectorElement1));
          vectorElement1 = Models::CorsRule();
        }
        xmlPath.pop_back();
      }
    }
  }
  return Response<Models::TableServiceProperties>(std::move(response), std::move(pRawResponse));
}

Azure::Response<Models::ServiceStatistics> TableServicesClient::GetStatistics(
    Models::GetServiceStatisticsOptions const& options,
    const Core::Context& context)
{
  (void)options;
  auto url = m_url;
  std::string host = url.GetHost();
  std::string accountName = host.substr(0, host.find('.'));
  accountName += "-secondary";
  url.SetHost(accountName + "." + host.substr(host.find('.') + 1));
  url.SetQueryParameters({{"restype", "service"}, {"comp", "stats"}});
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
    _internal::XmlReader reader(
        reinterpret_cast<const char*>(responseBody.data()), responseBody.size());
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
      if (node.Type == _internal::XmlNodeType::End)
      {
        break;
      }
      else if (node.Type == _internal::XmlNodeType::StartTag)
      {
        auto ite = XmlTagEnumMap.find(node.Name);
        xmlPath.push_back(ite == XmlTagEnumMap.end() ? XmlTagEnum::kUnknown : ite->second);
      }
      else if (node.Type == _internal::XmlNodeType::Text)
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
      else if (node.Type == _internal::XmlNodeType::Attribute)
      {
      }
      else if (node.Type == _internal::XmlNodeType::EndTag)
      {

        xmlPath.pop_back();
      }
    }
  }
  return Response<Models::ServiceStatistics>(std::move(response), std::move(pRawResponse));
}

TableClient::TableClient(std::string subscriptionId)
    : m_pipeline(new Core::Http::_internal::HttpPipeline({}, "storage-tables", "", {}, {})),
      m_url("https://management.azure.com"), m_subscriptionId(std::move(subscriptionId))
{
}

Azure::Response<Models::Table> TableClient::Create(Core::Context const& context)
{
  auto url = m_url;
  url.AppendPath("Tables");

  std::string jsonBody;
  {
    auto jsonRoot = Core::Json::_internal::json::object();

    jsonRoot["TableName"] = m_tableName;
    jsonBody = jsonRoot.dump();
  }

  Core::IO::MemoryBodyStream requestBody(
      reinterpret_cast<std::uint8_t const*>(jsonBody.data()), jsonBody.length());

  Core::Http::Request request(Core::Http::HttpMethod::Post, url, &requestBody);

  request.SetHeader("Content-Type", "application/json");
  request.SetHeader("Content-Length", std::to_string(requestBody.Length()));
  request.SetHeader("Accept", "application/json;odata=fullmetadata");
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

      response.TableName = jsonRoot["TableName"].get<std::string>();
      response.EditLink = jsonRoot["odata.editLink"].get<std::string>();
      response.Id = jsonRoot["odata.id"].get<std::string>();
      response.Metadata = jsonRoot["odata.metadata"].get<std::string>();
      response.Type = jsonRoot["odata.type"].get<std::string>();
    }
  }

  return Response<Models::Table>(std::move(response), std::move(rawResponse));
}

void Models::ListTablesPagedResponse::OnNextPage(const Azure::Core::Context& context)
{
  m_operationOptions.ContinuationToken = NextPageToken;
  *this = m_tableServiceClient->ListTables(m_operationOptions, context);
}

Models::ListTablesPagedResponse TableServicesClient::ListTables(
    Models::ListTablesOptions const& options,
    Azure::Core::Context const& context) const
{
  auto url = m_url;
  url.AppendPath("Tables");
  Core::Http::Request request(Core::Http::HttpMethod::Get, url);
  request.SetHeader("Accept", "application/json;odata=fullmetadata");
  if (options.Prefix.HasValue())
  {
    request.GetUrl().AppendQueryParameter("If-Match", options.Prefix.Value());
  }
  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();

  if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  Models::ListTablesPagedResponse response;
  {
    auto const& responseBody = rawResponse->GetBody();
    std::string responseString = std::string(responseBody.begin(), responseBody.end());
    if (responseBody.size() > 0)
    {
      auto const jsonRoot
          = Core::Json::_internal::json::parse(responseBody.begin(), responseBody.end());
      std::string metadataLink = jsonRoot["odata.metadata"].get<std::string>();
      for (auto value : jsonRoot["value"])
      {
        Models::Table table;
        table.TableName = value["TableName"].get<std::string>();
        table.EditLink = value["odata.editLink"].get<std::string>();
        table.Id = value["odata.id"].get<std::string>();
        table.Type = value["odata.type"].get<std::string>();
        table.Metadata = metadataLink;
        response.Tables.emplace_back(std::move(table));
      }
    }

    response.ServiceEndpoint = url.GetAbsoluteUrl();
    response.Prefix = options.Prefix;
    response.m_tableServiceClient = std::make_shared<TableServicesClient>(*this);
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
    Models::SetTableAccessPolicyOptions const& options,
    Core::Context const& context)

{
  auto url = m_url;
  url.AppendPath(m_tableName);
  url.AppendQueryParameter("comp", "acl");
  (void)options;
  std::string xmlBody;
  {
    _internal::XmlWriter writer;
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "SignedIdentifiers"});
    for (const auto& i1 : tableAccessPolicy.SignedIdentifiers)
    {
      writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "SignedIdentifier"});
      writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Id", i1.Id});
      writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "AccessPolicy"});
      if (i1.StartsOn.HasValue())
      {
        writer.Write(_internal::XmlNode{
            _internal::XmlNodeType::StartTag,
            "Start",
            i1.StartsOn.Value().ToString(
                Azure::DateTime::DateFormat::Rfc3339,
                Azure::DateTime::TimeFractionFormat::AllDigits)});
      }
      if (i1.ExpiresOn.HasValue())
      {
        writer.Write(_internal::XmlNode{
            _internal::XmlNodeType::StartTag,
            "Expiry",
            i1.ExpiresOn.Value().ToString(
                Azure::DateTime::DateFormat::Rfc3339,
                Azure::DateTime::TimeFractionFormat::AllDigits)});
      }
      writer.Write(
          _internal::XmlNode{_internal::XmlNodeType::StartTag, "Permission", i1.Permissions});
      writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
      writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
    }
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
    writer.Write(_internal::XmlNode{_internal::XmlNodeType::End});
    xmlBody = writer.GetDocument();
  }
  Core::IO::MemoryBodyStream requestBody(
      reinterpret_cast<const uint8_t*>(xmlBody.data()), xmlBody.length());

  auto request = Core::Http::Request(Core::Http::HttpMethod::Put, url, &requestBody);
  request.SetHeader("Content-Type", "application/xml; charset=UTF-8");
  request.SetHeader("Content-Length", std::to_string(requestBody.Length()));
  request.GetUrl().AppendQueryParameter("comp", "acl");
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
    Models::GetTableAccessPolicyOptions const& options,
    Core::Context const& context)
{
  (void)options;
  auto url = m_url;
  url.AppendPath(m_tableName);
  url.AppendQueryParameter("comp", "acl");
  Core::Http::Request request(Core::Http::HttpMethod::Get, url);

  auto pRawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = pRawResponse->GetStatusCode();
  if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
  {
    throw Core::RequestFailedException(pRawResponse);
  }

  Models::TableAccessPolicy response{};
  {
    const auto& responseBody = pRawResponse->GetBody();
    _internal::XmlReader reader(
        reinterpret_cast<const char*>(responseBody.data()), responseBody.size());
    enum class XmlTagEnum
    {
      kUnknown,
      kSignedIdentifiers,
      kSignedIdentifier,
      kId,
      kAccessPolicy,
      kStart,
      kExpiry,
      kPermission,
    };
    const std::unordered_map<std::string, XmlTagEnum> XmlTagEnumMap{
        {"SignedIdentifiers", XmlTagEnum::kSignedIdentifiers},
        {"SignedIdentifier", XmlTagEnum::kSignedIdentifier},
        {"Id", XmlTagEnum::kId},
        {"AccessPolicy", XmlTagEnum::kAccessPolicy},
        {"Start", XmlTagEnum::kStart},
        {"Expiry", XmlTagEnum::kExpiry},
        {"Permission", XmlTagEnum::kPermission},
    };
    std::vector<XmlTagEnum> xmlPath;
    Models::SignedIdentifier vectorElement1;
    while (true)
    {
      auto node = reader.Read();
      if (node.Type == _internal::XmlNodeType::End)
      {
        break;
      }
      else if (node.Type == _internal::XmlNodeType::StartTag)
      {
        auto ite = XmlTagEnumMap.find(node.Name);
        xmlPath.push_back(ite == XmlTagEnumMap.end() ? XmlTagEnum::kUnknown : ite->second);
      }
      else if (node.Type == _internal::XmlNodeType::Text)
      {
        if (xmlPath.size() == 3 && xmlPath[0] == XmlTagEnum::kSignedIdentifiers
            && xmlPath[1] == XmlTagEnum::kSignedIdentifier && xmlPath[2] == XmlTagEnum::kId)
        {
          vectorElement1.Id = node.Value;
        }
        else if (
            xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kSignedIdentifiers
            && xmlPath[1] == XmlTagEnum::kSignedIdentifier
            && xmlPath[2] == XmlTagEnum::kAccessPolicy && xmlPath[3] == XmlTagEnum::kStart)
        {
          vectorElement1.StartsOn
              = DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc3339);
        }
        else if (
            xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kSignedIdentifiers
            && xmlPath[1] == XmlTagEnum::kSignedIdentifier
            && xmlPath[2] == XmlTagEnum::kAccessPolicy && xmlPath[3] == XmlTagEnum::kExpiry)
        {
          vectorElement1.ExpiresOn
              = DateTime::Parse(node.Value, Azure::DateTime::DateFormat::Rfc3339);
        }
        else if (
            xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kSignedIdentifiers
            && xmlPath[1] == XmlTagEnum::kSignedIdentifier
            && xmlPath[2] == XmlTagEnum::kAccessPolicy && xmlPath[3] == XmlTagEnum::kPermission)
        {
          vectorElement1.Permissions = node.Value;
        }
      }
      else if (node.Type == _internal::XmlNodeType::Attribute)
      {
      }
      else if (node.Type == _internal::XmlNodeType::EndTag)
      {
        if (xmlPath.size() == 2 && xmlPath[0] == XmlTagEnum::kSignedIdentifiers
            && xmlPath[1] == XmlTagEnum::kSignedIdentifier)
        {
          response.SignedIdentifiers.push_back(std::move(vectorElement1));
          vectorElement1 = Models::SignedIdentifier();
        }
        xmlPath.pop_back();
      }
    }
  }

  return Response<Models::TableAccessPolicy>(std::move(response), std::move(pRawResponse));
}

Azure::Response<Models::DeleteResult> TableClient::Delete(

    Core::Context const& context)
{
  auto url = m_url;
  url.AppendPath("Tables('" + m_tableName + "')");

  Core::Http::Request request(Core::Http::HttpMethod::Delete, url);

  request.SetHeader("Content-Type", "application/json");
  request.SetHeader("Accept", "application/json;odata=fullmetadata");
  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();

  if (httpStatusCode != Core::Http::HttpStatusCode::NoContent)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  Models::DeleteResult response{};

  return Response<Models::DeleteResult>(std::move(response), std::move(rawResponse));
}

Azure::Response<Models::CreateEntityResult> TableClient::CreateEntity(
    Models::TableEntity const& tableEntity,
    Models::CreateEntityOptions const& options,
    Core::Context const& context)
{
  (void)options;
  auto url = m_url;
  url.AppendPath(m_tableName);

  std::string jsonBody;
  {
    auto jsonRoot = Core::Json::_internal::json::object();

    jsonRoot["PartitionKey"] = tableEntity.PartitionKey;
    jsonRoot["RowKey"] = tableEntity.RowKey;
    for (auto entry : tableEntity.Properties)
    {
      jsonRoot[entry.first] = entry.second;
    }

    jsonBody = jsonRoot.dump();
  }

  Core::IO::MemoryBodyStream requestBody(
      reinterpret_cast<std::uint8_t const*>(jsonBody.data()), jsonBody.length());

  Core::Http::Request request(Core::Http::HttpMethod::Post, url, &requestBody);

  request.SetHeader("Content-Type", "application/json");
  request.SetHeader("Content-Length", std::to_string(requestBody.Length()));
  request.SetHeader("Accept", "application/json;odata=nometadata");
  request.SetHeader("Prefer", "return-no-content");
  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();
  if (httpStatusCode != Core::Http::HttpStatusCode::NoContent)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  Models::CreateEntityResult response{};
  response.ETag = rawResponse->GetHeaders().at("ETag");
  return Response<Models::CreateEntityResult>(std::move(response), std::move(rawResponse));
}

Azure::Response<Models::UpdateEntityResult> TableClient::UpdateEntity(
    Models::TableEntity const& tableEntity,
    Models::UpdateEntityOptions const& options,
    Core::Context const& context)
{
  (void)options;
  auto url = m_url;
  url.AppendPath(
      m_tableName + 
      "(PartitionKey='" + 
      tableEntity.PartitionKey + 
      "',RowKey='" + 
      tableEntity.RowKey + 
      "')");

  std::string jsonBody;
  {
    auto jsonRoot = Core::Json::_internal::json::object();

    jsonRoot["PartitionKey"] = tableEntity.PartitionKey;
    jsonRoot["RowKey"] = tableEntity.RowKey;
    for (auto entry : tableEntity.Properties)
    {
      jsonRoot[entry.first] = entry.second;
    }

    jsonBody = jsonRoot.dump();
  }

  Core::IO::MemoryBodyStream requestBody(
      reinterpret_cast<std::uint8_t const*>(jsonBody.data()), jsonBody.length());

  Core::Http::Request request(Core::Http::HttpMethod::Put, url, &requestBody);

  request.SetHeader("Content-Type", "application/json");
  request.SetHeader("Content-Length", std::to_string(requestBody.Length()));
  request.SetHeader("Accept", "application/json;odata=nometadata");
  request.SetHeader("Prefer", "return-no-content");
  if (tableEntity.ETag.HasValue())
  {
    request.SetHeader("If-Match", tableEntity.ETag.Value());
  }
  else
  {
    request.SetHeader("If-Match", "*");
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
      m_tableName + "(PartitionKey='" + tableEntity.PartitionKey + "',RowKey='" + tableEntity.RowKey
      + "')");

  std::string jsonBody;
  {
    auto jsonRoot = Core::Json::_internal::json::object();

    jsonRoot["PartitionKey"] = tableEntity.PartitionKey;
    jsonRoot["RowKey"] = tableEntity.RowKey;
    for (auto entry : tableEntity.Properties)
    {
      jsonRoot[entry.first] = entry.second;
    }

    jsonBody = jsonRoot.dump();
  }

  Core::IO::MemoryBodyStream requestBody(
      reinterpret_cast<std::uint8_t const*>(jsonBody.data()), jsonBody.length());

  Core::Http::Request request(Core::Http::HttpMethod::Patch, url, &requestBody);

  request.SetHeader("Content-Type", "application/json");
  request.SetHeader("Content-Length", std::to_string(requestBody.Length()));
  request.SetHeader("Accept", "application/json;odata=nometadata");
  request.SetHeader("Prefer", "return-no-content");
  if (tableEntity.ETag.HasValue())
  {
    request.SetHeader("If-Match", tableEntity.ETag.Value());
  }
  else
  {
    request.SetHeader("If-Match", "*");
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
    Models::DeleteEntityOptions const& options,
    Core::Context const& context)
{
  (void)options;
  auto url = m_url;
  url.AppendPath(
      m_tableName + "(PartitionKey='" + tableEntity.PartitionKey + "',RowKey='" + tableEntity.RowKey
      + "')");

    Core::Http::Request request(Core::Http::HttpMethod::Delete, url);

  if (tableEntity.ETag.HasValue())
  {
    request.SetHeader("If-Match", tableEntity.ETag.Value());
  }
  else
  {
    request.SetHeader("If-Match", "*");
  }
  request.SetHeader("Accept", "application/json;odata=nometadata");
  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();
  if (httpStatusCode != Core::Http::HttpStatusCode::NoContent)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  Models::DeleteEntityResult response{};
  return Response<Models::DeleteEntityResult>(std::move(response), std::move(rawResponse));
}