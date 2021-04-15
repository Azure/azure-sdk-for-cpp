// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief A client used to perform cryptographic operations with Azure Key Vault keys.
 *
 */

#pragma once

#include <memory>
#include <string>

namespace Azure {
  namespace Security {
    namespace KeyVault {
      namespace Keys {
        namespace Cryptography {
  namespace _internal {

    struct CryptographyProvider
    {
      virtual ~CryptographyProvider() = default;

      virtual bool CanRemote() const = 0;
    };
}}}}}} // namespace Azure::Security::KeyVault::Keys::Cryptography::_internal
