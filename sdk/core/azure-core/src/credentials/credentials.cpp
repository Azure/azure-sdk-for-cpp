// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <credentials/credentials.hpp>
#include <internal/credentials_internal.hpp>

using namespace azure::core::credentials;

Credential::Credential() = default;
Credential::~Credential() noexcept = default;
Credential::Credential(Credential const&) = default;
Credential& Credential::operator=(Credential const&) = default;
void Credential::SetScopes(std::string const&) {}

std::shared_ptr<TokenCredential::Token> const TokenCredential::Token::Empty
    = std::shared_ptr<TokenCredential::Token>(new Token());

TokenCredential::TokenCredential() : _token(Token::Empty) {}
TokenCredential::~TokenCredential() noexcept = default;
TokenCredential::TokenCredential(TokenCredential const&) = default;
TokenCredential& TokenCredential::operator=(TokenCredential const&) = default;

void TokenCredential::SetScopes(std::string const& scopes)
{
  if (scopes != this->_token->Scopes)
  {
    this->_token = std::shared_ptr<Token>(new Token(scopes));
  }
}

std::shared_ptr<TokenCredential::Token> TokenCredential::GetToken() const { return this->_token; }

void TokenCredential::SetToken(
    std::string const& token,
    std::chrono::system_clock::time_point const& expiration)
{
  this->_token = std::shared_ptr<Token>(new Token(this->_token->Scopes, token, expiration));
}

ClientSecretCredential::ClientSecretCredential(
    std::string const& tenantId,
    std::string const& clientId,
    std::string const& clientSecret)
    : _tenantId(tenantId), _clientId(clientId), _clientSecret(clientSecret)
{
}

ClientSecretCredential::~ClientSecretCredential() noexcept = default;
ClientSecretCredential::ClientSecretCredential(ClientSecretCredential const&) = default;
ClientSecretCredential& ClientSecretCredential::operator=(ClientSecretCredential const&) = default;

std::string const& ClientSecretCredential::GetTenantId() const { return this->_tenantId; }
std::string const& ClientSecretCredential::GetClientId() const { return this->_clientId; }
std::string const& ClientSecretCredential::GetClientSecret() const { return this->_clientSecret; }
