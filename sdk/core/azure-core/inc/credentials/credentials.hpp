// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <context.hpp>
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
    virtual AccessToken GetToken(Context& context, std::vector<std::string> const& scopes)
        const = 0;

  protected:
    TokenCredential() {}

  private:
    TokenCredential(TokenCredential const&) = delete;
    void operator=(TokenCredential const&) = delete;
  };

  class ClientSecretCredential : public TokenCredential {
  private:
    std::string const m_tenantId;
    std::string const m_clientId;
    std::string const m_clientSecret;

  public:
    ClientSecretCredential(std::string tenantId, std::string clientId, std::string clientSecret)
        : m_tenantId(std::move(tenantId)), m_clientId(std::move(clientId)),
          m_clientSecret(std::move(clientSecret))
    {
    }

    AccessToken GetToken(Context& context, std::vector<std::string> const& scopes) const override;
  };

  class AuthenticationException : public std::runtime_error {
  public:
    explicit AuthenticationException(std::string const& msg) : std::runtime_error(msg) {}
  };

}}} // namespace Azure::Core::Credentials
