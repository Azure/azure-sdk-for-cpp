// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "serializers_test.hpp"

#include <chrono>
#include <sstream>
#include <string>
#include <thread>

using namespace Azure::Data::Tables;
using namespace Azure::Data::Tables::Models;
using namespace Azure::Data::Tables::_detail;

namespace Azure { namespace Data { namespace Test {

  TEST_F(SerializersTest, SetAccessPolicy)
  {
    auto policy = Azure::Data::Tables::Models::TableAccessPolicy();
    SignedIdentifier identifier;
    identifier.Id = "test";
    identifier.StartsOn = Azure::DateTime(2023, 12, 1, 1, 1, 1);
    identifier.ExpiresOn = Azure::DateTime(2023, 12, 2, 1, 1, 1);
    identifier.Permissions = "r";
    policy.SignedIdentifiers.emplace_back(identifier);
    auto serialized = Serializers::SetAccessPolicy(policy);

    auto data = Serializers::TableAccessPolicyFromXml(
        std::vector<uint8_t>(serialized.begin(), serialized.end()));
    EXPECT_EQ(data.SignedIdentifiers[0].Id, "test");
    EXPECT_EQ(data.SignedIdentifiers[0].Permissions, "r");
    EXPECT_EQ(data.SignedIdentifiers[0].StartsOn.Value(), Azure::DateTime(2023, 12, 1, 1, 1, 1));
    EXPECT_EQ(data.SignedIdentifiers[0].ExpiresOn.Value(), Azure::DateTime(2023, 12, 2, 1, 1, 1));
  }

  TEST_F(SerializersTest, SetServiceProperties)
  {
    auto policy = Azure::Data::Tables::Models::SetServicePropertiesOptions();
    policy.ServiceProperties.HourMetrics.Version = "1.0";
    policy.ServiceProperties.HourMetrics.IsEnabled = true;
    policy.ServiceProperties.HourMetrics.IncludeApis = true;
    policy.ServiceProperties.HourMetrics.RetentionPolicyDefinition.DataRetentionInDays = 1;
    policy.ServiceProperties.HourMetrics.RetentionPolicyDefinition.IsEnabled = true;

    policy.ServiceProperties.MinuteMetrics.Version = "1.0";
    policy.ServiceProperties.MinuteMetrics.IsEnabled = true;
    policy.ServiceProperties.MinuteMetrics.IncludeApis = true;
    policy.ServiceProperties.MinuteMetrics.RetentionPolicyDefinition.DataRetentionInDays = 1;
    policy.ServiceProperties.MinuteMetrics.RetentionPolicyDefinition.IsEnabled = true;

    policy.ServiceProperties.Logging.Version = "1.0";
    policy.ServiceProperties.Logging.Delete = true;
    policy.ServiceProperties.Logging.Read = true;
    policy.ServiceProperties.Logging.Write = true;
    policy.ServiceProperties.Logging.RetentionPolicyDefinition.DataRetentionInDays = 1;
    policy.ServiceProperties.Logging.RetentionPolicyDefinition.IsEnabled = true;

    policy.ServiceProperties.Cors.emplace_back(CorsRule());
    policy.ServiceProperties.Cors[0].AllowedOrigins = "*";
    policy.ServiceProperties.Cors[0].AllowedMethods = "GET";
    policy.ServiceProperties.Cors[0].AllowedHeaders = "*";
    policy.ServiceProperties.Cors[0].ExposedHeaders = "*";

    auto serialized = Serializers::SetServiceProperties(policy);
    auto data = Serializers::ServicePropertiesFromXml(
        std::vector<uint8_t>(serialized.begin(), serialized.end()));
    EXPECT_EQ(data.HourMetrics.Version, "1.0");
    EXPECT_EQ(data.HourMetrics.IsEnabled, true);
    EXPECT_EQ(data.HourMetrics.IncludeApis.Value(), true);
    EXPECT_EQ(data.HourMetrics.RetentionPolicyDefinition.DataRetentionInDays.Value(), 1);
    EXPECT_EQ(data.HourMetrics.RetentionPolicyDefinition.IsEnabled, true);

    EXPECT_EQ(data.MinuteMetrics.Version, "1.0");
    EXPECT_EQ(data.MinuteMetrics.IsEnabled, true);
    EXPECT_EQ(data.MinuteMetrics.IncludeApis.Value(), true);
    EXPECT_EQ(data.MinuteMetrics.RetentionPolicyDefinition.DataRetentionInDays.Value(), 1);
    EXPECT_EQ(data.MinuteMetrics.RetentionPolicyDefinition.IsEnabled, true);

    EXPECT_EQ(data.Logging.Version, "1.0");
    EXPECT_EQ(data.Logging.Delete, true);
    EXPECT_EQ(data.Logging.Read, true);
    EXPECT_EQ(data.Logging.Write, true);
    EXPECT_EQ(data.Logging.RetentionPolicyDefinition.DataRetentionInDays.Value(), 1);
    EXPECT_EQ(data.Logging.RetentionPolicyDefinition.IsEnabled, true);

    EXPECT_EQ(data.Cors[0].AllowedOrigins, "*");
    EXPECT_EQ(data.Cors[0].AllowedMethods, "GET");
    EXPECT_EQ(data.Cors[0].AllowedHeaders, "*");
    EXPECT_EQ(data.Cors[0].ExposedHeaders, "*");
  }
  TEST_F(SerializersTest, DeserializeEntitySimpleProperties)
  {
    auto json = R"({
    "PartitionKey": "p1",
    "RowKey": "r1",
    "Name": "Test Name",
    "Age": "30"
  })"_json;

    auto entity = Serializers::DeserializeEntity(json);

    EXPECT_EQ(entity.GetPartitionKey().Value, "p1");
    EXPECT_EQ(entity.GetRowKey().Value, "r1");
    EXPECT_EQ(entity.Properties["Name"].Value, "Test Name");
    EXPECT_EQ(entity.Properties["Age"].Value, "30");
  }

  TEST_F(SerializersTest, DeserializeEntityWithOdataType)
  {
    auto json = R"({
    "PartitionKey": "p2",
    "RowKey": "r2",
    "Completed": "true",
    "Completed@odata.type": "Edm.Boolean",
    "Score": "9.5",
    "Score@odata.type": "Edm.Double"
  })"_json;

    auto entity = Serializers::DeserializeEntity(json);

    EXPECT_EQ(entity.GetPartitionKey().Value, "p2");
    EXPECT_EQ(entity.GetRowKey().Value, "r2");
    EXPECT_EQ(entity.Properties["Completed"].Value, "true");
    EXPECT_EQ(entity.Properties["Completed"].Type.Value().ToString(), "Edm.Boolean");
    EXPECT_EQ(entity.Properties["Score"].Value, "9.5");
    EXPECT_EQ(entity.Properties["Score"].Type.Value().ToString(), "Edm.Double");
  }

  TEST_F(SerializersTest, DeserializeEntityWithNonStringType)
  {
    auto json = R"({
    "PartitionKey": "p2",
    "RowKey": "r2",
    "Completed": "true",
    "Completed@odata.type": "Edm.Boolean",
    "Score": "9.5",
    "Score@odata.type": "Edm.Double",
    "Age": 30
  })"_json;

    auto entity = Serializers::DeserializeEntity(json);

    EXPECT_EQ(entity.GetPartitionKey().Value, "p2");
    EXPECT_EQ(entity.GetRowKey().Value, "r2");
    EXPECT_EQ(entity.Properties["Completed"].Value, "true");
    EXPECT_EQ(entity.Properties["Completed"].Type.Value().ToString(), "Edm.Boolean");
    EXPECT_EQ(entity.Properties["Score"].Value, "9.5");
    EXPECT_EQ(entity.Properties["Score"].Type.Value().ToString(), "Edm.Double");
    EXPECT_EQ(entity.Properties["Age"].Value, "30");
  }
  TEST_F(SerializersTest, DeserializeEntityMissingProperties)
  {
    auto json = R"({
    "PartitionKey": "p3",
    "RowKey": "r3"
  })"_json;

    auto entity = Serializers::DeserializeEntity(json);

    EXPECT_EQ(entity.GetPartitionKey().Value, "p3");
    EXPECT_EQ(entity.GetRowKey().Value, "r3");
    EXPECT_FALSE(entity.Properties.empty());
  }

  TEST_F(SerializersTest, CreateEntity)
  {
    TableEntity entity;
    entity.SetPartitionKey("partition1");
    entity.SetRowKey("row1");
    entity.Properties["Name"] = TableEntityProperty("John Doe");

    auto serialized = Serializers::CreateEntity(entity);
    auto expectedJson = R"({"Name":"John Doe","PartitionKey":"partition1","RowKey":"row1"})";
    EXPECT_EQ(serialized, expectedJson);
  }

  TEST_F(SerializersTest, MergeEntity)
  {
    TableEntity entity;
    entity.SetPartitionKey("partition2");
    entity.SetRowKey("row2");
    entity.Properties["Status"] = TableEntityProperty("Active");

    auto serialized = Serializers::MergeEntity(entity);
    auto expectedJson = R"({"PartitionKey":"partition2","RowKey":"row2","Status":"Active"})";
    EXPECT_EQ(serialized, expectedJson);
  }

  TEST_F(SerializersTest, UpdateEntity)
  {
    TableEntity entity;
    entity.SetPartitionKey("partition3");
    entity.SetRowKey("row3");

    auto serialized = Serializers::UpdateEntity(entity);
    auto expectedJson = R"({"PartitionKey":"partition3","RowKey":"row3"})";
    EXPECT_EQ(serialized, expectedJson);
  }

  TEST_F(SerializersTest, Create)
  {
    std::string tableName = "MyTable";
    auto serialized = Serializers::Create(tableName);
    auto expectedJson = R"({"TableName":"MyTable"})";
    EXPECT_EQ(serialized, expectedJson);
  }

  TEST_F(SerializersTest, SetAccessPolicyComplex)
  {
    TableAccessPolicy policy;
    SignedIdentifier identifier1;
    identifier1.Id = "user1";
    identifier1.StartsOn = Azure::DateTime(2023, 1, 1, 0, 0, 0);
    identifier1.ExpiresOn = Azure::DateTime(2023, 1, 2, 0, 0, 0);
    identifier1.Permissions = "r";
    policy.SignedIdentifiers.emplace_back(identifier1);

    SignedIdentifier identifier2;
    identifier2.Id = "user2";
    identifier2.StartsOn = Azure::DateTime(2023, 1, 3, 0, 0, 0);
    identifier2.ExpiresOn = Azure::DateTime(2023, 1, 4, 0, 0, 0);
    identifier2.Permissions = "rw";
    policy.SignedIdentifiers.emplace_back(identifier2);

    auto serialized = Serializers::SetAccessPolicy(policy);
    auto data = Serializers::TableAccessPolicyFromXml(
        std::vector<uint8_t>(serialized.begin(), serialized.end()));

    EXPECT_EQ(data.SignedIdentifiers.size(), 2);
    EXPECT_EQ(data.SignedIdentifiers[0].Id, "user1");
    EXPECT_EQ(data.SignedIdentifiers[0].Permissions, "r");
    EXPECT_EQ(data.SignedIdentifiers[0].StartsOn.Value(), Azure::DateTime(2023, 1, 1, 0, 0, 0));
    EXPECT_EQ(data.SignedIdentifiers[0].ExpiresOn.Value(), Azure::DateTime(2023, 1, 2, 0, 0, 0));

    EXPECT_EQ(data.SignedIdentifiers[1].Id, "user2");
    EXPECT_EQ(data.SignedIdentifiers[1].Permissions, "rw");
    EXPECT_EQ(data.SignedIdentifiers[1].StartsOn.Value(), Azure::DateTime(2023, 1, 3, 0, 0, 0));
    EXPECT_EQ(data.SignedIdentifiers[1].ExpiresOn.Value(), Azure::DateTime(2023, 1, 4, 0, 0, 0));
  }

  TEST_F(SerializersTest, DeserializeEntityWithMixedTypes)
  {
    auto json = R"({
    "PartitionKey": "p4",
    "RowKey": "r4",
    "Name": "Jane Doe",
    "IsActive": "true",
    "IsActive@odata.type": "Edm.Boolean",
    "Salary": "5000.00",
    "Salary@odata.type": "Edm.Double"
  })"_json;

    auto entity = Serializers::DeserializeEntity(json);

    EXPECT_EQ(entity.GetPartitionKey().Value, "p4");
    EXPECT_EQ(entity.GetRowKey().Value, "r4");
    EXPECT_EQ(entity.Properties["Name"].Value, "Jane Doe");
    EXPECT_EQ(entity.Properties["IsActive"].Value, "true");
    EXPECT_EQ(entity.Properties["IsActive"].Type.Value().ToString(), "Edm.Boolean");
    EXPECT_EQ(entity.Properties["Salary"].Value, "5000.00");
    EXPECT_EQ(entity.Properties["Salary"].Type.Value().ToString(), "Edm.Double");
  }

  TEST_F(SerializersTest, DeserializeEntityWithComplexTypes)
  {
    auto json = R"({
    "PartitionKey": "p5",
    "RowKey": "r5",
    "Metadata": "{\"author\":\"John Doe\",\"year\":2023}",
    "Metadata@odata.type": "Edm.String"
  })"_json;

    auto entity = Serializers::DeserializeEntity(json);

    EXPECT_EQ(entity.GetPartitionKey().Value, "p5");
    EXPECT_EQ(entity.GetRowKey().Value, "r5");
    EXPECT_EQ(entity.Properties["Metadata"].Value, "{\"author\":\"John Doe\",\"year\":2023}");
    EXPECT_EQ(entity.Properties["Metadata"].Type.Value().ToString(), "Edm.String");
  }

  TEST_F(SerializersTest, DeserializeEntityWithNullValues)
  {
    auto json = R"({
    "PartitionKey": "p6",
    "RowKey": "r6",
    "Description": null,
    "Description@odata.type": "Edm.String"
  })"_json;

    auto entity = Serializers::DeserializeEntity(json);

    EXPECT_EQ(entity.GetPartitionKey().Value, "p6");
    EXPECT_EQ(entity.GetRowKey().Value, "r6");
    EXPECT_EQ(entity.Properties["Description"].Value, "null");
    EXPECT_EQ(entity.Properties["Description"].Type.Value().ToString(), "Edm.String");
  }

  TEST_F(SerializersTest, SetServicePropertiesBasic)
  {
    Models::SetServicePropertiesOptions options;
    options.ServiceProperties.Logging.Version = "2.0";
    options.ServiceProperties.Logging.Delete = false;
    options.ServiceProperties.Logging.Read = false;
    options.ServiceProperties.Logging.Write = false;
    options.ServiceProperties.Logging.RetentionPolicyDefinition.IsEnabled = false;

    auto serialized = Serializers::SetServiceProperties(options);
    EXPECT_NE(
        serialized.find("<Logging><Version>2.0</Version><Delete>false</Delete><Read>false</"
                        "Read><Write>false</Write>"),
        std::string::npos);
    EXPECT_NE(
        serialized.find("<RetentionPolicy><Enabled>false</Enabled></RetentionPolicy>"),
        std::string::npos);
  }

  TEST_F(SerializersTest, SetServicePropertiesAllEnabled)
  {
    Models::SetServicePropertiesOptions options;
    options.ServiceProperties.Logging.Version = "2.0";
    options.ServiceProperties.Logging.Delete = true;
    options.ServiceProperties.Logging.Read = true;
    options.ServiceProperties.Logging.Write = true;
    options.ServiceProperties.Logging.RetentionPolicyDefinition.IsEnabled = true;
    options.ServiceProperties.Logging.RetentionPolicyDefinition.DataRetentionInDays = 7;

    auto serialized = Serializers::SetServiceProperties(options);
    EXPECT_NE(
        serialized.find("<Logging><Version>2.0</Version><Delete>true</Delete><Read>true</"
                        "Read><Write>true</Write>"),
        std::string::npos);
    EXPECT_NE(
        serialized.find("<RetentionPolicy><Enabled>true</Enabled><Days>7</Days></RetentionPolicy>"),
        std::string::npos);
  }

  TEST_F(SerializersTest, SetServicePropertiesWithCors)
  {
    Models::SetServicePropertiesOptions options;
    Models::CorsRule corsRule;
    corsRule.AllowedOrigins = "http://example.com";
    corsRule.AllowedMethods = "GET, POST";
    corsRule.AllowedHeaders = "x-ms-meta-*";
    corsRule.ExposedHeaders = "x-ms-meta-data*";
    corsRule.MaxAgeInSeconds = 3600;
    options.ServiceProperties.Cors.push_back(corsRule);

    auto serialized = Serializers::SetServiceProperties(options);
    EXPECT_NE(serialized.find("<Cors><CorsRule>"), std::string::npos);
    EXPECT_NE(
        serialized.find("<AllowedOrigins>http://example.com</AllowedOrigins>"), std::string::npos);
    EXPECT_NE(serialized.find("<AllowedMethods>GET, POST</AllowedMethods>"), std::string::npos);
    EXPECT_NE(serialized.find("<AllowedHeaders>x-ms-meta-*</AllowedHeaders>"), std::string::npos);
    EXPECT_NE(
        serialized.find("<ExposedHeaders>x-ms-meta-data*</ExposedHeaders>"), std::string::npos);
    EXPECT_NE(serialized.find("<MaxAgeInSeconds>3600</MaxAgeInSeconds>"), std::string::npos);
  }

  TEST_F(SerializersTest, SetServicePropertiesHourMetrics)
  {
    Models::SetServicePropertiesOptions options;
    options.ServiceProperties.HourMetrics.Version = "1.0";
    options.ServiceProperties.HourMetrics.IsEnabled = true;
    options.ServiceProperties.HourMetrics.IncludeApis = true;
    options.ServiceProperties.HourMetrics.RetentionPolicyDefinition.IsEnabled = true;
    options.ServiceProperties.HourMetrics.RetentionPolicyDefinition.DataRetentionInDays = 7;

    auto serialized = Serializers::SetServiceProperties(options);
    EXPECT_NE(
        serialized.find("<HourMetrics><Version>1.0</Version><Enabled>true</"
                        "Enabled><IncludeAPIs>true</IncludeAPIs>"),
        std::string::npos);
    EXPECT_NE(
        serialized.find("<RetentionPolicy><Enabled>true</Enabled><Days>7</Days></RetentionPolicy>"),
        std::string::npos);
  }

  TEST_F(SerializersTest, SetServicePropertiesMinuteMetrics)
  {
    Models::SetServicePropertiesOptions options;
    options.ServiceProperties.MinuteMetrics.Version = "1.0";
    options.ServiceProperties.MinuteMetrics.IsEnabled = true;
    options.ServiceProperties.MinuteMetrics.IncludeApis = false;
    options.ServiceProperties.MinuteMetrics.RetentionPolicyDefinition.IsEnabled = true;
    options.ServiceProperties.MinuteMetrics.RetentionPolicyDefinition.DataRetentionInDays = 7;

    auto serialized = Serializers::SetServiceProperties(options);
    EXPECT_NE(
        serialized.find("<MinuteMetrics><Version>1.0</Version><Enabled>true</"
                        "Enabled><IncludeAPIs>false</IncludeAPIs>"),
        std::string::npos);
    EXPECT_NE(
        serialized.find("<RetentionPolicy><Enabled>true</Enabled><Days>7</Days></RetentionPolicy>"),
        std::string::npos);
  }

  TEST_F(SerializersTest, SetAccessPolicyEmpty)
  {
    TableAccessPolicy policy;
    auto serialized = Serializers::SetAccessPolicy(policy);
    auto data = Serializers::TableAccessPolicyFromXml(
        std::vector<uint8_t>(serialized.begin(), serialized.end()));
    EXPECT_TRUE(data.SignedIdentifiers.empty());
  }

  TEST_F(SerializersTest, SetAccessPolicySingleIdentifier)
  {
    TableAccessPolicy policy;
    SignedIdentifier identifier;
    identifier.Id = "singleId";
    identifier.StartsOn = Azure::DateTime(2023, 5, 1, 0, 0, 0);
    identifier.ExpiresOn = Azure::DateTime(2023, 5, 2, 0, 0, 0);
    identifier.Permissions = "rw";
    policy.SignedIdentifiers.emplace_back(identifier);

    auto serialized = Serializers::SetAccessPolicy(policy);
    auto data = Serializers::TableAccessPolicyFromXml(
        std::vector<uint8_t>(serialized.begin(), serialized.end()));

    ASSERT_EQ(data.SignedIdentifiers.size(), 1);
    EXPECT_EQ(data.SignedIdentifiers[0].Id, "singleId");
    EXPECT_EQ(data.SignedIdentifiers[0].Permissions, "rw");
    EXPECT_EQ(data.SignedIdentifiers[0].StartsOn.Value(), Azure::DateTime(2023, 5, 1, 0, 0, 0));
    EXPECT_EQ(data.SignedIdentifiers[0].ExpiresOn.Value(), Azure::DateTime(2023, 5, 2, 0, 0, 0));
  }

  TEST_F(SerializersTest, SetAccessPolicyMultipleIdentifiers)
  {
    TableAccessPolicy policy;
    SignedIdentifier identifier1, identifier2;
    identifier1.Id = "id1";
    identifier1.StartsOn = Azure::DateTime(2023, 6, 1, 0, 0, 0);
    identifier1.ExpiresOn = Azure::DateTime(2023, 6, 2, 0, 0, 0);
    identifier1.Permissions = "r";
    policy.SignedIdentifiers.emplace_back(identifier1);

    identifier2.Id = "id2";
    identifier2.StartsOn = Azure::DateTime(2023, 6, 3, 0, 0, 0);
    identifier2.ExpiresOn = Azure::DateTime(2023, 6, 4, 0, 0, 0);
    identifier2.Permissions = "w";
    policy.SignedIdentifiers.emplace_back(identifier2);

    auto serialized = Serializers::SetAccessPolicy(policy);
    auto data = Serializers::TableAccessPolicyFromXml(
        std::vector<uint8_t>(serialized.begin(), serialized.end()));

    ASSERT_EQ(data.SignedIdentifiers.size(), 2);
    EXPECT_EQ(data.SignedIdentifiers[0].Id, "id1");
    EXPECT_EQ(data.SignedIdentifiers[0].Permissions, "r");
    EXPECT_EQ(data.SignedIdentifiers[0].StartsOn.Value(), Azure::DateTime(2023, 6, 1, 0, 0, 0));
    EXPECT_EQ(data.SignedIdentifiers[0].ExpiresOn.Value(), Azure::DateTime(2023, 6, 2, 0, 0, 0));

    EXPECT_EQ(data.SignedIdentifiers[1].Id, "id2");
    EXPECT_EQ(data.SignedIdentifiers[1].Permissions, "w");
    EXPECT_EQ(data.SignedIdentifiers[1].StartsOn.Value(), Azure::DateTime(2023, 6, 3, 0, 0, 0));
    EXPECT_EQ(data.SignedIdentifiers[1].ExpiresOn.Value(), Azure::DateTime(2023, 6, 4, 0, 0, 0));
  }

  TEST_F(SerializersTest, TableAccessPolicyFromXml_EmptyXml)
  {
    std::string xml = "<SignedIdentifiers></SignedIdentifiers>";
    auto data = Serializers::TableAccessPolicyFromXml(std::vector<uint8_t>(xml.begin(), xml.end()));
    EXPECT_TRUE(data.SignedIdentifiers.empty());
  }

  TEST_F(SerializersTest, TableAccessPolicyFromXml_SingleSignedIdentifier)
  {
    std::string xml = R"(<SignedIdentifiers>
    <SignedIdentifier>
      <Id>testId</Id>
      <AccessPolicy>
        <Start>2023-01-01T00:00:00Z</Start>
        <Expiry>2023-01-02T00:00:00Z</Expiry>
        <Permission>r</Permission>
      </AccessPolicy>
    </SignedIdentifier>
  </SignedIdentifiers>)";
    auto data = Serializers::TableAccessPolicyFromXml(std::vector<uint8_t>(xml.begin(), xml.end()));
    ASSERT_EQ(data.SignedIdentifiers.size(), 1);
    EXPECT_EQ(data.SignedIdentifiers[0].Id, "testId");
    EXPECT_EQ(data.SignedIdentifiers[0].StartsOn.Value(), Azure::DateTime(2023, 1, 1, 0, 0, 0));
    EXPECT_EQ(data.SignedIdentifiers[0].ExpiresOn.Value(), Azure::DateTime(2023, 1, 2, 0, 0, 0));
    EXPECT_EQ(data.SignedIdentifiers[0].Permissions, "r");
  }

  TEST_F(SerializersTest, TableAccessPolicyFromXml_MultipleSignedIdentifiers)
  {
    std::string xml = R"(<SignedIdentifiers>
    <SignedIdentifier>
      <Id>testId1</Id>
      <AccessPolicy>
        <Start>2023-01-01T00:00:00Z</Start>
        <Expiry>2023-01-02T00:00:00Z</Expiry>
        <Permission>r</Permission>
      </AccessPolicy>
    </SignedIdentifier>
    <SignedIdentifier>
      <Id>testId2</Id>
      <AccessPolicy>
        <Start>2023-02-01T00:00:00Z</Start>
        <Expiry>2023-02-02T00:00:00Z</Expiry>
        <Permission>rw</Permission>
      </AccessPolicy>
    </SignedIdentifier>
  </SignedIdentifiers>)";
    auto data = Serializers::TableAccessPolicyFromXml(std::vector<uint8_t>(xml.begin(), xml.end()));
    ASSERT_EQ(data.SignedIdentifiers.size(), 2);
    EXPECT_EQ(data.SignedIdentifiers[0].Id, "testId1");
    EXPECT_EQ(data.SignedIdentifiers[0].Permissions, "r");
    EXPECT_EQ(data.SignedIdentifiers[1].Id, "testId2");
    EXPECT_EQ(data.SignedIdentifiers[1].Permissions, "rw");
  }

  TEST_F(SerializersTest, TableAccessPolicyFromXml_InvalidXml)
  {
    std::string xml = "<Invalid></Invalid>";
    auto data = Serializers::TableAccessPolicyFromXml(std::vector<uint8_t>(xml.begin(), xml.end()));
    EXPECT_TRUE(data.SignedIdentifiers.empty());
  }

  TEST_F(SerializersTest, TableAccessPolicyFromXml_MissingPermissions)
  {
    std::string xml = R"(<SignedIdentifiers>
    <SignedIdentifier>
      <Id>testId</Id>
      <AccessPolicy>
        <Start>2023-01-01T00:00:00Z</Start>
        <Expiry>2023-01-02T00:00:00Z</Expiry>
      </AccessPolicy>
    </SignedIdentifier>
  </SignedIdentifiers>)";
    auto data = Serializers::TableAccessPolicyFromXml(std::vector<uint8_t>(xml.begin(), xml.end()));
    ASSERT_EQ(data.SignedIdentifiers.size(), 1);
    EXPECT_EQ(data.SignedIdentifiers[0].Id, "testId");
    EXPECT_TRUE(data.SignedIdentifiers[0].Permissions.empty());
  }

}}} // namespace Azure::Data::Test
