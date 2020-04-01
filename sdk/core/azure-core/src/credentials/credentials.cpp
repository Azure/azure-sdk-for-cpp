// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <credentials/credentials.hpp>
#include <internal/credentials_internal.hpp>

using namespace azure::core::credentials;

void TokenCredential::SetScopes(std::string const& scopes)
{
  std::lock_guard<std::mutex> const lock(this->m_tokenInfoMutex);

  if (this->m_tokenInfo->Scopes != scopes)
  {
    this->m_tokenInfo = std::make_shared<TokenInfo>(scopes);
  }
}

std::string TokenCredential::GetToken()
{
  std::lock_guard<std::mutex> const lock(this->m_tokenInfoMutex);

  TokenInfo& tokenInfo = *this->m_tokenInfo;
  if (this->IsTokenExpired(tokenInfo.ExpiresAt))
  {
      std::string newTokenString;
      std::chrono::system_clock::time_point newExpiration;

      this->RefreshToken(tokenInfo.Scopes, newTokenString, newExpiration);
      tokenInfo.Update(newTokenString, newExpiration);
  }
}

bool TokenCredential::IsTokenExpired(std::chrono::system_clock::time_point const& tokenExpiration) const
{
  return tokenExpiration <= std::chrono::system_clock::now() - std::chrono::minutes(5);
}
