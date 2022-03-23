// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/chained_token_credential.hpp"

#include <utility>

using namespace Azure::Identity;

ChainedTokenCredential::ChainedTokenCredential(ChainedTokenCredential::Sources sources)
    : m_sources(std::move(sources))
{
}

ChainedTokenCredential::~ChainedTokenCredential() = default;

Azure::Core::Credentials::AccessToken ChainedTokenCredential::GetToken(
    Azure::Core::Credentials::TokenRequestContext const& tokenRequestContext,
    Azure::Core::Context const& context) const
{
  for (const auto& source : m_sources)
  {
    try
    {
      return source->GetToken(tokenRequestContext, context);
    }
    catch (const Azure::Core::Credentials::AuthenticationException&)
    {
      // Do nothing, next source will be tried.
    }
  }

  throw Azure::Core::Credentials::AuthenticationException(
      "Failed to get token from ChainedTokenCredential.");
}
