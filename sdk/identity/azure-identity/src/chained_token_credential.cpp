// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/chained_token_credential.hpp"
#include "azure/core/internal/diagnostics/log.hpp"
#include "private/chained_token_credential_impl.hpp"

#include <utility>

using namespace Azure::Identity;
using namespace Azure::Identity::_detail;
using namespace Azure::Core::Credentials;
using Azure::Core::Context;
using Azure::Core::Diagnostics::Logger;
using Azure::Core::Diagnostics::_internal::Log;

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

namespace {
constexpr auto IdentityPrefix = "Identity: ";
} // namespace

ChainedTokenCredentialImpl::ChainedTokenCredentialImpl(
    std::string const& credentialName,
    ChainedTokenCredential::Sources&& sources)
    : m_sources(std::move(sources))
{
  auto const logLevel = m_sources.empty() ? Logger::Level::Warning : Logger::Level::Informational;
  if (Log::ShouldWrite(logLevel))
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

    Log::Write(logLevel, IdentityPrefix + credentialName + ": Created" + credSourceDetails);
  }
}

AccessToken ChainedTokenCredentialImpl::GetToken(
    std::string const& credentialName,
    TokenRequestContext const& tokenRequestContext,
    Context const& context) const
{
  auto const sourcesSize = m_sources.size();

  if (sourcesSize == 0)
  {
    Log::Write(
        Logger::Level::Warning,
        IdentityPrefix + credentialName
            + ": Authentication did not succeed: List of sources is empty.");
  }

  for (size_t i = 0; i < sourcesSize; ++i)
  {
    try
    {
      auto token = m_sources[i]->GetToken(tokenRequestContext, context);

      Log::Write(
          Logger::Level::Informational,
          IdentityPrefix + credentialName + ": Successfully got token from "
              + m_sources[i]->GetCredentialName() + '.');

      return token;
    }
    catch (AuthenticationException const& e)
    {
      Log::Write(
          Logger::Level::Verbose,
          IdentityPrefix + credentialName + ": Failed to get token from "
              + m_sources[i]->GetCredentialName() + ": " + e.what());

      if ((sourcesSize - 1) == i) // On the last credential
      {
        Log::Write(
            Logger::Level::Warning,
            IdentityPrefix + credentialName
                + ": Didn't succeed to get a token from any credential in the chain.");
      }
    }
  }

  throw AuthenticationException("Failed to get token from " + credentialName + '.');
}
