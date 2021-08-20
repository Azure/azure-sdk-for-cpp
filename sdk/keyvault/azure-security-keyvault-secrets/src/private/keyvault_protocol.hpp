// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Provides a wrapper class for the Azure Core Pipeline for all Key Vault services where
 * common functionality is set up.
 *
 */

#pragma once

#include <azure/core/http/http.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_serializable.hpp>
#include <azure/core/response.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Security { namespace KeyVault { namespace _detail {

  /**
   * @brief The Protocol layer used by Key Vault clients.
   *
   */
  class KeyVaultProtocolClient final {
    Azure::Core::Url m_vaultUrl;
    Azure::Core::Http::_internal::HttpPipeline m_pipeline;
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

    /**
     * @brief Create a Key Vault request with payload.
     *
     * @param method The HTTP method.
     * @param content The HTTP payload.
     * @param path The HTTP request path.
     * @return A constructed request.
     */
    Azure::Core::Http::Request CreateRequest(
        Azure::Core::Http::HttpMethod method,
        Azure::Core::IO::BodyStream* content,
        std::vector<std::string> const& path) const;

    /**
     * @brief Start the HTTP transfer based on the \p request.
     *
     * @param context The context for per-operation options or cancellation.
     * @param request The HTTP request to be sent.
     * @return The raw response from the network.
     */
    std::unique_ptr<Azure::Core::Http::RawResponse> SendRequest(
        Azure::Core::Context const& context,
        Azure::Core::Http::Request& request) const;

  public:
    /**
     * @brief Construct a new Key Vault Protocol Client.
     *
     * @param vaultUrl The URL address for the Key Vault.
     * @param apiVersion The service API version.
     * @param pipeline The HTTP pipeline for sending requests with.
     */
    explicit KeyVaultProtocolClient(
        Azure::Core::Url vaultUrl,
        std::string apiVersion,
        Azure::Core::Http::_internal::HttpPipeline&& pipeline)
        : m_vaultUrl(std::move(vaultUrl)), m_pipeline(pipeline), m_apiVersion(std::move(apiVersion))
    {
    }

    /**
     * @brief Create and send the HTTP request. Uses the \p factoryFn function to create
     * the response type.
     *
     * @param context The context for per-operation options or cancellation.
     * @param method The method for the request.
     * @param factoryFn The function to deserialize and produce T from the raw response.
     * @param path A path for the request represented as a vector of strings.
     * @param query Optional query parameters for constructing the request.
     * @return The object produced by the \p factoryFn and the raw response from the network.
     */
    template <class T>
    Azure::Response<T> SendRequest(
        Azure::Core::Context const& context,
        Azure::Core::Http::HttpMethod method,
        std::function<T(Azure::Core::Http::RawResponse const& rawResponse)> factoryFn,
        std::vector<std::string> const& path,
        std::unique_ptr<std::map<std::string, std::string>> const& query = nullptr)
    {
      auto request = CreateRequest(method, path);
      if (query != nullptr)
      {
        for (auto const& queryParameter : *query)
        {
          request.GetUrl().AppendQueryParameter(queryParameter.first, queryParameter.second);
        }
      }
      auto response = SendRequest(context, request);
      // Saving the value in a local is required before passing it in to Response<T> to avoid
      // compiler optimizations re-ordering the `factoryFn` function call and the RawResponse move.
      T value = factoryFn(*response);
      return Azure::Response<T>(std::move(value), std::move(response));
    }

    /**
     * @brief Create and send the HTTP request with payload content. Uses the \p factoryFn function
     * to create the response type.
     *
     * @param context The context for per-operation options or cancellation.
     * @param method The method for the request.
     * @param content The HTTP payload.
     * @param factoryFn The function to deserialize and produce T from the raw response.
     * @param path A path for the request represented as a vector of strings.
     * @return The object produced by the \p factoryFn and the raw response from the network.
     */
    template <class T>
    Azure::Response<T> SendRequest(
        Azure::Core::Context const& context,
        Azure::Core::Http::HttpMethod method,
        Azure::Core::Json::_internal::JsonSerializable const& content,
        std::function<T(Azure::Core::Http::RawResponse const& rawResponse)> factoryFn,
        std::vector<std::string> const& path)
    {
      auto serialContent = content.Serialize();
      auto streamContent = Azure::Core::IO::MemoryBodyStream(
          reinterpret_cast<const uint8_t*>(serialContent.data()), serialContent.size());

      auto request = CreateRequest(method, &streamContent, path);
      auto response = SendRequest(context, request);
      // Saving the value in a local is required before passing it in to Response<T> to avoid
      // compiler optimizations re-ordering the `factoryFn` function call and the RawResponse move.
      T value = factoryFn(*response);
      return Azure::Response<T>(value, std::move(response));
    }

    template <class T>
    Azure::Response<T> SendRequest(
        Azure::Core::Context const& context,
        Azure::Core::Http::HttpMethod method,
        std::function<std::string()> serializeContentFn,
        std::function<T(Azure::Core::Http::RawResponse const& rawResponse)> factoryFn,
        std::vector<std::string> const& path)
    {
      auto serialContent = serializeContentFn();
      auto streamContent = Azure::Core::IO::MemoryBodyStream(
          reinterpret_cast<const uint8_t*>(serialContent.data()), serialContent.size());

      auto request = CreateRequest(method, &streamContent, path);
      auto response = SendRequest(context, request);
      // Saving the value in a local is required before passing it in to Response<T> to avoid
      // compiler optimizations re-ordering the `factoryFn` function call and the RawResponse move.
      T value = factoryFn(*response);
      return Azure::Response<T>(value, std::move(response));
    }

    /**
     * @brief Create a key vault request and send it using the Azure Core pipeline directly to avoid
     * checking the respone code.
     *
     * @param context A context for cancellation.
     * @param method The HTTP method for the request.
     * @param path The path for the request.
     * @return A unique ptr to an HTTP raw response.
     */
    std::unique_ptr<Azure::Core::Http::RawResponse> Send(
        Azure::Core::Context const& context,
        Azure::Core::Http::HttpMethod method,
        std::vector<std::string> const& path)
    {
      auto request = CreateRequest(method, path);
      // Use the core pipeline directly to avoid checking the response code.
      return m_pipeline.Send(request, context);
    }

    /**
     * @brief Get the Url used to create the secret client.
     *
     * @return A constant reference to the Url.
     */
    Azure::Core::Url const& GetUrl() const { return m_vaultUrl; }
  };
}}}} // namespace Azure::Security::KeyVault::_detail
