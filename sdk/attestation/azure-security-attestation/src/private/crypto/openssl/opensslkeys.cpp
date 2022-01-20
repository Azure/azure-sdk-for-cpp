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
#include "opensslkeys.hpp"
#include <openssl/bio.h>
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
    } // namespace _details

    std::string OpenSSLAsymmetricKey::ExportPrivateKey()
    {
      auto bio(_details::make_openssl_unique(BIO_new, BIO_s_mem()));
      if (PEM_write_bio_PrivateKey(bio.get(), m_pkey.get(), nullptr, nullptr, 0, nullptr, nullptr)
          != 1)
      {
        throw std::runtime_error("Could not write private key");
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
        throw std::runtime_error("Could not write public key");
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

    bool RsaOpenSSLAsymmetricKey::VerifySignature() const { return false; }
    std::vector<uint8_t> RsaOpenSSLAsymmetricKey::SignBuffer(std::vector<uint8_t> const&) const
    {
      return {};
    }

    std::unique_ptr<AsymmetricKey> Crypto::CreateRsaKey(size_t keySize)
    {
      return std::make_unique<RsaOpenSSLAsymmetricKey>(keySize);
    }

    std::unique_ptr<AsymmetricKey> Crypto::ImportPublicKey(std::string const& pemEncodedKey)
    {
      return OpenSSLAsymmetricKey::ImportPublicKey(pemEncodedKey);
    }

    std::unique_ptr<AsymmetricKey> Crypto::ImportPrivateKey(std::string const& pemEncodedKey)
    {
      return OpenSSLAsymmetricKey::ImportPrivateKey(pemEncodedKey);
    }

    namespace _details {
      std::string GetOpenSSLError(std::string const& what)
      {
        auto _bio(make_openssl_unique(BIO_new, BIO_s_mem()));

        if (ERR_peek_error() != 0)
        {
          ERR_print_errors(_bio.get());
        }
        else
        {
          BIO_printf(_bio.get(), "Unknown error: %hs.", what.c_str());
        }
        std::vector<uint8_t> returnValue(BIO_ctrl_pending(_bio.get()));

        int res = BIO_read(_bio.get(), returnValue.data(), static_cast<int>(returnValue.size()));
        if (res == 0 || res == -1 || res == -2)
        {
          return std::string(returnValue.begin(), returnValue.end());
        }
        return "Uknown error" + what;
      }

      OpenSSLException::OpenSSLException(std::string const& what)
          : runtime_error(GetOpenSSLError(what))
      {
      }

    } // namespace _details
}}}}} // namespace Azure::Security::Attestation::_private::Cryptography
