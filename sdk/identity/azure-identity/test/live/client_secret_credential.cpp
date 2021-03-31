// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/identity/client_secret_credential.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <cstdlib>

inline std::string GetEnv(std::string const& name)
{
#if defined(_MSC_VER)
#pragma warning(push)
// warning C4996: 'getenv': This function or variable may be unsafe. Consider using _dupenv_s
// instead.
#pragma warning(disable : 4996)
#endif
  auto const result = std::getenv(name.c_str());
  return result != nullptr ? result : "";
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
}

TEST(ClientSecretCredential, Basic)
{
  Azure::Identity::ClientSecretCredential const credential(
      GetEnv("AZURE_TENANT_ID"), GetEnv("AZURE_CLIENT_ID"), GetEnv("AZURE_CLIENT_SECRET"));

  auto const token = credential.GetToken(
      {{"https://vault.azure.net/.default"}}, Azure::Core::Context::GetApplicationContext());

  EXPECT_FALSE(token.Token.empty());
  EXPECT_GE(token.ExpiresOn, std::chrono::system_clock::now());
}
