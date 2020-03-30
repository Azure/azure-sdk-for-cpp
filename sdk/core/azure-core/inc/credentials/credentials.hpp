// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <string>

namespace azure
{

namespace credentials
{

class Credential;

namespace _internal
{
void SetScopes(Credential& credential, std::string const& scopes);
} // namespace _internal

class Credential
{
  friend void _internal::SetScopes(Credential&, std::string const&);

public:
  virtual ~Credential() noexcept {};

protected:
  Credential() = default;

private:
  Credential(Credential const&) = delete;
  void operator=(Credential const&) = delete;

  virtual void SetScopes(std::string const&) {}
};

namespace _internal
{

inline void SetScopes(Credential& credential, std::string const& scopes)
{
  credential.SetScopes(scopes);
}

} // namespace _internal

class TokenCredential : public Credential
{
private:
  std::string _token;
  std::string _scopes;
  std::chrono::steady_clock _expiresAt;

  TokenCredential(TokenCredential const&) = delete;
  void operator=(TokenCredential const&) = delete;

  void SetScopes(std::string const& scopes) override { _scopes = scopes; }

public:
  ~TokenCredential() override noexcept {}

protected:
  TokenCredential() = default;
};

class ClientSecretCredential : public TokenCredential
{
private:
  std::string _tenantId;
  std::string _clientId;
  std::string _clientSecret;

  ClientSecretCredential(ClientSecretCredential const&) = delete;
  void operator=(ClientSecretCredential const&) = delete;

public:
  ClientSecretCredential(
      std::string const& tenantId,
      std::string const& clientId,
      std::string const& clientSecret)
      : _tenantId(tenantId), _clientId(clientId), _clientSecret(clientSecret)
  {
  }

  ~ClientSecretCredential() noexcept override = default;
};

} // namespace credentials

} // namespace azure
