// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/chained_token_credential.hpp"

#include "azure/core/internal/diagnostics/log.hpp"

#include <utility>

using namespace Azure::Identity;
using namespace Azure::Core::Credentials;
using Azure::Core::Context;

ChainedTokenCredential::ChainedTokenCredential(ChainedTokenCredential::Sources sources)
    : m_sources(std::move(sources))
{
}

ChainedTokenCredential::~ChainedTokenCredential() = default;

AccessToken ChainedTokenCredential::GetToken(
    TokenRequestContext const& tokenRequestContext,
    Context const& context) const
{
  using Azure::Core::Diagnostics::Logger;
  using Azure::Core::Diagnostics::_internal::Log;

  auto n = 0;
  for (const auto& source : m_sources)
  {
    try
    {
      ++n;
      auto token = source->GetToken(tokenRequestContext, context);

      {
        const auto logLevel = Logger::Level::Informational;
        if (Log::ShouldWrite(logLevel))
        {
          Log::Write(
              logLevel,
              std::string("ChainedTokenCredential authentication attempt with credential #")
                  + std::to_string(n) + " did succeed.");
        }
      }

      return token;
    }
    catch (const AuthenticationException& e)
    {
      const auto logLevel = Logger::Level::Verbose;
      if (Log::ShouldWrite(logLevel))
      {
        Log::Write(
            logLevel,
            std::string("ChainedTokenCredential authentication attempt with credential #")
                + std::to_string(n) + " did not succeed: " + e.what());
      }
    }
  }

  if (n == 0)
  {
    const auto logLevel = Logger::Level::Verbose;
    if (Log::ShouldWrite(logLevel))
    {
      Log::Write(
          logLevel,
          "ChainedTokenCredential authentication did not succeed: list of sources is empty.");
    }
  }

  throw AuthenticationException("Failed to get token from ChainedTokenCredential.");
}