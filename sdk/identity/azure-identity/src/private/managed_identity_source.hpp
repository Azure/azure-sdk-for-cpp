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

  class AppServiceManagedIdentitySource final : public ManagedIdentitySource {
  private:
    Core::Http::Request m_request;

    explicit AppServiceManagedIdentitySource(
        std::string const& clientId,
        Core::Credentials::TokenCredentialOptions const& options,
        Core::Url endpointUrl,
        std::string const& secret);

  public:
    static std::unique_ptr<ManagedIdentitySource> Create(
        std::string const& clientId,
        Core::Credentials::TokenCredentialOptions const& options);

    Core::Credentials::AccessToken GetToken(
        Core::Credentials::TokenRequestContext const& tokenRequestContext,
        Core::Context const& context) const override;
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
