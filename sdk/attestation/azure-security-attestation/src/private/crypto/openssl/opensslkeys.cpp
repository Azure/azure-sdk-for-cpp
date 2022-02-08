// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Attestation client model support classes and functions.
 *
 * This file contains private classes used to support public model types.
 *
 */

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "../inc/crypto.hpp"
#include "opensslkeys.hpp"
#include <openssl/bio.h>
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

namespace Azure { namespace Security { namespace Attestation { namespace _private {
  namespace Cryptography {

    namespace _details {
      template <> struct type_map_helper<EVP_PKEY_CTX>
      {
        using type = basic_openssl_unique_ptr<EVP_PKEY_CTX, EVP_PKEY_CTX_free>;
      };

      template <> struct type_map_helper<EVP_MD_CTX>
      {
        using type = basic_openssl_unique_ptr<EVP_MD_CTX, EVP_MD_CTX_free>;
      };
    } // namespace _details

    std::string OpenSSLAsymmetricKey::ExportPrivateKey()
    {
      auto bio(_details::make_openssl_unique(BIO_new, BIO_s_mem()));
      if (PEM_write_bio_PrivateKey(bio.get(), m_pkey.get(), nullptr, nullptr, 0, nullptr, nullptr)
          != 1)
      {
        throw _details::OpenSSLException("Could not write private key");
      }
      std::vector<uint8_t> returnValue(BIO_ctrl_pending(bio.get()));

      int res = BIO_read(bio.get(), returnValue.data(), static_cast<int>(returnValue.size()));
      if (res == 0 || res == -1 || res == -2)
      {
        throw _details::OpenSSLException("BIO_read");
      }
      return std::string(returnValue.begin(), returnValue.end());
    }

    std::string OpenSSLAsymmetricKey::ExportPublicKey()
    {
      auto bio(_details::make_openssl_unique(BIO_new, BIO_s_mem()));
      if (PEM_write_bio_PUBKEY(bio.get(), m_pkey.get()) != 1)
      {
        throw _details::OpenSSLException("Could not write public key");
      }
      std::vector<uint8_t> returnValue(BIO_ctrl_pending(bio.get()));

      int res = BIO_read(bio.get(), returnValue.data(), static_cast<int>(returnValue.size()));
      if (res == 0 || res == -1 || res == -2)
      {
        throw _details::OpenSSLException("BIO_read");
      }
      return std::string(returnValue.begin(), returnValue.end());
    }

    std::unique_ptr<AsymmetricKey> OpenSSLAsymmetricKey::ImportPublicKey(
        std::string const& pemEncodedKey)
    {
      auto bio(_details::make_openssl_unique(
          BIO_new_mem_buf, pemEncodedKey.data(), static_cast<int>(pemEncodedKey.size())));
      EVP_PKEY* raw_pkey = nullptr;
      raw_pkey = PEM_read_bio_PUBKEY(bio.get(), nullptr, nullptr, nullptr);
      if (raw_pkey == nullptr)
      {
        throw _details::OpenSSLException("PEM_read_bio_PUBKEY");
      }
      _details::openssl_evp_pkey pkey(raw_pkey);
      raw_pkey = nullptr;
      if (EVP_PKEY_id(pkey.get()) == EVP_PKEY_RSA)
      {
        return std::make_unique<RsaOpenSSLAsymmetricKey>(std::move(pkey));
      }
      else if (EVP_PKEY_id(pkey.get()) == EVP_PKEY_EC)
      {
        return std::make_unique<EcdsaOpenSSLAsymmetricKey>(std::move(pkey));
      }
      throw std::runtime_error("Unknown key type passed to ImportPublicKey.");
    }

    /**
     *
     */
    std::unique_ptr<AsymmetricKey> OpenSSLAsymmetricKey::ImportPrivateKey(
        std::string const& pemEncodedKey)
    {
      auto bio(_details::make_openssl_unique(
          BIO_new_mem_buf, pemEncodedKey.data(), static_cast<int>(pemEncodedKey.size())));
      EVP_PKEY* raw_pkey = nullptr;
      raw_pkey = PEM_read_bio_PrivateKey(bio.get(), nullptr, nullptr, nullptr);
      if (raw_pkey == nullptr)
      {
        throw _details::OpenSSLException("PEM_read_bio_PUBKEY");
      }
      _details::openssl_evp_pkey pkey(raw_pkey);
      raw_pkey = nullptr;
      if (EVP_PKEY_id(pkey.get()) == EVP_PKEY_RSA)
      {
        return std::make_unique<RsaOpenSSLAsymmetricKey>(std::move(pkey));
      }
      else if (EVP_PKEY_id(pkey.get()) == EVP_PKEY_EC)
      {
        return std::make_unique<EcdsaOpenSSLAsymmetricKey>(std::move(pkey));
      }
      throw std::runtime_error("Unknown key type passed to ImportPublicKey.");
    }

    RsaOpenSSLAsymmetricKey::RsaOpenSSLAsymmetricKey(size_t keySize)
    {
      auto evpContext(_details::make_openssl_unique(EVP_PKEY_CTX_new_id, EVP_PKEY_RSA, nullptr));

      if (EVP_PKEY_keygen_init(evpContext.get()) != 1)
      {
        throw _details::OpenSSLException("Could not initialize keygen");
      }
      if (EVP_PKEY_CTX_set_rsa_keygen_bits(evpContext.get(), static_cast<int>(keySize)) != 1)
      {
        throw _details::OpenSSLException("Could not set keygen bits");
      }
      EVP_PKEY* pkey = nullptr;
      if (EVP_PKEY_keygen(evpContext.get(), &pkey) != 1)
      {
        throw _details::OpenSSLException("Could not keygen");
      }
      m_pkey.reset(pkey);
    }

    /** Sign a buffer with an RSA key.
     */
    std::vector<uint8_t> OpenSSLAsymmetricKey::SignBuffer(std::vector<uint8_t> const& payload) const
    {
      auto mdContext(_details::make_openssl_unique(EVP_MD_CTX_new));
      if (EVP_DigestSignInit(mdContext.get(), nullptr, EVP_sha256(), nullptr, m_pkey.get()) != 1)
      {
        throw _details::OpenSSLException("EVP_DigestSignInit");
      }

      if (EVP_DigestSignUpdate(mdContext.get(), payload.data(), static_cast<int>(payload.size()))
          != 1)
      {
        throw _details::OpenSSLException("EVP_DigestSignUpdate");
      }

      size_t signatureLength = 0;
      if (EVP_DigestSignFinal(mdContext.get(), nullptr, &signatureLength) != 1)
      {
        throw _details::OpenSSLException("EVP_DigestSignFinal(sizing)");
      }

      std::vector<uint8_t> returnValue(signatureLength);
      if (EVP_DigestSignFinal(mdContext.get(), returnValue.data(), &signatureLength) != 1)
      {
        throw _details::OpenSSLException("EVP_DigestSignFinal(sizing)");
      }
      returnValue.resize(signatureLength);
      return returnValue;
    }

    bool OpenSSLAsymmetricKey::VerifySignature(
        std::vector<uint8_t> const& payload,
        std::vector<uint8_t> const& signature) const
    {
      auto mdContext(_details::make_openssl_unique(EVP_MD_CTX_new));
      if (EVP_DigestVerifyInit(mdContext.get(), nullptr, EVP_sha256(), nullptr, m_pkey.get()) != 1)
      {
        throw _details::OpenSSLException("EVP_DigestVerifyInit");
      }

      if (EVP_DigestVerifyUpdate(mdContext.get(), payload.data(), static_cast<int>(payload.size()))
          != 1)
      {
        throw _details::OpenSSLException("EVP_DigestVerifyUpdate");
      }

      auto rv = EVP_DigestVerifyFinal(
          mdContext.get(), signature.data(), static_cast<int>(signature.size()));

      if (rv == 1)
      {
        return true;
      }
      else if (rv == 0)
      {
        return false;
      }
      else
      {
        throw _details::OpenSSLException("EVP_DigestVerifyFinal");
      }
    }

    EcdsaOpenSSLAsymmetricKey::EcdsaOpenSSLAsymmetricKey()
    {

      auto evpContext(_details::make_openssl_unique(EVP_PKEY_CTX_new_id, EVP_PKEY_EC, nullptr));
      if (EVP_PKEY_keygen_init(evpContext.get()) != 1)
      {
        throw _details::OpenSSLException("EVP_PKEY_keygen_init");
      }

      if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(evpContext.get(), NID_X9_62_prime256v1) != 1)
      {
        throw _details::OpenSSLException("EVP_PKEY_CTX_set_ec_paramgen_curve_nid");
      }

      EVP_PKEY* pkey = nullptr;
      if (EVP_PKEY_keygen(evpContext.get(), &pkey) != 1)
      {
        throw _details::OpenSSLException("EVP_PKEY_keygen");
      }
      m_pkey.reset(pkey);
    }

    namespace _details {
      std::string GetOpenSSLError(std::string const& what)
      {
        auto bio(make_openssl_unique(BIO_new, BIO_s_mem()));

        BIO_printf(bio.get(), "Error in %hs: ", what.c_str());
        if (ERR_peek_error() != 0)
        {
          ERR_print_errors(bio.get());
        }
        else
        {
          BIO_printf(bio.get(), "Unknown error.");
        }

        uint8_t* bioData;
        long bufferSize = BIO_get_mem_data(bio.get(), &bioData);
        std::string returnValue;
        returnValue.resize(bufferSize);
        memcpy(&returnValue[0], bioData, bufferSize);

        return returnValue;
      }

      OpenSSLException::OpenSSLException(std::string const& what)
          : runtime_error(GetOpenSSLError(what))
      {
      }

    } // namespace _details
}}}}} // namespace Azure::Security::Attestation::_private::Cryptography
