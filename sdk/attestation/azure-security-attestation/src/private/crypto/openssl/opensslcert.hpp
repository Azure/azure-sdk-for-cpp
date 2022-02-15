// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Attestation client model support classes and functions.
 *
 * This file contains private classes used to support public model types.
 *
 */

#pragma once

#include "../inc/crypto.hpp"
#include "../private/jsonhelpers_private.hpp"
#include "openssl_helpers.hpp"
#include <memory>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// cspell::words OpenSSL X509 OpenSSLX509

namespace Azure { namespace Security { namespace Attestation { namespace _detail {
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
  template <> struct type_map_helper<EVP_MD_CTX>
  {
    using type = basic_openssl_unique_ptr<EVP_MD_CTX, EVP_MD_CTX_free>;
  };

  using openssl_x509 = openssl_unique_ptr<X509>;
  using openssl_x509_name = openssl_unique_ptr<X509_NAME>;
  using openssl_asn1_time = openssl_unique_ptr<ASN1_TIME>;
  using openssl_x509_extension = openssl_unique_ptr<X509_EXTENSION>;

  /** Represents an X509 Certificate.
   *
   */
  class OpenSSLX509Certificate final : public Cryptography::X509Certificate {
    openssl_x509 m_certificate;
    friend class Cryptography;

  private:
    OpenSSLX509Certificate() = default;

  private:
    OpenSSLX509Certificate(openssl_x509&& x509) : X509Certificate(), m_certificate(std::move(x509))
    {
    }

    static openssl_x509_name ParseX509Name(std::string const& name);

    static std::string GetFormattedDnString(const X509_NAME* dn)
    {
      openssl_bio bio(make_openssl_unique(BIO_new, BIO_s_mem()));
      // Print the DN in a single line, but don't add spaces around the equals sign (mbedtls
      // doesn't add them, so if we want them to compare properly, we remove the spaces).
      int length = X509_NAME_print_ex(bio.get(), dn, 0, XN_FLAG_ONELINE & ~XN_FLAG_SPC_EQ);
      OPENSSL_CHECK_LEN(length);
      if (length == 0)
      {
        return "";
      }
      std::vector<uint8_t> formattedName(BIO_ctrl_pending(bio.get()));
      int res = BIO_read(bio.get(), formattedName.data(), static_cast<int>(formattedName.size()));
      OPENSSL_CHECK_BIO(res);
      return std::string(formattedName.begin(), formattedName.end());
    }
    static openssl_x509_extension CreateExtensionFromConfiguration(
        openssl_x509 const& subject,
        openssl_x509 const& issuer,
        int nid,
        const std::string& nidValue);

    static openssl_x509 CreateCertificate(
        std::unique_ptr<Cryptography::AsymmetricKey> const& newCertificateKey,
        std::string const& newCertificateSubject,
        std::unique_ptr<Cryptography::AsymmetricKey> const& signingKey,
        openssl_x509 const& issuer,
        time_t const currentTime,
        time_t const expirationTime,
        bool isLeafCertificate);

  protected:
    static std::unique_ptr<X509Certificate> CreateFromPrivateKey(
        std::unique_ptr<Cryptography::AsymmetricKey> const& key,
        std::string const& subjectName);

  public:
    virtual std::unique_ptr<Cryptography::AsymmetricKey> GetPublicKey() const override;
    virtual std::string ExportAsPEM() const override;
    virtual std::string ExportAsBase64() const override;
    std::string GetSubjectName() const override
    {
      return GetFormattedDnString(X509_get_subject_name(m_certificate.get()));
    }

    std::string GetIssuerName() const override
    {
      return GetFormattedDnString(X509_get_issuer_name(m_certificate.get()));
    }

    std::string GetThumbprint() const override
    {
      // X.509 thumbprints are calculated using SHA1, even though SHA1 is insecure.
      auto hash(make_openssl_unique(EVP_MD_CTX_new));
      EVP_DigestInit_ex(hash.get(), EVP_sha1(), nullptr);

      int len = i2d_X509(m_certificate.get(), nullptr);
      std::vector<uint8_t> thumbprintBuffer(len);
      unsigned char* buf = thumbprintBuffer.data();
      OPENSSL_CHECK_LEN(i2d_X509(m_certificate.get(), &buf));
      OPENSSL_CHECK(EVP_DigestUpdate(hash.get(), buf, thumbprintBuffer.size()));
      uint32_t hashLength = EVP_MAX_MD_SIZE;
      std::vector<uint8_t> hashedThumbprint(EVP_MAX_MD_SIZE);
      OPENSSL_CHECK(EVP_DigestFinal_ex(hash.get(), hashedThumbprint.data(), &hashLength));
      hashedThumbprint.resize(hashLength);

      auto hexThumbprint(JsonHelpers::BinaryToHexString(hashedThumbprint));
      // HexString uses an "a"-"f" alphabet, but the CLR hex encoder uses an "A"-"F" alphabet,
      // convert between them.
      for (auto& ch : hexThumbprint)
      {
        if (ch >= 'a' && ch <= 'f')
        {
          ch = static_cast<char>(toupper(ch));
        }
      }

      return hexThumbprint;
    }
    std::string GetAlgorithm() const override
    {
      X509_PUBKEY* pubkey = X509_get_X509_PUBKEY(m_certificate.get());
      ASN1_OBJECT* asn1Algorithm;
      const unsigned char* publicKey;
      int publicKeyLen;
      X509_ALGOR* algorithm;
      OPENSSL_CHECK(
          X509_PUBKEY_get0_param(&asn1Algorithm, &publicKey, &publicKeyLen, &algorithm, pubkey));

      int nid = OBJ_obj2nid(asn1Algorithm);
      if (nid == NID_rsaEncryption)
      {
        return "RS256";
      }
      else if (nid == NID_X9_62_id_ecPublicKey)
      {
        return "EC";
      }
      AZURE_ASSERT(false);
      abort();
    }

    std::string GetKeyType() const override
    {
      X509_PUBKEY* pubkey = X509_get_X509_PUBKEY(m_certificate.get());
      ASN1_OBJECT* asn1Algorithm;
      const unsigned char* publicKey;
      int publicKeyLen;
      X509_ALGOR* algorithm;
      OPENSSL_CHECK(
          X509_PUBKEY_get0_param(&asn1Algorithm, &publicKey, &publicKeyLen, &algorithm, pubkey));
      int nid = OBJ_obj2nid(asn1Algorithm);

      if (nid == NID_rsaEncryption)
      {
        return "RSA";
      }
      else if (nid == NID_X9_62_id_ecPublicKey)
      {
        return "EC";
      }
      AZURE_ASSERT(false);
      abort();
    }

    static std::unique_ptr<X509Certificate> Import(std::string const& pemEncodedKey);
  };

}}}} // namespace Azure::Security::Attestation::_detail