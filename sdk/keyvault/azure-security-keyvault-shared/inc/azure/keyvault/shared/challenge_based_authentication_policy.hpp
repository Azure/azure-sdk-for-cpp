// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Key Vault Challenge-Based Authentication Policy.
 *
 */

#pragma once

#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/credentials/authorization_challenge_parser.hpp>

#include <stdexcept>

namespace Azure { namespace Security { namespace KeyVault { namespace _internal {
  /**
   * @brief Challenge-Based Authentication Policy for Key Vault.
   *
   */
  class ChallengeBasedAuthenticationPolicy final
      : public Core::Http::Policies::_internal::BearerTokenAuthenticationPolicy {
  private:
    mutable Core::Credentials::TokenRequestContext m_tokenRequestContext;

  public:
    explicit ChallengeBasedAuthenticationPolicy(
        std::shared_ptr<Core::Credentials::TokenCredential const> credential,
        Core::Credentials::TokenRequestContext tokenRequestContext)
        : BearerTokenAuthenticationPolicy(credential, tokenRequestContext),
          m_tokenRequestContext(tokenRequestContext)
    {
    }

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<ChallengeBasedAuthenticationPolicy>(*this);
    }

  private:
    std::unique_ptr<Core::Http::RawResponse> AuthorizeAndSendRequest(
        Core::Http::Request& request,
        Core::Http::Policies::NextHttpPolicy& nextPolicy,
        Core::Context const& context) const override
    {
      AuthenticateAndAuthorizeRequest(request, m_tokenRequestContext, context);
      return nextPolicy.Send(request, context);
    }

    bool AuthorizeRequestOnChallenge(
        std::string const& challenge,
        Core::Http::Request& request,
        Core::Context const& context) const override
    {
      auto const scope = GetScope(challenge);
      if (scope.empty())
      {
        return false;
      }

      ValidateChallengeResponse(scope, request.GetUrl().GetHost());

      auto const tenantId = GetTenantId(GetAuthorization(challenge));
      m_tokenRequestContext.TenantId = tenantId;
      m_tokenRequestContext.Scopes = {scope};

      AuthenticateAndAuthorizeRequest(request, m_tokenRequestContext, context);
      return true;
    }

    static std::string TrimTrailingSlash(std::string const& s)
    {
      return (s.empty() || s.back() != '/') ? s : s.substr(0, s.size() - 1);
    }

    static std::string GetScope(std::string const& challenge)
    {
      using Core::Credentials::_internal::AuthorizationChallengeParser;

      auto resource
          = AuthorizationChallengeParser::GetChallengeParameter(challenge, "Bearer", "resource");

      return !resource.empty()
          ? (TrimTrailingSlash(resource) + "/.default")
          : AuthorizationChallengeParser::GetChallengeParameter(challenge, "Bearer", "scope");
    }

    static std::string GetAuthorization(std::string const& challenge)
    {
      using Core::Credentials::_internal::AuthorizationChallengeParser;

      auto authorization = AuthorizationChallengeParser::GetChallengeParameter(
          challenge, "Bearer", "authorization");

      return !authorization.empty() ? authorization
                                    : AuthorizationChallengeParser::GetChallengeParameter(
                                        challenge, "Bearer", "authorization_uri");
    }

    static bool TryParseUrl(std::string const& s, Core::Url& outUrl)
    {
      using Core::Url;
      try
      {
        outUrl = Url(s);
      }
      catch (std::out_of_range const&)
      {
        return false;
      }
      catch (std::invalid_argument const&)
      {
        return false;
      }

      return true;
    }

    static void ValidateChallengeResponse(std::string const& scope, std::string const& requestHost)
    {
      using Core::Url;
      using Core::Credentials::AuthenticationException;

      Url scopeUrl;
      if (!TryParseUrl(scope, scopeUrl))
      {
        throw AuthenticationException("The challenge contains invalid scope '" + scope + "'.");
      }

      auto const& scopeHost = scopeUrl.GetHost();

      // Check whether requestHost.ends_with(scopeHost)
      auto const requestHostLength = requestHost.length();
      auto const scopeHostLength = scopeHost.length();

      bool domainMismatch = requestHostLength < scopeHostLength;
      if (!domainMismatch)
      {
        auto const requestHostOffset = requestHostLength - scopeHostLength;
        for (size_t i = 0; i < scopeHostLength; ++i)
        {
          if (requestHost[requestHostOffset + i] != scopeHost[i])
          {
            domainMismatch = true;
            break;
          }
        }
      }

      if (domainMismatch)
      {
        throw AuthenticationException(
            "The challenge resource '" + scopeHost + "' does not match the requested domain.");
      }
    }

    static std::string GetTenantId(std::string const& authorization)
    {
      using Core::Url;
      using Core::Credentials::AuthenticationException;

      if (!authorization.empty())
      {
        Url authorizationUrl;
        if (TryParseUrl(authorization, authorizationUrl))
        {
          auto const& path = authorizationUrl.GetPath();
          if (!path.empty())
          {
            auto const firstSlash = path.find('/');
            if (firstSlash == std::string::npos)
            {
              return path;
            }
            else if (firstSlash > 0)
            {
              return path.substr(0, firstSlash);
            }
          }
        }
      }

      throw AuthenticationException(
          "The challenge authorization URI '" + authorization + "' is invalid.");
    }
  };
}}}} // namespace Azure::Security::KeyVault::_internal
