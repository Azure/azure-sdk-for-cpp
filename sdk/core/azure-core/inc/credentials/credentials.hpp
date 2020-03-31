// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
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
  struct Token
  {
  public:
    std::string Scopes;
    std::string TokenString;
    std::chrono::system_clock::time_point ExpiresAt;
  };

  Token m_token;
  mutable std::mutex m_tokenMutex;

  void SetScopes(std::string const& scopes) override;

  Token GetToken() const;

  void SetToken(
      std::string const& tokenString,
      std::chrono::system_clock::time_point const& expiresAt);

  void SetToken(Token const& token);

public:
  class Internal;

protected:
  TokenCredential() = default;

  TokenCredential(TokenCredential const& other) : Credential(other), m_token(other.GetToken()) {}

  TokenCredential& operator=(TokenCredential const& other)
  {
    this->SetToken(other.GetToken());
    return *this;
  }
};

class ClientSecretCredential : public TokenCredential
{
  std::string m_tenantId;
  std::string m_clientId;
  std::string m_clientSecret;

public:
  class Internal;

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
