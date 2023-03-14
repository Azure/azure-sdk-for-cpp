// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/chained_token_credential.hpp"

#include "azure/core/internal/diagnostics/log.hpp"

#include <utility>

using namespace Azure::Identity;
using namespace Azure::Core::Credentials;
using Azure::Core::Context;
using Azure::Core::Diagnostics::Logger;
using Azure::Core::Diagnostics::_internal::Log;

namespace {
constexpr auto IdentityPrefix = "Identity: ";
} // namespace

ChainedTokenCredential::ChainedTokenCredential(ChainedTokenCredential::Sources sources)
    : ChainedTokenCredential(sources, {})
{
}

ChainedTokenCredential::ChainedTokenCredential(
    ChainedTokenCredential::Sources sources,
    std::string const& enclosingCredential)
    : TokenCredential("ChainedTokenCredential"), m_sources(std::move(sources))
{
  auto const credentialName = GetCredentialName();
  m_logPrefix = IdentityPrefix
      + (enclosingCredential.empty() ? credentialName
                                     : (enclosingCredential + " -> " + credentialName))
      + ": ";

  auto const logLevel = m_sources.empty() ? Logger::Level::Warning : Logger::Level::Informational;
  if (Log::ShouldWrite(logLevel))
  {
    std::string credSourceDetails = " with EMPTY chain of credentials.";
    if (!m_sources.empty())
    {
      credSourceDetails = " with the following credentials: ";

      auto const sourcesSize = m_sources.size();
      for (size_t i = 0; i < (sourcesSize - 1); ++i)
      {
        if (i != 0)
        {
          credSourceDetails += ", ";
        }

        credSourceDetails += m_sources[i]->GetCredentialName();
      }

      credSourceDetails += '.';
    }

    Log::Write(
        logLevel,
        IdentityPrefix
            + (enclosingCredential.empty() ? (credentialName + ": Created")
                                           : (enclosingCredential + ": Created " + credentialName))
            + credSourceDetails);
  }
}

ChainedTokenCredential::~ChainedTokenCredential() = default;

AccessToken ChainedTokenCredential::GetToken(
    TokenRequestContext const& tokenRequestContext,
    Context const& context) const
{
  auto const sourcesSize = m_sources.size();

  if (sourcesSize == 0)
  {
    const auto logLevel = Logger::Level::Warning;
    if (Log::ShouldWrite(logLevel))
    {
      Log::Write(
          logLevel, m_logPrefix + "Authentication did not succeed: List of sources is empty.");
    }
  }
  else
  {
    for (size_t i = 0; i < sourcesSize; ++i)
    {
      try
      {
        auto token = m_sources[i]->GetToken(tokenRequestContext, context);

        {
          auto const logLevel = Logger::Level::Informational;
          if (Log::ShouldWrite(logLevel))
          {
            Log::Write(
                logLevel,
                m_logPrefix + "Successfully got token from " + m_sources[i]->GetCredentialName()
                    + '.');
          }
        }

        return token;
      }
      catch (AuthenticationException const& e)
      {
        {
          auto const logLevel = Logger::Level::Verbose;
          if (Log::ShouldWrite(logLevel))
          {
            Log::Write(
                logLevel,
                m_logPrefix + "Failed to get token from " + m_sources[i]->GetCredentialName() + ": "
                    + e.what());
          }
        }

        if ((sourcesSize - 1) == i) // On the last credential
        {
          auto const logLevel = Logger::Level::Warning;
          if (Log::ShouldWrite(logLevel))
          {
            Log::Write(
                logLevel,
                m_logPrefix + "Didn't succeed to get a token from any credential in the chain.");
          }
        }
      }
    }
  }

  throw AuthenticationException("Failed to get token from " + GetCredentialName() + '.');
}
