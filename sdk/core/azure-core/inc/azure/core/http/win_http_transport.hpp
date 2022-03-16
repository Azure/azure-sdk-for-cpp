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

#include <type_traits>
#include <vector>
#include <winhttp.h>

namespace Azure { namespace Core { namespace Http {

  namespace _detail {

    constexpr static size_t DefaultUploadChunkSize = 1024 * 64;
    constexpr static size_t MaximumUploadChunkSize = 1024 * 1024;

    struct HandleManager final
    {
      Context const& m_context;
      Request& m_request;
      HINTERNET m_sessionHandle;
      HINTERNET m_connectionHandle;
      HINTERNET m_requestHandle;

      HandleManager(Request& request, Context const& context)
          : m_request(request), m_context(context)
      {
        m_sessionHandle = NULL;
        m_connectionHandle = NULL;
        m_requestHandle = NULL;
      }

      ~HandleManager()
      {
        // Close the handles and set them to null to avoid multiple calls to WinHTT to close the
        // handles.
        if (m_requestHandle)
        {
          WinHttpCloseHandle(m_requestHandle);
          m_requestHandle = NULL;
        }

        if (m_connectionHandle)
        {
          WinHttpCloseHandle(m_connectionHandle);
          m_connectionHandle = NULL;
        }

        if (m_sessionHandle)
        {
          WinHttpCloseHandle(m_sessionHandle);
          m_sessionHandle = NULL;
        }
      }
    };

    class WinHttpStream final : public Azure::Core::IO::BodyStream {
    private:
      std::unique_ptr<HandleManager> m_handleManager;
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
      WinHttpStream(std::unique_ptr<HandleManager> handleManager, int64_t contentLength)
          : m_handleManager(std::move(handleManager)), m_contentLength(contentLength),
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
     * @brief When `true`, allows an invalid certificate authority. If this flag is set, the
     * application does not receive a WINHTTP_CALLBACK_STATUS_FLAG_INVALID_CA callback.
     */
    bool IgnoreUnknownServerCert = false;
  };

  /**
   * @brief Concrete implementation of an HTTP transport that uses WinHTTP when sending and
   * receiving requests and responses over the wire.
   */
  class WinHttpTransport final : public HttpTransport {
  private:
    WinHttpTransportOptions m_options;

    void CreateSessionHandle(std::unique_ptr<_detail::HandleManager>& handleManager);
    void CreateConnectionHandle(std::unique_ptr<_detail::HandleManager>& handleManager);
    void CreateRequestHandle(std::unique_ptr<_detail::HandleManager>& handleManager);
    void Upload(std::unique_ptr<_detail::HandleManager>& handleManager);
    void SendRequest(std::unique_ptr<_detail::HandleManager>& handleManager);
    void ReceiveResponse(std::unique_ptr<_detail::HandleManager>& handleManager);
    int64_t GetContentLength(
        std::unique_ptr<_detail::HandleManager>& handleManager,
        HttpMethod requestMethod,
        HttpStatusCode responseStatusCode);
    std::unique_ptr<RawResponse> SendRequestAndGetResponse(
        std::unique_ptr<_detail::HandleManager> handleManager,
        HttpMethod requestMethod);

  public:
    /**
     * @brief Constructs `%WinHttpTransport`.
     *
     * @param options Optional parameter to override the default settings.
     */
    WinHttpTransport(WinHttpTransportOptions const& options = WinHttpTransportOptions())
        : m_options(options)
    {
    }

    /**
     * @brief Implements the HTTP transport interface to send an HTTP Request and produce an HTTP
     * RawResponse.
     *
     * @param context A context to control the request lifetime.
     * @param request an HTTP request to be send.
     * @return A unique pointer to an HTTP RawResponse.
     */
    virtual std::unique_ptr<RawResponse> Send(Request& request, Context const& context) override;
  };

}}} // namespace Azure::Core::Http
