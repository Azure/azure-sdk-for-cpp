//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once

#include <memory>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace Azure { namespace Security { namespace Attestation { namespace _detail {

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

  // *** Now users can say openssl_unique_ptr<T> if they want:
  template <typename T> using openssl_unique_ptr = typename type_map_helper<T>::type;

  // *** Or the current solution's convenience aliases:
  using openssl_evp_pkey = openssl_unique_ptr<EVP_PKEY>;
  using openssl_bio = openssl_unique_ptr<BIO>;

#ifdef __cpp_nontype_template_parameter_auto
  // *** Wrapper function that calls a given OpensslApi, and returns the corresponding
  // openssl_unique_ptr<T>:
  template <auto OpensslApi, typename... Args> auto make_openssl_unique(Args&&... args)
  {
    auto raw = OpensslApi(
        forward<Args>(args)...); // forwarding is probably unnecessary, could use const Args&...
    // check raw
    using T = remove_pointer_t<decltype(raw)>; // no need to request T when we can see
                                               // what OpensslApi returned
    return openssl_unique_ptr<T>{raw};
  }
#else // ^^^ C++17 ^^^ / vvv C++14 vvv
  template <typename Api, typename... Args>
  auto make_openssl_unique(Api& OpensslApi, Args&&... args)
  {
    auto raw = OpensslApi(std::forward<Args>(
        args)...); // forwarding is probably unnecessary, could use const Args&...
    // check raw
    using T = std::remove_pointer_t<decltype(raw)>; // no need to request T when we can see
                                                    // what OpensslApi returned
    return openssl_unique_ptr<T>{raw};
  }
#endif // ^^^ C++14 ^^^

  extern std::string GetOpenSSLError(std::string const& what);

  struct OpenSSLException : public std::runtime_error
  {
    OpenSSLException(std::string const& what);
  };

}}}} // namespace Azure::Security::Attestation::_detail
