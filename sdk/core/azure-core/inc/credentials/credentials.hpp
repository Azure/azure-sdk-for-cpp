// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <memory>
#include <string>

namespace azure
{
namespace core
{
namespace credentials
{

class Credential
{
public:
  class _internal;
  virtual ~Credential() noexcept;

protected:
  Credential();

  Credential(Credential const& other);
  Credential& operator=(Credential const& other);

private:
  virtual void SetScopes(std::string const& scopes);
};

class TokenCredential : public Credential
{
public:
  class _internal;
  ~TokenCredential() noexcept override;

protected:
  TokenCredential();

  TokenCredential(TokenCredential const& other);
  TokenCredential& operator=(TokenCredential const& other);

private:
  struct Token;
  std::shared_ptr<Token> _token;

  void SetScopes(std::string const& scopes) override;

  std::shared_ptr<Token> GetToken() const;
  void SetToken(std::string const& token, std::chrono::system_clock::time_point const& expiration);
};

class ClientSecretCredential : public TokenCredential
{
public:
  class _internal;

  ClientSecretCredential(
      std::string const& tenantId,
      std::string const& clientId,
      std::string const& clientSecret);

  ~ClientSecretCredential() noexcept override;

  ClientSecretCredential(ClientSecretCredential const& other);
  ClientSecretCredential& operator=(ClientSecretCredential const& other);

private:
  std::string _tenantId;
  std::string _clientId;
  std::string _clientSecret;

  std::string const& GetTenantId() const;
  std::string const& GetClientId() const;
  std::string const& GetClientSecret() const;
};

} // namespace credentials
} // namespace core
} // namespace azure
