// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/template/template_client.hpp>

using namespace Azure::Template;

TEST(Template, GetValue)
{
  TemplateClient templateClient;

  EXPECT_EQ(templateClient.GetValue(1, 1, 1), -1);
  EXPECT_EQ(templateClient.GetValue(2, 1, 1), -1);
  EXPECT_EQ(templateClient.GetValue(2, 2, 1), -1);

  EXPECT_EQ(templateClient.GetValue(12, 12, 12), -1);
  EXPECT_EQ(templateClient.GetValue(11, 12, 12), -1);
  EXPECT_EQ(templateClient.GetValue(11, 11, 12), -1);

  EXPECT_EQ(templateClient.GetValue(8, 8, 11), 4);
  EXPECT_EQ(templateClient.GetValue(11, 11, 11), 4);

  EXPECT_EQ(templateClient.GetValue(9, 9, 7), 4);
  EXPECT_EQ(templateClient.GetValue(9, 9, 8), 0);
  EXPECT_EQ(templateClient.GetValue(9, 9, 11), 4);

  EXPECT_EQ(templateClient.GetValue(6, 6, 6), 4);
  EXPECT_EQ(templateClient.GetValue(6, 6, 2), 1);

  EXPECT_EQ(templateClient.GetValue(4, 4, 4), 1);
  EXPECT_EQ(templateClient.GetValue(4, 4, 5), 4);
  EXPECT_EQ(templateClient.GetValue(4, 4, 6), 4);

  EXPECT_EQ(templateClient.GetValue(7, 7, 7), 4);
  EXPECT_EQ(templateClient.GetValue(5, 5, 5), 3);
  EXPECT_EQ(templateClient.GetValue(10, 10, 6), 0);

  EXPECT_EQ(templateClient.GetValue(11, 6, 5), 3);
  EXPECT_EQ(templateClient.GetValue(6, 11, 6), 3);

  EXPECT_EQ(templateClient.GetValue(11, 4, 4), 3);
  EXPECT_EQ(templateClient.GetValue(11, 6, 3), 3);
  EXPECT_EQ(templateClient.GetValue(11, 6, 2), 1);

  EXPECT_EQ(templateClient.GetValue(11, 7, 9), 1);
  EXPECT_EQ(templateClient.GetValue(11, 7, 8), 0);
  EXPECT_EQ(templateClient.GetValue(11, 7, 7), 0);
  EXPECT_EQ(templateClient.GetValue(11, 7, 6), 2);

  EXPECT_EQ(templateClient.GetValue(11, 8, 6), 2);
  EXPECT_EQ(templateClient.GetValue(11, 9, 6), 0);

  EXPECT_EQ(templateClient.GetValue(5, 6, 11), 3);
  EXPECT_EQ(templateClient.GetValue(10, 7, 6), 0);

  EXPECT_EQ(templateClient.GetValue(10, 3, 6), 0);

  EXPECT_EQ(templateClient.GetValue(10, 2, 4), 0);
  EXPECT_EQ(templateClient.GetValue(10, 2, 3), 1);
  EXPECT_EQ(templateClient.GetValue(10, 2, 7), 1);

  EXPECT_EQ(templateClient.GetValue(5, 5, 9), 3);

  EXPECT_EQ(templateClient.GetValue(4, 5, 6), 3);
  EXPECT_EQ(templateClient.GetValue(4, 5, 2), 1);
}
