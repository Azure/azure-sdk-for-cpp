// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Provides access to the private content from a cryptographic client.
 *
 */

#pragma once

#include "azure/keyvault/keys/cryptography/cryptography_client.hpp"

#include <azure/core/http/http.hpp>

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
        return CryptographyClient(std::move(keyId), apiVersion, std::move(pipeline));
      }
    };

}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail
