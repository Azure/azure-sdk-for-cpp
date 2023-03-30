// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once

#include <azure/core/internal/strings.hpp>

#include <algorithm>
#include <cstring>
#include <memory>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
/**
 * @brief The Cryptography class provides a set of basic cryptographic primatives required
 * by the attestation samples.
 */

namespace _detail {
// Helpers to provide RAII wrappers for OpenSSL types.
template <typename T, void (&Deleter)(T*)> struct openssl_deleter
{
  void operator()(T* obj) { Deleter(obj); }
};
template <typename T, void (&FreeFunc)(T*)>
using basic_openssl_unique_ptr = std::unique_ptr<T, openssl_deleter<T, FreeFunc>>;

// *** Given just T, map it to the corresponding FreeFunc:
template <typename T> struct type_map_helper;
template <> struct type_map_helper<EVP_PKEY>
{
  using type = basic_openssl_unique_ptr<EVP_PKEY, EVP_PKEY_free>;
};

template <> struct type_map_helper<BIO>
{
  using type = basic_openssl_unique_ptr<BIO, BIO_free_all>;
};
template <> struct type_map_helper<X509>
{
  using type = basic_openssl_unique_ptr<X509, X509_free>;
};
template <> struct type_map_helper<EVP_MD_CTX>
{
  using type = basic_openssl_unique_ptr<EVP_MD_CTX, EVP_MD_CTX_free>;
};

// *** Now users can say openssl_unique_ptr<T> if they want:
template <typename T> using openssl_unique_ptr = typename type_map_helper<T>::type;

// *** Or the current solution's convenience aliases:
using openssl_evp_pkey = openssl_unique_ptr<EVP_PKEY>;
using openssl_bio = openssl_unique_ptr<BIO>;

template <typename Api, typename... Args> auto make_openssl_unique(Api& OpensslApi, Args&&... args)
{
  auto raw = OpensslApi(
      std::forward<Args>(args)...); // forwarding is probably unnecessary, could use const Args&...
  // check raw
  using T = std::remove_pointer_t<decltype(raw)>; // no need to request T when we can see
                                                  // what OpensslApi returned
  return openssl_unique_ptr<T>{raw};
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

struct OpenSSLException : public std::runtime_error
{
  OpenSSLException(std::string const& what) : runtime_error(GetOpenSSLError(what)) {}
};

} // namespace _detail

class Cryptography {
public:
  /**
   * @brief Represents an X.509 certificate.
   *
   */
  class X509Certificate {
  private:
    ::_detail::openssl_unique_ptr<X509> m_certificate;
    // Delete the copy constructor and assignment operator for this certificate.
    X509Certificate(const X509Certificate&) = delete;
    X509Certificate& operator=(const X509Certificate&) = delete;

    std::string BinaryToHexString(std::vector<uint8_t> const& src) const
    {
      static constexpr char hexMap[]
          = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
      std::string output(static_cast<size_t>(src.size()) * 2, ' ');
      const uint8_t* input = src.data();

      for (size_t i = 0; i < src.size(); i++)
      {
        output[2 * i] = hexMap[(input[i] & 0xF0) >> 4];
        output[2 * i + 1] = hexMap[input[i] & 0x0F];
      }

      return output;
    }
    static std::string GetFormattedDnString(const X509_NAME* dn)
    {
      ::_detail::openssl_bio bio(::_detail::make_openssl_unique(BIO_new, BIO_s_mem()));
      // Print the DN in a single line, but don't add spaces around the equals sign (mbedtls
      // doesn't add them, so if we want them to compare properly, we remove the spaces).
      int length = X509_NAME_print_ex(bio.get(), dn, 0, XN_FLAG_ONELINE & ~XN_FLAG_SPC_EQ);
      if (length < 0)
      {
        throw ::_detail::OpenSSLException("X509_NAME_print_ex");
      }

      // Now extract the data from the BIO and return it as a string.
      uint8_t* base64data;
      long bufferSize = BIO_get_mem_data(bio.get(), &base64data);
      std::string returnValue;
      returnValue.resize(bufferSize);
      memcpy(&returnValue[0], base64data, bufferSize);
      return returnValue;
    }

  private:
    X509Certificate() {}
    X509Certificate(::_detail::openssl_unique_ptr<X509>&& x509) : m_certificate(std::move(x509)) {}

  public:
    virtual ~X509Certificate() {}

    std::string GetSubjectName() const
    {
      return GetFormattedDnString(X509_get_subject_name(m_certificate.get()));
    }

    std::string GetIssuerName() const
    {
      return GetFormattedDnString(X509_get_issuer_name(m_certificate.get()));
    }

    /**
     * @brief Get the Thumbprint for this certificate.
     *
     * @details The Thumbprint of a certificate is a hex encoded SHA1 hash of the DER
     * serialization of the certificate. It can be used to uniquely identify a certificate.
     *
     * @return std::string Certificate thumbprint.
     */
    virtual std::string GetThumbprint() const
    {
      // X.509 thumbprints are calculated using SHA1, even though SHA1 is insecure.
      auto hash(::_detail::make_openssl_unique(EVP_MD_CTX_new));
      EVP_DigestInit_ex(hash.get(), EVP_sha1(), nullptr);

      int len = i2d_X509(m_certificate.get(), nullptr);
      std::vector<uint8_t> thumbprintBuffer(len);
      unsigned char* buf = thumbprintBuffer.data();
      if (i2d_X509(m_certificate.get(), &buf) < 0)
      {
        throw ::_detail::OpenSSLException("i2d_X509");
      }
      if (EVP_DigestUpdate(hash.get(), thumbprintBuffer.data(), thumbprintBuffer.size()) != 1)
      {
        throw ::_detail::OpenSSLException("EVP_DigestUpdate");
      }
      uint32_t hashLength = EVP_MAX_MD_SIZE;
      std::vector<uint8_t> hashedThumbprint(EVP_MAX_MD_SIZE);
      if (EVP_DigestFinal_ex(hash.get(), hashedThumbprint.data(), &hashLength) != 1)
      {
        throw ::_detail::OpenSSLException("EVP_DigestFinal_ex");
      }
      hashedThumbprint.resize(hashLength);

      // HexString uses an "a"-"f" alphabet, but the CLR hex encoder uses an "A"-"F" alphabet,
      // so we need to uppercase them.
      return Azure::Core::_internal::StringExtensions::ToUpper(BinaryToHexString(hashedThumbprint));
    }

    static std::unique_ptr<Cryptography::X509Certificate> Import(
        std::string const& pemEncodedString)
    {
      auto bio(::_detail::make_openssl_unique(
          BIO_new_mem_buf, pemEncodedString.data(), static_cast<int>(pemEncodedString.size())));
      X509* raw_x509 = nullptr;
      raw_x509 = PEM_read_bio_X509(bio.get(), nullptr, nullptr, nullptr);
      if (raw_x509 == nullptr)
      {
        throw ::_detail::OpenSSLException("PEM_read_bio_X509"); // LCOV_EXCL_LINE
      }
      ::_detail::openssl_unique_ptr<X509> x509(raw_x509);
      raw_x509 = nullptr;
      return std::unique_ptr<X509Certificate>(new X509Certificate(std::move(x509)));
    }
  };

  static std::unique_ptr<Cryptography::X509Certificate> ImportX509Certificate(
      std::string const& pemEncodedCertificate)
  {
    return X509Certificate::Import(pemEncodedCertificate);
  }

  /**
   * @brief Convert a base64 encoded value to the PEM encoded equivalent.
   *
   * @param base64 base64 encoded value.
   * @param pemType Type of the object to be converted - typically "CERTIFICATE", "PRIVATE KEY",
   * or "PUBLIC KEY".
   * @return std::string PEM encoded representation of the base64 value.
   */
  static std::string PemFromBase64(std::string const& base64, std::string const& pemType)
  {
    std::string rv;
    rv += "-----BEGIN ";
    rv += pemType;
    rv += "-----\r\n ";
    std::string encodedValue(base64);

    // Insert crlf characters every 80 characters into the base64 encoded key to make it
    // prettier.
    size_t insertPos = 80;
    while (insertPos < encodedValue.length())
    {
      encodedValue.insert(insertPos, "\r\n");
      insertPos += 82; /* 80 characters plus the \r\n we just inserted */
    }

    rv += encodedValue;
    rv += "\r\n-----END ";
    rv += pemType;
    rv += "-----\r\n ";
    return rv;
  }
};
