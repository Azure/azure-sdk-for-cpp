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
#include "azure/keyvault/keys/cryptography/key_sign_parameters.hpp"
#include "azure/keyvault/keys/cryptography/key_verify_parameters.hpp"
#include "azure/keyvault/keys/cryptography/key_wrap_parameters.hpp"
#include "azure/keyvault/keys/cryptography/sign_result.hpp"
#include "azure/keyvault/keys/cryptography/unwrap_result.hpp"
#include "azure/keyvault/keys/cryptography/verify_result.hpp"
#include "azure/keyvault/keys/cryptography/wrap_result.hpp"

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
    };

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
    };

    /***************** Encrypt Parameters *****************/
    struct DecryptParametersSerializer
    {
      static std::string DecryptParametersSerialize(
          Azure::Security::KeyVault::Keys::Cryptography::DecryptParameters const& parameters);
    };

    /***************** WrapKey Result *****************/
    struct WrapResultSerializer
    {
      static WrapResult WrapResultDeserialize(Azure::Core::Http::RawResponse const& rawResponse);
    };

    /***************** WrapKey Parameters *****************/
    struct KeyWrapParametersSerializer
    {
      static std::string KeyWrapParametersSerialize(
          Azure::Security::KeyVault::Keys::Cryptography::_detail::KeyWrapParameters const&
              parameters);
    };

    /***************** UnwrapKey Result *****************/
    struct UnwrapResultSerializer
    {
      static UnwrapResult UnwrapResultDeserialize(
          Azure::Core::Http::RawResponse const& rawResponse);
    };

    /***************** Sign Result *****************/
    struct SignResultSerializer
    {
      static SignResult SignResultDeserialize(Azure::Core::Http::RawResponse const& rawResponse);
    };

    /***************** Key sign Parameters *****************/
    struct KeySignParametersSerializer
    {
      static std::string KeySignParametersSerialize(
          Azure::Security::KeyVault::Keys::Cryptography::_detail::KeySignParameters const&
              parameters);
    };

    /***************** Verify Result *****************/
    struct VerifyResultSerializer
    {
      static VerifyResult VerifyResultDeserialize(
          Azure::Core::Http::RawResponse const& rawResponse);
    };

    /***************** Key Verify Parameters *****************/
    struct KeyVerifyParametersSerializer
    {
      static std::string KeyVerifyParametersSerialize(
          Azure::Security::KeyVault::Keys::Cryptography::_detail::KeyVerifyParameters const&
              parameters);
    };

}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail
