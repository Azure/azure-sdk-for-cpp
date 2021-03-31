// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/identity/environment_credential.hpp>

#include <gtest/gtest.h>

#include <chrono>

TEST(EnvironmentCredential, ClientSecret)
{
  Azure::Identity::EnvironmentCredential const credential;

  auto const token = credential.GetToken(
      {{"https://vault.azure.net/.default"}}, Azure::Core::Context::GetApplicationContext());

  EXPECT_FALSE(token.Token.empty());
  EXPECT_GE(token.ExpiresOn, std::chrono::system_clock::now());
}
