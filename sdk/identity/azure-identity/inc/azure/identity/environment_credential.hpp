// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Environment Credential.
 */

#pragma once

#include <azure/core/credentials.hpp>

#include <memory>

namespace Azure { namespace Identity {

  /**
   * @brief An environment credential.
   */
  class EnvironmentCredential : public Core::TokenCredential {
    std::unique_ptr<TokenCredential> m_credentialImpl;

  public:
    /**
     * Constructs an environment credential.
     */
    explicit EnvironmentCredential();

    Core::AccessToken GetToken(Core::Context const& context, std::vector<std::string> const& scopes)
        const override;
  };

}} // namespace Azure::Identity
