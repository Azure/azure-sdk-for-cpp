// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <credentials/credentials.hpp>

namespace azure
{

namespace credentials
{

class Credential::_internal
{
public:
  static std::string const& GetScopes(Credential const& credential)
  {
    return credential.GetScopes();
  }

  static void SetScopes(Credential& credential, std::string const& scopes)
  {
    credential.SetScopes(scopes);
  }
};

class TokenCredential::_internal
{
public:
  static std::string const& GetToken(TokenCredential const& credential)
  {
    return credential.GetToken();
  }

  static std::chrono::steady_clock const& GetTokenExpiration(TokenCredential const& credential)
  {
    return credential.GetTokenExpiration();
  }

  static void SetToken(
      TokenCredential& credential,
      std::string const& token,
      std::chrono::steady_clock const& expiration)
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

} // namespace credentials

} // namespace azure
