// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/template/template_client.hpp>

using namespace Azure::Template;

TEST(Template, Basic)
{
  TemplateClient templateClient;

  EXPECT_FALSE(templateClient.ClientVersion().empty());
}

TEST(Template, GetValue1)
{
  TemplateClient templateClient;

  EXPECT_EQ(templateClient.GetValue1(-1), 0);
  EXPECT_EQ(templateClient.GetValue1(0), 1);
  EXPECT_EQ(templateClient.GetValue1(1), 2);
}
