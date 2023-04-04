// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/internal/credentials/authorization_challenge_parser.hpp>

#include <gtest/gtest.h>

using Azure::Core::Credentials::_detail::AuthorizationChallengeHelper;
using Azure::Core::Credentials::_internal::AuthorizationChallengeParser;

using Azure::Core::Http::HttpStatusCode;
using Azure::Core::Http::RawResponse;

namespace {
RawResponse CreateRawResponseWithWwwAuthHeader(
    std::string const& value,
    HttpStatusCode httpStatusCode = HttpStatusCode::Unauthorized)
{
  RawResponse result(1, 1, httpStatusCode, "Test");
  result.SetHeader("WWW-Authenticate", value);
  return result;
}

std::string GetChallengeParameterFromResponse(
    RawResponse const& response,
    std::string const& challengeScheme,
    std::string const& challengeParameter)
{
  return AuthorizationChallengeParser::GetChallengeParameter(
      AuthorizationChallengeHelper::GetChallenge(response), challengeScheme, challengeParameter);
}
} // namespace

TEST(AuthorizationChallengeParser, Simple)
{
  EXPECT_EQ(
      GetChallengeParameterFromResponse(
          CreateRawResponseWithWwwAuthHeader("Bearer key=value"), "Bearer", "key"),
      "value");
}

TEST(AuthorizationChallengeParser, EmptyString)
{
  EXPECT_EQ(
      GetChallengeParameterFromResponse(CreateRawResponseWithWwwAuthHeader(""), "Bearer", "key"),
      "");
}

TEST(AuthorizationChallengeParser, Non401)
{
  EXPECT_EQ(
      GetChallengeParameterFromResponse(
          CreateRawResponseWithWwwAuthHeader("Bearer key=value", HttpStatusCode::Ok),
          "Bearer",
          "key"),
      "");
}

TEST(AuthorizationChallengeParser, NoHeader)
{
  EXPECT_EQ(
      GetChallengeParameterFromResponse(
          RawResponse(1, 1, HttpStatusCode::Unauthorized, "Test"), "Bearer", "key"),
      "");
}

TEST(AuthorizationChallengeParser, KeyNotFound)
{
  EXPECT_EQ(
      GetChallengeParameterFromResponse(
          CreateRawResponseWithWwwAuthHeader("Bearer otherkey=value"), "Bearer", "key"),
      "");
}

TEST(AuthorizationChallengeParser, SchemeNotFound)
{
  EXPECT_EQ(
      GetChallengeParameterFromResponse(
          CreateRawResponseWithWwwAuthHeader("Basic key=value"), "Bearer", "key"),
      "");
}

TEST(AuthorizationChallengeParser, NotFoundForScheme)
{
  EXPECT_EQ(
      GetChallengeParameterFromResponse(
          CreateRawResponseWithWwwAuthHeader("Basic key=value, Bearer otherkey=value"),
          "Bearer",
          "key"),
      "");
}

TEST(AuthorizationChallengeParser, MultiplSchemeMatch)
{
  EXPECT_EQ(
      GetChallengeParameterFromResponse(
          CreateRawResponseWithWwwAuthHeader(
              "Basic key=value1, Bearer key=value2, Digest key=value3"),
          "Bearer",
          "key"),
      "value2");
}

TEST(AuthorizationChallengeParser, Quoted)
{
  EXPECT_EQ(
      GetChallengeParameterFromResponse(
          CreateRawResponseWithWwwAuthHeader("Bearer key=\"v a l u e\""), "Bearer", "key"),
      "v a l u e");
}

TEST(AuthorizationChallengeParser, CaeInsufficientClaimsChallenge)
{
  auto const response = CreateRawResponseWithWwwAuthHeader(
      "Bearer realm=\"\", "
      "authorization_uri=\"https://login.microsoftonline.com/common/oauth2/authorize\", "
      "client_id=\"00000003-0000-0000-c000-000000000000\", error=\"insufficient_claims\", "
      "claims=\"eyJhY2Nlc3NfdG9rZW4iOiB7ImZvbyI6ICJiYXIifX0=\"");

  EXPECT_EQ(GetChallengeParameterFromResponse(response, "Bearer", "realm"), "");

  EXPECT_EQ(
      GetChallengeParameterFromResponse(response, "Bearer", "authorization_uri"),
      "https://login.microsoftonline.com/common/oauth2/authorize");

  EXPECT_EQ(
      GetChallengeParameterFromResponse(response, "Bearer", "client_id"),
      "00000003-0000-0000-c000-000000000000");

  EXPECT_EQ(GetChallengeParameterFromResponse(response, "Bearer", "error"), "insufficient_claims");

  EXPECT_EQ(
      GetChallengeParameterFromResponse(response, "Bearer", "claims"),
      "eyJhY2Nlc3NfdG9rZW4iOiB7ImZvbyI6ICJiYXIifX0=");
}

TEST(AuthorizationChallengeParser, CaeSessionsRevokedClaimsChallenge)
{
  auto const response = CreateRawResponseWithWwwAuthHeader(
      "Bearer authorization_uri=\"https://login.windows-ppe.net/\", error=\"invalid_token\", "
      "error_description=\"User session has been revoked\", "
      "claims="
      "\"eyJhY2Nlc3NfdG9rZW4iOnsibmJmIjp7ImVzc2VudGlhbCI6dHJ1ZSwgInZhbHVlIjoiMTYwMzc0MjgwMCJ9fX0="
      "\"");

  EXPECT_EQ(
      GetChallengeParameterFromResponse(response, "Bearer", "authorization_uri"),
      "https://login.windows-ppe.net/");

  EXPECT_EQ(GetChallengeParameterFromResponse(response, "Bearer", "error"), "invalid_token");

  EXPECT_EQ(
      GetChallengeParameterFromResponse(response, "Bearer", "error_description"),
      "User session has been revoked");

  EXPECT_EQ(
      GetChallengeParameterFromResponse(response, "Bearer", "claims"),
      "eyJhY2Nlc3NfdG9rZW4iOnsibmJmIjp7ImVzc2VudGlhbCI6dHJ1ZSwgInZhbHVlIjoiMTYwMzc0MjgwMCJ9fX0=");
}

TEST(AuthorizationChallengeParser, KeyVaultChallenge)
{
  auto const response = CreateRawResponseWithWwwAuthHeader(
      "Bearer "
      "authorization=\"https://login.microsoftonline.com/72f988bf-86f1-41af-91ab-2d7cd011db47\", "
      "resource=\"https://vault.azure.net\"");

  EXPECT_EQ(
      GetChallengeParameterFromResponse(response, "Bearer", "authorization"),
      "https://login.microsoftonline.com/72f988bf-86f1-41af-91ab-2d7cd011db47");

  EXPECT_EQ(
      GetChallengeParameterFromResponse(response, "Bearer", "resource"), "https://vault.azure.net");
}

TEST(AuthorizationChallengeParser, ArmChallenge)
{
  auto const response = CreateRawResponseWithWwwAuthHeader(
      "Bearer authorization_uri=\"https://login.windows.net/\", error=\"invalid_token\", "
      "error_description=\"The authentication failed because of missing 'Authorization' header.\"");

  EXPECT_EQ(
      GetChallengeParameterFromResponse(response, "Bearer", "authorization_uri"),
      "https://login.windows.net/");

  EXPECT_EQ(GetChallengeParameterFromResponse(response, "Bearer", "error"), "invalid_token");

  EXPECT_EQ(
      GetChallengeParameterFromResponse(response, "Bearer", "error_description"),
      "The authentication failed because of missing 'Authorization' header.");
}

TEST(AuthorizationChallengeParser, StorageChallenge)
{
  auto const response = CreateRawResponseWithWwwAuthHeader(
      "Bearer "
      "authorization_uri=https://login.microsoftonline.com/72f988bf-86f1-41af-91ab-2d7cd011db47/"
      "oauth2/authorize resource_id=https://storage.azure.com");

  EXPECT_EQ(
      GetChallengeParameterFromResponse(response, "Bearer", "authorization_uri"),
      "https://login.microsoftonline.com/72f988bf-86f1-41af-91ab-2d7cd011db47/oauth2/authorize");

  EXPECT_EQ(
      GetChallengeParameterFromResponse(response, "Bearer", "resource_id"),
      "https://storage.azure.com");
}
