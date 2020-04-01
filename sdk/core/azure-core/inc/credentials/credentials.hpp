// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <memory>
#include <mutex>
#include <string>

namespace azure
{
namespace core
{
namespace credentials
{

class Credential
{
  virtual void SetScopes(std::string const& scopes) { (void)scopes; }

public:
  class Internal;
  virtual ~Credential() noexcept = default;

protected:
  Credential() = default;

  Credential(Credential const& other) = default;
  Credential& operator=(Credential const& other) = default;
};

class TokenCredential : public Credential
{
  class TokenInfo;

  std::shared_ptr<TokenInfo> m_tokenInfo;
  std::mutex m_tokenInfoMutex;

  void SetScopes(std::string const& scopes) override;

  std::string GetToken();

  virtual bool IsTokenExpired(std::chrono::system_clock::time_point const& tokenExpiration) const;

  virtual void RefreshToken(
          std::string const& scopes,
          std::string& newTokenString,
          std::chrono::system_clock::time_point& newExpiration)
          = 0;

public:
  class Internal;

protected:
  TokenCredential() = default;
  TokenCredential(TokenCredential const& other) : Credential(other)
  {
  }

  TokenCredential& operator=(TokenCredential const& other) = default;
};

class ClientSecretCredential : public TokenCredential
{
  std::string m_tenantId;
  std::string m_clientId;
  std::string m_clientSecret;
  std::mutex m_mutex;

public:
  ClientSecretCredential(
      std::string const& tenantId,
      std::string const& clientId,
      std::string const& clientSecret)
      : m_tenantId(tenantId), m_clientId(clientId), m_clientSecret(clientSecret)
  {
  }

  ClientSecretCredential(ClientSecretCredential const& other)
      : TokenCredential(other), m_tenantId(other.m_tenantId), m_clientId(other.m_clientId),
        m_clientSecret(other.m_clientSecret)
  {
  }

  ClientSecretCredential& operator=(ClientSecretCredential const& other)
  {
    this->m_tenantId = other.m_tenantId;
    this->m_clientId = other.m_clientId;
    this->m_clientSecret = other.m_clientSecret;
    return *this;
  }
};

} // namespace credentials
} // namespace core
} // namespace azure
