//  Copyright (c) Microsoft Corporation. All rights reserved.
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
      virtual ~EnvironmentOverride() { SetVariables(m_originalEnv); }
      explicit EnvironmentOverride(std::map<std::string, std::string> const& environment);
    };

    struct TokenRequestSimulationResult final
    {
      struct RequestInfo final
      {
        Core::Http::HttpMethod HttpMethod;
        std::string AbsoluteUrl;
        Core::CaseInsensitiveMap Headers;
        std::string Body;
      };

      struct ResponseInfo
      {
        std::chrono::system_clock::time_point EarliestExpiration;
        std::chrono::system_clock::time_point LatestExpiration;
        Core::Credentials::AccessToken AccessToken;
      };

      std::vector<RequestInfo> Requests;
      std::vector<ResponseInfo> Responses;
    };

    struct TokenRequestSimulationServerResponse
    {
      Core::Http::HttpStatusCode StatusCode;
      std::string Body;
      Core::CaseInsensitiveMap Headers;
    };

    using CreateCredentialCallback
        = std::function<std::unique_ptr<Core::Credentials::TokenCredential const>(
            std::shared_ptr<Azure::Core::Http::HttpTransport> const& transport)>;

    using GetTokenCallback = std::function<Core::Credentials::AccessToken(
        Core::Credentials::TokenCredential const& credential,
        Core::Credentials::TokenRequestContext const& tokenRequestContext,
        Core::Context const& context)>;

    static GetTokenCallback const DefaultGetToken;

    static TokenRequestSimulationResult SimulateTokenRequest(
        CreateCredentialCallback const& createCredential,
        std::vector<decltype(Core::Credentials::TokenRequestContext::Scopes)> const&
            tokenRequestContextScopes,
        std::vector<TokenRequestSimulationServerResponse> const& responses,
        GetTokenCallback getToken = DefaultGetToken);

    static TokenRequestSimulationResult SimulateTokenRequest(
        CreateCredentialCallback const& createCredential,
        std::vector<decltype(Core::Credentials::TokenRequestContext::Scopes)> const&
            tokenRequestContextScopes,
        std::vector<std::string> const& responseBodies,
        GetTokenCallback getToken = DefaultGetToken)
    {
      using Core::Http::HttpStatusCode;
      std::vector<TokenRequestSimulationServerResponse> responses;
      for (auto const& responseBody : responseBodies)
      {
        responses.push_back({HttpStatusCode::Ok, responseBody, {}});
      }

      return SimulateTokenRequest(createCredential, tokenRequestContextScopes, responses, getToken);
    }
  };
}}}} // namespace Azure::Identity::Test::_detail
