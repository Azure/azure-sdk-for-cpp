// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/chained_token_credential.hpp"

#include <azure/core/diagnostics/logger.hpp>

#include <string>
#include <utility>
#include <vector>

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
  TestCredential(std::string token = "") : TokenCredential("TestCredential"), m_token(token) {}

  mutable bool WasInvoked = false;

  AccessToken GetToken(TokenRequestContext const&, Context const&) const override
  {
    WasInvoked = true;

    if (m_token.empty())
    {
      throw AuthenticationException("Test Error");
    }

    AccessToken token;
    token.Token = m_token;
    return token;
  }
};
} // namespace

TEST(ChainedTokenCredential, GetCredentialName)
{
  ChainedTokenCredential const cred(ChainedTokenCredential::Sources{});
  EXPECT_EQ(cred.GetCredentialName(), "ChainedTokenCredential");
}

TEST(ChainedTokenCredential, Success)
{
  auto c1 = std::make_shared<TestCredential>("Token1");
  auto c2 = std::make_shared<TestCredential>("Token2");
  ChainedTokenCredential cred({c1, c2});

  EXPECT_FALSE(c1->WasInvoked);
  EXPECT_FALSE(c2->WasInvoked);

  auto token = cred.GetToken({}, {});
  EXPECT_EQ(token.Token, "Token1");

  EXPECT_TRUE(c1->WasInvoked);
  EXPECT_FALSE(c2->WasInvoked);
}

TEST(ChainedTokenCredential, Empty)
{
  ChainedTokenCredential cred({});
  EXPECT_THROW(cred.GetToken({}, {}), AuthenticationException);
}

TEST(ChainedTokenCredential, ErrorThenSuccess)
{
  auto c1 = std::make_shared<TestCredential>();
  auto c2 = std::make_shared<TestCredential>("Token2");
  ChainedTokenCredential cred({c1, c2});

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
  ChainedTokenCredential cred({c1, c2});

  EXPECT_FALSE(c1->WasInvoked);
  EXPECT_FALSE(c2->WasInvoked);

  EXPECT_THROW(cred.GetToken({}, {}), AuthenticationException);

  EXPECT_TRUE(c1->WasInvoked);
  EXPECT_TRUE(c2->WasInvoked);
}

TEST(ChainedTokenCredential, Logging)
{
  using Azure::Core::Diagnostics::Logger;
  using LogMsgVec = std::vector<std::pair<Logger::Level, std::string>>;
  LogMsgVec log;
  Logger::SetLevel(Logger::Level::Verbose);
  Logger::SetListener([&](auto lvl, auto msg) { log.push_back(std::make_pair(lvl, msg)); });

  {
    ChainedTokenCredential cred({});
    EXPECT_EQ(log.size(), LogMsgVec::size_type(1));
    EXPECT_EQ(log[0].first, Logger::Level::Warning);
    EXPECT_EQ(
        log[0].second,
        "Identity: ChainedTokenCredential: Created with EMPTY chain of credentials.");

    log.clear();
    EXPECT_THROW(static_cast<void>(cred.GetToken({}, {})), AuthenticationException);
    EXPECT_EQ(log.size(), LogMsgVec::size_type(1));
    EXPECT_EQ(log[0].first, Logger::Level::Warning);
    EXPECT_EQ(
        log[0].second,
        "Identity: ChainedTokenCredential: "
        "Authentication did not succeed: List of sources is empty.");
  }

  {
    log.clear();
    auto c = std::make_shared<TestCredential>();
    ChainedTokenCredential cred({c});
    EXPECT_EQ(log.size(), LogMsgVec::size_type(1));
    EXPECT_EQ(log[0].first, Logger::Level::Informational);
    EXPECT_EQ(
        log[0].second,
        "Identity: ChainedTokenCredential: Created with the following credentials: "
        "TestCredential.");

    log.clear();
    EXPECT_FALSE(c->WasInvoked);

    EXPECT_THROW(static_cast<void>(cred.GetToken({}, {})), AuthenticationException);
    EXPECT_TRUE(c->WasInvoked);

    EXPECT_EQ(log.size(), LogMsgVec::size_type(2));

    EXPECT_EQ(log[0].first, Logger::Level::Verbose);
    EXPECT_EQ(
        log[0].second,
        "Identity: ChainedTokenCredential: Failed to get token from TestCredential: "
        "Test Error");

    EXPECT_EQ(log[1].first, Logger::Level::Warning);
    EXPECT_EQ(
        log[1].second,
        "Identity: ChainedTokenCredential: "
        "Didn't succeed to get a token from any credential in the chain.");
  }

  {
    log.clear();
    auto c1 = std::make_shared<TestCredential>();
    auto c2 = std::make_shared<TestCredential>("Token2");
    ChainedTokenCredential cred({c1, c2});
    EXPECT_EQ(log.size(), LogMsgVec::size_type(1));
    EXPECT_EQ(log[0].first, Logger::Level::Informational);
    EXPECT_EQ(
        log[0].second,
        "Identity: ChainedTokenCredential: Created with the following credentials: "
        "TestCredential, TestCredential.");

    log.clear();
    EXPECT_FALSE(c1->WasInvoked);
    EXPECT_FALSE(c2->WasInvoked);

    auto token = cred.GetToken({}, {});
    EXPECT_EQ(token.Token, "Token2");

    EXPECT_TRUE(c1->WasInvoked);
    EXPECT_TRUE(c2->WasInvoked);

    EXPECT_EQ(log.size(), LogMsgVec::size_type(2));

    EXPECT_EQ(log[0].first, Logger::Level::Verbose);
    EXPECT_EQ(
        log[0].second,
        "Identity: ChainedTokenCredential: Failed to get token from TestCredential: "
        "Test Error");

    EXPECT_EQ(log[1].first, Logger::Level::Informational);
    EXPECT_EQ(
        log[1].second,
        "Identity: ChainedTokenCredential: Successfully got token from TestCredential.");
  }

  Logger::SetListener(nullptr);
}
