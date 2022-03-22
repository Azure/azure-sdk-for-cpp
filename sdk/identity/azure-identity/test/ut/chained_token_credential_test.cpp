// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/chained_token_credential.hpp"

#include <string>

#include <gtest/gtest.h>

using Azure::Identity::ChainedTokenCredential;

using Azure::Core::Context;
using Azure::Core::Credentials::AccessToken;
using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenCredential;
using Azure::Core::Credentials::TokenRequestContext;

namespace {
class TestCredential : public TokenCredential {
private:
  std::string m_token;

public:
  TestCredential(std::string token = "") : m_token(token) {}

  mutable bool WasInvoked = false;

  AccessToken GetToken(TokenRequestContext const&, Context const&) const override
  {
    WasInvoked = true;

    if (m_token.empty())
    {
      throw AuthenticationException("");
    }

    AccessToken token;
    token.Token = m_token;
    return token;
  }
};
} // namespace

TEST(ChainedTokenCredential, Success)
{
  auto c1 = std::make_shared<TestCredential>("Token1");
  auto c2 = std::make_shared<TestCredential>("Token2");
  ChainedTokenCredential cred(ChainedTokenCredential::Sources{c1, c2});

  EXPECT_FALSE(c1->WasInvoked);
  EXPECT_FALSE(c2->WasInvoked);

  auto token = cred.GetToken({}, {});
  EXPECT_EQ(token.Token, "Token1");

  EXPECT_TRUE(c1->WasInvoked);
  EXPECT_FALSE(c2->WasInvoked);
}

TEST(ChainedTokenCredential, Empty)
{
  ChainedTokenCredential cred(ChainedTokenCredential::Sources{});
  EXPECT_THROW(cred.GetToken({}, {}), AuthenticationException);
}

TEST(ChainedTokenCredential, ErrorThenSuccess)
{
  auto c1 = std::make_shared<TestCredential>();
  auto c2 = std::make_shared<TestCredential>("Token2");
  ChainedTokenCredential cred(ChainedTokenCredential::Sources{c1, c2});

  EXPECT_FALSE(c1->WasInvoked);
  EXPECT_FALSE(c2->WasInvoked);

  auto token = cred.GetToken({}, {});
  EXPECT_EQ(token.Token, "Token2");

  EXPECT_TRUE(c1->WasInvoked);
  EXPECT_TRUE(c2->WasInvoked);
}

TEST(ChainedTokenCredential, AllErrors)
{
  auto c1 = std::make_shared<TestCredential>();
  auto c2 = std::make_shared<TestCredential>();
  ChainedTokenCredential cred(ChainedTokenCredential::Sources{c1, c2});

  EXPECT_FALSE(c1->WasInvoked);
  EXPECT_FALSE(c2->WasInvoked);

  EXPECT_THROW(cred.GetToken({}, {}), AuthenticationException);

  EXPECT_TRUE(c1->WasInvoked);
  EXPECT_TRUE(c2->WasInvoked);
}
