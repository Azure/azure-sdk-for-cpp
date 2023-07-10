// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/json/json.hpp>

#include <gtest/gtest.h>

using json = Azure::Core::Json::_internal::json;

// Just a simple test to ensure that Azure Core internal is wrapping nlohmann json
TEST(Json, create)
{
  json j;
  j["pi"] = 3.141;
  std::string expected("{\"pi\":3.141}");

  EXPECT_EQ(expected, j.dump());
}

TEST(Json, duplicateName)
{
  json jsonRoot = json::parse(R"({"KeyName": 1, "AnotherObject": {"KeyName": 2}})");
  int value = 0;
  if (jsonRoot.contains("KeyName"))
  {
    value = jsonRoot["KeyName"].get<int>();
  }
  EXPECT_EQ(value, 1);

  jsonRoot = json::parse(R"({"AnotherObject": {"KeyName": 2}})");
  value = 0;

  // The nested KeyName property is considered not found, when at the root.
  if (jsonRoot.contains("KeyName"))
  {
    value = jsonRoot["KeyName"].get<int>();
  }
  EXPECT_EQ(value, 0);

  // The nested KeyName property is considered found, when navigating to the nested object first.
  if (jsonRoot["AnotherObject"].contains("KeyName"))
  {
    value = jsonRoot["AnotherObject"]["KeyName"].get<int>();
  }
  EXPECT_EQ(value, 2);
}
