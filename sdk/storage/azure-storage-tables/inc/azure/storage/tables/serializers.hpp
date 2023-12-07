// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/response.hpp>
#include <azure/core/url.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/shared_key_policy_lite.hpp>
#include <azure/storage/common/internal/storage_bearer_token_auth.hpp>
#include <azure/storage/common/internal/storage_per_retry_policy.hpp>
#include <azure/storage/common/internal/storage_service_version_policy.hpp>
#include <azure/storage/common/internal/storage_switch_to_secondary_policy.hpp>
#include <azure/storage/common/internal/xml_wrapper.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_credential.hpp>
#include <azure/storage/tables/dll_import_export.hpp>
#include <azure/storage/tables/models.hpp>
#include <azure/storage/tables/rest_client.hpp>
#include <azure/storage/tables/rtti.hpp>
#include <azure/storage/tables/transactions.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Azure { namespace Storage { namespace Tables {
  class Serializers final {
  public:
    static std::string const CreateEntity(Models::TableEntity const& tableEntity)
    {
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
      return jsonBody;
    }

    static std::string const MergeEntity(Models::TableEntity const& tableEntity)
    {
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
      return jsonBody;
    }

    static std::string const UpdateEntity(Models::TableEntity const& tableEntity)
    {
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
      return jsonBody;
    }

    static std::string const SetAccessPolicy(Models::TableAccessPolicy const& tableAccessPolicy)
    {
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
      return xmlBody;
    }

    static std::string const Create(std::string const& tableName)
    {
      std::string jsonBody;
      {
        auto jsonRoot = Core::Json::_internal::json::object();

        jsonRoot["TableName"] = tableName;
        jsonBody = jsonRoot.dump();
      }
      return jsonBody;
    }

    static std::string const SetServiceProperties(
        Models::SetServicePropertiesOptions const& options)
    {
      std::string xmlBody;
      {
        _internal::XmlWriter writer;
        writer.Write(
            _internal::XmlNode{_internal::XmlNodeType::StartTag, "StorageServiceProperties"});
        writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Logging"});
        writer.Write(_internal::XmlNode{
            _internal::XmlNodeType::StartTag,
            "Version",
            options.ServiceProperties.Logging.Version});
        writer.Write(_internal::XmlNode{
            _internal::XmlNodeType::StartTag,
            "Delete",
            options.ServiceProperties.Logging.Delete ? "true" : "false"});
        writer.Write(_internal::XmlNode{
            _internal::XmlNodeType::StartTag,
            "Read",
            options.ServiceProperties.Logging.Read ? "true" : "false"});
        writer.Write(_internal::XmlNode{
            _internal::XmlNodeType::StartTag,
            "Write",
            options.ServiceProperties.Logging.Write ? "true" : "false"});
        writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "RetentionPolicy"});
        writer.Write(_internal::XmlNode{
            _internal::XmlNodeType::StartTag,
            "Enabled",
            options.ServiceProperties.Logging.RetentionPolicyDefinition.IsEnabled ? "true"
                                                                                  : "false"});
        if (options.ServiceProperties.Logging.RetentionPolicyDefinition.Days.HasValue())
        {
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::StartTag,
              "Days",
              std::to_string(
                  options.ServiceProperties.Logging.RetentionPolicyDefinition.Days.Value())});
        }
        writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "HourMetrics"});
        writer.Write(_internal::XmlNode{
            _internal::XmlNodeType::StartTag,
            "Version",
            options.ServiceProperties.HourMetrics.Version});
        writer.Write(_internal::XmlNode{
            _internal::XmlNodeType::StartTag,
            "Enabled",
            options.ServiceProperties.HourMetrics.IsEnabled ? "true" : "false"});
        if (options.ServiceProperties.HourMetrics.IncludeApis.HasValue())
        {
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::StartTag,
              "IncludeAPIs",
              options.ServiceProperties.HourMetrics.IncludeApis.Value() ? "true" : "false"});
        }
        writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "RetentionPolicy"});
        writer.Write(_internal::XmlNode{
            _internal::XmlNodeType::StartTag,
            "Enabled",
            options.ServiceProperties.HourMetrics.RetentionPolicyDefinition.IsEnabled ? "true"
                                                                                      : "false"});
        if (options.ServiceProperties.HourMetrics.RetentionPolicyDefinition.Days.HasValue())
        {
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::StartTag,
              "Days",
              std::to_string(
                  options.ServiceProperties.HourMetrics.RetentionPolicyDefinition.Days.Value())});
        }
        writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "MinuteMetrics"});
        writer.Write(_internal::XmlNode{
            _internal::XmlNodeType::StartTag,
            "Version",
            options.ServiceProperties.MinuteMetrics.Version});
        writer.Write(_internal::XmlNode{
            _internal::XmlNodeType::StartTag,
            "Enabled",
            options.ServiceProperties.MinuteMetrics.IsEnabled ? "true" : "false"});
        if (options.ServiceProperties.MinuteMetrics.IncludeApis.HasValue())
        {
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::StartTag,
              "IncludeAPIs",
              options.ServiceProperties.MinuteMetrics.IncludeApis.Value() ? "true" : "false"});
        }
        writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "RetentionPolicy"});
        writer.Write(_internal::XmlNode{
            _internal::XmlNodeType::StartTag,
            "Enabled",
            options.ServiceProperties.MinuteMetrics.RetentionPolicyDefinition.IsEnabled ? "true"
                                                                                        : "false"});
        if (options.ServiceProperties.MinuteMetrics.RetentionPolicyDefinition.Days.HasValue())
        {
          writer.Write(_internal::XmlNode{
              _internal::XmlNodeType::StartTag,
              "Days",
              std::to_string(
                  options.ServiceProperties.MinuteMetrics.RetentionPolicyDefinition.Days.Value())});
        }
        writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        writer.Write(_internal::XmlNode{_internal::XmlNodeType::StartTag, "Cors"});
        for (const auto& i1 : options.ServiceProperties.Cors)
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
              _internal::XmlNodeType::StartTag,
              "MaxAgeInSeconds",
              std::to_string(i1.MaxAgeInSeconds)});
          writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        }
        writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        writer.Write(_internal::XmlNode{_internal::XmlNodeType::EndTag});
        writer.Write(_internal::XmlNode{_internal::XmlNodeType::End});
        xmlBody = writer.GetDocument();
      }
      return xmlBody;
    }
  };
}}} // namespace Azure::Storage::Tables
