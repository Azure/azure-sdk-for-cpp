// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/credentials/token_credential_options.hpp>
#include <azure/core/url.hpp>

#include "token_credential_impl.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Identity { namespace _detail {
  class ManagedIdentitySource : protected TokenCredentialImpl {
  public:
    virtual Core::Credentials::AccessToken GetToken(
        Core::Credentials::TokenRequestContext const& tokenRequestContext,
        Core::Context const& context) const = 0;

  protected:
    static Core::Url ParseEndpointUrl(std::string const& url, char const* envVarName);

    explicit ManagedIdentitySource(Core::Credentials::TokenCredentialOptions const& options)
        : TokenCredentialImpl(options)
    {
    }
  };

  class AppServiceManagedIdentitySource : public ManagedIdentitySource {
  private:
    Core::Http::Request m_request;

  protected:
    explicit AppServiceManagedIdentitySource(
        std::string const& clientId,
        Core::Credentials::TokenCredentialOptions const& options,
        Core::Url endpointUrl,
        std::string const& secret,
        std::string const& apiVersion,
        std::string const& secretHeaderName,
        std::string const& clientIdHeaderName);

    template <typename T>
    static std::unique_ptr<ManagedIdentitySource> Create(
        std::string const& clientId,
        Core::Credentials::TokenCredentialOptions const& options,
        const char* endpointVarName,
        const char* secretVarName);

  public:
    Core::Credentials::AccessToken GetToken(
        Core::Credentials::TokenRequestContext const& tokenRequestContext,
        Core::Context const& context) const override final;
  };

  class AppServiceV2017ManagedIdentitySource final : public AppServiceManagedIdentitySource {
    friend class AppServiceManagedIdentitySource;

  private:
    explicit AppServiceV2017ManagedIdentitySource(
        std::string const& clientId,
        Core::Credentials::TokenCredentialOptions const& options,
        Core::Url endpointUrl,
        std::string const& secret)
        : AppServiceManagedIdentitySource(
            clientId,
            options,
            endpointUrl,
            secret,
            "2017-09-01",
            "secret",
            "clientid")
    {
    }

  public:
    static std::unique_ptr<ManagedIdentitySource> Create(
        std::string const& clientId,
        Core::Credentials::TokenCredentialOptions const& options);
  };

  class AppServiceV2019ManagedIdentitySource final : public AppServiceManagedIdentitySource {
    friend class AppServiceManagedIdentitySource;

  private:
    explicit AppServiceV2019ManagedIdentitySource(
        std::string const& clientId,
        Core::Credentials::TokenCredentialOptions const& options,
        Core::Url endpointUrl,
        std::string const& secret)
        : AppServiceManagedIdentitySource(
            clientId,
            options,
            endpointUrl,
            secret,
            "2019-08-01",
            "X-IDENTITY-HEADER",
            "client_id")
    {
    }

  public:
    static std::unique_ptr<ManagedIdentitySource> Create(
        std::string const& clientId,
        Core::Credentials::TokenCredentialOptions const& options);
  };

  class CloudShellManagedIdentitySource final : public ManagedIdentitySource {
  private:
    Core::Url m_url;
    std::string m_body;

    explicit CloudShellManagedIdentitySource(
        std::string const& clientId,
        Core::Credentials::TokenCredentialOptions const& options,
        Core::Url endpointUrl);

  public:
    static std::unique_ptr<ManagedIdentitySource> Create(
        std::string const& clientId,
        Core::Credentials::TokenCredentialOptions const& options);

    Core::Credentials::AccessToken GetToken(
        Core::Credentials::TokenRequestContext const& tokenRequestContext,
        Core::Context const& context) const override;
  };

  class AzureArcManagedIdentitySource final : public ManagedIdentitySource {
  private:
    Core::Url m_url;

    explicit AzureArcManagedIdentitySource(
        Core::Credentials::TokenCredentialOptions const& options,
        Core::Url endpointUrl);

  public:
    static std::unique_ptr<ManagedIdentitySource> Create(
        std::string const& clientId,
        Core::Credentials::TokenCredentialOptions const& options);

    Core::Credentials::AccessToken GetToken(
        Core::Credentials::TokenRequestContext const& tokenRequestContext,
        Core::Context const& context) const override;
  };

  class ImdsManagedIdentitySource final : public ManagedIdentitySource {
  private:
    Core::Http::Request m_request;

    explicit ImdsManagedIdentitySource(
        std::string const& clientId,
        Core::Credentials::TokenCredentialOptions const& options);

  public:
    static std::unique_ptr<ManagedIdentitySource> Create(
        std::string const& clientId,
        Core::Credentials::TokenCredentialOptions const& options);

    Core::Credentials::AccessToken GetToken(
        Core::Credentials::TokenRequestContext const& tokenRequestContext,
        Core::Context const& context) const override;
  };
}}} // namespace Azure::Identity::_detail
