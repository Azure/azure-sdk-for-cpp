// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Attestation client model support classes and functions.
 *
 * This file contains private classes used to support public model types.
 *
 */

#include <azure/core/internal/json/json.hpp>

#include <azure/core/base64.hpp>
#include <ctime>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "../inc/crypto.hpp"
#include "opensslcert.hpp"
#include "opensslkeys.hpp"
#include <openssl/bio.h>
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/x509v3.h>

// cspell::words OpenSSL X509 OpenSSLX509

namespace Azure { namespace Security { namespace Attestation {
  namespace _private {
    namespace Cryptography {

      std::unique_ptr<X509Certificate> OpenSSLX509Certificate::Import(
          std::string const& pemEncodedString)
      {
        auto bio(_details::make_openssl_unique(
            BIO_new_mem_buf, pemEncodedString.data(), static_cast<int>(pemEncodedString.size())));
        X509* raw_x509 = nullptr;
        raw_x509 = PEM_read_bio_X509(bio.get(), nullptr, nullptr, nullptr);
        if (raw_x509 == nullptr)
        {
          throw _details::OpenSSLException("PEM_read_bio_PUBKEY");
        }
        _details::openssl_x509 x509(raw_x509);
        raw_x509 = nullptr;
        return std::unique_ptr<OpenSSLX509Certificate>(new OpenSSLX509Certificate(std::move(x509)));
      }

      std::string OpenSSLX509Certificate::ExportAsPEM() const
      {
        auto bio(_details::make_openssl_unique(BIO_new, BIO_s_mem()));
        if (PEM_write_bio_X509(bio.get(), m_certificate.get()) != 1)
        {
          throw std::runtime_error("Could not write X509 certificate.");
        }
        std::vector<uint8_t> returnValue(BIO_ctrl_pending(bio.get()));

        int res = BIO_read(bio.get(), returnValue.data(), static_cast<int>(returnValue.size()));
        if (res == 0 || res == -1 || res == -2)
        {
          throw _details::OpenSSLException("BIO_read");
        }
        return std::string(returnValue.begin(), returnValue.end());
      }

      _details::openssl_x509_name OpenSSLX509Certificate::ParseX509Name(
          std::string const& name)
      {
        _details::openssl_x509_name returnValue(_details::make_openssl_unique(X509_NAME_new));
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
          components.push_back(std::make_pair(type, value));
        }
        for (auto comp : components)
        {
          int nid = OBJ_txt2nid(comp.first.c_str());
          if (nid == NID_undef)
          {
            throw std::runtime_error("Could not parse unknown attribute " + comp.first);
          }
          if (X509_NAME_add_entry_by_NID(
                  returnValue.get(),
                  nid,
                  MBSTRING_UTF8,
                  reinterpret_cast<const unsigned char*>(comp.second.c_str()),
                  static_cast<int>(comp.second.length()),
                  -1,
                  0)
              != 1)
          {
            throw _details::OpenSSLException("X509_NAME_add_entry_by_NID");
          }
        }
        return returnValue;
      }

      _details::openssl_x509_extension OpenSSLX509Certificate::CreateExtensionFromConfiguration(
          _details::openssl_x509 const& subject,
          _details::openssl_x509 const& issuer,
          int nid,
          const std::string& nidValue)
      {
        X509V3_CTX context;

        X509V3_set_ctx_nodb(&context); //cspell: disable-line
        X509V3_set_ctx(&context, issuer.get(), subject.get(), nullptr, nullptr, 0);

        return _details::make_openssl_unique(X509V3_EXT_conf_nid, 
            nullptr, &context, nid, nidValue.c_str());
      }


      /// <summary>
      /// Create or derive a new X.509 certificate.
      /// </summary>
      /// <param name="newCertificateKey">Public key for the new certificate.</param>
      /// <param name="newCertificateSubject">Subject Name for the new certificate.</param>
      /// <param name="signingKey">Private key used to sign the new certificate. This can be the
      /// same as the newCertificateKey or it can be an issuer private key.</param> <param
      /// name="issuer">If this is to be a derived certificate, the issuer of the certificate, or
      /// null if this is self signed.</param> <param name="currentTime">The issuance time (in UTC)
      /// for the certificate.</param> <param name="expirationTime">The expiration time (in UTC) for
      /// the certificate.</param> <param name="isLeafCertificate">True if this is a leaf
      /// certificate.</param> <param name="extensionsToAdd">The set of X.509v3 extensions to add to
      /// the stock extensions.</param> <returns></returns>
      _details::openssl_x509 OpenSSLX509Certificate::CreateCertificate(
          std::unique_ptr<AsymmetricKey> const& newCertificateKey,
          std::string const& newCertificateSubject,
          std::unique_ptr<AsymmetricKey> const& signingKey,
          _details::openssl_x509 const& issuer,
          time_t const currentTime,
          time_t const expirationTime,
          bool isLeafCertificate)
      {
        _details::openssl_x509 certificate(_details::make_openssl_unique(X509_new));

        _details::openssl_x509_name subjectName(ParseX509Name(newCertificateSubject));

        if (X509_set_subject_name(certificate.get(), subjectName.get()) != 1)
        {
          throw _details::OpenSSLException("X509_set_subject_name");
        }

        if (issuer)
        {
          if (X509_set_issuer_name(certificate.get(), X509_get_subject_name(issuer.get()))
              != 1)
          {
            throw _details::OpenSSLException("X509_set_issuer_name");
          }
        }
        else
        {
          if (X509_set_issuer_name(certificate.get(), subjectName.get()) != 1)
          {
            throw _details::OpenSSLException("X509_set_issuer_name");
          }
        }

        // Export the key to be included in the certificate.
        {
          auto exportedPublicKey = newCertificateKey->ExportPublicKey();
          auto publicKey = Crypto::ImportPublicKey(exportedPublicKey);
          // We know that Crypto::ImportPublicKey always returns an OpenSSLAsymmetricKey.
          // THis is a bit of a hack but it's an acceptable assumption to make.
          OpenSSLAsymmetricKey* key = static_cast<OpenSSLAsymmetricKey*>(publicKey.get());
          if (X509_set_pubkey(certificate.get(), key->GetKey().get()) != 1)
          {
            throw _details::OpenSSLException("X509_set_pubkey");
          }
        }

        if (X509_set_version(certificate.get(), 2) != 1) // Version 3 certificate.
        {
          throw _details::OpenSSLException("X509_set_version");
        }
        // Transfer the serial number from the current certificate to the child if this is a derived
        // certificate.
        if (issuer)
        {
          if (
              X509_set_serialNumber(certificate.get(), X509_get_serialNumber(issuer.get())) != 1)
          {
            throw _details::OpenSSLException("X509_set_serialNumber");
          }
        }
        else
        {
          auto serialNumber(_details::make_openssl_unique(ASN1_INTEGER_new));
          if (ASN1_INTEGER_set(serialNumber.get(), 1) != 1)
          {
            throw _details::OpenSSLException("ASN1_INTEGER_set");
          }
          if (X509_set_serialNumber(certificate.get(), serialNumber.get()) != 1)
          {
            throw _details::OpenSSLException("X509_set_serialNumber");
          }
        }

        {
          auto extension = CreateExtensionFromConfiguration(
              certificate,
              certificate,
              NID_basic_constraints,
              (isLeafCertificate ? "CA:FALSE" : "CA:TRUE, pathlen:0"));
          if (X509_add_ext(certificate.get(), extension.get(), -1) != 1)
          {
            throw _details::OpenSSLException("X509_add_ext");
          }
        }

        {
          // Set Not Before Time (time before which certificate is not valid).
          {
            _details::openssl_asn1_time notBeforeTime(
                _details::make_openssl_unique(ASN1_TIME_adj, nullptr, currentTime, 0, 0));

            if (X509_set1_notBefore(certificate.get(), notBeforeTime.get()) != 1)
            {
              throw _details::OpenSSLException("X509_set1_notBefore");
            }
          }
          // Set Not After Time (time after which certificate is not valid).
          {
            _details::openssl_asn1_time notAfterTime(
                _details::make_openssl_unique(ASN1_TIME_adj, nullptr, expirationTime, 0, 0));

            if (X509_set1_notAfter(certificate.get(), notAfterTime.get()) != 1)
            {
              throw _details::OpenSSLException("X509_set1_notAfter");
            }
          }
        }

        {// Add the subject Key ID - this is the thumbprint of the public key. Note that we have to
         // have called X509_set_pubkey before this call.
         {auto extension = CreateExtensionFromConfiguration(
              certificate, certificate, NID_subject_key_identifier, "hash");
        if (X509_add_ext(certificate.get(), extension.get(), -1) != 1)
        {
          throw _details::OpenSSLException("X509_add_ext");
        }
      }

      // Add the authority Key ID - Note that this needs to be done *after* setting the subject key
      // identifier.
      {
        auto extension(_details::make_openssl_unique(X509_EXTENSION_new));
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
        if (X509_add_ext(certificate.get(), extension.get(), -1) != 1)
        {
          throw _details::OpenSSLException("X509_add_ext");
        }
      }
    } // namespace Cryptography

    {
      auto extension = CreateExtensionFromConfiguration(
          certificate,
          certificate,
          NID_key_usage,
          (issuer ? "critical,keyCertSign,digitalSignature" : "critical,keyCertSign"));
      if (X509_add_ext(certificate.get(), extension.get(), -1) != 1)
      {
        throw _details::OpenSSLException("X509_add_ext");
      }
    }

    // Export the key to sign the certificate.
    {
      std::unique_ptr<AsymmetricKey> privateKey;
      {
        auto exportedPrivateKey = signingKey->ExportPrivateKey();
        privateKey = Crypto::ImportPrivateKey(exportedPrivateKey);
      }
      // We know that Crypto::ImportPublicKey always returns an OpenSSLAsymmetricKey.
      // This is a bit of a hack but it's an acceptable assumption to make.
      OpenSSLAsymmetricKey* key = static_cast<OpenSSLAsymmetricKey*>(privateKey.get());
      if (X509_sign(certificate.get(), key->GetKey().get(), EVP_sha256()) == 0)
      {
        throw _details::OpenSSLException("X509_sign");
      }
    }

    return certificate;
  } // namespace _private

  std::unique_ptr<X509Certificate> OpenSSLX509Certificate::CreateFromPrivateKey(
      std::unique_ptr<AsymmetricKey> const& key,
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
    _details::openssl_x509 certificate(CreateCertificate(
        key, // Key for the derived certificate.
        subjectName, // Name of the derived certificate.
        key, // Key to use to sign the certificate.
        _details::openssl_x509{nullptr}, // Issuer (create self signed certificate)
        utcTime,
        expirationTime,
        false // Not a leaf certificate.
        )); // Extensions

    return std::unique_ptr<OpenSSLX509Certificate>(
        new OpenSSLX509Certificate(std::move(certificate)));
  }

  std::unique_ptr<AsymmetricKey> OpenSSLX509Certificate::GetPublicKey() const
  {
    _details::openssl_evp_pkey pkey(X509_get0_pubkey(m_certificate.get()));
    if (EVP_PKEY_up_ref(pkey.get()) != 1)
    {
      throw std::runtime_error("Could not write X509 certificate.");
    }
    return std::unique_ptr<OpenSSLAsymmetricKey>(new OpenSSLAsymmetricKey(std::move(pkey)));
  }
}}} // namespace Azure::Security::Attestation
}
} // namespace Azure::Security::Attestation::_private::Cryptography
