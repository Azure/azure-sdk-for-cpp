// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#if 0 // NONFUNCTIONAL
/**
 * @brief Attestation client model support classes and functions.
 *
 * This file contains private classes used to support public model types.
 *
 */

#pragma once

#include "azure/core/internal/json/json.hpp"

#include "azure/core/base64.hpp"
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "../inc/crypto.hpp"
#include "bcrypt_helpers.hpp"
#include <Windows.h>
#include <bcrypt.h>

// cspell::words BCrypt X509 BCryptX509

namespace Azure { namespace Security { namespace Attestation { namespace _internal {
  namespace Cryptography {

    /** Represents an X509 Certificate.
     *
     */
    class BCryptX509Certificate final : public X509Certificate {
      friend class Crypto;

    private:
      BCryptX509Certificate() = default;

    private:
      //      BCryptX509Certificate(_details::BCrypt_x509&& x509)
      //          : X509Certificate(), m_certificate(std::move(x509))
      //      {
      //      }

    protected:
      static std::unique_ptr<X509Certificate> CreateFromPrivateKey(
          std::unique_ptr<AsymmetricKey> const& key,
          std::string const& subjectName);

    public:
      virtual std::unique_ptr<AsymmetricKey> GetPublicKey() const override;
      virtual std::string ExportAsPEM() const override;
      std::string GetSubjectName() const override { throw std::runtime_error("Not implemented"); }

      std::string GetIssuerName() const override { throw std::runtime_error("Not implemented"); }

      static std::unique_ptr<X509Certificate> Import(std::string const& pemEncodedKey);
    };

}}}}} // namespace Azure::Security::Attestation::_private::Cryptography
#endif // NONFUNCTIONAL