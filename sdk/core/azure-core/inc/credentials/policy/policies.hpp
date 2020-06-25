// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <credentials/credentials.hpp>
#include <http/policy.hpp>
#include <memory>
#include <string>
#include <utility>

namespace Azure { namespace Core { namespace Credentials { namespace Policy {

  class BearerTokenAuthenticationPolicy : Http::HttpPolicy {
  private:
    std::shared_ptr<TokenCredential const> m_credential;
    std::vector<std::string const> m_scopes;
    AccessToken m_accessToken;

    BearerTokenAuthenticationPolicy(BearerTokenAuthenticationPolicy const&) = delete;
    void operator=(BearerTokenAuthenticationPolicy const&) = delete;

  public:
    BearerTokenAuthenticationPolicy(
        std::shared_ptr<TokenCredential const> const& credential,
        std::string const& scope)
        : m_credential(credential)
    {
      m_scopes.push_back(scope);
    }

    BearerTokenAuthenticationPolicy(
        std::shared_ptr<TokenCredential const> const& credential,
        std::vector<std::string> const& scopes)
        : m_credential(credential), m_scopes(scopes)
    {
    }

    BearerTokenAuthenticationPolicy(
        std::shared_ptr<TokenCredential const> const& credential,
        std::vector<std::string> const&& scopes)
        : m_credential(credential), m_scopes(std::move(scopes))
    {
    }

    template <typename ScopesIterator>
    BearerTokenAuthenticationPolicy(
        std::shared_ptr<TokenCredential const> const& credential,
        ScopesIterator const& scopesBegin,
        ScopesIterator const& scopesEnd)
        : m_credential(credential), m_scopes(scopesBegin, scopesEnd)
    {
    }

    ~HttpPolicy() override {}

    HttpPolicy* Clone() const override
    {
      return new BearerTokenAuthenticationPolicy(m_credential, m_scopes);
    }

    std::unique_ptr<Response> Send(Context& context, Request& request, NextHttpPolicy policy)
        const override;
  };

}}}} // namespace Azure::Core::Credentials::Policy
