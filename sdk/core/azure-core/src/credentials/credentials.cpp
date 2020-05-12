// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <credentials/credentials.hpp>
#include <internal/credentials_internal.hpp>

using namespace Azure::Core::Credentials;

std::string TokenCredential::UpdateTokenNonThreadSafe(Token& token)
{
  std::string newTokenString;
  std::chrono::system_clock::time_point newExpiration;

  this->RefreshToken(newTokenString, newExpiration);

  token.m_tokenString = newTokenString;
  token.m_expiresAt = newExpiration;

  return newTokenString;
}

bool TokenCredential::IsTokenExpired(
    std::chrono::system_clock::time_point const& tokenExpiration) const
{
  return tokenExpiration <= std::chrono::system_clock::now() - std::chrono::minutes(5);
}

TokenCredential::TokenCredential(TokenCredential const& other) : Credential(other)
{
  this->Init(other);
}

TokenCredential& TokenCredential::operator=(TokenCredential const& other)
{
  std::lock_guard<std::mutex> const thisTokenPtrLock(this->m_mutex);
  this->Init(other);
  return *this;
}

void TokenCredential::Init(TokenCredential const& other)
{
  std::lock_guard<std::mutex> const otherTokenPtrLock(const_cast<std::mutex&>(other.m_mutex));
  this->m_token = other.m_token;
}

std::string TokenCredential::GetToken()
{
  std::lock_guard<std::mutex> const tokenPtrLock(this->m_mutex);

  if (!this->m_token)
  {
    this->m_token = std::make_shared<Token>();
    return UpdateTokenNonThreadSafe(*this->m_token);
  }

  std::lock_guard<std::mutex> const tokenLock(this->m_token->m_mutex);
  Token& token = *this->m_token;
  return this->IsTokenExpired(token.m_expiresAt) ? UpdateTokenNonThreadSafe(token)
                                                 : token.m_tokenString;
}

void TokenCredential::ResetToken()
{
  std::lock_guard<std::mutex> const tokenPtrLock(this->m_mutex);
  this->m_token.reset();
}

void ClientSecretCredential::SetScopes(std::string const& scopes)
{
  std::lock_guard<std::mutex> const clientSecretPtrLock(this->m_mutex);

  if (scopes == this->m_clientSecret->m_scopes)
    return;

  this->TokenCredential::ResetToken();

  if (this->m_clientSecret.unique())
  {
    this->m_clientSecret->m_scopes = scopes;
    return;
  }

  this->m_clientSecret = std::make_shared<ClientSecret>(
      this->m_clientSecret->m_tenantId,
      this->m_clientSecret->m_clientId,
      this->m_clientSecret->m_clientSecret,
      scopes);
}

std::string ClientSecretCredential::GetToken()
{
  std::lock_guard<std::mutex> const clientSecretPtrLock(this->m_mutex);
  return this->TokenCredential::GetToken();
}

void ClientSecretCredential::RefreshToken(
    std::string& newTokenString,
    std::chrono::system_clock::time_point& newExpiration)
{
  // TODO: get token using scopes, tenantId, clientId, and clientSecretId.
  (void)newTokenString;
  (void)newExpiration;
}

ClientSecretCredential::ClientSecretCredential(
    std::string const& tenantId,
    std::string const& clientId,
    std::string const& clientSecret)
    : m_clientSecret(new ClientSecret(tenantId, clientId, clientSecret))
{
}

ClientSecretCredential::ClientSecretCredential(ClientSecretCredential const& other)
    : TokenCredential(other, 0)
{
  std::lock_guard<std::mutex> const otherClientSecretPtrLock(
      const_cast<std::mutex&>(other.m_mutex));
  this->TokenCredential::Init(other);
}

ClientSecretCredential& ClientSecretCredential::operator=(ClientSecretCredential const& other)
{
  std::lock_guard<std::mutex> const otherClientSecretPtrLock(this->m_mutex);
  this->TokenCredential::operator=(other);
  return *this;
}
