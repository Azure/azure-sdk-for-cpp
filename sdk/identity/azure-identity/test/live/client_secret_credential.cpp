// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/identity/client_secret_credential.hpp>

#include "common.hpp"

TEST(ClientSecretCredential, Basic)
{
  Azure::Identity::ClientSecretCredential const credential(
      GetEnv("AZURE_TENANT_ID"), GetEnv("AZURE_CLIENT_ID"), GetEnv("AZURE_CLIENT_SECRET"));

  auto const token = credential.GetToken(
      {{GetEnv("AZURE_KEYVAULT_URL") + ".default"}}, Azure::Core::Context::GetApplicationContext());

  EXPECT_FALSE(token.Token.empty());
  EXPECT_GE(token.ExpiresOn, std::chrono::system_clock::now());
}
