// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/pipeline.hpp>
#include <azure/core/internal/json.hpp>
#include <azure/core/internal/json_serializable.hpp>
#include <azure/core/response.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace Common { namespace Internal {

  /**
   * @brief The HTTP pipeline used by KeyVault clients.
   */
  class KeyVaultPipeline {
    Azure::Core::Http::Url m_vaultUrl;
    Azure::Core::Http::HttpPipeline m_pipeline;
    std::string m_apiVersion;

    /**
     * @brief Create a Request to be sent.
     *
     * @param method Represent an HTTP method.
     * @param path The path for the HTTP request.
     * @return A constructed request.
     */
    Azure::Core::Http::Request CreateRequest(
        Azure::Core::Http::HttpMethod method,
        std::vector<std::string> const& path) const;

    Azure::Core::Http::Request CreateRequest(
        Azure::Core::Http::HttpMethod method,
        Azure::Core::Http::BodyStream* content,
        std::vector<std::string> const& path) const;

    std::unique_ptr<Azure::Core::Http::RawResponse> SendRequest(
        Azure::Core::Context const& context,
        Azure::Core::Http::Request& request) const;

  public:
    /**
     * @brief Construct a new Key Vault Pipeline.
     *
     * @param vaultUrl The url address for the Key Vault.
     * @param policies The policies to use for building the KeyVaultPipeline.
     */
    explicit KeyVaultPipeline(
        Azure::Core::Http::Url vaultUrl,
        std::string apiVersion,
        std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> const& policies)
        : m_vaultUrl(std::move(vaultUrl)), m_pipeline(policies), m_apiVersion(std::move(apiVersion))
    {
    }

    /**
     * @brief Create and send the HTTP request using. Uses the result factory function to create
     * the response type.
     *
     *
     * @param context The context for per-operation options or cancellation.
     * @param method The method for the request.
     * @param factoryFn The function to deserialize and produce T from the raw response.
     * @param path A path for the request represented as a vector of strings.
     * @return Azure::Core::Response<TResult>
     */
    template <class T>
    Azure::Core::Response<T> SendRequest(
        Azure::Core::Context const& context,
        Azure::Core::Http::HttpMethod method,
        std::function<T(Azure::Core::Http::RawResponse const& rawResponse)> factoryFn,
        std::vector<std::string> const& path)
    {
      auto request = CreateRequest(method, path);
      auto response = SendRequest(context, request);
      return Azure::Core::Response<T>(factoryFn(*response), std::move(response));
    }

    template <class T>
    Azure::Core::Response<T> SendRequest(
        Azure::Core::Context const& context,
        Azure::Core::Http::HttpMethod method,
        Azure::Core::Internal::Json::JsonSerializable const& content,
        std::function<T(Azure::Core::Http::RawResponse const& rawResponse)> factoryFn,
        std::vector<std::string> const& path)
    {
      auto serialContent = content.Serialize();
      auto streamContent = Azure::Core::Http::MemoryBodyStream(
          reinterpret_cast<const uint8_t*>(serialContent.data()), serialContent.size());

      auto request = CreateRequest(method, &streamContent, path);
      auto response = SendRequest(context, request);
      return Azure::Core::Response<T>(factoryFn(*response), std::move(response));
    }
  };
}}}}} // namespace Azure::Security::KeyVault::Common::Internal
