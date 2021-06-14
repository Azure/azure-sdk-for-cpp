// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/identity/internal/token_credential_impl.hpp"

#include <azure/core/credentials/token_credential_options.hpp>
#include <azure/core/url.hpp>

#include <memory>
#include <string>

namespace Azure { namespace Identity { namespace _detail {
  class ManagedIdentitySource : public TokenCredentialImpl {
  protected:
    static Core::Url ParseEndpointUrl(std::string const& url, char const* envVarName);

    explicit ManagedIdentitySource(Core::Credentials::TokenCredentialOptions const& options)
        : TokenCredentialImpl(options)
    {
    }
  };

  class AppServiceManagedIdentitySource final : public ManagedIdentitySource {
  public:
    static std::unique_ptr<ManagedIdentitySource> Create(
        std::string const& clientId,
        Core::Credentials::TokenCredentialOptions const& options);

  private:
    Core::Http::Request m_request;

    explicit AppServiceManagedIdentitySource(
        std::string const& clientId,
        Core::Credentials::TokenCredentialOptions const& options,
        Core::Url endpointUrl,
        std::string const& secret);

    TokenRequest GetRequest(
        Core::Credentials::TokenRequestContext const& tokenRequestContext) const final;
  };

  class CloudShellManagedIdentitySource final : public ManagedIdentitySource {
  public:
    static std::unique_ptr<ManagedIdentitySource> Create(
        std::string const& clientId,
        Core::Credentials::TokenCredentialOptions const& options);

  private:
    Core::Url m_url;
    std::string m_body;

    explicit CloudShellManagedIdentitySource(
        std::string const& clientId,
        Core::Credentials::TokenCredentialOptions const& options,
        Core::Url endpointUrl);

    TokenRequest GetRequest(
        Core::Credentials::TokenRequestContext const& tokenRequestContext) const final;
  };

  class AzureArcManagedIdentitySource final : public ManagedIdentitySource {
  public:
    static std::unique_ptr<ManagedIdentitySource> Create(
        std::string const& clientId,
        Core::Credentials::TokenCredentialOptions const& options);

  private:
    Core::Url m_url;

    explicit AzureArcManagedIdentitySource(
        Core::Credentials::TokenCredentialOptions const& options,
        Core::Url endpointUrl);

    TokenRequest GetRequest(
        Core::Credentials::TokenRequestContext const& tokenRequestContext) const final;

    bool ShouldRetry(
        Core::Http::HttpStatusCode statusCode,
        Core::Http::RawResponse const& response,
        TokenRequest& request) const final;
  };

  class ImdsManagedIdentitySource final : public ManagedIdentitySource {
  public:
    static std::unique_ptr<ManagedIdentitySource> Create(
        std::string const& clientId,
        Core::Credentials::TokenCredentialOptions const& options);

  private:
    Core::Http::Request m_request;

    explicit ImdsManagedIdentitySource(
        std::string const& clientId,
        Core::Credentials::TokenCredentialOptions const& options);

    TokenRequest GetRequest(
        Core::Credentials::TokenRequestContext const& tokenRequestContext) const final;
  };

  // class ServiceFabricManagedIdentitySource final : public ManagedIdentitySource {
  // public:
  //   static std::unique_ptr<ManagedIdentitySource> Create(
  //       std::string const& clientId,
  //       Core::Credentials::TokenCredentialOptions const& options);
  //
  // private:
  //   Core::Http::Request m_request;
  //
  //   static std::string GetIdentityServerThumbprint();
  //
  //   static Core::Credentials::TokenCredentialOptions SetServiceFabricTransport(
  //       Core::Credentials::TokenCredentialOptions const& options);
  //
  //   explicit ServiceFabricManagedIdentitySource(
  //       std::string const& clientId,
  //       Core::Credentials::TokenCredentialOptions const& options,
  //       Core::Url endpointUrl,
  //       std::string const& secret);
  //
  //   TokenRequest GetRequest(
  //       Core::Credentials::TokenRequestContext const& tokenRequestContext) const final;
  // };
}}} // namespace Azure::Identity::_detail
