// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief #Azure::Core::Http::HttpTransport implementation via WinHTTP.
 */

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/policies/policy.hpp"
#include "azure/core/http/transport.hpp"
#include "azure/core/internal/unique_handle.hpp"
#include "azure/core/platform.hpp"

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <memory>
#include <type_traits>
#include <vector>
#include <wincrypt.h>
#include <winhttp.h>

namespace Azure { namespace Core {
  namespace _internal {
    /**
     * @brief  Unique handle for WinHTTP HINTERNET handles.
     *
     * @note HINTERNET is declared as a "void *". THis means that this definition subsumes all other
     * `void *` types when used with Azure::Core::_internal::UniqueHandle.
     *
     */
    template <> struct UniqueHandleHelper<HINTERNET>
    {
      static void FreeHandle(HINTERNET obj) { WinHttpCloseHandle(obj); }

      using type = BasicUniqueHandle<void, FreeHandle>;
    };
  } // namespace _internal

  namespace Http {

    namespace _detail {

      constexpr static size_t DefaultUploadChunkSize = 1024 * 64;
      constexpr static size_t MaximumUploadChunkSize = 1024 * 1024;

      // Forward declaration for WinHttpRequest.
      class WinHttpRequest;
    } // namespace _detail

    /**
     * @brief Sets the WinHTTP session and connection options used to customize the behavior of the
     * transport.
     */
    struct WinHttpTransportOptions final
    {
      /**
       * @brief When `true`, allows an invalid certificate authority.
       */
      bool IgnoreUnknownCertificateAuthority{false};

      /**
       * Proxy information.
       */

      /**
       * @brief If True, enables the use of the system default proxy.
       *
       * @remarks Set this to "true" if you would like to use a local HTTP proxy like "Fiddler" to
       * capture and analyze HTTP traffic.
       *
       * Set to "false" by default because it is not recommended to use a proxy for production and
       * Fiddler's proxy interferes with the HTTP functional tests.
       */
      bool EnableSystemDefaultProxy{false};

      /**
       * @brief If True, enables checks for certificate revocation.
       */
      bool EnableCertificateRevocationListCheck{false};

      /**
       * @brief Proxy information.
       *
       * @remark The Proxy Information string is composed of a set of elements
       * formatted as follows:
       * ([<scheme>=][<scheme>"://"]<server>[":"<port>])
       *
       * Each element should be separated with semicolons or whitespace.
       */
      std::string ProxyInformation;

      /**
       * @brief User name for proxy authentication.
       */
      Azure::Nullable<std::string> ProxyUserName;

      /**
       * @brief Password for proxy authentication.
       */
      Azure::Nullable<std::string> ProxyPassword;

      /**
       * @brief Array of Base64 encoded DER encoded X.509 certificate.  These certificates should
       * form a chain of certificates which will be used to validate the server certificate sent by
       * the server.
       */
      std::vector<std::string> ExpectedTlsRootCertificates;
    };

    /**
     * @brief Concrete implementation of an HTTP transport that uses WinHTTP when sending and
     * receiving requests and responses over the wire.
     */
    class WinHttpTransport : public HttpTransport {
    private:
      WinHttpTransportOptions m_options;
      // m_sessionhandle is const to ensure immutability.
      const Azure::Core::_internal::UniqueHandle<HINTERNET> m_sessionHandle;

      Azure::Core::_internal::UniqueHandle<HINTERNET> CreateSessionHandle();
      Azure::Core::_internal::UniqueHandle<HINTERNET> CreateConnectionHandle(
          Azure::Core::Url const& url,
          Azure::Core::Context const& context);
      std::unique_ptr<_detail::WinHttpRequest> CreateRequestHandle(
          Azure::Core::_internal::UniqueHandle<HINTERNET> const& connectionHandle,
          Azure::Core::Url const& url,
          Azure::Core::Http::HttpMethod const& method);

      // Callback to allow a derived transport to extract the request handle. Used for WebSocket
      // transports.
      virtual void OnUpgradedConnection(std::unique_ptr<_detail::WinHttpRequest> const&){};

      /**
       * @brief Throw an exception based on the Win32 Error code
       *
       * @param exceptionMessage Message describing error.
       * @param error Win32 Error code.
       */
      void GetErrorAndThrow(const std::string& exceptionMessage, DWORD error = GetLastError());

    public:
      /**
       * @brief Constructs `%WinHttpTransport`.
       *
       * @param options Optional parameter to override the default settings.
       */
      WinHttpTransport(WinHttpTransportOptions const& options = WinHttpTransportOptions());

      /**
       * @brief Constructs `%WinHttpTransport`.
       *
       * @param options Optional parameter to override the default settings.
       */
      /**
       * @brief Constructs `%WinHttpTransport` object based on common Azure HTTP Transport Options
       *
       * @param options Common Azure Core Transport Options to override the default settings.
       */
      WinHttpTransport(Azure::Core::Http::Policies::TransportOptions const& options);

      /**
       * @brief Implements the HTTP transport interface to send an HTTP Request and produce an
       * HTTP RawResponse.
       *
       * @param context A context to control the request lifetime.
       * @param request an HTTP request to be send.
       * @return A unique pointer to an HTTP RawResponse.
       */
      virtual std::unique_ptr<RawResponse> Send(Request& request, Context const& context) override;

      // See also:
      // [Core Guidelines C.35: "A base class destructor should be either public
      // and virtual or protected and
      // non-virtual"](http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c35-a-base-class-destructor-should-be-either-public-and-virtual-or-protected-and-non-virtual)
      virtual ~WinHttpTransport();
    };

  } // namespace Http
}} // namespace Azure::Core
