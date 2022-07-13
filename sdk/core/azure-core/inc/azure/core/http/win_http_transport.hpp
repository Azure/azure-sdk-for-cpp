// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief #Azure::Core::Http::HttpTransport implementation via WinHTTP.
 */

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/transport.hpp"

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
#include <winhttp.h>

namespace Azure { namespace Core { namespace Http {

  namespace _detail {

    constexpr static size_t DefaultUploadChunkSize = 1024 * 64;
    constexpr static size_t MaximumUploadChunkSize = 1024 * 1024;

    // unique_ptr class wrapping an HINTERNET handle
    class HINTERNET_deleter {
    public:
      void operator()(HINTERNET handle) noexcept
      {
        if (handle != nullptr)
        {
          WinHttpCloseHandle(handle);
        }
      }
    };
    using unique_HINTERNET = std::unique_ptr<void, HINTERNET_deleter>;

    class WinHttpStream final : public Azure::Core::IO::BodyStream {
    private:
      _detail::unique_HINTERNET m_requestHandle;
      bool m_isEOF;

      /**
       * @brief This is a copy of the value of an HTTP response header `content-length`. The value
       * is received as string and parsed to size_t. This field avoids parsing the string header
       * every time from HTTP RawResponse.
       *
       * @remark This value is also used to avoid trying to read more data from network than what we
       * are expecting to.
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
      WinHttpStream(_detail::unique_HINTERNET& requestHandle, int64_t contentLength)
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
    bool IgnoreUnknownCertificateAuthority = false;
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
    _detail::unique_HINTERNET m_sessionHandle;

    _detail::unique_HINTERNET CreateSessionHandle();
    _detail::unique_HINTERNET CreateConnectionHandle(
        Azure::Core::Url const& url,
        Azure::Core::Context const& context);
    _detail::unique_HINTERNET CreateRequestHandle(
        _detail::unique_HINTERNET& connectionHandle,
        Azure::Core::Url const& url,
        Azure::Core::Http::HttpMethod const& method);
    void Upload(
        _detail::unique_HINTERNET& requestHandle,
        Azure::Core::Http::Request& request,
        Azure::Core::Context const& context);
    void SendRequest(
        _detail::unique_HINTERNET& requestHandle,
        Azure::Core::Http::Request& request,
        Azure::Core::Context const& context);
    void ReceiveResponse(
        _detail::unique_HINTERNET& requestHandle,
        Azure::Core::Context const& context);
    int64_t GetContentLength(
        _detail::unique_HINTERNET& requestHandle,
        HttpMethod requestMethod,
        HttpStatusCode responseStatusCode);
    std::unique_ptr<RawResponse> SendRequestAndGetResponse(
        _detail::unique_HINTERNET& requestHandle,
        HttpMethod requestMethod);

    // Callback to allow a derived transport to extract the request handle. Used for WebSocket
    // transports.
  protected:
    virtual void OnResponseReceived(_detail::unique_HINTERNET&){};
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

    virtual ~WinHttpTransport() = default;
  };

}}} // namespace Azure::Core::Http
