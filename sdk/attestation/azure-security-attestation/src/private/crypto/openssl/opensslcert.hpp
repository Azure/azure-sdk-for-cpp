// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Attestation client model support classes and functions.
 *
 * This file contains private classes used to support public model types.
 *
 */

#pragma once

#include <azure/core/internal/json/json.hpp>

#include <azure/core/base64.hpp>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "..\inc\crypto.hpp"
#include "openssl_helpers.hpp"
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>

// cspell::words OpenSSL X509

namespace Azure { namespace Security { namespace Attestation { namespace _private {
  namespace Cryptography {

    namespace _details {
      template <> struct type_map_helper<X509>
      {
        using type = basic_openssl_unique_ptr<X509, X509_free>;
      };

      template <> struct type_map_helper<X509_NAME>
      {
        using type = basic_openssl_unique_ptr<X509_NAME, X509_NAME_free>;
      };

      template <> struct type_map_helper<ASN1_TIME>
      {
          using type = basic_openssl_unique_ptr<ASN1_TIME, ASN1_TIME_free>;
      };
      template <> struct type_map_helper<X509_EXTENSION>
      {
        using type = basic_openssl_unique_ptr<X509_EXTENSION, X509_EXTENSION_free>;
      };

      using openssl_x509=openssl_unique_ptr<X509>;
      using openssl_x509_name = openssl_unique_ptr<X509_NAME>;
      using openssl_asn1_time = openssl_unique_ptr<ASN1_TIME>;
      using openssl_x509_extension= openssl_unique_ptr<X509_EXTENSION>;
    } // namespace _details

    /** Represents an X509 Certificate.
     *
     */
    class OpenSSLX509Certificate final : public X509Certificate {
      _details::openssl_x509 m_certificate;
      friend class Crypto;
    private:
      OpenSSLX509Certificate() = default;

    private:
      OpenSSLX509Certificate(_details::openssl_x509 && x509) 
          : X509Certificate()
          , m_certificate(std::move(x509)) {}

      static _details::openssl_x509_name ParseX509Name(std::string const& name);

      static std::string GetFormattedDnString(const X509_NAME* dn)
      {
        _details::openssl_bio bio(_details::make_openssl_unique(BIO_new, BIO_s_mem()));
        // Print the DN in a single line, but don't add spaces around the equals sign (mbedtls
        // doesn't add them, so if we want them to compare properly, we remove the spaces).
        int length = X509_NAME_print_ex(bio.get(), dn, 0, XN_FLAG_ONELINE & ~XN_FLAG_SPC_EQ);
        if (length < 0)
        {
          throw _details::OpenSSLException("X509_NAME_print_ex");
        }
        if (length == 0)
        {
          return "";
        }
        std::vector<uint8_t> formattedName(BIO_ctrl_pending(bio.get()));
        int res = BIO_read(bio.get(), formattedName.data(), static_cast<int>(formattedName.size()));
        if (res == 0 || res == -1 || res == -2)
        {
          throw _details::OpenSSLException("BIO_read");
        }
        return std::string(formattedName.begin(), formattedName.end());
      }
      static _details::openssl_x509_extension CreateExtensionFromConfiguration(
          _details::openssl_x509 const& subject,
          _details::openssl_x509 const& issuer,
          int nid,
          const std::string& nidValue);

      static _details::openssl_x509 CreateCertificate(
          std::unique_ptr<AsymmetricKey> const& newCertificateKey,
          std::string const& newCertificateSubject,
          std::unique_ptr<AsymmetricKey> const& signingKey,
          _details::openssl_x509 const& issuer,
          time_t const currentTime,
          time_t const expirationTime,
          bool isLeafCertificate);

    protected:
      static std::unique_ptr<X509Certificate> CreateFromPrivateKey(
          std::unique_ptr<AsymmetricKey> const& key,
          std::string const& subjectName);
    public:

      virtual std::unique_ptr<AsymmetricKey> GetPublicKey() const override;
      virtual std::string ExportAsPEM() const override;
      std::string GetSubjectName() const
      {
        return GetFormattedDnString(X509_get_subject_name(m_certificate.get()));
      }

      std::string GetIssuerName() const
      {
        return GetFormattedDnString(X509_get_issuer_name(m_certificate.get()));
      }

      static std::unique_ptr<X509Certificate> Import(std::string const& pemEncodedKey);
    };

}}}}} // namespace Azure::Security::Attestation::_private::Cryptography