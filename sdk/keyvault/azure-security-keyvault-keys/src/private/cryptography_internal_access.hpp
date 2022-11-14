//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Provides access to the private content from a cryptographic client.
 *
 */

#pragma once

#include <azure/core/http/http.hpp>

#include "azure/keyvault/keys/cryptography/cryptography_client.hpp"

#include <string>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {
  namespace _detail {

    class CryptoClientInternalAccess {
      CryptoClientInternalAccess() = delete;

    public:
      static CryptographyClient CreateCryptographyClient(
          Azure::Core::Url keyId,
          std::string const& apiVersion,
          std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> pipeline)
      {
        return CryptographyClient(keyId, apiVersion, pipeline);
      }
    };

}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail
