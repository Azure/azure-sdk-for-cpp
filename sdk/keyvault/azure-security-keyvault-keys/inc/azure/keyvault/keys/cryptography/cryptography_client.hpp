// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief A client used to perform cryptographic operations with Azure Key Vault keys.
 *
 */

#pragma once

#include <azure/keyvault/common/internal/keyvault_pipeline.hpp>

#include <memory>
#include <string>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {

  class CryptographyClient {
  private:
    std::shared_ptr<Azure::Security::KeyVault::_internal::KeyVaultPipeline> m_pipeline;

  public:
    explicit CryptographyClient(
        std::string const& vaultUrl,
        std::shared_ptr<Core::Credentials::TokenCredential const> credential,
        KeyClientOptions options = KeyClientOptions());
  };
}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography
