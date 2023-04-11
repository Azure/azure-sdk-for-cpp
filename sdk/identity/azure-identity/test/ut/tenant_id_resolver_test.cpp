// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/tenant_id_resolver.hpp"

#include "credential_test_helper.hpp"

#include <gtest/gtest.h>

using Azure::Identity::_detail::TenantIdResolver;

using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenRequestContext;
using Azure::Identity::Test::_detail::CredentialTestHelper;

TEST(TenantIdResolver, RequestedTenantIdEmpty)
{
  CredentialTestHelper::EnvironmentOverride const env(std::map<std::string, std::string>{
      {"AZURE_IDENTITY_DISABLE_MULTITENANTAUTH", ""}, // Default, i.e. NOT disabled
  });

  auto const tenantId = TenantIdResolver::Resolve("aA", {}, {});

  EXPECT_EQ(tenantId, "aA");
}

TEST(TenantIdResolver, RequestedTenantIdEqualsExplicitTenantId)
{
  CredentialTestHelper::EnvironmentOverride const env(std::map<std::string, std::string>{
      {"AZURE_IDENTITY_DISABLE_MULTITENANTAUTH", "0"}, // Default, i.e. NOT disabled
  });

  TokenRequestContext trc;
  trc.TenantId = "Aa";

  auto const tenantId = TenantIdResolver::Resolve("aA", trc, {});

  EXPECT_EQ(tenantId, "aA");
}

TEST(TenantIdResolver, Adfs)
{
  CredentialTestHelper::EnvironmentOverride const env(std::map<std::string, std::string>{
      {"AZURE_IDENTITY_DISABLE_MULTITENANTAUTH", "false"}, // Default, i.e. NOT disabled
  });

  TokenRequestContext trc;
  trc.TenantId = "bB";

  auto const tenantId = TenantIdResolver::Resolve("aDfS", trc, {});

  EXPECT_EQ(tenantId, "aDfS");
}

TEST(TenantIdResolver, Disabled1)
{
  CredentialTestHelper::EnvironmentOverride const env(std::map<std::string, std::string>{
      {"AZURE_IDENTITY_DISABLE_MULTITENANTAUTH", "1"}, // Should be DISABLED
  });

  TokenRequestContext trc;
  trc.TenantId = "bB";

  auto const tenantId = TenantIdResolver::Resolve("aA", trc, {});

  EXPECT_EQ(tenantId, "aA");
}

TEST(TenantIdResolver, DisabledTrue)
{
  CredentialTestHelper::EnvironmentOverride const env(std::map<std::string, std::string>{
      {"AZURE_IDENTITY_DISABLE_MULTITENANTAUTH", "tRuE"}, // Should be DISABLED
  });

  TokenRequestContext trc;
  trc.TenantId = "bB";

  auto const tenantId = TenantIdResolver::Resolve("aA", trc, {});

  EXPECT_EQ(tenantId, "aA");
}

TEST(TenantIdResolver, Wildcard)
{
  CredentialTestHelper::EnvironmentOverride const env(std::map<std::string, std::string>{
      {"AZURE_IDENTITY_DISABLE_MULTITENANTAUTH", "2"}, // Not a value that should be recognized
  });

  TokenRequestContext trc;
  trc.TenantId = "bB";

  auto const tenantId = TenantIdResolver::Resolve("aA", trc, {"cC", "*", "dD"});

  EXPECT_EQ(tenantId, "bB");
}

TEST(TenantIdResolver, Match)
{
  CredentialTestHelper::EnvironmentOverride const env(std::map<std::string, std::string>{
      {"AZURE_IDENTITY_DISABLE_MULTITENANTAUTH", "T"}, // Not a value that should be recognized
  });

  TokenRequestContext trc;
  trc.TenantId = "bB";

  auto const tenantId = TenantIdResolver::Resolve("bA", trc, {"cC", "Bb", "dD"});

  EXPECT_EQ(tenantId, "bB");
}

TEST(TenantIdResolver, NoMatch)
{
  CredentialTestHelper::EnvironmentOverride const env(std::map<std::string, std::string>{
      {"AZURE_IDENTITY_DISABLE_MULTITENANTAUTH", "yes"}, // Not a value that should be recognized
  });

  TokenRequestContext trc;
  trc.TenantId = "bB";

  EXPECT_THROW(
      static_cast<void>(TenantIdResolver::Resolve("aA", trc, {"cC", "dD"})),
      AuthenticationException);
}

TEST(TenantIdResolver, NoMatchEmpty)
{
  CredentialTestHelper::EnvironmentOverride const env(std::map<std::string, std::string>{
      {"AZURE_IDENTITY_DISABLE_MULTITENANTAUTH", "on"}, // Not a value that should be recognized
  });

  TokenRequestContext trc;
  trc.TenantId = "bB";

  EXPECT_THROW(
      static_cast<void>(TenantIdResolver::Resolve("aA", trc, {})), AuthenticationException);
}
