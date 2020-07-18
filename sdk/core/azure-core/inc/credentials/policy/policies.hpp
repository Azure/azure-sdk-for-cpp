// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <credentials/credentials.hpp>
#include <http/policy.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

namespace Azure { namespace Core { namespace Credentials { namespace Policy {

  class BearerTokenAuthenticationPolicy : public Http::HttpPolicy {
  private:
    std::shared_ptr<TokenCredential const> const m_credential;
    std::vector<std::string> m_scopes;

    mutable AccessToken m_accessToken;
    mutable std::mutex m_accessTokenMutex;

    BearerTokenAuthenticationPolicy(BearerTokenAuthenticationPolicy const&) = delete;
    void operator=(BearerTokenAuthenticationPolicy const&) = delete;

  public:
    explicit BearerTokenAuthenticationPolicy(
        std::shared_ptr<TokenCredential const> credential,
        std::string scope)
        : m_credential(std::move(credential))
    {
      m_scopes.emplace_back(std::move(scope));
    }

    explicit BearerTokenAuthenticationPolicy(
        std::shared_ptr<TokenCredential const> credential,
        std::vector<std::string> scopes)
        : m_credential(std::move(credential)), m_scopes(std::move(scopes))
    {
    }

    template <typename ScopesIterator>
    explicit BearerTokenAuthenticationPolicy(
        std::shared_ptr<TokenCredential const> credential,
        ScopesIterator const& scopesBegin,
        ScopesIterator const& scopesEnd)
        : m_credential(std::move(credential)), m_scopes(scopesBegin, scopesEnd)
    {
    }

    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<BearerTokenAuthenticationPolicy>(m_credential, m_scopes);
    }

    std::unique_ptr<Http::RawResponse> Send(
        Context& context,
        Http::Request& request,
        Http::NextHttpPolicy policy) const override;
  };

}}}} // namespace Azure::Core::Credentials::Policy
