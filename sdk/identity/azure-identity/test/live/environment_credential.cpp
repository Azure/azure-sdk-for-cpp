// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/identity/environment_credential.hpp>

#include "common.hpp"

TEST(EnvironmentCredential, ClientSecret)
{
  Azure::Identity::EnvironmentCredential const credential;

  auto const token = credential.GetToken(
      {{GetEnv("AZURE_KEYVAULT_URL") + ".default"}}, Azure::Core::Context::GetApplicationContext());

  EXPECT_FALSE(token.Token.empty());
  EXPECT_GE(token.ExpiresOn, std::chrono::system_clock::now());
}
