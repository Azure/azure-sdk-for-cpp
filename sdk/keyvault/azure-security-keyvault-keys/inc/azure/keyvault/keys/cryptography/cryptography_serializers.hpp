// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Centralize the serialize and de-serialize methods for the key vault keys Cryptography
 * models.
 *
 */

#pragma once

#include <azure/core/http/http.hpp>
#include <azure/core/internal/json/json.hpp>

#include "azure/keyvault/keys/cryptography/decrypt_parameters.hpp"
#include "azure/keyvault/keys/cryptography/decrypt_result.hpp"
#include "azure/keyvault/keys/cryptography/encrypt_parameters.hpp"
#include "azure/keyvault/keys/cryptography/encrypt_result.hpp"

#include <string>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {
  namespace _detail {

    /***************** Encrypt Result *****************/
    struct EncryptResultSerializer
    {
      static EncryptResult EncryptResultDeserialize(
          Azure::Core::Http::RawResponse const& rawResponse);
    }; // namespace EncryptResultSerializer

    /***************** Encrypt Parameters *****************/
    struct EncryptParametersSerializer
    {
      static std::string EncryptParametersSerialize(
          Azure::Security::KeyVault::Keys::Cryptography::EncryptParameters const& parameters);
    };

    /***************** Decrypt Result *****************/
    struct DecryptResultSerializer
    {
      static DecryptResult DecryptResultDeserialize(
          Azure::Core::Http::RawResponse const& rawResponse);
    }; // namespace DecryptResultSerializer

    /***************** Encrypt Parameters *****************/
    struct DecryptParametersSerializer
    {
      static std::string DecryptParametersSerialize(
          Azure::Security::KeyVault::Keys::Cryptography::DecryptParameters const& parameters);
    };

}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail
