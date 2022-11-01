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
#include <cassert>
#include <memory>
#include <mutex>
#pragma warning(push)
#pragma warning(disable : 6553)
#include <wil\resource.h>
#pragma warning(pop)
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

    // An HttpOperation reflects an outstanding WinHTTP action. The WinHttpAction object allows for
    // several classes of HttpOperation to be in progress at the same time. Those roughly are:
    //
    // 1) Receive Operations
    // 2) Send Operations
    // 3) Close Operations (used only for WebSocket handles).
    // 4) Handle Closing operations
    // There can be only one operation outstanding at a time for each category of operation.
    class HttpOperation {
      wil::unique_event m_operationCompleteEvent;
      // Mutex protecting all mutable members of the class.
      std::mutex m_operationStateMutex;
      bool m_operationStarted{};
      DWORD m_stowedError{};
      DWORD_PTR m_stowedErrorInformation{};
      DWORD m_bytesAvailable{};
      WINHTTP_WEB_SOCKET_STATUS m_webSocketStatus{};
      DWORD m_operationMutexOwner{};

    public:
      HttpOperation() : m_operationCompleteEvent(CreateEvent(nullptr, TRUE, FALSE, nullptr))
      {
        if (!m_operationCompleteEvent)
        {
          throw std::runtime_error("Error creating Action Complete Event.");
        }
      }

      /**
       * @brief Start an HttpOperation.
       *
       * StartOperation is called before starting an HttpOperation. It resets the internal state of
       * the HTTP Operation to a known state, and ensures that WaitForSingleObject will block (by
       * resetting the operation complete event to the not-signalled state).
       */
      void StartOperation()
      {
        // Reset the internal operation state.
        std::unique_lock<std::mutex> lock(m_operationStateMutex);
        assert(!m_operationStarted);
        m_stowedError = ERROR_SUCCESS;
        m_stowedErrorInformation = static_cast<DWORD_PTR>(-1);
        m_bytesAvailable = 0;
        m_webSocketStatus.dwBytesTransferred = 0;
        m_webSocketStatus.eBufferType = WINHTTP_WEB_SOCKET_BINARY_FRAGMENT_BUFFER_TYPE;
        m_operationStarted = true;

        // Reset the manual operation complete event so we will block until it's set to the
        // signalled state.
        m_operationCompleteEvent.ResetEvent();
      }

      /**
       * @brief Mark an HttpOperation as complete.
       */
      void CompleteOperation()
      {
        std::unique_lock<std::mutex> lock(m_operationStateMutex);
        // Note that we cannot assert that m_operationStarted is true here.
        // That's because WinHTTP calls the status callback with an AsyncAction of 0
        // to reflect that all outstanding calls need to fail with an error.
        if (m_operationStarted)
        {
          m_operationStarted = false;
          m_operationCompleteEvent.SetEvent();
        }
      }
      DWORD WaitForSingleObject(DWORD waitTimeout)
      {
        return ::WaitForSingleObject(m_operationCompleteEvent.get(), waitTimeout);
      }
      void UpdateStowedError(DWORD_PTR stowedErrorInformation, DWORD stowedError)
      {
        std::unique_lock<std::mutex> lock(m_operationStateMutex);
        m_stowedError = stowedError;
        m_stowedErrorInformation = stowedErrorInformation;
      }
      void UpdateBytesAvailable(DWORD bytesAvailable)
      {
        std::unique_lock<std::mutex> lock(m_operationStateMutex);
        m_bytesAvailable = bytesAvailable;
      }
      void UpdateWebSocketStatus(WINHTTP_WEB_SOCKET_STATUS* webSocketStatus)
      {
        std::unique_lock<std::mutex> lock(m_operationStateMutex);
        m_webSocketStatus = *webSocketStatus;
      }

      DWORD GetStowedError()
      {
        std::unique_lock<std::mutex> lock(m_operationStateMutex);
        return m_stowedError;
      }
      DWORD_PTR GetStowedErrorInformation()
      {
        std::unique_lock<std::mutex> lock(m_operationStateMutex);
        return m_stowedErrorInformation;
      }
      DWORD GetBytesAvailable()
      {
        std::unique_lock<std::mutex> lock(m_operationStateMutex);
        return m_bytesAvailable;
      }
      WINHTTP_WEB_SOCKET_STATUS const* GetWebSocketStatus()
      {
        std::unique_lock<std::mutex> lock(m_operationStateMutex);
        return &m_webSocketStatus;
      }
    };

    // Containing HTTP request, used during the status operation callback.
    WinHttpRequest* const m_httpRequest{};
    // True if this action is for a websocket transport.
    const bool m_isWebSocketAction{false};

    HttpOperation m_sendOperation;
    HttpOperation m_receiveOperation;
    HttpOperation m_closeOperation;
    HttpOperation m_handleClosingOperation;

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

    HttpOperation& OperationFromActionStatus(DWORD callbackStatus);
    HttpOperation& OperationFromAsyncResult(DWORD_PTR asyncResult);

  public:
    /**
     * @brief Create a new WinHttpAction object associated with a specific WinHttpRequest.
     *
     * @param request Http Request associated with the action.
     *
     * @remarks If the WinHttpRequest object is null, this is a hint that the WinHttpAction is
     * associated with a WebSocket request, since WebSocket operations don't have an associated
     * WinHttpRequest object.
     */
    WinHttpAction(WinHttpRequest* request)
        // Create a non-inheritable anonymous manual reset event intialized as unset.
        : m_httpRequest(request), m_isWebSocketAction(request == nullptr)
    {
    }

    /**
     * Register the WinHTTP Status callback used by the action.
     *
     * @param internetHandle HINTERNET to register the callback.
     * @returns The status of the operation.
     */
    bool RegisterWinHttpStatusCallback(
        Azure::Core::_internal::UniqueHandle<HINTERNET> const& internetHandle);
    /**
     * Unregisters the WinHTTP Status callback used by the action.
     *
     * @param internetHandle HINTERNET to register the callback.
     * @returns The status of the operation.
     */
    bool UnregisterWinHttpStatusCallback(
        Azure::Core::_internal::UniqueHandle<HINTERNET> const& internetHandle);

    /**
     * @brief WaitForAction - Waits for an action to complete.
     *
     * @remarks The WaitForAction method waits until an action initiated by the `callback` function
     * has completed. Every pollDuration milliseconds, it checks to see if the context specified for
     * the request has been cancelled (or times out).
     *
     * @param initiateAction - Function called to initiate an action. Always called in the waiting
     *        thread.
     * @param expectedCallbackStatus - Wait until the expectedStatus event occurs.
     * @param pollDuration - The time to wait for a ping to complete. Defaults to 800ms because it
     *        seems like a reasonable minimum responsiveness value (also this is the default retry
     *        policy delay).
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
     * @brief Notify a caller that a close action has completed successfully.
     *
     * @remarks Completes a wait operation initiated by WaitForAction, for a close operation.
     *
     * @note This function is only used for WebSocket transports.
     *
     */
    void CompleteCloseAction(DWORD actionToComplete);
    /**
     * @brief Notify a caller that the underlying HTTP request handle has been closed.
     *
     * @remarks Completes a wait operation initiated by WaitForAction, for a close operation.
     */
    void CompleteHandleCloseAction(DWORD actionToComplete);

    /**
     * @brief Notify a caller that the action has completed successfully.
     *
     * @remarks Completes a wait operation initiated by WaitForAction, for send operations.
     */
    void CompleteSendAction(DWORD actionToComplete);

    /**
     * @brief Notify a caller that the action has completed successfully.
     *
     * @remarks Completes a wait operation initiated by WaitForAction, for receive operations.
     */
    void CompleteReceiveAction(DWORD actionToComplete);

    /**
     * @brief Notify a caller that the action has completed successfully and reflect the bytes
     * available
     */
    void CompleteReceiveActionWithData(DWORD actionToComplete, DWORD bytesAvailable);

    /**
     * @brief Notify a caller that the WebSocket action has completed successfully.
     */
    void CompleteSendActionWithWebSocketStatus(
        DWORD actionToComplete,
        LPVOID statusInformation,
        DWORD statusInformationLength);
    /**
     * @brief Notify a caller that the WebSocket action has completed successfully.
     */
    void CompleteReceiveActionWithWebSocketStatus(
        DWORD actionToComplete,
        LPVOID statusInformation,
        DWORD statusInformationLength);

    /**
     * @brief Notify a caller that the action has completed with an error and save the error code
     * and information.
     *
     * @param actionToComplete - event received.
     * @param stowedErrorInformation - stowed error information from WinHTTP.
     * @param stowedError - Win32 error code.
     */
    void CompleteActionWithError(DWORD_PTR stowedErrorInformation, DWORD stowedError);
    DWORD GetStowedError(DWORD actionToComplete);
    DWORD_PTR GetStowedErrorInformation(DWORD actionToComplete);
    DWORD GetBytesAvailable(DWORD actionToComplete);
    WINHTTP_WEB_SOCKET_STATUS const* const GetWebSocketStatus(DWORD actionToComplete);
  };

  /**
   * @brief A WinHttpRequest object encapsulates an HTTP operation.
   */
  class WinHttpRequest final {
    Azure::Core::_internal::UniqueHandle<HINTERNET> m_requestHandle;
    std::unique_ptr<WinHttpAction> m_httpAction;
    std::vector<std::string> m_expectedTlsRootCertificates;
    // Thread used to asynchronously close a request handle if the expected root certificate does
    // not match.
    std::mutex m_handleClosedLock;
    std::thread m_handleCloseThread;
    bool m_requestHandleClosed{false};

    /*
     * Adds the specified trusted certificates to the specified certificate store.
     */
    bool AddCertificatesToStore(
        std::vector<std::string> const& trustedCertificates,
        HCERTSTORE const hCertStore) const;
    /*
     * Verifies that the certificate context is in the trustedCertificates set of certificates.
     */
    bool VerifyCertificatesInChain(
        std::vector<std::string> const& trustedCertificates,
        PCCERT_CONTEXT serverCertificate) const;
    /**
     * @brief Throw an exception based on the Win32 Error code
     *
     * @param exceptionMessage Message describing error.
     * @param error Win32 Error code.
     */
    void GetErrorAndThrow(const std::string& exceptionMessage, DWORD error = GetLastError()) const;

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
    HINTERNET const GetRequestHandle() { return m_requestHandle.get(); }
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
