// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// Code generated by Microsoft (R) AutoRest Code Generator.
// Changes may cause incorrect behavior and will be lost if the code is regenerated.

#include "azure/storage/tables/rest_client.hpp"

#include <azure/core/exception.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/http_status_code.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/io/body_stream.hpp>
#include <azure/core/internal/environment.hpp>
using namespace Azure::Storage::Tables;

AllowedMethods const AllowedMethods::Delete{"DELETE"};
AllowedMethods const AllowedMethods::Get{"GET"};
AllowedMethods const AllowedMethods::Head{"HEAD"};
AllowedMethods const AllowedMethods::Merge{"MERGE"};
AllowedMethods const AllowedMethods::Post{"POST"};
AllowedMethods const AllowedMethods::Options{"OPTIONS"};
AllowedMethods const AllowedMethods::Put{"PUT"};
AllowedMethods const AllowedMethods::Patch{"PATCH"};
AllowedMethods const AllowedMethods::Connect{"CONNECT"};
AllowedMethods const AllowedMethods::Trace{"TRACE"};
/*
TableServicesClient::TableServicesClient(
    std::string subscriptionId,
    const TableClientOptions& options)
    : m_pipeline(new Core::Http::_internal::HttpPipeline({}, "storage-tables", "", {}, {})),
      m_url("https://management.azure.com"), m_subscriptionId(std::move(subscriptionId))
{
  (void)options;
}
*/
const TablesAudience TablesAudience::PublicAudience(Azure::Storage::_internal::TablesManagementScope);
Azure::Response<ListTableServices> TableServicesClient::List(
    ListOptions const& options,
    Core::Context const& context)
{
  auto url = m_url;
  url.AppendPath("subscriptions/");
  url.AppendPath(!m_subscriptionId.empty() ? Core::Url::Encode(m_subscriptionId) : "null");
  url.AppendPath("resourceGroups/");
  url.AppendPath(
      !options.ResourceGroupName.empty() ? Core::Url::Encode(options.ResourceGroupName) : "null");
  url.AppendPath("providers/Microsoft.Storage/storageAccounts/");
  url.AppendPath(!options.AccountName.empty() ? Core::Url::Encode(options.AccountName) : "null");
  url.AppendPath("tableServices");

  url.SetQueryParameters({{"api-version", "2023-01-01"}});

  Core::Http::Request request(Core::Http::HttpMethod::Get, url);

  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();

  if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  ListTableServices response{};
  {
    auto const& responseBody = rawResponse->GetBody();
    if (responseBody.size() > 0)
    {
      auto const jsonRoot
          = Core::Json::_internal::json::parse(responseBody.begin(), responseBody.end());

      for (auto const& jsonItem : jsonRoot["value"])
      {
        TableServiceProperties vectorRootItem{};

        for (auto const& jsonRulesItem : jsonItem["properties"]["cors"]["corsRules"])
        {
          CorsRule vectorItem{};

          for (auto const& jsonSubItem : jsonRulesItem["allowedOrigins"])
          {
            std::string allowedOrigins{};

            allowedOrigins = jsonSubItem.get<std::string>();

            vectorItem.AllowedOrigins.emplace_back(std::move(allowedOrigins));
          }

          for (auto const& jsonSubItem : jsonRulesItem["allowedMethods"])
          {
            AllowedMethods allowedMethods{};

            allowedMethods = AllowedMethods(jsonSubItem.get<std::string>());

           vectorItem.AllowedMethods.emplace_back(std::move(allowedMethods));
          }

          vectorItem.MaxAgeInSeconds = jsonRulesItem["maxAgeInSeconds"].is_string()
              ? std::stoi(jsonRulesItem["maxAgeInSeconds"].get<std::string>())
              : jsonRulesItem["maxAgeInSeconds"].get<std::int32_t>();

          for (auto const& jsonSubItem : jsonRulesItem["exposedHeaders"])
          {
           std::string exposedHeaders{};

            exposedHeaders = jsonSubItem.get<std::string>();

                vectorItem.ExposedHeaders.emplace_back(std::move(exposedHeaders));
          }

          for (auto const& jsonSubItem : jsonRulesItem["allowedHeaders"])
          {
                std::string allowedHeaders{};

            allowedHeaders = jsonSubItem.get<std::string>();

                vectorItem.AllowedHeaders.emplace_back(std::move(allowedHeaders));
          }

           vectorRootItem.Properties.Cors.CorsRules.emplace_back(std::move(vectorItem));
        }

        response.Value.emplace_back(std::move(vectorRootItem));
      }
    }
  }

  return Response<ListTableServices>(std::move(response), std::move(rawResponse));
}

Azure::Response<TableServiceProperties> TableServicesClient::SetServiceProperties(
    SetServicePropertiesOptions const& options,
    Core::Context const& context)
{
  auto url = m_url;
  url.AppendPath("subscriptions/");
  
  url.AppendPath(!m_subscriptionId.empty() ? Core::Url::Encode(m_subscriptionId) : "null");
  url.AppendPath("resourceGroups/");
  url.AppendPath(
      !options.ResourceGroupName.empty() ? Core::Url::Encode(options.ResourceGroupName) : "null");
  url.AppendPath("providers/Microsoft.Storage/storageAccounts/");
  url.AppendPath(!options.AccountName.empty() ? Core::Url::Encode(options.AccountName) : "null");
  url.AppendPath("tableServices/default");

  url.SetQueryParameters({{"api-version", "2023-01-01"}});

  std::string jsonBody;
  {
    auto jsonRoot = Core::Json::_internal::json::object();
    jsonRoot["properties"]["cors"]["corsRules"] = Core::Json::_internal::json::array();

    for (auto i = 0; i < options.Parameters.Properties.Cors.CorsRules.size(); ++i)
    {
      jsonRoot["properties"]["cors"]["corsRules"][i]["allowedOrigins"]
          = Core::Json::_internal::json::array();

      for (auto j = 0; j < options.Parameters.Properties.Cors.CorsRules[i].AllowedOrigins.size();
           j++)
      {
        jsonRoot["properties"]["cors"]["corsRules"][i]["allowedOrigins"][j]
            = options.Parameters.Properties.Cors.CorsRules[i].AllowedOrigins[j];
      }

      jsonRoot["properties"]["cors"]["corsRules"][i]["allowedMethods"]
          = Core::Json::_internal::json::array();

      for (auto k = 0; k < options.Parameters.Properties.Cors.CorsRules[i].AllowedMethods.size();
           k++)
      {
        jsonRoot["properties"]["cors"]["corsRules"][i]["allowedMethods"][k]
            = options.Parameters.Properties.Cors.CorsRules[i].AllowedMethods[k].ToString();
      }

      jsonRoot["properties"]["cors"]["corsRules"][i]["maxAgeInSeconds"]
          = std::to_string(options.Parameters.Properties.Cors.CorsRules[i].MaxAgeInSeconds);
      jsonRoot["properties"]["cors"]["corsRules"][i]["exposedHeaders"]
          = Core::Json::_internal::json::array();

      for (auto l = 0; l < options.Parameters.Properties.Cors.CorsRules[i].ExposedHeaders.size();
           l++)
      {
        jsonRoot["properties"]["cors"]["corsRules"][i]["exposedHeaders"][l]
            = options.Parameters.Properties.Cors.CorsRules[i].ExposedHeaders[l];
      }

      jsonRoot["properties"]["cors"]["corsRules"][i]["allowedHeaders"]
          = Core::Json::_internal::json::array();

      for (auto m = 0; m < options.Parameters.Properties.Cors.CorsRules[i].AllowedHeaders.size();
           m++)
      {
        jsonRoot["properties"]["cors"]["corsRules"][i]["allowedHeaders"][m]
            = options.Parameters.Properties.Cors.CorsRules[i].AllowedHeaders[m];
      }
    }

    jsonBody = jsonRoot.dump();
  }

  Core::IO::MemoryBodyStream requestBody(
      reinterpret_cast<std::uint8_t const*>(jsonBody.data()), jsonBody.length());

  Core::Http::Request request(Core::Http::HttpMethod::Put, url, &requestBody);

  request.SetHeader("Content-Type", "application/json");
  request.SetHeader("Content-Length", std::to_string(requestBody.Length()));

  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();

  if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  TableServiceProperties response{};
  { 
    auto const& responseBody = rawResponse->GetBody();
    std::string responseString (responseBody.begin(), responseBody.end());
    if (responseBody.size() > 0)
    {
      auto const jsonRoot
          = Core::Json::_internal::json::parse(responseBody.begin(), responseBody.end());

      for (auto const& jsonItem : jsonRoot["properties"]["cors"]["corsRules"])
      {
        CorsRule vectorItem{};

        for (auto const& jsonSubItem : jsonItem["allowedOrigins"])
        {
          std::string origins{};

          origins = jsonSubItem.get<std::string>();

          vectorItem.AllowedOrigins.emplace_back(std::move(origins));
        }

        for (auto const& jsonSubItem : jsonItem["allowedMethods"])
        {
          AllowedMethods methods{};

          methods = AllowedMethods(jsonSubItem.get<std::string>());

          vectorItem.AllowedMethods.emplace_back(std::move(methods));
        }

        vectorItem.MaxAgeInSeconds = jsonItem["maxAgeInSeconds"].is_string()
            ? std::stoi(jsonItem["maxAgeInSeconds"].get<std::string>())
            : jsonItem["maxAgeInSeconds"].get<std::int32_t>();

        for (auto const& jsonSubItem : jsonItem["exposedHeaders"])
        {
          std::string headers{};

          headers = jsonSubItem.get<std::string>();

          vectorItem.ExposedHeaders.emplace_back(std::move(headers));
        }

        for (auto const& jsonSubItem : jsonItem["allowedHeaders"])
        {
          std::string allowedHeaders{};

          allowedHeaders = jsonSubItem.get<std::string>();

          vectorItem.AllowedHeaders.emplace_back(std::move(allowedHeaders));
        }

        response.Properties.Cors.CorsRules.emplace_back(std::move(vectorItem));
      }
    } 
  }

  return Response<TableServiceProperties>(std::move(response), std::move(rawResponse));
}

Azure::Response<TableServiceProperties> TableServicesClient::GetServiceProperties(
    GetServicePropertiesOptions const& options,
    Core::Context const& context)
{
  (void)options;
  auto url = m_url;
  url.AppendPath("subscriptions/");
  url.AppendPath(!m_subscriptionId.empty() ? Core::Url::Encode(m_subscriptionId) : "null");
  url.AppendPath("resourceGroups/");
  url.AppendPath( !options.ResourceGroupName.empty() ? Core::Url::Encode(options.ResourceGroupName) : "null");
  url.AppendPath("providers/Microsoft.Storage/storageAccounts/");
  url.AppendPath(!options.AccountName.empty() ? Core::Url::Encode(options.AccountName) : "null");
  url.AppendPath("tableServices/default");

  url.SetQueryParameters({{"api-version", "2023-01-01"}});

  Core::Http::Request request(Core::Http::HttpMethod::Get, url);

  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();

  if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  TableServiceProperties response{};
  {
     auto const& responseBody = rawResponse->GetBody();
    
     if (responseBody.size() > 0)
    {
      auto const jsonRoot
          = Core::Json::_internal::json::parse(responseBody.begin(), responseBody.end());

      for (auto const& jsonItem : jsonRoot["properties"]["cors"]["corsRules"])
      {
        CorsRule vectorItem{};

        for (auto const& jsonSubItem : jsonItem["allowedOrigins"])
        {
          std::string allowedOrigins{};

          allowedOrigins = jsonSubItem.get<std::string>();

          vectorItem.AllowedOrigins.emplace_back(std::move(allowedOrigins));
        }

        for (auto const& jsonSubItem : jsonItem["allowedMethods"])
        {
          AllowedMethods allowedMethods{};

          allowedMethods = AllowedMethods(jsonSubItem.get<std::string>());

          vectorItem.AllowedMethods.emplace_back(std::move(allowedMethods));
        }

        vectorItem.MaxAgeInSeconds = jsonItem["maxAgeInSeconds"].is_string()
            ? std::stoi(jsonItem["maxAgeInSeconds"].get<std::string>())
            : jsonItem["maxAgeInSeconds"].get<std::int32_t>();

        for (auto const& jsonSubItem : jsonItem["exposedHeaders"])
        {
          std::string exposedHeaders{};

          exposedHeaders = jsonSubItem.get<std::string>();

          vectorItem.ExposedHeaders.emplace_back(std::move(exposedHeaders));
        }

        for (auto const& jsonSubItem : jsonItem["allowedHeaders"])
        {
          std::string allowedHeaders{};

          allowedHeaders = jsonSubItem.get<std::string>();

          vectorItem.AllowedHeaders.emplace_back(std::move(allowedHeaders));
        }

        response.Properties.Cors.CorsRules.emplace_back(std::move(vectorItem));
      }
    }
  }

    return Response<TableServiceProperties>(std::move(response), std::move(rawResponse));
}

TableClient::TableClient(std::string subscriptionId)
    : m_pipeline(new Core::Http::_internal::HttpPipeline({}, "storage-tables", "", {}, {})),
      m_url("https://management.azure.com"), m_subscriptionId(std::move(subscriptionId))
{
}

Azure::Response<Table> TableClient::Create(
    CreateOptions const& options,
    Core::Context const& context)
{
    auto url = m_url;
    url.AppendPath("subscriptions/");
    url.AppendPath(!m_subscriptionId.empty() ? Core::Url::Encode(m_subscriptionId) : "null");
    url.AppendPath("resourceGroups/");
    url.AppendPath(
        !options.ResourceGroupName.empty() ? Core::Url::Encode(options.ResourceGroupName) : "null");
    url.AppendPath("providers/Microsoft.Storage/storageAccounts/");
    url.AppendPath(!options.AccountName.empty() ? Core::Url::Encode(options.AccountName) : "null");
    url.AppendPath("tableServices/default/tables/");
    url.AppendPath(!options.TableName.empty() ? Core::Url::Encode(options.TableName) : "null");

    url.SetQueryParameters({{"api-version", "2023-01-01"}});

    std::string jsonBody;
    {
      auto jsonRoot = Core::Json::_internal::json::object();

      jsonRoot["properties"]["tableName"] = options.Parameters.Properties.TableName;
      jsonRoot["properties"]["signedIdentifiers"] = Core::Json::_internal::json::array();

      for (auto i = 0; i < options.Parameters.Properties.SignedIdentifiers.size(); ++i)
      {
        jsonRoot["properties"]["signedIdentifiers"][i]["id"]
            = options.Parameters.Properties.SignedIdentifiers[i].Id;

        if (options.Parameters.Properties.SignedIdentifiers[i].AccessPolicy.StartTime.HasValue())
        {
          jsonRoot["properties"]["signedIdentifiers"][i]["accessPolicy"]["startTime"]
              = options.Parameters.Properties.SignedIdentifiers[i]
                    .AccessPolicy.StartTime.Value()
                    .ToString();
        }

        if (options.Parameters.Properties.SignedIdentifiers[i].AccessPolicy.ExpiryTime.HasValue())
        {
          jsonRoot["properties"]["signedIdentifiers"][i]["accessPolicy"]["expiryTime"]
              = options.Parameters.Properties.SignedIdentifiers[i]
                    .AccessPolicy.ExpiryTime.Value()
                    .ToString();
        }

        jsonRoot["properties"]["signedIdentifiers"][i]["accessPolicy"]["permission"]
            = options.Parameters.Properties.SignedIdentifiers[i].AccessPolicy.Permission;
      }

      jsonBody = jsonRoot.dump();
    }

    Core::IO::MemoryBodyStream requestBody(
        reinterpret_cast<std::uint8_t const*>(jsonBody.data()), jsonBody.length());

    Core::Http::Request request(Core::Http::HttpMethod::Put, url, &requestBody);

    request.SetHeader("Content-Type", "application/json");
    request.SetHeader("Content-Length", std::to_string(requestBody.Length()));

    auto rawResponse = m_pipeline->Send(request, context);
    auto const httpStatusCode = rawResponse->GetStatusCode();

    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw Core::RequestFailedException(rawResponse);
    }

    Table response{};
    {
      auto const& responseBody = rawResponse->GetBody();
      if (responseBody.size() > 0)
      {
        auto const jsonRoot
            = Core::Json::_internal::json::parse(responseBody.begin(), responseBody.end());

        response.Properties.TableName = jsonRoot["tableName"].get<std::string>();

        for (auto const& jsonItem : jsonRoot["signedIdentifiers"])
        {
          TableSignedIdentifier vectorItem{};

          vectorItem.Id = jsonItem["id"].get<std::string>();

          if (jsonItem.contains("startTime"))
          {
            vectorItem.AccessPolicy.StartTime = DateTime::Parse(
                jsonItem["startTime"].get<std::string>(), DateTime::DateFormat::Rfc3339);
          }

          if (jsonItem.contains("expiryTime"))
          {
            vectorItem.AccessPolicy.ExpiryTime = DateTime::Parse(
                jsonItem["expiryTime"].get<std::string>(), DateTime::DateFormat::Rfc3339);
          }

          vectorItem.AccessPolicy.Permission = jsonItem["permission"].get<std::string>();

          response.Properties.SignedIdentifiers.emplace_back(std::move(vectorItem));
        }
      }
    }

    return Response<Table>(std::move(response), std::move(rawResponse));
}

Azure::Response<Table> TableClient::Update(
    UpdateOptions const& options,
    Core::Context const& context)
{
    auto url = m_url;
    url.AppendPath("subscriptions/");
    url.AppendPath(!m_subscriptionId.empty() ? Core::Url::Encode(m_subscriptionId) : "null");
    url.AppendPath("resourceGroups/");
    url.AppendPath(
        !options.ResourceGroupName.empty() ? Core::Url::Encode(options.ResourceGroupName) : "null");
    url.AppendPath("providers/Microsoft.Storage/storageAccounts/");
    url.AppendPath(!options.AccountName.empty() ? Core::Url::Encode(options.AccountName) : "null");
    url.AppendPath("tableServices/default/tables/");
    url.AppendPath(!options.TableName.empty() ? Core::Url::Encode(options.TableName) : "null");

    url.SetQueryParameters({{"api-version", "2023-01-01"}});

    std::string jsonBody;
    {
      auto jsonRoot = Core::Json::_internal::json::object();

      jsonRoot["properties"]["tableName"] = options.Parameters.Properties.TableName;
      jsonRoot["properties"]["signedIdentifiers"] = Core::Json::_internal::json::array();

      for (auto i = 0; i < options.Parameters.Properties.SignedIdentifiers.size(); ++i)
      {
        jsonRoot["properties"]["signedIdentifiers"][i]["id"]
            = options.Parameters.Properties.SignedIdentifiers[i].Id;

        if (options.Parameters.Properties.SignedIdentifiers[i].AccessPolicy.StartTime.HasValue())
        {
          jsonRoot["properties"]["signedIdentifiers"][i]["accessPolicy"]["startTime"]
              = options.Parameters.Properties.SignedIdentifiers[i]
                    .AccessPolicy.StartTime.Value()
                    .ToString();
        }

        if (options.Parameters.Properties.SignedIdentifiers[i].AccessPolicy.ExpiryTime.HasValue())
        {
          jsonRoot["properties"]["signedIdentifiers"][i]["accessPolicy"]["expiryTime"]
              = options.Parameters.Properties.SignedIdentifiers[i]
                    .AccessPolicy.ExpiryTime.Value()
                    .ToString();
        }

        jsonRoot["properties"]["signedIdentifiers"][i]["accessPolicy"]["permission"]
            = options.Parameters.Properties.SignedIdentifiers[i].AccessPolicy.Permission;
      }

      jsonBody = jsonRoot.dump();
    }

    Core::IO::MemoryBodyStream requestBody(
        reinterpret_cast<std::uint8_t const*>(jsonBody.data()), jsonBody.length());

    Core::Http::Request request(Core::Http::HttpMethod::Patch, url, &requestBody);

    request.SetHeader("Content-Type", "application/json");
    request.SetHeader("Content-Length", std::to_string(requestBody.Length()));

    auto rawResponse = m_pipeline->Send(request, context);
    auto const httpStatusCode = rawResponse->GetStatusCode();

    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw Core::RequestFailedException(rawResponse);
    }

    Table response{};
    {
      auto const& responseBody = rawResponse->GetBody();
      if (responseBody.size() > 0)
      {
        auto const jsonRoot
            = Core::Json::_internal::json::parse(responseBody.begin(), responseBody.end());

        response.Properties.TableName = jsonRoot["tableName"].get<std::string>();

        for (auto const& jsonItem : jsonRoot["signedIdentifiers"])
        {
          TableSignedIdentifier vectorItem{};

          vectorItem.Id = jsonItem["id"].get<std::string>();

          if (jsonItem.contains("startTime"))
          {
            vectorItem.AccessPolicy.StartTime = DateTime::Parse(
                jsonItem["startTime"].get<std::string>(), DateTime::DateFormat::Rfc3339);
          }

          if (jsonItem.contains("expiryTime"))
          {
            vectorItem.AccessPolicy.ExpiryTime = DateTime::Parse(
                jsonItem["expiryTime"].get<std::string>(), DateTime::DateFormat::Rfc3339);
          }

          vectorItem.AccessPolicy.Permission = jsonItem["permission"].get<std::string>();

          response.Properties.SignedIdentifiers.emplace_back(std::move(vectorItem));
        }
      }
    }

    return Response<Table>(std::move(response), std::move(rawResponse));
}

Azure::Response<Table> TableClient::Get(GetOptions const& options, Core::Context const& context)
{
    auto url = m_url;
    url.AppendPath("subscriptions/");
    url.AppendPath(!m_subscriptionId.empty() ? Core::Url::Encode(m_subscriptionId) : "null");
    url.AppendPath("resourceGroups/");
    url.AppendPath(
        !options.ResourceGroupName.empty() ? Core::Url::Encode(options.ResourceGroupName) : "null");
    url.AppendPath("providers/Microsoft.Storage/storageAccounts/");
    url.AppendPath(!options.AccountName.empty() ? Core::Url::Encode(options.AccountName) : "null");
    url.AppendPath("tableServices/default/tables/");
    url.AppendPath(!options.TableName.empty() ? Core::Url::Encode(options.TableName) : "null");

    url.SetQueryParameters({{"api-version", "2023-01-01"}});

    Core::Http::Request request(Core::Http::HttpMethod::Get, url);

    auto rawResponse = m_pipeline->Send(request, context);
    auto const httpStatusCode = rawResponse->GetStatusCode();

    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw Core::RequestFailedException(rawResponse);
    }

    Table response{};
    {
      auto const& responseBody = rawResponse->GetBody();
      if (responseBody.size() > 0)
      {
        auto const jsonRoot
            = Core::Json::_internal::json::parse(responseBody.begin(), responseBody.end());

        response.Properties.TableName = jsonRoot["tableName"].get<std::string>();

        for (auto const& jsonItem : jsonRoot["signedIdentifiers"])
        {
          TableSignedIdentifier vectorItem{};

          vectorItem.Id = jsonItem["id"].get<std::string>();

          if (jsonItem.contains("startTime"))
          {
            vectorItem.AccessPolicy.StartTime = DateTime::Parse(
                jsonItem["startTime"].get<std::string>(), DateTime::DateFormat::Rfc3339);
          }

          if (jsonItem.contains("expiryTime"))
          {
            vectorItem.AccessPolicy.ExpiryTime = DateTime::Parse(
                jsonItem["expiryTime"].get<std::string>(), DateTime::DateFormat::Rfc3339);
          }

          vectorItem.AccessPolicy.Permission = jsonItem["permission"].get<std::string>();

          response.Properties.SignedIdentifiers.emplace_back(std::move(vectorItem));
        }
      }
    }

    return Response<Table>(std::move(response), std::move(rawResponse));
}

Azure::Response<DeleteResult> TableClient::Delete(
    DeleteOptions const& options,
    Core::Context const& context)
{
    auto url = m_url;
    url.AppendPath("subscriptions/");
    url.AppendPath(!m_subscriptionId.empty() ? Core::Url::Encode(m_subscriptionId) : "null");
    url.AppendPath("resourceGroups/");
    url.AppendPath(
        !options.ResourceGroupName.empty() ? Core::Url::Encode(options.ResourceGroupName) : "null");
    url.AppendPath("providers/Microsoft.Storage/storageAccounts/");
    url.AppendPath(!options.AccountName.empty() ? Core::Url::Encode(options.AccountName) : "null");
    url.AppendPath("tableServices/default/tables/");
    url.AppendPath(!options.TableName.empty() ? Core::Url::Encode(options.TableName) : "null");

    url.SetQueryParameters({{"api-version", "2023-01-01"}});

    Core::Http::Request request(Core::Http::HttpMethod::Delete, url);

    auto rawResponse = m_pipeline->Send(request, context);
    auto const httpStatusCode = rawResponse->GetStatusCode();

    if (httpStatusCode != Core::Http::HttpStatusCode::NoContent)
    {
      throw Core::RequestFailedException(rawResponse);
    }

    DeleteResult response{};

    return Response<DeleteResult>(std::move(response), std::move(rawResponse));
}

Azure::Response<ListTableResource> TableClient::List(
    ListOptions const& options,
    Core::Context const& context)
{
    auto url = m_url;
    url.AppendPath("subscriptions/");
    url.AppendPath(!m_subscriptionId.empty() ? Core::Url::Encode(m_subscriptionId) : "null");
    url.AppendPath("resourceGroups/");
    url.AppendPath(
        !options.ResourceGroupName.empty() ? Core::Url::Encode(options.ResourceGroupName) : "null");
    url.AppendPath("providers/Microsoft.Storage/storageAccounts/");
    url.AppendPath(!options.AccountName.empty() ? Core::Url::Encode(options.AccountName) : "null");
    url.AppendPath("tableServices/default/tables");

    url.SetQueryParameters({{"api-version", "2023-01-01"}});

    Core::Http::Request request(Core::Http::HttpMethod::Get, url);

    auto rawResponse = m_pipeline->Send(request, context);
    auto const httpStatusCode = rawResponse->GetStatusCode();

    if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
    {
      throw Core::RequestFailedException(rawResponse);
    }

    ListTableResource response{};
    {
    // auto const& responseBody = rawResponse->GetBody();
    /* if (responseBody.size() > 0)
    {
      auto const jsonRoot
          = Core::Json::_internal::json::parse(responseBody.begin(), responseBody.end());

      for (auto const& jsonItem : jsonRoot["value"])
      {
        Table vectorItem{};

        vectorItem.Properties.TableName = jsonItem["tableName"].get<std::string>();

        for (auto const& jsonItem : jsonItem["signedIdentifiers"])
        {
          TableSignedIdentifier vectorItem{};

          vectorItem.Id = jsonItem["id"].get<std::string>();

          if (jsonItem.contains("startTime"))
          {
            vectorItem.AccessPolicy.StartTime = DateTime::Parse(
                jsonItem["startTime"].get<std::string>(), DateTime::DateFormat::Rfc3339);
          }

          if (jsonItem.contains("expiryTime"))
          {
            vectorItem.AccessPolicy.ExpiryTime = DateTime::Parse(
                jsonItem["expiryTime"].get<std::string>(), DateTime::DateFormat::Rfc3339);
          }

          vectorItem.AccessPolicy.Permission = jsonItem["permission"].get<std::string>();

          vectorItem.Properties.SignedIdentifiers.emplace_back(std::move(vectorItem));
        }

        response.Value.emplace_back(std::move(vectorItem));
      }

      response.NextLink = jsonRoot["nextLink"].get<std::string>();
    }
 */ }

    return Response<ListTableResource>(std::move(response), std::move(rawResponse));
}
