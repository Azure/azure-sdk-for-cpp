// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <credentials/credentials.hpp>

#include <chrono>
#include <functional>
#include <mutex>
#include <string>

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

class TokenCredential::Token
{
  friend class TokenCredential;
  friend class detail::CredentialTest;

  std::string m_tokenString;
  std::chrono::system_clock::time_point m_expiresAt;
  std::mutex m_mutex;
};

class TokenCredential::Internal
{
public:
  static std::string GetToken(TokenCredential& credential) { return credential.GetToken(); }
};

class ClientSecretCredential::ClientSecret
{
  friend class ClientSecretCredential;
  friend class detail::CredentialTest;

  std::string m_tenantId;
  std::string m_clientId;
  std::string m_clientSecret;
  std::string m_scopes;

public:
  ClientSecret(
      std::string const& tenantId,
      std::string const& clientId,
      std::string const& clientSecret)
      : m_tenantId(tenantId), m_clientId(clientId), m_clientSecret(clientSecret)
  {
  }

  ClientSecret(
      std::string const& tenantId,
      std::string const& clientId,
      std::string const& clientSecret,
      std::string const& scopes)
      : m_tenantId(tenantId), m_clientId(clientId), m_clientSecret(clientSecret), m_scopes(scopes)
  {
  }
};

} // namespace credentials
} // namespace core
} // namespace azure
