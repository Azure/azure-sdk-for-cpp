// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure.hpp"

#include <chrono>
#include <memory>
#include <mutex>
#include <string>

namespace Azure { namespace Core { namespace Credentials {

  namespace Details {
    class CredentialTest;
  }

  class Credential {
    virtual void SetScopes(std::string const& scopes) { AZURE_UNREFERENCED_PARAMETER(scopes); }

  public:
    class Internal;
    virtual ~Credential() noexcept = default;

  protected:
    Credential() = default;

    Credential(Credential const& other) = default;
    Credential& operator=(Credential const& other) = default;
  };

  class TokenCredential : public Credential {
    friend class Details::CredentialTest;
    class Token;

    std::shared_ptr<Token> m_token;
    std::mutex m_mutex;

    std::string UpdateTokenNonThreadSafe(Token& token);

    virtual bool IsTokenExpired(std::chrono::system_clock::time_point const& tokenExpiration) const;

    virtual void RefreshToken(
        std::string& newTokenString,
        std::chrono::system_clock::time_point& newExpiration)
        = 0;

  public:
    class Internal;

    TokenCredential(TokenCredential const& other);
    TokenCredential& operator=(TokenCredential const& other);

  protected:
    TokenCredential() = default;
    TokenCredential(TokenCredential const& other, int) : Credential(other) {}

    void Init(TokenCredential const& other);
    virtual std::string GetToken();
    void ResetToken();
  };

  class ClientSecretCredential : public TokenCredential {
    friend class Details::CredentialTest;
    class ClientSecret;

    std::shared_ptr<ClientSecret> m_clientSecret;
    std::mutex m_mutex;

    void SetScopes(std::string const& scopes) override;

    std::string GetToken() override;

    void RefreshToken(
        std::string& newTokenString,
        std::chrono::system_clock::time_point& newExpiration) override;

  public:
    ClientSecretCredential(
        std::string const& tenantId,
        std::string const& clientId,
        std::string const& clientSecret);

    ClientSecretCredential(ClientSecretCredential const& other);
    ClientSecretCredential& operator=(ClientSecretCredential const& other);
  };

}}} // namespace Azure::Core::Credentials
