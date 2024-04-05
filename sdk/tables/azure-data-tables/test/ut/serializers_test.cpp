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

}}} // namespace Azure::Data::Test
