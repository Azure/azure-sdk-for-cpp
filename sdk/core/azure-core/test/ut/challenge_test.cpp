// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/http/policies/policy.hpp>
#include <gtest/gtest.h>

using namespace Azure::Core;
using namespace Azure::Core::Http::Policies::_internal;

TEST(ChallengeParameters, emptyString)
{
  ChallengeParameters challenge("");
  EXPECT_TRUE(challenge.Schema.empty());
  EXPECT_TRUE(challenge.AuthorizationUri.GetPath().empty());
  EXPECT_EQ(challenge.Scopes.size(), size_t(0));
  EXPECT_TRUE(challenge.TenantId.empty());
  EXPECT_TRUE(challenge.IsEmpty());
}

TEST(ChallengeParameters, invalidString1)
{
  ChallengeParameters challenge("Anna has apples");
  EXPECT_TRUE(challenge.Schema.empty());
  EXPECT_TRUE(challenge.AuthorizationUri.GetPath().empty());
  EXPECT_EQ(challenge.Scopes.size(), size_t(0));
  EXPECT_TRUE(challenge.TenantId.empty());
  EXPECT_TRUE(challenge.IsEmpty());
}

TEST(ChallengeParameters, invalidString2)
{
  ChallengeParameters challenge("Bearer has apples");
  EXPECT_FALSE(challenge.Schema.empty());
  EXPECT_EQ(challenge.Schema, Azure::Core::Http::Policies::_detail::BearerName);
  EXPECT_TRUE(challenge.AuthorizationUri.GetPath().empty());
  EXPECT_EQ(challenge.Scopes.size(), size_t(0));
  EXPECT_TRUE(challenge.TenantId.empty());
  EXPECT_FALSE(challenge.IsEmpty());
}

TEST(ChallengeParameters, validString)
{
  std::string validData
      = "Bearer authorization=\"https://login.windows.net/72f988bf-86f1-41af-91ab-2d7cd011db47\", "
        "resource=\"https://vault.azure.net\"";
  ChallengeParameters challenge(validData);

  EXPECT_EQ(challenge.Schema, Azure::Core::Http::Policies::_detail::BearerName);
  EXPECT_EQ(
      challenge.AuthorizationUri.GetPath(),
      "72f988bf-86f1-41af-91ab-2d7cd011db47/oauth2/v2.0/token");
  EXPECT_EQ(challenge.Scopes.size(), size_t(1));
  EXPECT_EQ(challenge.Scopes[0], "https://vault.azure.net/.default");
  EXPECT_EQ(challenge.TenantId, "72f988bf-86f1-41af-91ab-2d7cd011db47");
  EXPECT_FALSE(challenge.IsEmpty());
}

TEST(ChallengeParameters, validStringAlt1)
{
  std::string validData
      = "Bearer "
        "authorization_uri=\"https://login.windows.net/72f988bf-86f1-41af-91ab-2d7cd011db47\", "
        "resource=\"https://vault.azure.net\"";
  ChallengeParameters challenge(validData);

  EXPECT_EQ(challenge.Schema, Azure::Core::Http::Policies::_detail::BearerName);
  EXPECT_EQ(
      challenge.AuthorizationUri.GetPath(),
      "72f988bf-86f1-41af-91ab-2d7cd011db47/oauth2/v2.0/token");
  EXPECT_EQ(challenge.Scopes.size(), size_t(1));
  EXPECT_EQ(challenge.Scopes[0], "https://vault.azure.net/.default");
  EXPECT_EQ(challenge.TenantId, "72f988bf-86f1-41af-91ab-2d7cd011db47");
  EXPECT_FALSE(challenge.IsEmpty());
}

TEST(ChallengeParameters, validStringAlt2)
{
  std::string validData
      = "Bearer "
        "authorization_uri=\"https://login.windows.net/72f988bf-86f1-41af-91ab-2d7cd011db47\", "
        "scope=\"https://vault.azure.net/.default\"";
  ChallengeParameters challenge(validData);

  EXPECT_EQ(challenge.Schema, Azure::Core::Http::Policies::_detail::BearerName);
  EXPECT_EQ(
      challenge.AuthorizationUri.GetPath(),
      "72f988bf-86f1-41af-91ab-2d7cd011db47/oauth2/v2.0/token");
  EXPECT_EQ(challenge.Scopes.size(), size_t(1));
  EXPECT_EQ(challenge.Scopes[0], "https://vault.azure.net/.default");
  EXPECT_EQ(challenge.TenantId, "72f988bf-86f1-41af-91ab-2d7cd011db47");
  EXPECT_FALSE(challenge.IsEmpty());
}

TEST(ChallengeParameters, validString2)
{
  std::string validData
      = "Bearer authorization=\"https://login.windows.net/72f988bf-86f1-41af-91ab-2d7cd011db47\"";
  ChallengeParameters challenge(validData);
  EXPECT_EQ(challenge.Schema, Azure::Core::Http::Policies::_detail::BearerName);
  EXPECT_EQ(
      challenge.AuthorizationUri.GetPath(),
      "72f988bf-86f1-41af-91ab-2d7cd011db47/oauth2/v2.0/token");
  EXPECT_EQ(challenge.Scopes.size(), size_t(0));
  EXPECT_EQ(challenge.TenantId, "72f988bf-86f1-41af-91ab-2d7cd011db47");
  EXPECT_FALSE(challenge.IsEmpty());
}

TEST(ChallengeParameters, validString3)
{
  std::string validData = "Bearer resource=\"https://vault.azure.net\"";
  ChallengeParameters challenge(validData);
  EXPECT_EQ(challenge.Schema, Azure::Core::Http::Policies::_detail::BearerName);
  EXPECT_TRUE(challenge.AuthorizationUri.GetPath().empty());
  EXPECT_EQ(challenge.Scopes.size(), size_t(1));
  EXPECT_EQ(challenge.Scopes[0], "https://vault.azure.net/.default");
  EXPECT_TRUE(challenge.TenantId.empty());
  EXPECT_FALSE(challenge.IsEmpty());
}

TEST(ChallengeParameters, validStringPartial1)
{
  std::string validData
      = "Bearer authorization=\"https://login.windows.net/72f988bf-86f1-41af-91ab-2d7cd011db47\" "
        "apples";
  ChallengeParameters challenge(validData);
  EXPECT_EQ(challenge.Schema, Azure::Core::Http::Policies::_detail::BearerName);
  EXPECT_EQ(
      challenge.AuthorizationUri.GetPath(),
      "72f988bf-86f1-41af-91ab-2d7cd011db47/oauth2/v2.0/token");
  EXPECT_EQ(challenge.Scopes.size(), size_t(0));
  EXPECT_EQ(challenge.TenantId, "72f988bf-86f1-41af-91ab-2d7cd011db47");
  EXPECT_FALSE(challenge.IsEmpty());
}

TEST(ChallengeParameters, validStringPartial2)
{
  std::string validData = "Bearer resource=\"https://vault.azure.net\" apples";
  ChallengeParameters challenge(validData);
  EXPECT_EQ(challenge.Schema, Azure::Core::Http::Policies::_detail::BearerName);
  EXPECT_TRUE(challenge.AuthorizationUri.GetPath().empty());
  EXPECT_EQ(challenge.Scopes.size(), size_t(1));
  EXPECT_EQ(challenge.Scopes[0], "https://vault.azure.net/.default");
  EXPECT_TRUE(challenge.TenantId.empty());
  EXPECT_FALSE(challenge.IsEmpty());
}

TEST(ChallengeParameters, validStringPartial3)
{
  std::string validData
      = "Bearer apples "
        "authorization=\"https://login.windows.net/72f988bf-86f1-41af-91ab-2d7cd011db47\" "
        "apples";
  ChallengeParameters challenge(validData);
  EXPECT_EQ(challenge.Schema, Azure::Core::Http::Policies::_detail::BearerName);
  EXPECT_EQ(
      challenge.AuthorizationUri.GetPath(),
      "72f988bf-86f1-41af-91ab-2d7cd011db47/oauth2/v2.0/token");
  EXPECT_EQ(challenge.Scopes.size(), size_t(0));
  EXPECT_EQ(challenge.TenantId, "72f988bf-86f1-41af-91ab-2d7cd011db47");
  EXPECT_FALSE(challenge.IsEmpty());
}

TEST(ChallengeParameters, validStringPartial4)
{
  std::string validData = "Bearer apples resource=\"https://vault.azure.net\" apples";
  ChallengeParameters challenge(validData);
  EXPECT_EQ(challenge.Schema, Azure::Core::Http::Policies::_detail::BearerName);
  EXPECT_TRUE(challenge.AuthorizationUri.GetPath().empty());
  EXPECT_EQ(challenge.Scopes.size(), size_t(1));
  EXPECT_EQ(challenge.Scopes[0], "https://vault.azure.net/.default");
  EXPECT_TRUE(challenge.TenantId.empty());
  EXPECT_FALSE(challenge.IsEmpty());
}