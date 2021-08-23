// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <gtest/gtest.h>

#include <vector>

using namespace Azure::Core::Http::Policies::_internal;
using namespace Azure::Core;

TEST(TokenScopes, GenerateScope)
{
  auto scopes = TokenScopes::GetScopeFromUrl(Url("https://account.managedhsm.azure.net"));
  ASSERT_EQ(1, scopes.size());
  ASSERT_EQ("https://managedhsm.azure.net/.default", scopes[0]);
}

TEST(TokenScopes, GenerateScopeUrlWithPath)
{
  auto scopes = TokenScopes::GetScopeFromUrl(Url("https://account.managedhsm.azure.net/some/path"));
  ASSERT_EQ(1, scopes.size());
  ASSERT_EQ("https://managedhsm.azure.net/.default", scopes[0]);
}

TEST(TokenScopes, GenerateIgnoreNoDefault)
{
  auto scopes = TokenScopes::GetScopeFromUrl(Url("https://account"));
  ASSERT_EQ(0, scopes.size());
}

TEST(TokenScopes, GenerateIgnoreWithPort)
{
  auto scopes = TokenScopes::GetScopeFromUrl(Url("https://account:8080"));
  ASSERT_EQ(0, scopes.size());
}
