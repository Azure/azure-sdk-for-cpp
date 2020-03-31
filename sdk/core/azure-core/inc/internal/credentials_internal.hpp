// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <credentials/credentials.hpp>

namespace azure
{
namespace core
{
namespace credentials
{

class Credential::_internal
{
public:
  static void SetScopes(Credential& credential, std::string const& scopes)
  {
    credential.SetScopes(scopes);
  }
};

class TokenCredential::_internal
{
public:
  static std::shared_ptr<Token> GetToken(TokenCredential const& credential)
  {
    return credential.GetToken();
  }

  static void SetToken(
      TokenCredential& credential,
      std::string const& token,
      std::chrono::system_clock::time_point const& expiration)
  {
    credential.SetToken(token, expiration);
  }
};

class ClientSecretCredential::_internal
{
public:
  static std::string const& GetTenantId(ClientSecretCredential const& credential)
  {
    return credential.GetTenantId();
  }

  static std::string const& GetClientId(ClientSecretCredential const& credential)
  {
    return credential.GetClientId();
  }

  static std::string const& GetClientSecret(ClientSecretCredential const& credential)
  {
    return credential.GetClientSecret();
  }
};

struct TokenCredential::Token
{
  std::string const TokenString;
  std::string const Scopes;
  std::chrono::system_clock::time_point const ExpiresAt;

  explicit Token(std::string const& scopes) : Scopes(scopes){};

  Token(
      std::string const& scopes,
      std::string const& token,
      std::chrono::system_clock::time_point const expiresAt)
      : Scopes(scopes), TokenString(token), ExpiresAt(expiresAt){};

  static std::shared_ptr<Token> const Empty;

private:
  Token() {}
};

} // namespace credentials
} // namespace core
} // namespace azure
