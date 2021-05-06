// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/identity/environment_credential.hpp>

#include <gtest/gtest.h>

#include <chrono>

#include "getenv.hpp"

TEST(EnvironmentCredential, ClientSecret)
{
  // See EnvironmentCredential's documentation - these below are the environment variables it reads
  // from.
  EXPECT_FALSE(GetEnv("AZURE_TENANT_ID").empty());
  EXPECT_FALSE(GetEnv("AZURE_CLIENT_ID").empty());
  EXPECT_FALSE(GetEnv("AZURE_CLIENT_SECRET").empty());

  Azure::Identity::EnvironmentCredential const credential;

  auto const token = credential.GetToken(
      {{"https://vault.azure.net/.default"}}, Azure::Core::Context::ApplicationContext);

  EXPECT_FALSE(token.Token.empty());
  EXPECT_GE(token.ExpiresOn, std::chrono::system_clock::now());
}
