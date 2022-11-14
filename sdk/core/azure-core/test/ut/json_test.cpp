//  Copyright (c) Microsoft Corporation. All rights reserved.
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
