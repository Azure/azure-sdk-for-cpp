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
