// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <credentials/credentials.hpp>

#include <mutex>

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

class TokenCredential::TokenInfo
{
  friend class TokenCredential;

public:
  std::string const Scopes;
  std::string TokenString;
  std::chrono::system_clock::time_point ExpiresAt;

private:
  explicit TokenInfo(std::string const& scopes) : Scopes(scopes) {}

  void Update(
      std::string& tokenString,
      std::chrono::system_clock::time_point const& expiresAt)
  {
    this->TokenString = tokenString;
    this->ExpiresAt = expiresAt;
  }
};

class TokenCredential::Internal
{
public:
  static std::string GetToken(TokenCredential& credential)
  {
    return credential.GetToken();
  }
};

} // namespace credentials
} // namespace core
} // namespace azure
