// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/identity/chained_token_credential.hpp"

namespace Azure { namespace Identity { namespace _detail {

  class ChainedTokenCredentialImpl final {
  public:
    ChainedTokenCredentialImpl(
        std::string const& credentialName,
        ChainedTokenCredential::Sources&& sources);

    Core::Credentials::AccessToken GetToken(
        std::string const& credentialName,
        Core::Credentials::TokenRequestContext const& tokenRequestContext,
        Core::Context const& context) const;

  private:
    ChainedTokenCredential::Sources m_sources;
  };

}}} // namespace Azure::Identity::_detail
