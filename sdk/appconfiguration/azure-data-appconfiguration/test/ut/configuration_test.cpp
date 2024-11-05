// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/data/appconfiguration.hpp>
#include <azure/identity.hpp>

#include <gtest/gtest.h>

using namespace Azure::Data::AppConfiguration;

TEST(ConfigurationClient, Basic)
{
  auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();
  ConfigurationClient configurationClient("serviceUrl", credential);
  EXPECT_EQ(configurationClient.GetUrl(), "serviceUrl");
}
