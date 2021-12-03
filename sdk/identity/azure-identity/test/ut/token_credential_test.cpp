// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "credential_base_test.hpp"

#include <gtest/gtest.h>

using namespace Azure::Identity::Test;
using namespace Azure::Identity;

TEST_F(TokenCredentialTest, ClientSecret)
{
  std::string const testName(GetTestName());
  auto const clientSecretCredential = GetClientSecretCredential(testName);

  auto const token = clientSecretCredential->GetToken(
      {{"https://vault.azure.net/.default"}}, Azure::Core::Context::ApplicationContext);

  EXPECT_FALSE(token.Token.empty());
  EXPECT_GE(token.ExpiresOn, std::chrono::system_clock::now());
}

TEST_F(TokenCredentialTest, EnvironmentCredential)
{
  std::string const testName(GetTestName());
  auto const clientSecretCredential = GetEnvironmentCredential(testName);

  auto const token = clientSecretCredential->GetToken(
      {{"https://vault.azure.net/.default"}}, Azure::Core::Context::ApplicationContext);

  EXPECT_FALSE(token.Token.empty());
  EXPECT_GE(token.ExpiresOn, std::chrono::system_clock::now());
}