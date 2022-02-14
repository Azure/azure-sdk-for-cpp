// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Attestation client model support classes and functions.
 *
 * This file contains private classes used to support public model types.
 *
 */

#include "opensslcert.hpp"
#include "../inc/crypto.hpp"
#include "openssl_helpers.hpp"
#include "opensslkeys.hpp"
#include <cstring>
#include <ctime>
#include <memory>
#include <openssl/asn1.h>
#include <openssl/bio.h>
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/x509v3.h>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// cspell::words OpenSSL X509 OpenSSLX509

namespace Azure { namespace Security { namespace Attestation { namespace _detail {

  std::unique_ptr<Cryptography::X509Certificate> OpenSSLX509Certificate::Import(
      std::string const& pemEncodedString)
  {
    auto bio(make_openssl_unique(
        BIO_new_mem_buf, pemEncodedString.data(), static_cast<int>(pemEncodedString.size())));
    X509* raw_x509 = nullptr;
    raw_x509 = PEM_read_bio_X509(bio.get(), nullptr, nullptr, nullptr);
    OPENSSL_CHECK_NULL(raw_x509);
    openssl_x509 x509(raw_x509);
    raw_x509 = nullptr;
    return std::unique_ptr<OpenSSLX509Certificate>(new OpenSSLX509Certificate(std::move(x509)));
  }

  std::string OpenSSLX509Certificate::ExportAsPEM() const
  {
    auto bio(make_openssl_unique(BIO_new, BIO_s_mem()));
    OPENSSL_CHECK(PEM_write_bio_X509(bio.get(), m_certificate.get()));
    std::vector<uint8_t> returnValue(BIO_ctrl_pending(bio.get()));

    int res = BIO_read(bio.get(), returnValue.data(), static_cast<int>(returnValue.size()));
    OPENSSL_CHECK_BIO(res);
    return std::string(returnValue.begin(), returnValue.end());
  }

  std::string OpenSSLX509Certificate::ExportAsBase64() const
  {
    // Create a base64 encoding BIO - this will base64 encode all data written to it.
    auto base64bio(make_openssl_unique(BIO_new, BIO_f_base64()));
    BIO_set_flags(base64bio.get(), BIO_FLAGS_BASE64_NO_NL);

    // Allocate a raw BIO that will hold the actual base64 encoded output.
    auto bio(make_openssl_unique(BIO_new, BIO_s_mem()));

    // And associate the output BIO with the base64 bio. Note that by associating these
    // the base64bio takes ownership of the output BIO.
    BIO_push(base64bio.get(), bio.release());
    // Serialize the certificate as a Base64 encoded DER encoded blob into the bio.
    OPENSSL_CHECK(i2d_X509_bio(base64bio.get(), m_certificate.get()));
    OPENSSL_CHECK(BIO_flush(base64bio.get()));

    // Now that we've written to the underlying bio, pop it back
    // to the bio local so we can retrieve the base64 data which was written.
    bio.reset(BIO_pop(base64bio.get()));

    uint8_t* base64data;
    long bufferSize = BIO_get_mem_data(bio.get(), &base64data);
    std::string returnValue;
    returnValue.resize(bufferSize);
    memcpy(&returnValue[0], base64data, bufferSize);

    return returnValue;
  }

  /**
   * @brief Trim whitespace from the start of the string.
   *
   * @param s String to trim.
   */
  static inline void ltrim(std::string& s) { s.erase(s.find_last_not_of(" \t\r\n") + 1); }

  /**
   * @brief Trim whitespace from the end of the string.
   *
   * @param s String to trim.
   */
  static inline void rtrim(std::string& s) { s.erase(0, s.find_first_not_of(" \t\r\n")); }

  /**
   * @brief Trim whitespace from the start and of the string.
   *
   * @param s String to trim.
   */
  static inline void trim(std::string& s)
  {
    rtrim(s);
    ltrim(s);
  }

  openssl_x509_name OpenSSLX509Certificate::ParseX509Name(std::string const& name)
  {
    openssl_x509_name returnValue(make_openssl_unique(X509_NAME_new));
    // List of name components - pair.first == the type (CN", pair.second == the value
    // ("Subject").
    std::vector<std::pair<std::string, std::string>> components;

    std::string type;
    std::string value;
    bool collectType = true;
    bool collectValue = false;
    for (auto it = name.begin(); it != name.end(); ++it)
    {
      auto ch = *it;
      if (ch == '=')
      {
        collectType = false;
        collectValue = true;
      }
      else if (ch == ',')
      {
        if (!value.empty() && !type.empty())
        {
          trim(type);
          trim(value);
          components.push_back(std::make_pair(type, value));
        }
        collectType = true;
        collectValue = false;
        type.clear();
        value.clear();
      }
      else
      {
        // Enable quoting of characters.
        if (ch == '\\')
        {
          ch = *++it;
        }
        if (collectType)
        {
          type.push_back(ch);
        }
        else if (collectValue)
        {
          value.push_back(ch);
        }
      }
    }

    // Include the final element.
    if (!value.empty() && !type.empty())
    {
      trim(type);
      trim(value);
      components.push_back(std::make_pair(type, value));
    }
    for (auto comp : components)
    {
      int nid = OBJ_txt2nid(comp.first.c_str());
      if (nid == NID_undef)
      {
        throw std::runtime_error("Could not parse unknown attribute " + comp.first);
      }
      OPENSSL_CHECK(X509_NAME_add_entry_by_NID(
          returnValue.get(),
          nid,
          MBSTRING_UTF8,
          reinterpret_cast<const unsigned char*>(comp.second.c_str()),
          static_cast<int>(comp.second.length()),
          -1,
          0));
    }
    return returnValue;
  }

  openssl_x509_extension OpenSSLX509Certificate::CreateExtensionFromConfiguration(
      openssl_x509 const& subject,
      openssl_x509 const& issuer,
      int nid,
      const std::string& nidValue)
  {
    X509V3_CTX context;

    X509V3_set_ctx_nodb(&context); // cspell: disable-line
    X509V3_set_ctx(&context, issuer.get(), subject.get(), nullptr, nullptr, 0);

    return make_openssl_unique(X509V3_EXT_conf_nid, nullptr, &context, nid, nidValue.c_str());
  }

  /**
   * @brief Create or derive a new X.509 certificate.
   *
   * @param newCertificateKey Public key for the new certificate.
   * @param newCertificateSubject Subject name for the new certificate.
   * @param signingKey Private key used to sign the new certificate. This can be the same as the
   * newCertificateKey or it can be an issuer private key.
   * @param issuer If this is to be a derived certificate, the issuer of the certificat, or null if
   * this is self signed.
   * @param currentTime The issuance time (in UTC) for the certificate.
   * @param expirationTime The expiration time (in UTC) for the certificate.
   * @param isLeafCertificate True if this is a leaf certificate.
   * @return openssl_x509 Returns the newly created certificate.
   */
  openssl_x509 OpenSSLX509Certificate::CreateCertificate(
      std::unique_ptr<Cryptography::AsymmetricKey> const& newCertificateKey,
      std::string const& newCertificateSubject,
      std::unique_ptr<Cryptography::AsymmetricKey> const& signingKey,
      openssl_x509 const& issuer,
      time_t const currentTime,
      time_t const expirationTime,
      bool isLeafCertificate)
  {
    openssl_x509 certificate(make_openssl_unique(X509_new));

    openssl_x509_name subjectName(ParseX509Name(newCertificateSubject));

    if (X509_set_subject_name(certificate.get(), subjectName.get()) != 1)
    {
      throw OpenSSLException("X509_set_subject_name");
    }

    if (issuer)
    {
      OPENSSL_CHECK(X509_set_issuer_name(certificate.get(), X509_get_subject_name(issuer.get())));
    }
    else
    {
      OPENSSL_CHECK(X509_set_issuer_name(certificate.get(), subjectName.get()));
    }

    // Export the key to be included in the certificate.
    {
      auto exportedPublicKey = newCertificateKey->ExportPublicKey();
      auto publicKey = Cryptography::ImportPublicKey(exportedPublicKey);
      // We know that Crypto::ImportPublicKey always returns an OpenSSLAsymmetricKey.
      // THis is a bit of a hack but it's an acceptable assumption to make.
      OpenSSLAsymmetricKey* key = static_cast<OpenSSLAsymmetricKey*>(publicKey.get());
      OPENSSL_CHECK(X509_set_pubkey(certificate.get(), key->GetKey().get()));
    }

    OPENSSL_CHECK(X509_set_version(certificate.get(), 2)); // Version 3 certificate.

    // Transfer the serial number from the current certificate to the child if this is a
    // derived certificate.
    if (issuer)
    {
      OPENSSL_CHECK(X509_set_serialNumber(certificate.get(), X509_get_serialNumber(issuer.get())));
    }
    else
    {
      auto serialNumber(make_openssl_unique(ASN1_INTEGER_new));
      OPENSSL_CHECK(ASN1_INTEGER_set(serialNumber.get(), 1));
      OPENSSL_CHECK(X509_set_serialNumber(certificate.get(), serialNumber.get()));
    }

    {
      auto extension = CreateExtensionFromConfiguration(
          certificate,
          certificate,
          NID_basic_constraints,
          (isLeafCertificate ? "CA:FALSE" : "CA:TRUE, pathlen:0"));
      OPENSSL_CHECK(X509_add_ext(certificate.get(), extension.get(), -1));
    }

    { // Set Not Before Time (time before which certificate is not valid).
      openssl_asn1_time notBeforeTime(
          make_openssl_unique(ASN1_TIME_adj, nullptr, currentTime, 0, 0));

      OPENSSL_CHECK(X509_set1_notBefore(certificate.get(), notBeforeTime.get()));
    }

    // Set Not After Time (time after which certificate is not valid).
    {
      openssl_asn1_time notAfterTime(
          make_openssl_unique(ASN1_TIME_adj, nullptr, expirationTime, 0, 0));

      OPENSSL_CHECK(X509_set1_notAfter(certificate.get(), notAfterTime.get()));
    }

    { // Add the subject Key ID - this is the thumbprint of the public key. Note that we have to
      // have called X509_set_pubkey before this call.
      auto extension = CreateExtensionFromConfiguration(
          certificate, certificate, NID_subject_key_identifier, "hash");
      OPENSSL_CHECK(X509_add_ext(certificate.get(), extension.get(), -1));
    } // namespace _internal

    // Add the authority Key ID - Note that this needs to be done *after* setting the subject key
    // identifier.
    {
      auto extension(make_openssl_unique(X509_EXTENSION_new));
      if (issuer)
      {
        extension = CreateExtensionFromConfiguration(
            certificate, issuer, NID_authority_key_identifier, "keyid:always");
      }
      else
      {
        extension = CreateExtensionFromConfiguration(
            certificate, certificate, NID_authority_key_identifier, "keyid:always");
      }
      OPENSSL_CHECK(X509_add_ext(certificate.get(), extension.get(), -1));
    }

    {
      auto extension = CreateExtensionFromConfiguration(
          certificate,
          certificate,
          NID_key_usage,
          (issuer ? "critical,keyCertSign,digitalSignature" : "critical,keyCertSign"));
      OPENSSL_CHECK(X509_add_ext(certificate.get(), extension.get(), -1));
    }

    // Export the key to sign the certificate.
    {
      std::unique_ptr<Cryptography::AsymmetricKey> privateKey;
      {
        auto exportedPrivateKey = signingKey->ExportPrivateKey();
        privateKey = Cryptography::ImportPrivateKey(exportedPrivateKey);
      }
      // We know that Crypto::ImportPublicKey always returns an OpenSSLAsymmetricKey.
      // This is a bit of a hack but it's an acceptable assumption to make.
      OpenSSLAsymmetricKey* key = static_cast<OpenSSLAsymmetricKey*>(privateKey.get());
      if (X509_sign(certificate.get(), key->GetKey().get(), EVP_sha256()) == 0)
      {
        throw OpenSSLException("X509_sign");
      }
    }

    return certificate;
  } // namespace _internal
  std::unique_ptr<Cryptography::X509Certificate> OpenSSLX509Certificate::CreateFromPrivateKey(
      std::unique_ptr<Cryptography::AsymmetricKey> const& key,
      std::string const& subjectName)
  {
    auto utcTime(time(nullptr));
    time_t expirationTime;
    {
      // This certificate expires in 8 hours.
      struct std::tm currentTime;
#if _WINDOWS
      gmtime_s(&currentTime, &utcTime);
#else
      gmtime_r(&utcTime, &currentTime);
#endif
      // Our derived certificates expire in 8 hours (they're only used for test).
      currentTime.tm_hour += 8;

      // Normalize the expiration time based on the adjustments. This will handle
      // wrapping hours, days, etc.
#if _WINDOWS
      expirationTime = _mkgmtime(&currentTime); // cspell:disable-line
#else
      expirationTime = timegm(&currentTime); // cspell:disable-line
#endif
    }
    openssl_x509 certificate(CreateCertificate(
        key, // Key for the derived certificate.
        subjectName, // Name of the derived certificate.
        key, // Key to use to sign the certificate.
        openssl_x509{nullptr}, // Issuer (create self signed certificate)
        utcTime,
        expirationTime,
        false // Not a leaf certificate.
        )); // Extensions

    return std::unique_ptr<OpenSSLX509Certificate>(
        new OpenSSLX509Certificate(std::move(certificate)));
  }

  std::unique_ptr<Cryptography::AsymmetricKey> OpenSSLX509Certificate::GetPublicKey() const
  {
    openssl_evp_pkey pkey(X509_get0_pubkey(m_certificate.get()));
    OPENSSL_CHECK(EVP_PKEY_up_ref(pkey.get()));
    return std::unique_ptr<OpenSSLAsymmetricKey>(new OpenSSLAsymmetricKey(std::move(pkey)));
  }
}}}} // namespace Azure::Security::Attestation::_detail
