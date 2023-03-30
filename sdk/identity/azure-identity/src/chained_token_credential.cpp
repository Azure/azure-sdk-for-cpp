// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/chained_token_credential.hpp"

#include "private/chained_token_credential_impl.hpp"
#include "private/identity_log.hpp"

#include "azure/core/internal/diagnostics/log.hpp"

#include <utility>

using namespace Azure::Identity;
using namespace Azure::Identity::_detail;
using namespace Azure::Core::Credentials;
using Azure::Core::Context;
using Azure::Identity::_detail::IdentityLog;

ChainedTokenCredential::ChainedTokenCredential(ChainedTokenCredential::Sources sources)
    : TokenCredential("ChainedTokenCredential"),
      m_impl(std::make_unique<ChainedTokenCredentialImpl>(GetCredentialName(), std::move(sources)))
{
}

ChainedTokenCredential::~ChainedTokenCredential() = default;

AccessToken ChainedTokenCredential::GetToken(
    TokenRequestContext const& tokenRequestContext,
    Context const& context) const
{
  return m_impl->GetToken(GetCredentialName(), tokenRequestContext, context);
}

ChainedTokenCredentialImpl::ChainedTokenCredentialImpl(
    std::string const& credentialName,
    ChainedTokenCredential::Sources&& sources)
    : m_sources(std::move(sources))
{
  auto const logLevel
      = m_sources.empty() ? IdentityLog::Level::Warning : IdentityLog::Level::Informational;

  if (IdentityLog::ShouldWrite(logLevel))
  {
    std::string credSourceDetails = " with EMPTY chain of credentials.";
    if (!m_sources.empty())
    {
      credSourceDetails = " with the following credentials: ";

      auto const sourcesSize = m_sources.size();
      for (size_t i = 0; i < sourcesSize; ++i)
      {
        if (i != 0)
        {
          credSourceDetails += ", ";
        }

        credSourceDetails += m_sources[i]->GetCredentialName();
      }

      credSourceDetails += '.';
    }

    IdentityLog::Write(logLevel, credentialName + ": Created" + credSourceDetails);
  }
}

AccessToken ChainedTokenCredentialImpl::GetToken(
    std::string const& credentialName,
    TokenRequestContext const& tokenRequestContext,
    Context const& context) const
{
  for (auto const& source : m_sources)
  {
    try
    {
      auto token = source->GetToken(tokenRequestContext, context);

      IdentityLog::Write(
          IdentityLog::Level::Informational,
          credentialName + ": Successfully got token from " + source->GetCredentialName() + '.');

      return token;
    }
    catch (AuthenticationException const& e)
    {
      IdentityLog::Write(
          IdentityLog::Level::Verbose,
          credentialName + ": Failed to get token from " + source->GetCredentialName() + ": "
              + e.what());
    }
  }

  IdentityLog::Write(
      IdentityLog::Level::Warning,
      credentialName
          + (m_sources.empty()
                 ? ": Authentication did not succeed: List of sources is empty."
                 : ": Didn't succeed to get a token from any credential in the chain."));

  throw AuthenticationException("Failed to get token from " + credentialName + '.');
}
