// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#if 0 // NOTFUNCTIONAL

/**
 * @brief Attestation client model support classes and functions.
 *
 * This file contains private classes used to support public model types.
 *
 */
// cspell: words PCERT PCRYPT hcryptprov hcryptkey
#include "azure/core/internal/json/json.hpp"

#include "azure/core/base64.hpp"
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "../inc/crypto.hpp"
#include "bcryptkeys.hpp"
#include <wil/result.h>

namespace Azure { namespace Security { namespace Attestation { namespace _internal {
  namespace Cryptography {

    using unique_private_key_info
        = wil::unique_any<PCRYPT_PRIVATE_KEY_INFO, decltype(LocalFree), LocalFree>;
    using unique_public_key_info
        = wil::unique_any<PCERT_PUBLIC_KEY_INFO, decltype(LocalFree), LocalFree>;
    using unique_ecc_private_key_info
        = wil::unique_any<PCRYPT_ECC_PRIVATE_KEY_INFO, decltype(LocalFree), LocalFree>;
    using unique_void = wil::unique_any<void*, decltype(LocalFree), LocalFree>;

    void replace_substr(std::string& str, std::string const& from, std::string const& to)
    {
      auto start = str.find(from);
      if (start != std::string::npos)
      {
        str.replace(start, from.length(), to);
      }
    }
    std::string BCryptAsymmetricKey::ExportPrivateKey()
    {
      DWORD size = 0;
      THROW_IF_WIN32_BOOL_FALSE(CryptExportKey(m_key.get(), 0, PRIVATEKEYBLOB, 0, nullptr, &size));
      std::vector<uint8_t> keyBuffer(size);
      THROW_IF_NTSTATUS_FAILED(
          CryptExportKey(m_key.get(), 0, PRIVATEKEYBLOB, 0, keyBuffer.data(), &size));

      THROW_IF_WIN32_BOOL_FALSE(CryptEncodeObject(
          X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
          PKCS_RSA_PRIVATE_KEY,
          keyBuffer.data(),
          nullptr,
          &size));
      std::vector<uint8_t> derKey(size);
      THROW_IF_WIN32_BOOL_FALSE(CryptEncodeObject(
          X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
          PKCS_RSA_PRIVATE_KEY,
          keyBuffer.data(),
          derKey.data(),
          &size));

      CRYPT_PRIVATE_KEY_INFO pki{};
      pki.Version = 0;
      pki.PrivateKey.cbData = static_cast<DWORD>(derKey.size());
      pki.PrivateKey.pbData = derKey.data();
      pki.Algorithm.pszObjId = const_cast<LPSTR>(szOID_RSA_RSA);

      THROW_IF_WIN32_BOOL_FALSE(CryptEncodeObject(
          X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, PKCS_PRIVATE_KEY_INFO, &pki, nullptr, &size));
      std::vector<uint8_t> privateKeyInfo(size);
      THROW_IF_WIN32_BOOL_FALSE(CryptEncodeObject(
          X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
          PKCS_PRIVATE_KEY_INFO,
          &pki,
          privateKeyInfo.data(),
          &size));

      DWORD pemLength = 0;
      THROW_IF_WIN32_BOOL_FALSE(CryptBinaryToStringA(
          privateKeyInfo.data(),
          static_cast<DWORD>(privateKeyInfo.size()),
          CRYPT_STRING_BASE64HEADER,
          nullptr,
          &pemLength));
      std::vector<char> pemKey(pemLength);

      THROW_IF_WIN32_BOOL_FALSE(CryptBinaryToStringA(
          privateKeyInfo.data(),
          static_cast<DWORD>(privateKeyInfo.size()),
          CRYPT_STRING_BASE64HEADER,
          pemKey.data(),
          &pemLength));

      // For reasons that remain unknown, CryptBinaryToString sticks in a header of "-----BEGIN
      // CERTIFICATE-----" for all encoded strings, fix that now.
      std::string returnValue(pemKey.begin(), pemKey.end());
      replace_substr(returnValue, "-----BEGIN CERTIFICATE-----", "-----BEGIN PRIVATE KEY-----");
      replace_substr(returnValue, "-----END CERTIFICATE-----", "-----END PRIVATE KEY-----");

      return returnValue;
    }

    std::string BCryptAsymmetricKey::ExportPublicKey()
    {
      DWORD size = 0;
      THROW_IF_WIN32_BOOL_FALSE(CryptExportKey(m_key.get(), 0, PUBLICKEYBLOB, 0, nullptr, &size));
      std::vector<uint8_t> keyBuffer(size);
      THROW_IF_NTSTATUS_FAILED(
          CryptExportKey(m_key.get(), 0, PUBLICKEYBLOB, 0, keyBuffer.data(), &size));

      THROW_IF_WIN32_BOOL_FALSE(CryptEncodeObject(
          X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
          CNG_RSA_PUBLIC_KEY_BLOB,
          keyBuffer.data(),
          nullptr,
          &size));
      std::vector<uint8_t> derKey(size);
      THROW_IF_WIN32_BOOL_FALSE(CryptEncodeObject(
          X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
          CNG_RSA_PUBLIC_KEY_BLOB,
          keyBuffer.data(),
          derKey.data(),
          &size));

      CERT_PUBLIC_KEY_INFO pki{};
      pki.PublicKey.cbData = static_cast<DWORD>(derKey.size());
      pki.PublicKey.pbData = derKey.data();
      pki.Algorithm.pszObjId = const_cast<LPSTR>(szOID_RSA_RSA);

      THROW_IF_WIN32_BOOL_FALSE(CryptEncodeObject(
          X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, X509_PUBLIC_KEY_INFO, &pki, nullptr, &size));
      std::vector<uint8_t> privateKeyInfo(size);
      THROW_IF_WIN32_BOOL_FALSE(CryptEncodeObject(
          X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
          X509_PUBLIC_KEY_INFO,
          &pki,
          privateKeyInfo.data(),
          &size));

      DWORD pemLength = 0;
      THROW_IF_WIN32_BOOL_FALSE(CryptBinaryToStringA(
          privateKeyInfo.data(),
          static_cast<DWORD>(privateKeyInfo.size()),
          CRYPT_STRING_BASE64HEADER,
          nullptr,
          &pemLength));
      std::vector<char> pemKey(pemLength);

      THROW_IF_WIN32_BOOL_FALSE(CryptBinaryToStringA(
          privateKeyInfo.data(),
          static_cast<DWORD>(privateKeyInfo.size()),
          CRYPT_STRING_BASE64HEADER,
          pemKey.data(),
          &pemLength));

      // For reasons that remain unknown, CryptBinaryToString sticks in a header of "-----BEGIN
      // CERTIFICATE-----" for all encoded strings, fix that now.
      std::string returnValue(pemKey.begin(), pemKey.end());
      replace_substr(returnValue, "-----BEGIN CERTIFICATE-----", "-----BEGIN PUBLIC KEY-----");
      replace_substr(returnValue, "-----END CERTIFICATE-----", "-----END PUBLIC KEY-----");

      return returnValue;
    }

    std::unique_ptr<AsymmetricKey> BCryptAsymmetricKey::ImportPublicKey(
        std::string const& publicKeyToImport)
    {
      ULONG keySize = 0;
      THROW_IF_WIN32_BOOL_FALSE(CryptStringToBinaryA(
          publicKeyToImport.c_str(),
          static_cast<DWORD>(publicKeyToImport.size()),
          CRYPT_STRING_BASE64HEADER,
          nullptr,
          &keySize,
          0,
          0));
      std::vector<uint8_t> keyBuffer(keySize);

      THROW_IF_WIN32_BOOL_FALSE(CryptStringToBinaryA(
          publicKeyToImport.c_str(),
          static_cast<DWORD>(publicKeyToImport.size()),
          CRYPT_STRING_BASE64HEADER,
          keyBuffer.data(),
          &keySize,
          0,
          0));
      unique_public_key_info publicKeyInfo;
      THROW_IF_WIN32_BOOL_FALSE(CryptDecodeObjectEx(
          X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
          X509_PUBLIC_KEY_INFO,
          keyBuffer.data(),
          keySize,
          CRYPT_DECODE_ALLOC_FLAG,
          nullptr,
          publicKeyInfo.addressof(),
          &keySize));
      if (strcmp(publicKeyInfo.get()->Algorithm.pszObjId, szOID_ECC_PUBLIC_KEY) == 0)
      {
        wil::unique_hcryptprov cryptProvider;
        THROW_IF_WIN32_BOOL_FALSE(CryptAcquireContextA(
            cryptProvider.addressof(),
            nullptr,
            MS_ENHANCED_PROV,
            PROV_EC_ECDSA_FULL,
            CRYPT_VERIFYCONTEXT));

        unique_ecc_private_key_info eccInfo;
        THROW_IF_WIN32_BOOL_FALSE(CryptDecodeObjectEx(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            X509_ECC_PRIVATE_KEY,
            publicKeyInfo.get()->PublicKey.pbData,
            publicKeyInfo.get()->PublicKey.cbData,
            CRYPT_DECODE_ALLOC_FLAG,
            nullptr,
            eccInfo.addressof(),
            &keySize));

        wil::unique_hcryptkey key;

        THROW_IF_WIN32_BOOL_FALSE(CryptImportKey(
            cryptProvider.get(),
            eccInfo.get()->PrivateKey.pbData,
            eccInfo.get()->PrivateKey.cbData,
            0,
            CRYPT_EXPORTABLE,
            key.addressof()));
        return std::unique_ptr<AsymmetricKey>(
            new EcdsaBCryptAsymmetricKey(std::move(cryptProvider), std::move(key)));
      }
      else if (strcmp(publicKeyInfo.get()->Algorithm.pszObjId, szOID_RSA_RSA) == 0)
      {
        wil::unique_hcryptprov cryptProvider;
        THROW_IF_WIN32_BOOL_FALSE(CryptAcquireContextA(
            cryptProvider.addressof(),
            nullptr,
            MS_ENHANCED_PROV,
            PROV_RSA_FULL,
            CRYPT_VERIFYCONTEXT));

        unique_void rsaKey;
        THROW_IF_WIN32_BOOL_FALSE(CryptDecodeObjectEx(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            CNG_RSA_PUBLIC_KEY_BLOB,
            publicKeyInfo.get()->PublicKey.pbData,
            publicKeyInfo.get()->PublicKey.cbData,
            CRYPT_DECODE_ALLOC_FLAG,
            nullptr,
            rsaKey.addressof(),
            &keySize));

        wil::unique_hcryptkey key;

        THROW_IF_WIN32_BOOL_FALSE(CryptImportKey(
            cryptProvider.get(),
            reinterpret_cast<uint8_t*>(rsaKey.get()),
            keySize,
            0,
            CRYPT_EXPORTABLE,
            key.addressof()));
        return std::unique_ptr<AsymmetricKey>(
            new RsaBCryptAsymmetricKey(std::move(cryptProvider), std::move(key)));
      }
      throw std::runtime_error("Not implemented");
    }

    /**
     *
     */
    std::unique_ptr<AsymmetricKey> BCryptAsymmetricKey::ImportPrivateKey(
        std::string const& privateKeyToImport)
    {
      ULONG keySize = 0;
      THROW_IF_WIN32_BOOL_FALSE(CryptStringToBinaryA(
          privateKeyToImport.c_str(),
          static_cast<DWORD>(privateKeyToImport.size()),
          CRYPT_STRING_BASE64HEADER,
          nullptr,
          &keySize,
          0,
          0));
      std::vector<uint8_t> keyBuffer(keySize);

      THROW_IF_WIN32_BOOL_FALSE(CryptStringToBinaryA(
          privateKeyToImport.c_str(),
          static_cast<DWORD>(privateKeyToImport.size()),
          CRYPT_STRING_BASE64HEADER,
          keyBuffer.data(),
          &keySize,
          0,
          0));
      unique_private_key_info privateKeyInfo;
      THROW_IF_WIN32_BOOL_FALSE(CryptDecodeObjectEx(
          X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
          PKCS_PRIVATE_KEY_INFO,
          keyBuffer.data(),
          keySize,
          CRYPT_DECODE_ALLOC_FLAG,
          nullptr,
          privateKeyInfo.addressof(),
          &keySize));
      if (strcmp(privateKeyInfo.get()->Algorithm.pszObjId, szOID_ECC_PUBLIC_KEY) == 0)
      {
        wil::unique_hcryptprov cryptProvider;
        THROW_IF_WIN32_BOOL_FALSE(CryptAcquireContextA(
            cryptProvider.addressof(),
            nullptr,
            MS_ENHANCED_PROV,
            PROV_EC_ECDSA_FULL,
            CRYPT_VERIFYCONTEXT));

        unique_ecc_private_key_info eccInfo;
        THROW_IF_WIN32_BOOL_FALSE(CryptDecodeObjectEx(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            X509_ECC_PRIVATE_KEY,
            privateKeyInfo.get()->PrivateKey.pbData,
            privateKeyInfo.get()->PrivateKey.cbData,
            CRYPT_DECODE_ALLOC_FLAG,
            nullptr,
            eccInfo.addressof(),
            &keySize));

        wil::unique_hcryptkey key;

        THROW_IF_WIN32_BOOL_FALSE(CryptImportKey(
            cryptProvider.get(),
            eccInfo.get()->PrivateKey.pbData,
            eccInfo.get()->PrivateKey.cbData,
            0,
            CRYPT_EXPORTABLE,
            key.addressof()));
        return std::unique_ptr<AsymmetricKey>(
            new EcdsaBCryptAsymmetricKey(std::move(cryptProvider), std::move(key)));
      }
      else if (strcmp(privateKeyInfo.get()->Algorithm.pszObjId, szOID_RSA_RSA) == 0)
      {
        wil::unique_hcryptprov cryptProvider;
        THROW_IF_WIN32_BOOL_FALSE(CryptAcquireContextA(
            cryptProvider.addressof(),
            nullptr,
            MS_ENHANCED_PROV,
            PROV_RSA_FULL,
            CRYPT_VERIFYCONTEXT));

        unique_void rsaKey;
        THROW_IF_WIN32_BOOL_FALSE(CryptDecodeObjectEx(
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            PKCS_RSA_PRIVATE_KEY,
            privateKeyInfo.get()->PrivateKey.pbData,
            privateKeyInfo.get()->PrivateKey.cbData,
            CRYPT_DECODE_ALLOC_FLAG,
            nullptr,
            rsaKey.addressof(),
            &keySize));

        wil::unique_hcryptkey key;

        THROW_IF_WIN32_BOOL_FALSE(CryptImportKey(
            cryptProvider.get(),
            reinterpret_cast<uint8_t*>(rsaKey.get()),
            keySize,
            0,
            CRYPT_EXPORTABLE,
            key.addressof()));
        return std::unique_ptr<AsymmetricKey>(
            new RsaBCryptAsymmetricKey(std::move(cryptProvider), std::move(key)));
      }
      throw std::runtime_error("Not implemented");
    }

    RsaBCryptAsymmetricKey::RsaBCryptAsymmetricKey(size_t)
    {
      throw std::runtime_error("Not implemented");
    }
    bool BCryptAsymmetricKey::VerifySignature(
        std::vector<uint8_t> const&,
        std::vector<uint8_t> const&) const
    {
      throw std::runtime_error("Not implemented");
    }

    /** Sign a buffer with an RSA key.
     */
    std::vector<uint8_t> BCryptAsymmetricKey::SignBuffer(std::vector<uint8_t> const&) const
    {
      throw std::runtime_error("Not implemented");
    }

    EcdsaBCryptAsymmetricKey::EcdsaBCryptAsymmetricKey() {}

    namespace _details {
      std::string GetBCryptError(std::string const& what) { return "Uknown error" + what; }

      BCryptException::BCryptException(std::string const& what)
          : runtime_error(GetBCryptError(what))
      {
      }

    } // namespace _details
}}}}} // namespace Azure::Security::Attestation::_private::Cryptography
#endif // NOTFUNCTIONAL