// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief #Azure::Core::Http::HttpTransport implementation via WinHTTP.
 * cspell:words HCERTIFICATECHAIN PCCERT CCERT HCERTCHAINENGINE HCERTSTORE
 */

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/transport.hpp"
#include "azure/core/internal/unique_handle.hpp"

#include <azure/core/platform.hpp>

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

      class WinHttpStream final : public Azure::Core::IO::BodyStream {
      private:
        Azure::Core::_internal::UniqueHandle<HINTERNET> m_requestHandle;
        bool m_isEOF;

        /**
         * @brief This is a copy of the value of an HTTP response header `content-length`. The value
         * is received as string and parsed to size_t. This field avoids parsing the string header
         * every time from HTTP RawResponse.
         *
         * @remark This value is also used to avoid trying to read more data from network than what
         * we are expecting to.
         *
         * @remark A value of -1 means the transfer encoding was chunked.
         *
         */
        int64_t m_contentLength;

        int64_t m_streamTotalRead;

        /**
         * @brief Implement #Azure::Core::IO::BodyStream::OnRead(). Calling this function pulls data
         * from the wire.
         *
         * @param context A context to control the request lifetime.
         * @param buffer Buffer where data from wire is written to.
         * @param count The number of bytes to read from the network.
         * @return The actual number of bytes read from the network.
         */
        size_t OnRead(uint8_t* buffer, size_t count, Azure::Core::Context const& context) override;

      public:
        WinHttpStream(
            Azure::Core::_internal::UniqueHandle<HINTERNET>& requestHandle,
            int64_t contentLength)
            : m_requestHandle(std::move(requestHandle)), m_contentLength(contentLength),
              m_isEOF(false), m_streamTotalRead(0)
        {
        }

        /**
         * @brief Implement #Azure::Core::IO::BodyStream length.
         *
         * @return The size of the payload.
         */
        int64_t Length() const override { return this->m_contentLength; }
      };
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
      std::string ProxyUserName;

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

      // This should remain immutable and not be modified after calling the ctor, to avoid threading
      // issues.
      Azure::Core::_internal::UniqueHandle<HINTERNET> m_sessionHandle;
      bool m_requestHandleClosed{false};

      Azure::Core::_internal::UniqueHandle<HINTERNET> CreateSessionHandle();
      Azure::Core::_internal::UniqueHandle<HINTERNET> CreateConnectionHandle(
          Azure::Core::Url const& url,
          Azure::Core::Context const& context);
      Azure::Core::_internal::UniqueHandle<HINTERNET> CreateRequestHandle(
          Azure::Core::_internal::UniqueHandle<HINTERNET> const& connectionHandle,
          Azure::Core::Url const& url,
          Azure::Core::Http::HttpMethod const& method);
      void Upload(
          Azure::Core::_internal::UniqueHandle<HINTERNET> const& requestHandle,
          Azure::Core::Http::Request& request,
          Azure::Core::Context const& context);
      void SendRequest(
          Azure::Core::_internal::UniqueHandle<HINTERNET> const& requestHandle,
          Azure::Core::Http::Request& request,
          Azure::Core::Context const& context);
      void ReceiveResponse(
          Azure::Core::_internal::UniqueHandle<HINTERNET> const& requestHandle,
          Azure::Core::Context const& context);
      int64_t GetContentLength(
          Azure::Core::_internal::UniqueHandle<HINTERNET> const& requestHandle,
          HttpMethod requestMethod,
          HttpStatusCode responseStatusCode);
      std::unique_ptr<RawResponse> SendRequestAndGetResponse(
          Azure::Core::_internal::UniqueHandle<HINTERNET>& requestHandle,
          HttpMethod requestMethod);

      /*
       * Callback from WinHTTP called after the TLS certificates are received when the caller sets
       * expected TLS root certificates.
       */
      static void CALLBACK StatusCallback(
          HINTERNET hInternet,
          DWORD_PTR dwContext,
          DWORD dwInternetStatus,
          LPVOID lpvStatusInformation,
          DWORD dwStatusInformationLength) noexcept;
      /*
       * Callback from WinHTTP called after the TLS certificates are received when the caller sets
       * expected TLS root certificates.
       */
      void OnHttpStatusOperation(HINTERNET hInternet, DWORD dwInternetStatus);
      /*
       * Adds the specified trusted certificates to the specified certificate store.
       */
      bool AddCertificatesToStore(
          std::vector<std::string> const& trustedCertificates,
          HCERTSTORE const hCertStore);
      /*
       * Verifies that the certificate context is in the trustedCertificates set of certificates.
       */
      bool VerifyCertificatesInChain(
          std::vector<std::string> const& trustedCertificates,
          PCCERT_CONTEXT serverCertificate);

      // Callback to allow a derived transport to extract the request handle. Used for WebSocket
      // transports.
      virtual void OnUpgradedConnection(Azure::Core::_internal::UniqueHandle<HINTERNET> const&){};
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
       * @brief Implements the HTTP transport interface to send an HTTP Request and produce an HTTP
       * RawResponse.
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
      virtual ~WinHttpTransport() = default;
    };

  } // namespace Http
}} // namespace Azure::Core
