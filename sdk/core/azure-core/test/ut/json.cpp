// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Just a simple test to ensure that Azure Core is bringing nlohmann header
TEST(Json, create)
{
  json j;
  j["pi"] = 3.141;
  std::string expected("{\"pi\":3.141}");

  EXPECT_EQ(expected, j.dump());
}
