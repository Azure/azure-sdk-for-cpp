// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/data/tables/internal/serializers.hpp"

#include <azure/core/internal/json/json.hpp>

using namespace Azure::Data::Tables::_detail::Xml;
using namespace Azure::Data::Tables;
using namespace Azure::Data::Tables::Models;
namespace Azure { namespace Data { namespace Tables { namespace _detail {
  std::string const Serializers::CreateEntity(Models::TableEntity const& tableEntity)
  {
    std::string jsonBody;
    {
      auto jsonRoot = Core::Json::_internal::json::object();

      jsonRoot["PartitionKey"] = tableEntity.GetPartitionKey().Value;
      jsonRoot["RowKey"] = tableEntity.GetRowKey().Value;
      for (auto entry : tableEntity.Properties)
      {
        jsonRoot[entry.first] = entry.second.Value;
        if (entry.second.Type.HasValue())
        {
          jsonRoot[entry.first + "@odata.type"] = entry.second.Type.Value().ToString();
        }
      }
      jsonBody = jsonRoot.dump();
    }
    return jsonBody;
  }

  std::string const Serializers::MergeEntity(Models::TableEntity const& tableEntity)
  {
    return CreateEntity(tableEntity);
  }

  std::string const Serializers::UpdateEntity(Models::TableEntity const& tableEntity)
  {
    return CreateEntity(tableEntity);
  }

  std::string const Serializers::SetAccessPolicy(Models::TableAccessPolicy const& tableAccessPolicy)
  {
    std::string xmlBody;
    {
      XmlWriter writer;
      writer.Write(XmlNode{XmlNodeType::StartTag, "SignedIdentifiers"});
      for (const auto& i1 : tableAccessPolicy.SignedIdentifiers)
      {
        writer.Write(XmlNode{XmlNodeType::StartTag, "SignedIdentifier"});
        writer.Write(XmlNode{XmlNodeType::StartTag, "Id", i1.Id});
        writer.Write(XmlNode{XmlNodeType::StartTag, "AccessPolicy"});
        if (i1.StartsOn.HasValue())
        {
          writer.Write(XmlNode{
              XmlNodeType::StartTag,
              "Start",
              i1.StartsOn.Value().ToString(
                  Azure::DateTime::DateFormat::Rfc3339,
                  Azure::DateTime::TimeFractionFormat::AllDigits)});
        }
        if (i1.ExpiresOn.HasValue())
        {
          writer.Write(XmlNode{
              XmlNodeType::StartTag,
              "Expiry",
              i1.ExpiresOn.Value().ToString(
                  Azure::DateTime::DateFormat::Rfc3339,
                  Azure::DateTime::TimeFractionFormat::AllDigits)});
        }
        writer.Write(XmlNode{XmlNodeType::StartTag, "Permission", i1.Permissions});
        writer.Write(XmlNode{XmlNodeType::EndTag});
        writer.Write(XmlNode{XmlNodeType::EndTag});
      }
      writer.Write(XmlNode{XmlNodeType::EndTag});
      writer.Write(XmlNode{XmlNodeType::End});
      xmlBody = writer.GetDocument();
    }
    return xmlBody;
  }

  std::string const Serializers::Create(std::string const& tableName)
  {
    std::string jsonBody;
    {
      auto jsonRoot = Azure::Core::Json::_internal::json::object();

      jsonRoot["TableName"] = tableName;
      jsonBody = jsonRoot.dump();
    }
    return jsonBody;
  }

  std::string const Serializers::SetServiceProperties(
      Models::SetServicePropertiesOptions const& options)
  {
    std::string xmlBody;
    {
      XmlWriter writer;
      writer.Write(XmlNode{XmlNodeType::StartTag, "StorageServiceProperties"});
      writer.Write(XmlNode{XmlNodeType::StartTag, "Logging"});
      writer.Write(
          XmlNode{XmlNodeType::StartTag, "Version", options.ServiceProperties.Logging.Version});
      writer.Write(XmlNode{
          XmlNodeType::StartTag,
          "Delete",
          options.ServiceProperties.Logging.Delete ? "true" : "false"});
      writer.Write(XmlNode{
          XmlNodeType::StartTag,
          "Read",
          options.ServiceProperties.Logging.Read ? "true" : "false"});
      writer.Write(XmlNode{
          XmlNodeType::StartTag,
          "Write",
          options.ServiceProperties.Logging.Write ? "true" : "false"});
      writer.Write(XmlNode{XmlNodeType::StartTag, "RetentionPolicy"});
      writer.Write(XmlNode{
          XmlNodeType::StartTag,
          "Enabled",
          options.ServiceProperties.Logging.RetentionPolicyDefinition.IsEnabled ? "true"
                                                                                : "false"});
      if (options.ServiceProperties.Logging.RetentionPolicyDefinition.DataRetentionInDays
              .HasValue())
      {
        writer.Write(XmlNode{
            XmlNodeType::StartTag,
            "Days",
            std::to_string(options.ServiceProperties.Logging.RetentionPolicyDefinition
                               .DataRetentionInDays.Value())});
      }
      writer.Write(XmlNode{XmlNodeType::EndTag});
      writer.Write(XmlNode{XmlNodeType::EndTag});
      writer.Write(XmlNode{XmlNodeType::StartTag, "HourMetrics"});
      writer.Write(
          XmlNode{XmlNodeType::StartTag, "Version", options.ServiceProperties.HourMetrics.Version});
      writer.Write(XmlNode{
          XmlNodeType::StartTag,
          "Enabled",
          options.ServiceProperties.HourMetrics.IsEnabled ? "true" : "false"});
      if (options.ServiceProperties.HourMetrics.IncludeApis.HasValue())
      {
        writer.Write(XmlNode{
            XmlNodeType::StartTag,
            "IncludeAPIs",
            options.ServiceProperties.HourMetrics.IncludeApis.Value() ? "true" : "false"});
      }
      writer.Write(XmlNode{XmlNodeType::StartTag, "RetentionPolicy"});
      writer.Write(XmlNode{
          XmlNodeType::StartTag,
          "Enabled",
          options.ServiceProperties.HourMetrics.RetentionPolicyDefinition.IsEnabled ? "true"
                                                                                    : "false"});
      if (options.ServiceProperties.HourMetrics.RetentionPolicyDefinition.DataRetentionInDays
              .HasValue())
      {
        writer.Write(XmlNode{
            XmlNodeType::StartTag,
            "Days",
            std::to_string(options.ServiceProperties.HourMetrics.RetentionPolicyDefinition
                               .DataRetentionInDays.Value())});
      }
      writer.Write(XmlNode{XmlNodeType::EndTag});
      writer.Write(XmlNode{XmlNodeType::EndTag});
      writer.Write(XmlNode{XmlNodeType::StartTag, "MinuteMetrics"});
      writer.Write(XmlNode{
          XmlNodeType::StartTag, "Version", options.ServiceProperties.MinuteMetrics.Version});
      writer.Write(XmlNode{
          XmlNodeType::StartTag,
          "Enabled",
          options.ServiceProperties.MinuteMetrics.IsEnabled ? "true" : "false"});
      if (options.ServiceProperties.MinuteMetrics.IncludeApis.HasValue())
      {
        writer.Write(XmlNode{
            XmlNodeType::StartTag,
            "IncludeAPIs",
            options.ServiceProperties.MinuteMetrics.IncludeApis.Value() ? "true" : "false"});
      }
      writer.Write(XmlNode{XmlNodeType::StartTag, "RetentionPolicy"});
      writer.Write(XmlNode{
          XmlNodeType::StartTag,
          "Enabled",
          options.ServiceProperties.MinuteMetrics.RetentionPolicyDefinition.IsEnabled ? "true"
                                                                                      : "false"});
      if (options.ServiceProperties.MinuteMetrics.RetentionPolicyDefinition.DataRetentionInDays
              .HasValue())
      {
        writer.Write(XmlNode{
            XmlNodeType::StartTag,
            "Days",
            std::to_string(options.ServiceProperties.MinuteMetrics.RetentionPolicyDefinition
                               .DataRetentionInDays.Value())});
      }
      writer.Write(XmlNode{XmlNodeType::EndTag});
      writer.Write(XmlNode{XmlNodeType::EndTag});
      writer.Write(XmlNode{XmlNodeType::StartTag, "Cors"});
      for (const auto& i1 : options.ServiceProperties.Cors)
      {
        writer.Write(XmlNode{XmlNodeType::StartTag, "CorsRule"});
        writer.Write(XmlNode{XmlNodeType::StartTag, "AllowedOrigins", i1.AllowedOrigins});
        writer.Write(XmlNode{XmlNodeType::StartTag, "AllowedMethods", i1.AllowedMethods});
        writer.Write(XmlNode{XmlNodeType::StartTag, "AllowedHeaders", i1.AllowedHeaders});
        writer.Write(XmlNode{XmlNodeType::StartTag, "ExposedHeaders", i1.ExposedHeaders});
        writer.Write(
            XmlNode{XmlNodeType::StartTag, "MaxAgeInSeconds", std::to_string(i1.MaxAgeInSeconds)});
        writer.Write(XmlNode{XmlNodeType::EndTag});
      }
      writer.Write(XmlNode{XmlNodeType::EndTag});
      writer.Write(XmlNode{XmlNodeType::EndTag});
      writer.Write(XmlNode{XmlNodeType::End});
      xmlBody = writer.GetDocument();
    }
    return xmlBody;
  }

  Models::TableAccessPolicy Serializers::TableAccessPolicyFromXml(std::vector<uint8_t> responseData)
  {
    Models::TableAccessPolicy response;
    XmlReader reader(reinterpret_cast<const char*>(responseData.data()), responseData.size());
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
      else if (node.Type == XmlNodeType::Attribute)
      {
      }
      else if (node.Type == XmlNodeType::EndTag)
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

    return response;
  }

  Models::TableServiceProperties Serializers::ServicePropertiesFromXml(
      std::vector<uint8_t> responseData)
  {
    Models::TableServiceProperties response;
    XmlReader reader(reinterpret_cast<const char*>(responseData.data()), responseData.size());
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
          response.Logging.RetentionPolicyDefinition.IsEnabled = node.Value == std::string("true");
        }
        else if (
            xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kLogging && xmlPath[2] == XmlTagEnum::kRetentionPolicy
            && xmlPath[3] == XmlTagEnum::kDays)
        {
          response.Logging.RetentionPolicyDefinition.DataRetentionInDays = std::stoi(node.Value);
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
          response.HourMetrics.RetentionPolicyDefinition.IsEnabled
              = node.Value == std::string("true");
        }
        else if (
            xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kHourMetrics && xmlPath[2] == XmlTagEnum::kRetentionPolicy
            && xmlPath[3] == XmlTagEnum::kDays)
        {
          response.HourMetrics.RetentionPolicyDefinition.DataRetentionInDays
              = std::stoi(node.Value);
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
          response.MinuteMetrics.RetentionPolicyDefinition.IsEnabled
              = node.Value == std::string("true");
        }
        else if (
            xmlPath.size() == 4 && xmlPath[0] == XmlTagEnum::kStorageServiceProperties
            && xmlPath[1] == XmlTagEnum::kMinuteMetrics
            && xmlPath[2] == XmlTagEnum::kRetentionPolicy && xmlPath[3] == XmlTagEnum::kDays)
        {
          response.MinuteMetrics.RetentionPolicyDefinition.DataRetentionInDays
              = std::stoi(node.Value);
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
      else if (node.Type == XmlNodeType::Attribute)
      {
      }
      else if (node.Type == XmlNodeType::EndTag)
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
    return response;
  }

  Models::TableEntity Serializers::DeserializeEntity(Azure::Core::Json::_internal::json json)
  {
    Models::TableEntity tableEntity{};

    auto properties = json.get<std::map<std::string, std::string>>();
    std::vector<std::string> erasable;
    for (auto property : properties)
    {
      auto value = property.second;
      auto name = property.first;
      std::string typeFieldName = name + "@odata.type";
      if (properties.find(typeFieldName) != properties.end())
      {
        auto type = properties[typeFieldName];
        tableEntity.Properties[name] = TableEntityProperty(
            value, static_cast<Azure::Data::Tables::Models::TableEntityDataType>(type));
        erasable.push_back(typeFieldName);
      }
      else
      {
        tableEntity.Properties[name] = TableEntityProperty(value);
      }
    }
    for (auto erase : erasable)
    {
      tableEntity.Properties.erase(erase);
    }
    return tableEntity;
  }
}}}} // namespace Azure::Data::Tables::_detail
