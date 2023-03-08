// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/chained_token_credential.hpp"

#include "azure/core/internal/diagnostics/log.hpp"
#include <azure/core/azure_assert.hpp>

#include <utility>

using namespace Azure::Identity;
using namespace Azure::Core::Credentials;
using Azure::Core::Context;
using Azure::Core::Diagnostics::Logger;
using Azure::Core::Diagnostics::_internal::Log;

namespace {
std::string const IdentityPrefix = "Identity: ";
}

ChainedTokenCredential::ChainedTokenCredential(ChainedTokenCredential::Sources sources)
    : ChainedTokenCredential(sources, {}, {})
{
}

ChainedTokenCredential::ChainedTokenCredential(
    ChainedTokenCredential::Sources sources,
    std::string const& enclosingCredential,
    std::vector<std::string> sourcesFriendlyNames)
    : m_sources(std::move(sources)), m_sourcesFriendlyNames(std::move(sourcesFriendlyNames))
{
  // LCOV_EXCL_START
  AZURE_ASSERT(m_sourcesFriendlyNames.empty() || m_sourcesFriendlyNames.size() == m_sources.size());
  // LCOV_EXCL_STOP

  auto const logLevel = m_sources.empty() ? Logger::Level::Warning : Logger::Level::Informational;
  if (Log::ShouldWrite(logLevel))
  {
    std::string credentialsList;
    if (!m_sourcesFriendlyNames.empty())
    {
      auto const sourceDescriptionsSize = m_sourcesFriendlyNames.size();
      for (size_t i = 0; i < (sourceDescriptionsSize - 1); ++i)
      {
        credentialsList += m_sourcesFriendlyNames[i] + ", ";
      }

      credentialsList += m_sourcesFriendlyNames.back();
    }

    Log::Write(
        logLevel,
        IdentityPrefix
            + (enclosingCredential.empty()
                   ? "ChainedTokenCredential: Created"
                   : (enclosingCredential + ": Created ChainedTokenCredential"))
            + " with "
            + (m_sourcesFriendlyNames.empty()
                   ? (std::to_string(m_sources.size()) + " credentials.")
                   : (std::string("the following credentials: ") + credentialsList + '.')));
  }

  m_logPrefix = IdentityPrefix
      + (enclosingCredential.empty() ? "ChainedTokenCredential"
                                     : (enclosingCredential + " -> ChainedTokenCredential"))
      + ": ";

  if (m_sourcesFriendlyNames.empty())
  {
    auto const sourcesSize = m_sources.size();
    for (size_t i = 1; i <= sourcesSize; ++i)
    {
      m_sourcesFriendlyNames.push_back(std::string("credential #") + std::to_string(i));
    }
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
                m_logPrefix + "Successfully got token from " + m_sourcesFriendlyNames[i] + '.');
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
                m_logPrefix + "Failed to get token from " + m_sourcesFriendlyNames[i] + ": "
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

  throw AuthenticationException("Failed to get token from ChainedTokenCredential.");
}
