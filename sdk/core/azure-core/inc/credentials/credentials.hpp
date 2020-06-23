// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "context.hpp"

#include <chrono>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace Azure { namespace Core { namespace Credentials {

  struct AccessToken
  {
    std::string Token;
    std::chrono::system_clock::time_point ExpiresOn;
  };

  class TokenCredential {
  public:
    virtual AccessToken GetToken(
        Azure::Core::Context& context,
        std::vector<std::string const> const& scopes) const = 0;

  private:
    TokenCredential(TokenCredential const&) = delete;
    void operator=(TokenCredential const&) = delete;
  };

  class ClientSecretCredential : public Azure::Core::TokenCredential {
  private:
    std::string const m_tenantId;
    std::string const m_clientId;
    std::string const m_clientSecret;

  public:
    ClientSecretCredential(
        std::string const& tenantId,
        std::string const& clientId,
        std::string const& clientSecret)
        : m_tenantId(tenantId), m_clientId(clientId), m_clientSecret(clientSecret)
    {
    }

    ClientSecretCredential(
        std::string const&& tenantId,
        std::string const& clientId,
        std::string const& clientSecret)
        : m_tenantId(std::move(tenantId)), m_clientId(clientId), m_clientSecret(clientSecret)
    {
    }

    ClientSecretCredential(
        std::string const&& tenantId,
        std::string const&& clientId,
        std::string const& clientSecret)
        : m_tenantId(std::move(tenantId)), m_clientId(std::move(clientId)),
          m_clientSecret(clientSecret)
    {
    }

    ClientSecretCredential(
        std::string const&& tenantId,
        std::string const&& clientId,
        std::string const&& clientSecret)
        : m_tenantId(std::move(tenantId)), m_clientId(std::move(clientId)),
          m_clientSecret(std::move(clientSecret))
    {
    }

    AccessToken GetToken(
        Azure::Core::Context& context,
        std::vector<std::string const> const& scopes) const override;
  };

  class AuthenticationException : public std::runtime_error {
  public:
    explicit AuthenticationException(std::string const& msg) : std::runtime_error(msg) {}
  };

}}} // namespace Azure::Core::Credentials
