// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/template/template_client.hpp>

using namespace Azure::Template;

TEST(Template, Basic)
{
  TemplateClient templateClient;

}

TEST(Template, GetValue)
{
  TemplateClient templateClient;

  EXPECT_EQ(templateClient.GetValue(-1), 0);
  EXPECT_EQ(templateClient.GetValue(0), 1);
  EXPECT_EQ(templateClient.GetValue(1), 2);
}
