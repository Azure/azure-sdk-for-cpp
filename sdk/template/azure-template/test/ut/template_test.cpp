// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/template/template_client.hpp>

#include <gtest/gtest.h>

using namespace Azure::Template;

TEST(Template, Basic) { TemplateClient templateClient; }

TEST(Template, GetValue)
{
  TemplateClient templateClient;

  EXPECT_EQ(templateClient.GetValue(-1), 0);
  EXPECT_EQ(templateClient.GetValue(0), 0);
  EXPECT_EQ(templateClient.GetValue(1), 10);
  EXPECT_EQ(templateClient.GetValue(22), -100);
  EXPECT_EQ(templateClient.GetValue(21), 150);
  EXPECT_EQ(templateClient.GetValue(11), 20);
  EXPECT_EQ(templateClient.GetValue(14), 1);
  EXPECT_EQ(templateClient.GetValue(18), 0);
}
