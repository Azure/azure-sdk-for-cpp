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

class Credential::Internal
{
public:
  static void SetScopes(Credential& credential, std::string const& scopes)
  {
    credential.SetScopes(scopes);
  }
};

class TokenCredential::Internal
{
public:
  static Token GetToken(TokenCredential const& credential)
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

class ClientSecretCredential::Internal
{
public:
  static std::string const& GetTenantId(ClientSecretCredential const& credential)
  {
    return credential.m_tenantId;
  }

  static std::string const& GetClientId(ClientSecretCredential const& credential)
  {
    return credential.m_clientId;
  }

  static std::string const& GetClientSecret(ClientSecretCredential const& credential)
  {
    return credential.m_clientSecret;
  }
};

} // namespace credentials
} // namespace core
} // namespace azure
