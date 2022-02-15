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
    OPENSSL_CHECK(
        PEM_write_bio_PrivateKey(bio.get(), m_pkey.get(), nullptr, nullptr, 0, nullptr, nullptr));
    std::vector<uint8_t> returnValue(BIO_ctrl_pending(bio.get()));

    int res = BIO_read(bio.get(), returnValue.data(), static_cast<int>(returnValue.size()));
    OPENSSL_CHECK_BIO(res);
    return std::string(returnValue.begin(), returnValue.end());
  }

  std::string OpenSSLAsymmetricKey::ExportPublicKey()
  {
    auto bio(make_openssl_unique(BIO_new, BIO_s_mem()));
    OPENSSL_CHECK(PEM_write_bio_PUBKEY(bio.get(), m_pkey.get()));
    std::vector<uint8_t> returnValue(BIO_ctrl_pending(bio.get()));

    int res = BIO_read(bio.get(), returnValue.data(), static_cast<int>(returnValue.size()));
    OPENSSL_CHECK_BIO(res);
    return std::string(returnValue.begin(), returnValue.end());
  }

  std::unique_ptr<Cryptography::AsymmetricKey> OpenSSLAsymmetricKey::ImportPublicKey(
      std::string const& pemEncodedKey)
  {
    auto bio(make_openssl_unique(
        BIO_new_mem_buf, pemEncodedKey.data(), static_cast<int>(pemEncodedKey.size())));
    EVP_PKEY* raw_pkey = nullptr;
    raw_pkey = PEM_read_bio_PUBKEY(bio.get(), nullptr, nullptr, nullptr);
    OPENSSL_CHECK_NULL(raw_pkey);
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
    assert(false);
    abort();
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
    OPENSSL_CHECK_NULL(raw_pkey);

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
    assert(false);
    abort();
  }

  RsaOpenSSLAsymmetricKey::RsaOpenSSLAsymmetricKey(size_t keySize)
  {
    auto evpContext(make_openssl_unique(EVP_PKEY_CTX_new_id, EVP_PKEY_RSA, nullptr));

    OPENSSL_CHECK(EVP_PKEY_keygen_init(evpContext.get()));
    OPENSSL_CHECK(EVP_PKEY_CTX_set_rsa_keygen_bits(evpContext.get(), static_cast<int>(keySize)));
    EVP_PKEY* pkey = nullptr;
    OPENSSL_CHECK(EVP_PKEY_keygen(evpContext.get(), &pkey));
    m_pkey.reset(pkey);
  }

  /** Sign a buffer with an RSA key.
   */
  std::vector<uint8_t> OpenSSLAsymmetricKey::SignBuffer(std::vector<uint8_t> const& payload) const
  {
    auto mdContext(make_openssl_unique(EVP_MD_CTX_new));
    OPENSSL_CHECK(
        EVP_DigestSignInit(mdContext.get(), nullptr, EVP_sha256(), nullptr, m_pkey.get()));

    OPENSSL_CHECK(
        EVP_DigestSignUpdate(mdContext.get(), payload.data(), static_cast<int>(payload.size())));

    size_t signatureLength = 0;
    OPENSSL_CHECK(EVP_DigestSignFinal(mdContext.get(), nullptr, &signatureLength));

    std::vector<uint8_t> returnValue(signatureLength);
    OPENSSL_CHECK(EVP_DigestSignFinal(mdContext.get(), returnValue.data(), &signatureLength));
    returnValue.resize(signatureLength);
    return returnValue;
  }

  bool OpenSSLAsymmetricKey::VerifySignature(
      std::vector<uint8_t> const& payload,
      std::vector<uint8_t> const& signature) const
  {
    auto mdContext(make_openssl_unique(EVP_MD_CTX_new));
    OPENSSL_CHECK(
        EVP_DigestVerifyInit(mdContext.get(), nullptr, EVP_sha256(), nullptr, m_pkey.get()));

    OPENSSL_CHECK(
        EVP_DigestVerifyUpdate(mdContext.get(), payload.data(), static_cast<int>(payload.size())));

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
      AZURE_ASSERT_MSG(false, GetOpenSSLError("EVP_DigestVerifyFinal").c_str());
      throw std::runtime_error("Not reached");
    }
  }

  EcdsaOpenSSLAsymmetricKey::EcdsaOpenSSLAsymmetricKey()
  {

    auto evpContext(make_openssl_unique(EVP_PKEY_CTX_new_id, EVP_PKEY_EC, nullptr));
    OPENSSL_CHECK(EVP_PKEY_keygen_init(evpContext.get()));

    OPENSSL_CHECK(EVP_PKEY_CTX_set_ec_paramgen_curve_nid(evpContext.get(), NID_X9_62_prime256v1));

    EVP_PKEY* pkey = nullptr;
    OPENSSL_CHECK(EVP_PKEY_keygen(evpContext.get(), &pkey));
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
