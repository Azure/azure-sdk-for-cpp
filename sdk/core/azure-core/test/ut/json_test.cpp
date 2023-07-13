// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

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

TEST(Json, utf8BOM)
{
  // Verify that the UTF-8 BOM bytes (0xEF, 0xBB, 0xBF) are skipped when parsing JSON using the
  // library.
  std::array<uint8_t, 8> v{239, 187, 191, '5'};
  json jsonRoot = json::parse(v);
  EXPECT_EQ(jsonRoot.get<int>(), 5);
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
