// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief #HttpTransport implementation via WinHttp.
 */

#pragma once
//#ifdef BUILD_TRANSPORT_WINHTTP_ADAPTER

#include "azure/core/http/http.hpp"
#include "azure/core/http/policy.hpp"

#include <type_traits>
#include <vector>
#include <winhttp.h>

namespace Azure { namespace Core { namespace Http {

  namespace Details {

    constexpr static int64_t DefaultUploadChunkSize = 1024 * 64;
    constexpr static int64_t MaximumUploadChunkSize = 1024 * 1024 * 1024;

    class WinHttpStream : public BodyStream {
    private:
      HINTERNET m_sessionHandle;
      HINTERNET m_connectionHandle;
      HINTERNET m_requestHandle;
      bool m_isEOF;

      /**
       * @brief This is a copy of the value of an HTTP response header `content-length`. The value
       * is received as string and parsed to size_t. This field avoid parsing the string header
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

    public:
      WinHttpStream(
          HINTERNET sessionHandle,
          HINTERNET connectionHandle,
          HINTERNET requestHandle,
          int64_t contentLength)
          : m_sessionHandle(sessionHandle), m_connectionHandle(connectionHandle),
            m_requestHandle(requestHandle), m_contentLength(contentLength)
      {
        this->m_isEOF = false;
        this->m_streamTotalRead = 0;
      }

      ~WinHttpStream() override
      {
        WinHttpCloseHandle(this->m_requestHandle);
        WinHttpCloseHandle(this->m_connectionHandle);
        WinHttpCloseHandle(this->m_sessionHandle);
      }

      /**
       * @brief Implement #BodyStream length.
       *
       * @return The size of the payload.
       */
      int64_t Length() const override { return this->m_contentLength; }

      /**
       * @brief Implement #BodyStream read. Calling this function pulls data from the wire.
       *
       * @param context #Context so that operation can be canceled.
       * @param buffer Buffer where data from wire is written to.
       * @param count The number of bytes to read from the network.
       * @return The actual number of bytes read from the network.
       */
      int64_t Read(Azure::Core::Context const& context, uint8_t* buffer, int64_t count) override;
    };
  } // namespace Details

  /**
   * @brief Concrete implementation of an HTTP Transport that uses WinHttp.
   *
   */
  class WinHttpTransport : public HttpTransport {
  public:
    // WinHttpTransport() {}

    //~WinHttpTransport() {}

    /**
     * @brief Implements interface to send an HTTP Request and produce an HTTP RawResponse.
     *
     * @param context #Context so that operation can be canceled.
     * @param request an HTTP Request to be send.
     * @return unique ptr to an HTTP RawResponse.
     */
    virtual std::unique_ptr<RawResponse> Send(Context const& context, Request& request) override;
  };

}}} // namespace Azure::Core::Http

//#endif
