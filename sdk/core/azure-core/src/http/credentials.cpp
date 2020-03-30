// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <credentials/credentials.hpp>
#include <string>

namespace azure
{

namespace credentials
{

Credential::Credential() = default;
Credential::~Credential() noexcept = default;
Credential::Credential(Credential const&) = default;
Credential& Credential::operator=(Credential const&) = default;

std::string const& Credential::GetScopes() const
{
  static std::string empty;
  return empty;
}

void Credential::SetScopes(std::string const&) {}

TokenCredential::TokenCredential() = default;
TokenCredential::~TokenCredential() noexcept = default;
TokenCredential::TokenCredential(TokenCredential const&) = default;
TokenCredential& TokenCredential::operator=(TokenCredential const&) = default;

std::string const& TokenCredential::GetScopes() { return this->scopes; }
void TokenCredential::SetScopes(std::string const& scopes) { this->_scopes = scopes; }

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

}; // namespace credentials

} // namespace azure

} // namespace azure
