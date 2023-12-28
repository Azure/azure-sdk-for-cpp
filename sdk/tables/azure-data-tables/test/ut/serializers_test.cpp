// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "serializers_test.hpp"

#include <chrono>
#include <sstream>
#include <string>
#include <thread>
using namespace Azure::Data::Tables;
using namespace Azure::Data::Tables::Models;

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
    EXPECT_EQ(
        serialized,
        "<?xml version=\"1.0\" "
        "encoding=\"utf-8\"?><SignedIdentifiers><SignedIdentifier><Id>test</"
        "Id><AccessPolicy><Start>2023-12-01T01:01:01.0000000Z</"
        "Start><Expiry>2023-12-02T01:01:01.0000000Z</Expiry><Permission>r</Permission></"
        "AccessPolicy></SignedIdentifier></SignedIdentifiers>");
  }

  TEST_F(SerializersTest, SetServiceProperties)
  {
    auto policy = Azure::Data::Tables::Models::SetServicePropertiesOptions();
    policy.ServiceProperties.HourMetrics.Version = "1.0";
    policy.ServiceProperties.HourMetrics.IsEnabled = true;
    policy.ServiceProperties.HourMetrics.IncludeApis = true;
    policy.ServiceProperties.HourMetrics.RetentionPolicyDefinition.Days = 1;
    policy.ServiceProperties.HourMetrics.RetentionPolicyDefinition.IsEnabled = true;

    policy.ServiceProperties.MinuteMetrics.Version = "1.0";
    policy.ServiceProperties.MinuteMetrics.IsEnabled = true;
    policy.ServiceProperties.MinuteMetrics.IncludeApis = true;
    policy.ServiceProperties.MinuteMetrics.RetentionPolicyDefinition.Days = 1;
    policy.ServiceProperties.MinuteMetrics.RetentionPolicyDefinition.IsEnabled = true;

    policy.ServiceProperties.Logging.Version = "1.0";
    policy.ServiceProperties.Logging.Delete = true;
    policy.ServiceProperties.Logging.Read = true;
    policy.ServiceProperties.Logging.Write = true;
    policy.ServiceProperties.Logging.RetentionPolicyDefinition.Days = 1;
    policy.ServiceProperties.Logging.RetentionPolicyDefinition.IsEnabled = true;

    policy.ServiceProperties.Cors.emplace_back(CorsRule());
    policy.ServiceProperties.Cors[0].AllowedOrigins = "*";
    policy.ServiceProperties.Cors[0].AllowedMethods = "GET";
    policy.ServiceProperties.Cors[0].AllowedHeaders = "*";
    policy.ServiceProperties.Cors[0].ExposedHeaders = "*";

    auto serialized = Serializers::SetServiceProperties(policy);
    EXPECT_EQ(
        serialized,
        "<?xml version=\"1.0\" "
        "encoding=\"utf-8\"?><StorageServiceProperties><Logging><Version>1.0</"
        "Version><Delete>true</Delete><Read>true</Read><Write>true</"
        "Write><RetentionPolicy><Enabled>true</Enabled><Days>1</Days></RetentionPolicy></"
        "Logging><HourMetrics><Version>1.0</Version><Enabled>true</Enabled><IncludeAPIs>true</"
        "IncludeAPIs><RetentionPolicy><Enabled>true</Enabled><Days>1</Days></RetentionPolicy></"
        "HourMetrics><MinuteMetrics><Version>1.0</Version><Enabled>true</"
        "Enabled><IncludeAPIs>true</IncludeAPIs><RetentionPolicy><Enabled>false</Enabled><Days>1</"
        "Days></RetentionPolicy></MinuteMetrics><Cors><CorsRule><AllowedOrigins>*</"
        "AllowedOrigins><AllowedMethods>GET</AllowedMethods><AllowedHeaders>*</"
        "AllowedHeaders><ExposedHeaders>*</ExposedHeaders><MaxAgeInSeconds>0</MaxAgeInSeconds></"
        "CorsRule></Cors></StorageServiceProperties>");
  }

}}} // namespace Azure::Data::Test
