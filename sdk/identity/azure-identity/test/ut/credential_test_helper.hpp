// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/case_insensitive_containers.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/http_status_code.hpp>
#include <azure/core/http/transport.hpp>

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Azure { namespace Identity { namespace Test { namespace _detail {
  class CredentialTestHelper final {
  private:
    CredentialTestHelper() = delete;
    ~CredentialTestHelper() = delete;

  public:
    class EnvironmentOverride {
      std::map<std::string, std::string> m_originalEnv;

      static void SetVariables(std::map<std::string, std::string> const& vars);

    public:
      static bool const IsEnvironmentAvailable;

      virtual ~EnvironmentOverride() { SetVariables(m_originalEnv); }
      explicit EnvironmentOverride(std::map<std::string, std::string> const& environment);
    };

    struct TokenRequestSimulationResult final
    {
      struct Request final
      {
        Core::Http::HttpMethod HttpMethod;
        std::string AbsoluteUrl;
        Core::CaseInsensitiveMap Headers;
        std::string Body;
      };

      std::vector<Request> Requests;

      struct
      {
        std::chrono::system_clock::time_point EarliestExpiration;
        std::chrono::system_clock::time_point LatestExpiration;
        Core::Credentials::AccessToken AccessToken;
      } Response;
    };

    using CreateCredentialCallback
        = std::function<std::unique_ptr<Core::Credentials::TokenCredential>(
            std::shared_ptr<Azure::Core::Http::HttpTransport> const& transport)>;

    static TokenRequestSimulationResult SimulateTokenRequest(
        CreateCredentialCallback const& createCredential,
        Core::Credentials::TokenRequestContext const& tokenRequestContext,
        std::vector<std::pair<Core::Http::HttpStatusCode, std::string>> const& responses);

    static TokenRequestSimulationResult SimulateTokenRequest(
        CreateCredentialCallback const& createCredential,
        Core::Credentials::TokenRequestContext const& tokenRequestContext,
        std::string const& responseBody)
    {
      using Core::Http::HttpStatusCode;
      return SimulateTokenRequest(
          createCredential, tokenRequestContext, {{HttpStatusCode::Ok, responseBody}});
    }
  };
}}}} // namespace Azure::Identity::Test::_detail
