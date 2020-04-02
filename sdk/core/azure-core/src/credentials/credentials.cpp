// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <credentials/credentials.hpp>

using namespace azure::core::credentials;

void TokenCredential::SetScopes(std::string const& scopes)
{
  std::lock_guard<std::mutex> const lock(this->m_tokenMutex);
  if (this->m_token.Scopes != scopes)
  {
    this->m_token = { scopes, {}, {} };
  }
}

TokenCredential::Token TokenCredential::GetToken() const
{
  std::lock_guard<std::mutex> const lock(this->m_tokenMutex);
  return this->m_token;
}

void TokenCredential::SetToken(
    std::string const& tokenString,
    std::chrono::system_clock::time_point const& expiresAt)
{
  this->SetToken({ this->m_token.Scopes, tokenString, expiresAt });
}

void TokenCredential::SetToken(Token const& token)
{
  std::lock_guard<std::mutex> const lock(this->m_tokenMutex);
  this->m_token = token;
}
