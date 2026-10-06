// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "session_authentication_policy.hpp"

#include "token_credential_session_provider.hpp"

#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>
#include <azure/storage/common/internal/storage_url.hpp>

#include <stdexcept>

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {

  namespace {
    const Azure::Core::Context::Key SessionContextKey;
  }

  class SessionSigningPolicy final : public Azure::Core::Http::Policies::HttpPolicy {
  public:
    explicit SessionSigningPolicy(std::string accountName) : m_accountName(std::move(accountName))
    {
    }

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<SessionSigningPolicy>(*this);
    }

    std::unique_ptr<Azure::Core::Http::RawResponse> Send(
        Azure::Core::Http::Request& request,
        Azure::Core::Http::Policies::NextHttpPolicy nextPolicy,
        Azure::Core::Context const& context) const override
    {
      SessionTokenInfo session;
      if (context.TryGetValue(SessionContextKey, session))
      {
        request.RemoveHeader(Azure::Storage::_internal::HttpHeaderAuthorization);
        request.SetHeader(
            Azure::Storage::_internal::HttpHeaderAuthorization,
            "Session " + session.Token + ":"
                + Azure::Storage::_internal::SharedKeyPolicy::GetSignature(
                    request, m_accountName, session.Key));
      }
      return nextPolicy.Send(request, context);
    }

  private:
    std::string m_accountName;
  };

  class SessionAuthenticationPolicy final
      : public Azure::Storage::_internal::StorageBearerTokenAuthenticationPolicy {
  public:
    SessionAuthenticationPolicy(
        std::shared_ptr<const Azure::Core::Credentials::TokenCredential> credential,
        Azure::Core::Credentials::TokenRequestContext tokenRequestContext,
        bool enableTenantDiscovery,
        std::shared_ptr<Blobs::SessionProvider> provider)
        : StorageBearerTokenAuthenticationPolicy(
            std::move(credential),
            std::move(tokenRequestContext),
            enableTenantDiscovery),
          m_provider(provider->m_provider)
    {
    }

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<SessionAuthenticationPolicy>(*this);
    }

  private:
    std::shared_ptr<TokenCredentialSessionProvider> m_provider;

    std::unique_ptr<Azure::Core::Http::RawResponse> AuthorizeAndSendRequest(
        Azure::Core::Http::Request& request,
        Azure::Core::Http::Policies::NextHttpPolicy& nextPolicy,
        Azure::Core::Context const& context) const override
    {
      if (!m_provider->IsRequestEligible(request))
      {
        return StorageBearerTokenAuthenticationPolicy::AuthorizeAndSendRequest(
            request, nextPolicy, context);
      }

      auto session = m_provider->GetSession(request, context);
      if (!session.HasValue())
      {
        return StorageBearerTokenAuthenticationPolicy::AuthorizeAndSendRequest(
            request, nextPolicy, context);
      }

      auto sessionContext = context.WithValue(SessionContextKey, SessionTokenInfo(session.Value()));
      auto response = nextPolicy.Send(request, sessionContext);
      if (response->GetStatusCode() != Azure::Core::Http::HttpStatusCode::Unauthorized)
      {
        return response;
      }

      m_provider->InvalidateSession(request, session.Value());
      request.RemoveHeader(Azure::Storage::_internal::HttpHeaderAuthorization);
      return StorageBearerTokenAuthenticationPolicy::AuthorizeAndSendRequest(
          request, nextPolicy, context);
    }
  };

  TokenAuthenticationPolicies CreateTokenAuthenticationPolicies(
      const std::string& clientUrl,
      std::shared_ptr<const Azure::Core::Credentials::TokenCredential> credential,
      Azure::Core::Credentials::TokenRequestContext tokenRequestContext,
      bool enableTenantDiscovery,
      const BlobClientOptions& options)
  {
    const auto createBearerPolicy = [&credential,
                                     &tokenRequestContext,
                                     enableTenantDiscovery]() -> TokenAuthenticationPolicies {
      TokenAuthenticationPolicies policies;
      policies.TokenAuthPolicy
          = std::make_unique<Azure::Storage::_internal::StorageBearerTokenAuthenticationPolicy>(
              std::move(credential), std::move(tokenRequestContext), enableTenantDiscovery);
      return policies;
    };

    if (options.Session.Mode != SessionMode::Enabled)
    {
      return createBearerPolicy();
    }

    auto provider = options.Session.Provider;
    auto accountName = options.Session.AccountName;
    if (!provider || accountName.empty())
    {
      auto urlParts = Azure::Storage::_internal::ParseStorageUrl(Azure::Core::Url(clientUrl));
      if (!urlParts.HasValue())
      {
        throw std::invalid_argument(
            "SessionOptions.AccountName and the Blob service URL could not be determined from the "
            "client URL. Specify both SessionOptions.AccountName and SessionOptions.Provider for "
            "custom endpoints.");
      }
      if (!accountName.empty() && accountName != urlParts.Value().AccountName)
      {
        throw std::invalid_argument(
            "SessionOptions.AccountName does not match the account name in the client URL.");
      }
      accountName = std::move(urlParts.Value().AccountName);
      if (!provider)
      {
        provider = std::make_shared<Blobs::ContainerSessionProvider>(
            urlParts.Value().ServiceUrl, credential, options);
      }
    }
    TokenAuthenticationPolicies policies;
    policies.TokenAuthPolicy = std::make_unique<SessionAuthenticationPolicy>(
        std::move(credential),
        std::move(tokenRequestContext),
        enableTenantDiscovery,
        std::move(provider));
    policies.FinalAuthPolicy = std::make_unique<SessionSigningPolicy>(std::move(accountName));
    return policies;
  }

}}}} // namespace Azure::Storage::Blobs::_detail
