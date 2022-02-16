// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief Attestation client model support classes and functions.
 *
 * This file contains private classes used to support public model types.
 *
 */

#include "opensslkeys.hpp"
#include "../inc/crypto.hpp"
#include "openssl_helpers.hpp"
#include <azure/core/azure_assert.hpp>
#include <cassert>
#include <cstring>
#include <memory>
#include <openssl/bio.h>
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace Azure { namespace Security { namespace Attestation { namespace _detail {

  template <> struct type_map_helper<EVP_PKEY_CTX>
  {
    using type = basic_openssl_unique_ptr<EVP_PKEY_CTX, EVP_PKEY_CTX_free>;
  };

  template <> struct type_map_helper<EVP_MD_CTX>
  {
    using type = basic_openssl_unique_ptr<EVP_MD_CTX, EVP_MD_CTX_free>;
  };

  std::string OpenSSLAsymmetricKey::ExportPrivateKey()
  {
    auto bio(make_openssl_unique(BIO_new, BIO_s_mem()));
    if (PEM_write_bio_PrivateKey(bio.get(), m_pkey.get(), nullptr, nullptr, 0, nullptr, nullptr)
        != 1)
    {
      throw OpenSSLException("PEM_write_bio_PrivateKey");
    }
    // Now extract the data from the BIO and return it as a string.
    uint8_t* base64data;
    long bufferSize = BIO_get_mem_data(bio.get(), &base64data);
    std::string returnValue;
    returnValue.resize(bufferSize);
    memcpy(&returnValue[0], base64data, bufferSize);
    return returnValue;
  }

  std::string OpenSSLAsymmetricKey::ExportPublicKey()
  {
    auto bio(make_openssl_unique(BIO_new, BIO_s_mem()));
    if (PEM_write_bio_PUBKEY(bio.get(), m_pkey.get()) != 1)
    {
      throw OpenSSLException("PEM_write_bio_PUBKEY");
    }
    // Now extract the data from the BIO and return it as a string.
    uint8_t* base64data;
    long bufferSize = BIO_get_mem_data(bio.get(), &base64data);
    std::string returnValue;
    returnValue.resize(bufferSize);
    memcpy(&returnValue[0], base64data, bufferSize);
    return returnValue;
  }

  std::unique_ptr<Cryptography::AsymmetricKey> OpenSSLAsymmetricKey::ImportPublicKey(
      std::string const& pemEncodedKey)
  {
    auto bio(make_openssl_unique(
        BIO_new_mem_buf, pemEncodedKey.data(), static_cast<int>(pemEncodedKey.size())));
    EVP_PKEY* raw_pkey = nullptr;
    raw_pkey = PEM_read_bio_PUBKEY(bio.get(), nullptr, nullptr, nullptr);
    if (raw_pkey == nullptr)
    {
      throw OpenSSLException("Parse Public Key Import");
    }
    openssl_evp_pkey pkey(raw_pkey);
    raw_pkey = nullptr;
    if (EVP_PKEY_id(pkey.get()) == EVP_PKEY_RSA)
    {
      return std::make_unique<RsaOpenSSLAsymmetricKey>(std::move(pkey));
    }
    else if (EVP_PKEY_id(pkey.get()) == EVP_PKEY_EC)
    {
      return std::make_unique<EcdsaOpenSSLAsymmetricKey>(std::move(pkey));
    }
    throw std::invalid_argument("Unknown key type passed to ImportPublicKey");
  }

  /**
   *
   */
  std::unique_ptr<Cryptography::AsymmetricKey> OpenSSLAsymmetricKey::ImportPrivateKey(
      std::string const& pemEncodedKey)
  {
    auto bio(make_openssl_unique(
        BIO_new_mem_buf, pemEncodedKey.data(), static_cast<int>(pemEncodedKey.size())));
    EVP_PKEY* raw_pkey = nullptr;
    raw_pkey = PEM_read_bio_PrivateKey(bio.get(), nullptr, nullptr, nullptr);
    if (raw_pkey == nullptr)
    {
      throw OpenSSLException("Parse Private Key Import");
    }

    openssl_evp_pkey pkey(raw_pkey);
    raw_pkey = nullptr;
    if (EVP_PKEY_id(pkey.get()) == EVP_PKEY_RSA)
    {
      return std::make_unique<RsaOpenSSLAsymmetricKey>(std::move(pkey));
    }
    else if (EVP_PKEY_id(pkey.get()) == EVP_PKEY_EC)
    {
      return std::make_unique<EcdsaOpenSSLAsymmetricKey>(std::move(pkey));
    }
    throw std::invalid_argument("Unknown key type passed to ImportPrivateKey");
  }

  RsaOpenSSLAsymmetricKey::RsaOpenSSLAsymmetricKey(size_t keySize)
  {
    auto evpContext(make_openssl_unique(EVP_PKEY_CTX_new_id, EVP_PKEY_RSA, nullptr));

    if (EVP_PKEY_keygen_init(evpContext.get()) != 1)
    {
      throw OpenSSLException("EVP_PKEY_keygen_init");
    }
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(evpContext.get(), static_cast<int>(keySize)) != 1)
    {
      throw OpenSSLException("EVP_PKEY_CTX_set_rsa_keygen_bits");
    }
    EVP_PKEY* pkey = nullptr;
    if (EVP_PKEY_keygen(evpContext.get(), &pkey) != 1)
    {
      throw OpenSSLException("EVP_PKEY_keygen");
    }
    m_pkey.reset(pkey);
  }

  /** Sign a buffer with an RSA key.
   */
  std::vector<uint8_t> OpenSSLAsymmetricKey::SignBuffer(std::vector<uint8_t> const& payload) const
  {
    auto mdContext(make_openssl_unique(EVP_MD_CTX_new));
    if (EVP_DigestSignInit(mdContext.get(), nullptr, EVP_sha256(), nullptr, m_pkey.get()) != 1)
    {
      throw OpenSSLException("EVP_DigestSignInit");
    }

    if (EVP_DigestSignUpdate(mdContext.get(), payload.data(), static_cast<int>(payload.size()))
        != 1)
    {
      throw OpenSSLException("EVP_DigestSignUpdate");
    }

    size_t signatureLength = 0;
    if (EVP_DigestSignFinal(mdContext.get(), nullptr, &signatureLength) != 1)
    {
      throw OpenSSLException("EVP_DigestSignFinal");
    }

    std::vector<uint8_t> returnValue(signatureLength);
    if (EVP_DigestSignFinal(mdContext.get(), returnValue.data(), &signatureLength) != 1)
    {
      throw OpenSSLException("EVP_DigestSignFinal");
    }
    returnValue.resize(signatureLength);
    return returnValue;
  }

  bool OpenSSLAsymmetricKey::VerifySignature(
      std::vector<uint8_t> const& payload,
      std::vector<uint8_t> const& signature) const
  {
    auto mdContext(make_openssl_unique(EVP_MD_CTX_new));
    if (EVP_DigestVerifyInit(mdContext.get(), nullptr, EVP_sha256(), nullptr, m_pkey.get()) != 1)
    {
      throw OpenSSLException("EVP_DigestVerifyInit");
    }

    if (EVP_DigestVerifyUpdate(mdContext.get(), payload.data(), static_cast<int>(payload.size()))
        != 1)
    {
      throw OpenSSLException("EVP_DigestVerifyInit");
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
      // Force a failure.
      throw OpenSSLException("EVP_DigestVerifyFinal");
    }
  }

  EcdsaOpenSSLAsymmetricKey::EcdsaOpenSSLAsymmetricKey()
  {

    auto evpContext(make_openssl_unique(EVP_PKEY_CTX_new_id, EVP_PKEY_EC, nullptr));
    if (EVP_PKEY_keygen_init(evpContext.get()) != 1)
    {
      throw OpenSSLException("EVP_PKEY_keygen_init");
    }

    if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(evpContext.get(), NID_X9_62_prime256v1) != 1)
    {
      throw OpenSSLException("EVP_PKEY_CTX_set_ec_paramgen_curve_nid");
    }

    EVP_PKEY* pkey = nullptr;
    if (EVP_PKEY_keygen(evpContext.get(), &pkey) != 1)
    {
      throw OpenSSLException("EVP_PKEY_keygen");
    }
    m_pkey.reset(pkey);
  }

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

  OpenSSLException::OpenSSLException(std::string const& what) : runtime_error(GetOpenSSLError(what))
  {
  }

}}}} // namespace Azure::Security::Attestation::_detail
