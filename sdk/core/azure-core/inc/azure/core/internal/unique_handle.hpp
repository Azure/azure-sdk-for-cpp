// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include <memory>

namespace Azure { namespace Core { namespace _internal {

  /**
   * Helpers to provide RAII wrappers for OpenSSL, WinHTTP, CURL, and other types.
   *
   * These classes enable callers to wrap std::unique_ptr for arbitrary 3rd party library types.
   *
   * To add support for a 3rd party type, you need to define a helper structure which lets
   * std::unique_ptr know how to delete the underlying object:
   *
   *  template <> struct UniqueHandleHelper<CURL>
   *  {
   *    using type = BasicUniqueHandle<CURL, curl_easy_cleanup>;
   *  };
   *
   * Note that for some types (HINTERNET for example), the helper needs to be a bit more
   * complicated: template <> struct UniqueHandleHelper<HINTERNET>
   *  {
   *    static void FreeHandle(HINTERNET obj) { WinHttpCloseHandle(obj); }
   *    using type = BasicUniqueHandle<void, FreeHandle>;
   *  };
   *
   * This is because WinHttpCloseHandle returns a BOOL, not void, so we need to wrap the call to
   * WinHttpCloseHandle(). In addition, an HINTERNET is a `void *` type, which means that the
   * std::unique_handle underlying type needs to be `void`.
   *
   */

  /**
   * @brief std::unique_ptr deleter class which provides a specialization for a deleter function.
   */
  template <typename T, void (&Deleter)(T*)> struct UniqueHandleDeleter
  {
    void operator()(T* obj) { Deleter(obj); }
  };

  /**
   * @brief Inner specialization for std::unique_ptr for the
   */
  template <typename T, void (&FreeFunc)(T*)>
  using BasicUniqueHandle = std::unique_ptr<T, UniqueHandleDeleter<T, FreeFunc>>;

  /**
   * @brief Helper template structure to wrap the map between the type and free function
   *
   *  template <> struct UniqueHandleHelper<HINTERNET>
   *  {
   *    using type = BasicUniqueHandle<HINTERNET, WinHttpCloseHandle>;
   *  };
   *
   *  template <> struct UniqueHandleHelper<CURL>
   *  {
   *    using type = BasicUniqueHandle<CURL, curl_easy_cleanup>;
   *  };
   */
  // *** Given just T, map it to the corresponding FreeFunc
  template <typename T> struct UniqueHandleHelper;

  // *** Now users can say UniqueHandle<T> if they want:
  template <typename T, template <typename> class U = UniqueHandleHelper>
  using UniqueHandle = typename U<T>::type;
}}} // namespace Azure::Core::_internal
