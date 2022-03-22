// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Chained Token Credential.
 */

#pragma once

#include <azure/core/credentials/credentials.hpp>

#include <memory>
#include <vector>

namespace Azure { namespace Identity {
  /**
   * @brief Chained Token Credential provides a token credential implementation which chains
   * multiple `%Core::Credentials::TokenCredential` implementations to be tried in order until one
   * of the `%GetToken()` methods returns an access token.
   *
   */
  class ChainedTokenCredential final : public Core::Credentials::TokenCredential {
  public:
    /**
     * @brief A container type to store the ordered chain of credentials.
     *
     */
    using Sources = std::vector<std::shared_ptr<Core::Credentials::TokenCredential>>;

  private:
    Sources m_sources;

  public:
    /**
     * @brief Constructs a Chained Token Credential.
     *
     * @param sources The ordered chain of `%Core::Credentials::TokenCredential` implementations to
     * tried when calling `%GetToken()`
     */
    explicit ChainedTokenCredential(Sources sources);

    /**
     * @brief Destructs `%ChainedTokenCredential`.
     *
     */
    ~ChainedTokenCredential() override;

    /**
     * @brief Gets an authentication token.
     *
     * @param tokenRequestContext A context to get the token in.
     * @param context A context to control the request lifetime.
     *
     * @throw Azure::Core::Credentials::AuthenticationException Authentication error occurred.
     */
    Core::Credentials::AccessToken GetToken(
        Core::Credentials::TokenRequestContext const& tokenRequestContext,
        Core::Context const& context) const override;
  };

}} // namespace Azure::Identity
