// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

// cspell:words PCCERT HCERTSTORE

/**
 * @file
 * @brief #Azure::Core::Http::HttpTransport request support classes.
 */

#pragma once

#include "azure/core/http/win_http_transport.hpp"
#include "azure/core/url.hpp"
#include <Windows.h>
#include <memory>
#include <mutex>
#include <wil\resource.h>
#include <winhttp.h>

namespace Azure { namespace Core { namespace Http { namespace _detail {

  class WinHttpRequest;
  /**
   * @brief An outstanding WinHTTP action. This object is used to process asynchronous WinHTTP
   * actions.
   *
   * The WinHttpRequest object has a WinHttpAction associated with it to convert asynchronous
   * WinHTTP operations to synchronous operations.
   *
   */
  class WinHttpAction final {
    // The WinHttpRequest class is friend because it needs to have access to StatusCallback when it
    // registers for notifications, but there's no other reason to make it public.
    friend class WinHttpRequest;

    // Containing HTTP request, used during the status operation callback.
    WinHttpRequest* m_httpRequest{};
    std::mutex m_actionCompleteMutex;
    wil::unique_event m_actionCompleteEvent;
    DWORD m_expectedStatus{};
    DWORD m_stowedError{};
    DWORD_PTR m_stowedErrorInformation{};
    DWORD m_bytesAvailable{};

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
    void OnHttpStatusOperation(
        HINTERNET hInternet,
        DWORD internetStatus,
        LPVOID statusInformation,
        DWORD statusInformationLength);

  public:
    /**
     * @brief Create a new WinHttpAction object associated with a specific WinHttpRequest.
     *
     * @param request Http Request associated with the action.
     */
    WinHttpAction(WinHttpRequest* request)
        // Create a non-inheritable anonymous manual reset event intialized as unset.
        : m_httpRequest(request), m_actionCompleteEvent(CreateEvent(nullptr, TRUE, FALSE, nullptr))
    {
      if (!m_actionCompleteEvent)
      {
        throw std::runtime_error("Error creating Action Complete Event.");
      }
    }
    /**
     * @brief WaitForAction - Waits for an action to complete.
     *
     * @remarks The WaitForAction method waits until an action initiated by the `callback` function
     * has completed. Every pollDuration milliseconds, it checks to see if the context specified for
     * therequest has been cancelled (or times out).
     *
     * @param initiateAction - Function called to initiate an action. Always called in the waiting
     *        thread.
     * @param expectedCallbackStatus - Wait until the expectedStatus event occurs.
     * @param pollDuration - The time to wait for a ping to complete.
     * @param context - Context for the operation.
     *
     * @returns true if the action completed normally, false if there was an error.

     * @remarks If there is an error, the caller can determine the error code by calling
     * GetStowedError() and GetStowedErrorInformation()
     */
    bool WaitForAction(
        std::function<void()> initiateAction,
        DWORD expectedCallbackStatus,
        Azure::Core::Context const& context,
        Azure::DateTime::duration const& pollDuration = std::chrono::milliseconds(800));

    /**
     * @brief Notify a caller that the action has completed successfully.
     */
    void CompleteAction();

    /**
     * @brief Notify a caller that the action has completed successfully and reflect the bytes
     * available
     */
    void CompleteActionWithData(DWORD bytesAvailable);

    /**
     * @brief Notify a caller that the action has completed with an error and save the error code
     * and information.
     */
    void CompleteActionWithError(DWORD_PTR stowedErrorInformation, DWORD stowedError);
    DWORD GetStowedError();
    DWORD_PTR GetStowedErrorInformation();
    DWORD GetBytesAvailable();
  };

  /**
   * @brief A WinHttpRequest object encapsulates an HTTP operation.
   */
  class WinHttpRequest final {
    bool m_requestHandleClosed{false};
    Azure::Core::_internal::UniqueHandle<HINTERNET> m_requestHandle;
    std::unique_ptr<WinHttpAction> m_httpAction;
    std::vector<std::string> m_expectedTlsRootCertificates;

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
    /**
     * @brief Throw an exception based on the Win32 Error code
     *
     * @param exceptionMessage Message describing error.
     * @param error Win32 Error code.
     */
    void GetErrorAndThrow(const std::string& exceptionMessage, DWORD error = GetLastError());

  public:
    WinHttpRequest(
        Azure::Core::_internal::UniqueHandle<HINTERNET> const& connectionHandle,
        Azure::Core::Url const& url,
        Azure::Core::Http::HttpMethod const& method,
        WinHttpTransportOptions const& options);

    ~WinHttpRequest();

    void Upload(Azure::Core::Http::Request& request, Azure::Core::Context const& context);
    void SendRequest(Azure::Core::Http::Request& request, Azure::Core::Context const& context);
    void ReceiveResponse(Azure::Core::Context const& context);
    int64_t GetContentLength(HttpMethod requestMethod, HttpStatusCode responseStatusCode);
    std::unique_ptr<RawResponse> SendRequestAndGetResponse(HttpMethod requestMethod);
    size_t ReadData(uint8_t* buffer, size_t bufferSize, Azure::Core::Context const& context);
    void EnableWebSocketsSupport();
    void HandleExpectedTlsRootCertificates(HINTERNET hInternet);
  };

  class WinHttpStream final : public Azure::Core::IO::BodyStream {
  private:
    std::unique_ptr<_detail::WinHttpRequest> m_requestHandle;
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
    WinHttpStream(std::unique_ptr<_detail::WinHttpRequest>& requestHandle, int64_t contentLength)
        : m_requestHandle(std::move(requestHandle)), m_contentLength(contentLength), m_isEOF(false),
          m_streamTotalRead(0)
    {
    }

    /**
     * @brief Implement #Azure::Core::IO::BodyStream length.
     *
     * @return The size of the payload.
     */
    int64_t Length() const override { return this->m_contentLength; }
  };

}}}} // namespace Azure::Core::Http::_detail
