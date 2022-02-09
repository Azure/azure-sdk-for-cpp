// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#if 0 // NOTFUNCTIONAL
/**
 * @brief Attestation client model support classes and functions.
 *
 * This file contains private classes used to support public model types.
 *
 */

#include "azure/core/internal/json/json.hpp"

#include "azure/core/base64.hpp"
#include <ctime>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "../inc/crypto.hpp"
#include "bcryptcert.hpp"
#include "bcryptkeys.hpp"

// cspell::words BCrypt X509 BCryptX509

namespace Azure { namespace Security { namespace Attestation { namespace _internal {
  namespace Cryptography {

    std::unique_ptr<X509Certificate> BCryptX509Certificate::Import(
        std::string const& )
    {
      throw std::runtime_error("Not implemented");
    }

    std::string BCryptX509Certificate::ExportAsPEM() const
    {
      throw std::runtime_error("Not implemented");
    }

    std::unique_ptr<X509Certificate> BCryptX509Certificate::CreateFromPrivateKey(
        std::unique_ptr<AsymmetricKey> const& ,
        std::string const& )
    {
      throw std::runtime_error("Not implemented");
    }

    std::unique_ptr<AsymmetricKey> BCryptX509Certificate::GetPublicKey() const
    {
      throw std::runtime_error("Not implemented");
    }
}}}}} // namespace Azure::Security::Attestation::_internal::Cryptography
#endif // NOTFUNCTIONAL