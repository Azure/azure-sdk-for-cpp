// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <credentials/credentials.hpp>
#include <string>

using namespace azure::core::credentials;

Credential::Credential() = default;
Credential::~Credential() noexcept = default;
Credential::Credential(Credential const&) = default;
Credential& Credential::operator=(Credential const&) = default;

std::string const& Credential::GetScopes() const
{
  static std::string const empty;
  return empty;
}

void Credential::SetScopes(std::string const&) {}

TokenCredential::TokenCredential() = default;
TokenCredential::~TokenCredential() noexcept = default;
TokenCredential::TokenCredential(TokenCredential const&) = default;
TokenCredential& TokenCredential::operator=(TokenCredential const&) = default;

std::string const& TokenCredential::GetScopes() const { return this->_scopes; }

void TokenCredential::SetScopes(std::string const& scopes)
{
  // TODO: mutex
  this->_token = std::string();
  this->_expiresAt = std::chrono::system_clock::time_point();

  this->_scopes = scopes;
}

std::string const& TokenCredential::GetToken() const { return this->_token; }

std::chrono::system_clock::time_point const& TokenCredential::GetTokenExpiration() const
{
  return this->_expiresAt;
}

void TokenCredential::SetToken(
    std::string const& token,
    std::chrono::system_clock::time_point const& expiration)
{
  // TODO: mutex
  this->_token = token;
  this->_expiresAt = expiration;
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
