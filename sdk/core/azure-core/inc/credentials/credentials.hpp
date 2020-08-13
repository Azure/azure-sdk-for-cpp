// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <context.hpp>

#include <chrono>
#include <memory>
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
    virtual AccessToken GetToken(Context const& context, std::vector<std::string> const& scopes)
        const = 0;

    virtual ~TokenCredential() = default;

  protected:
    TokenCredential() {}

  private:
    TokenCredential(TokenCredential const&) = delete;
    void operator=(TokenCredential const&) = delete;
  };

  class ClientSecretCredential : public TokenCredential {
  private:
    static std::string const g_aadGlobalAuthority;

    std::string m_tenantId;
    std::string m_clientId;
    std::string m_clientSecret;
    std::string m_authority;

  public:
    explicit ClientSecretCredential(
        std::string tenantId,
        std::string clientId,
        std::string clientSecret,
        std::string authority = g_aadGlobalAuthority)
        : m_tenantId(std::move(tenantId)), m_clientId(std::move(clientId)),
          m_clientSecret(std::move(clientSecret)), m_authority(std::move(authority))
    {
    }

    AccessToken GetToken(Context const& context, std::vector<std::string> const& scopes)
        const override;
  };

  class AuthenticationException : public std::runtime_error {
  public:
    explicit AuthenticationException(std::string const& msg) : std::runtime_error(msg) {}
  };

  class EnvironmentCredential : public TokenCredential {
    std::unique_ptr<TokenCredential> m_credentialImpl;

  public:
    explicit EnvironmentCredential();

    AccessToken GetToken(Context const& context, std::vector<std::string> const& scopes)
        const override;
  };

}}} // namespace Azure::Core::Credentials
