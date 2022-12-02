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

#include "azure/keyvault/keys/cryptography/cryptography_client_models.hpp"

#include "key_sign_parameters.hpp"
#include "key_verify_parameters.hpp"
#include "key_wrap_parameters.hpp"

#include <string>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {
  namespace _detail {

    /***************** Encrypt Result *****************/
    class EncryptResultSerializer final {
    public:
      static EncryptResult EncryptResultDeserialize(
          Azure::Core::Http::RawResponse const& rawResponse);
    };

    /***************** Encrypt Parameters *****************/
    class EncryptParametersSerializer final {
    public:
      static std::string EncryptParametersSerialize(
          Azure::Security::KeyVault::Keys::Cryptography::EncryptParameters const& parameters);
    };

    /***************** Decrypt Result *****************/
    class DecryptResultSerializer final {
    public:
      static DecryptResult DecryptResultDeserialize(
          Azure::Core::Http::RawResponse const& rawResponse);
    };

    /***************** Encrypt Parameters *****************/
    class DecryptParametersSerializer final {
    public:
      static std::string DecryptParametersSerialize(
          Azure::Security::KeyVault::Keys::Cryptography::DecryptParameters const& parameters);
    };

    /***************** WrapKey Result *****************/
    class WrapResultSerializer final {
    public:
      static WrapResult WrapResultDeserialize(Azure::Core::Http::RawResponse const& rawResponse);
    };

    /***************** WrapKey Parameters *****************/
    class KeyWrapParametersSerializer final {
    public:
      static std::string KeyWrapParametersSerialize(
          Azure::Security::KeyVault::Keys::Cryptography::_detail::KeyWrapParameters const&
              parameters);
    };

    /***************** UnwrapKey Result *****************/
    class UnwrapResultSerializer final {
    public:
      static UnwrapResult UnwrapResultDeserialize(
          Azure::Core::Http::RawResponse const& rawResponse);
    };

    /***************** Sign Result *****************/
    class SignResultSerializer final {
    public:
      static SignResult SignResultDeserialize(Azure::Core::Http::RawResponse const& rawResponse);
    };

    /***************** Key sign Parameters *****************/
    class KeySignParametersSerializer final {
    public:
      static std::string KeySignParametersSerialize(
          Azure::Security::KeyVault::Keys::Cryptography::_detail::KeySignParameters const&
              parameters);
    };

    /***************** Verify Result *****************/
    class VerifyResultSerializer final {
    public:
      static VerifyResult VerifyResultDeserialize(
          Azure::Core::Http::RawResponse const& rawResponse);
    };

    /***************** Key Verify Parameters *****************/
    class KeyVerifyParametersSerializer final {
    public:
      static std::string KeyVerifyParametersSerialize(
          Azure::Security::KeyVault::Keys::Cryptography::_detail::KeyVerifyParameters const&
              parameters);
    };

}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_detail