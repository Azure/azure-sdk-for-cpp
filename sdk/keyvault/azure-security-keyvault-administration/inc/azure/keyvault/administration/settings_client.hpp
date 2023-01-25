// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the Key Vault Administration client.
 *
 */

#pragma once
#include <azure/core/context.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/response.hpp>
#include <azure/keyvault/administration/rest_client_models.hpp>
#include <azure/keyvault/administration/settings_client_options.hpp>
#include <memory>
#include <string>

using namespace Azure::Security::KeyVault::Administration::Models;

namespace Azure { namespace Security { namespace KeyVault { namespace Administration {
  /**
   * @brief Settings Client class.
   */
  class SettingsClient final {

  private:
    Azure::Core::Url m_vaultUrl;
    std::string m_apiVersion;
    // Using a shared pipeline for a client to share it with LRO (like delete key).
    std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;

  public:
    /**
     * @brief Destructor.
     *
     */
    virtual ~SettingsClient() = default;

    /**
     * @brief Construct a new settings client object.
     *
     * @param vaultUrl The URL address where the client will send the requests to.
     * @param credential The authentication method to use.
     * @param options The options to customize the client behavior.
     */
    explicit SettingsClient(
        std::string const& vaultUrl,
        std::shared_ptr<Core::Credentials::TokenCredential const> credential,
        SettingsClientOptions options = SettingsClientOptions());

    /**
     * @brief Construct a new settings client object from another settings client.
     *
     * @param settingsClient An existing key vault settings client.
     */
    explicit SettingsClient(SettingsClient const& settingsClient) = default;

  public:
    /**
     * @brief Update a setting.
     *
     * @param name Name of the setting to update.
     * @param value Value of the settnig .
     * @param context Operation Context.
     *
     * @returns Response containing the new updated setting.
     */
    Azure::Response<Setting> UpdateSetting(
        std::string const& name,
        std::string const& value,
        const Azure::Core::Context& context = Azure::Core::Context{}) const;

    /**
     * @brief Gets an existing setting.
     *
     * @param name Name of setting to get.
     * @param context Operation context.
     *
     * @returns response containing the setting.
     */
    Azure::Response<Setting> GetSetting(
        std::string const& name,
        const Azure::Core::Context& context = Azure::Core::Context{}) const;

    /**
     * @brief Gets all settings.
     *
     * @param context Operation context.
     *
     * @returns Response containing a list of settings.
     */
    Azure::Response<SettingsListResult> GetSettings(
        const Azure::Core::Context& context = Azure::Core::Context{}) const;

  private:
    Setting ParseSetting(std::vector<uint8_t> const& responseBody) const;

    std::unique_ptr<Azure::Core::Http::RawResponse> SendRequest(
        Azure::Core::Http::Request& request,
        Azure::Core::Context const& context) const;

    Azure::Core::Http::Request CreateRequest(
        Azure::Core::Http::HttpMethod method,
        std::vector<std::string> const& path = {},
        Azure::Core::IO::BodyStream* content = nullptr) const;
  };
}}}} // namespace Azure::Security::KeyVault::Administration
