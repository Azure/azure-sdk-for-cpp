﻿// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Defines the Key Vault Administration client.
 *
 */

#pragma once
#include "administration_client_options.hpp"
#include <azure/core/context.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/response.hpp>

#include <memory>
#include <string>

namespace Azure { namespace Security { namespace KeyVault { namespace Administration {
#if defined(TESTING_BUILD)
  namespace Test {
    class KeyVaultCertificateClientTest;
  }
#endif
  /**
   * @brief The CertificateClient provides synchronous methods to manage KeyVaultCertificate in
   * Azure Key Vault.
   *
   * @details The client supports retrieving KeyVaultCertificate.
   */
  class AdministrationClient
#if !defined(TESTING_BUILD)
      final
#endif
  {

  private:
    // Using a shared pipeline for a client to share it with LRO (like delete key)
    Azure::Core::Url m_vaultUrl;
    std::string m_apiVersion;
    std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;

  public:
    /**
     * @brief Destructor.
     *
     */
    virtual ~AdministrationClient() = default;

    /**
     * @brief Construct a new administration Client object
     *
     * @param vaultUrl The URL address where the client will send the requests to.
     * @param credential The authentication method to use.
     * @param options The options to customize the client behavior.
     */
    explicit AdministrationClient(
        std::string const& vaultUrl,
        std::shared_ptr<Core::Credentials::TokenCredential const> credential,
        AdministrationClientOptions options = AdministrationClientOptions());

    /**
     * @brief Construct a new administration Client object from another administration client.
     *
     * @param administration Client An existing key vault administration client.
     */
    explicit AdministrationClient(AdministrationClient const& administrationClient) = default;

  private:

    std::unique_ptr<Azure::Core::Http::RawResponse> SendRequest(
        Azure::Core::Http::Request& request,
        Azure::Core::Context const& context) const;

    Azure::Core::Http::Request CreateRequest(
        Azure::Core::Http::HttpMethod method,
        std::vector<std::string> const& path = {},
        Azure::Core::IO::BodyStream* content = nullptr) const;

    Azure::Core::Http::Request ContinuationTokenRequest(
        std::vector<std::string> const& path,
        const Azure::Nullable<std::string>& NextPageToken) const;
  };
}}}} // namespace Azure::Security::KeyVault::Certificates
