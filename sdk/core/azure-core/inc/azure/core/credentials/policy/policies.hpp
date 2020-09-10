// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Authentication policies.
 */

#pragma once

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/policy.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

namespace Azure { namespace Core { namespace Credentials { namespace Policy {

  /**
   * @brief Bearer Token authentication policy.
   */
  class BearerTokenAuthenticationPolicy : public Http::HttpPolicy {
  private:
    std::shared_ptr<TokenCredential const> const m_credential;
    std::vector<std::string> m_scopes;

    mutable AccessToken m_accessToken;
    mutable std::mutex m_accessTokenMutex;

    BearerTokenAuthenticationPolicy(BearerTokenAuthenticationPolicy const&) = delete;
    void operator=(BearerTokenAuthenticationPolicy const&) = delete;

  public:
    /**
     * @brief Construct a Bearer Token authentication policy with single authentication scope.
     *
     * @param credential A #TokenCredential to use with this policy.
     * @param scope Authentication scope.
     */
    explicit BearerTokenAuthenticationPolicy(
        std::shared_ptr<TokenCredential const> credential,
        std::string scope)
        : m_credential(std::move(credential))
    {
      m_scopes.emplace_back(std::move(scope));
    }

    /**
     * @brief Construct a Bearer Token authentication policy with multiple authentication scopes.
     *
     * @param credential A #TokenCredential to use with this policy.
     * @param scopes A vector of authentication scopes.
     */
    explicit BearerTokenAuthenticationPolicy(
        std::shared_ptr<TokenCredential const> credential,
        std::vector<std::string> scopes)
        : m_credential(std::move(credential)), m_scopes(std::move(scopes))
    {
    }

    /**
     * @brief Construct a Bearer Token authentication policy with multiple authentication scopes.
     *
     * @tparam A type of scopes sequence iterator.
     *
     * @param credential A #TokenCredential to use with this policy.
     * @param scopesBegin An iterator pointing to begin of the sequence of scopes to use.
     * @param scopesEnd An iterator pointing to an element after the last element in sequence of
     * scopes to use.
     */
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
        Context const& context,
        Http::Request& request,
        Http::NextHttpPolicy policy) const override;
  };

}}}} // namespace Azure::Core::Credentials::Policy
