// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief A client used to perform cryptographic operations with Azure Key Vault keys.
 *
 */

#pragma once

#include <azure/core/url.hpp>

#include <azure/keyvault/common/internal/keyvault_pipeline.hpp>

#include "azure/keyvault/keys/cryptography/cryptography_client_options.hpp"
#include "azure/keyvault/keys/cryptography/cryptography_provider.hpp"

#include <memory>
#include <string>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {
  namespace _internal {

    struct RemoteCryptographyClient
        : public Azure::Security::KeyVault::Keys::Cryptography::_internal::CryptographyProvider
    {

      std::shared_ptr<Azure::Security::KeyVault::_internal::KeyVaultPipeline> Pipeline;
      Azure::Core::Url KeyId;

      explicit RemoteCryptographyClient(
          std::string const& vaultUrl,
          std::shared_ptr<Core::Credentials::TokenCredential const> credential,
          CryptographyClientOptions options = CryptographyClientOptions());

      bool CanRemote() const override { return true; }
    };
}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_internal
