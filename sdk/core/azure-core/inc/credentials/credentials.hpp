// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <string>

namespace azure
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
  virtual std::string const& GetScopes() const;
  virtual void SetScopes(std::string const& scopes);
};

class TokenCredential : public Credential
{
public:
  class _internal;
  ~TokenCredential() override noexcept;

protected:
  TokenCredential();

  TokenCredential(TokenCredential const& other);
  TokenCredential& operator=(TokenCredential const& other);

private:
  std::string _token;
  std::string _scopes;
  std::chrono::steady_clock _expiresAt;

  std::string const& GetScopes() const override;
  void SetScopes(std::string const& scopes) override;

  std::string const& GetToken() const;
  std::chrono::steady_clock const& GetTokenExpiration() const;
  void SetToken(std::string const& token, std::chrono::steady_clock const& expiration);
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

} // namespace azure
