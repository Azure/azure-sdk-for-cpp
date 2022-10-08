// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
// cspell:words HCERTIFICATECHAIN PCCERT CCERT HCERTCHAINENGINE HCERTSTORE

#include "azure/core/http/http.hpp"

#include "azure/core/base64.hpp"
#include "azure/core/diagnostics/logger.hpp"
#include "azure/core/internal/diagnostics/log.hpp"
#include "azure/core/internal/strings.hpp"
#include "azure/core/internal/unique_handle.hpp"

#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
#include "azure/core/http/win_http_transport.hpp"
#endif
#include <Windows.h>
#include <algorithm>
#include <sstream>
#include <string>
#pragma warning(push)
#pragma warning(disable : 6553)
#include <wil/resource.h> // definitions for wil::unique_cert_chain_context and other RAII type wrappers for Windows types.
#pragma warning(pop)

#include <wincrypt.h>
#include <winhttp.h>

using Azure::Core::Context;
using namespace Azure::Core::Http;
using namespace Azure::Core::Diagnostics;
using namespace Azure::Core::Diagnostics::_internal;

namespace {

const std::string HttpScheme = "http";
const std::string WebSocketScheme = "ws";

inline std::wstring HttpMethodToWideString(HttpMethod method)
{
  // This string should be all uppercase.
  // Many servers treat HTTP verbs as case-sensitive, and the Internet Engineering Task Force (IETF)
  // Requests for Comments (RFCs) spell these verbs using uppercase characters only.

  std::string httpMethodString = method.ToString();

  // Assuming ASCII here is OK since the input is expected to be an HTTP method string.
  // Converting this way is only safe when the text is ASCII.
  std::wstring wideStr(httpMethodString.begin(), httpMethodString.end());
  return wideStr;
}

// Convert a UTF-8 string to a wide Unicode string.
// This assumes the input string is always null-terminated.
std::wstring StringToWideString(const std::string& str)
{
  // Since the strings being converted to wstring can be provided by the end user, and can contain
  // invalid characters, use the MB_ERR_INVALID_CHARS to validate and fail.

  // Passing in -1 so that the function processes the entire input string, including the terminating
  // null character.
  int sizeNeeded = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str.c_str(), -1, 0, 0);
  if (sizeNeeded == 0)
  {
    // Errors include:
    // ERROR_INSUFFICIENT_BUFFER
    // ERROR_INVALID_FLAGS
    // ERROR_INVALID_PARAMETER
    // ERROR_NO_UNICODE_TRANSLATION
    DWORD error = GetLastError();
    throw Azure::Core::Http::TransportException(
        "Unable to get the required transcoded size for the input string. Error Code: "
        + std::to_string(error) + ".");
  }

  std::wstring wideStr(sizeNeeded, L'\0');
  if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str.c_str(), -1, &wideStr[0], sizeNeeded)
      == 0)
  {
    DWORD error = GetLastError();
    throw Azure::Core::Http::TransportException(
        "Unable to transcode the input string to a wide string. Error Code: "
        + std::to_string(error) + ".");
  }
  return wideStr;
}

// Convert a wide Unicode string to a UTF-8 string.
std::string WideStringToString(const std::wstring& wideString)
{
  // We can't always assume the input wide string is null-terminated, so need to pass in the actual
  // size.
  size_t wideStrSize = wideString.size();
  if (wideStrSize > INT_MAX)
  {
    throw Azure::Core::Http::TransportException(
        "Input wide string is too large to fit within a 32-bit int.");
  }

  // Note, we are not using the flag WC_ERR_INVALID_CHARS here, because it is assumed the service
  // returns correctly encoded response headers and reason phrase strings.
  // The transport layer shouldn't do additional validation, and if WideCharToMultiByte replaces
  // invalid characters with the replacement character, that is fine.

  int wideStrLength = static_cast<int>(wideStrSize);
  int sizeNeeded
      = WideCharToMultiByte(CP_UTF8, 0, wideString.c_str(), wideStrLength, NULL, 0, NULL, NULL);
  if (sizeNeeded == 0)
  {
    // Errors include:
    // ERROR_INSUFFICIENT_BUFFER
    // ERROR_INVALID_FLAGS
    // ERROR_INVALID_PARAMETER
    // ERROR_NO_UNICODE_TRANSLATION
    DWORD error = GetLastError();
    throw Azure::Core::Http::TransportException(
        "Unable to get the required transcoded size for the input wide string. Error Code: "
        + std::to_string(error) + ".");
  }

  std::string str(sizeNeeded, 0);
  if (WideCharToMultiByte(
          CP_UTF8, 0, wideString.c_str(), wideStrLength, &str[0], sizeNeeded, NULL, NULL)
      == 0)
  {
    DWORD error = GetLastError();
    throw Azure::Core::Http::TransportException(
        "Unable to transcode the input wide string to a string. Error Code: "
        + std::to_string(error) + ".");
  }
  return str;
}

std::string WideStringToStringASCII(
    std::vector<WCHAR>::iterator wideStringStart,
    std::vector<WCHAR>::iterator wideStringEnd)
{
  // Converting this way is only safe when the text is ASCII.
#pragma warning(suppress : 4244)
  std::string str(wideStringStart, wideStringEnd);
  return str;
}

void ParseHttpVersion(
    const std::string& httpVersion,
    uint16_t* majorVersion,
    uint16_t* minorVersion)
{
  auto httpVersionEnd = httpVersion.end();

  // Set response code and HTTP version (i.e. HTTP/1.1)
  auto majorVersionStart
      = httpVersion.begin() + 5; // HTTP = 4, / = 1, moving to 5th place for version
  auto majorVersionEnd = std::find(majorVersionStart, httpVersionEnd, '.');
  auto majorVersionInt = std::stoi(std::string(majorVersionStart, majorVersionEnd));

  auto minorVersionStart = majorVersionEnd + 1; // start of minor version
  auto minorVersionInt = std::stoi(std::string(minorVersionStart, httpVersionEnd));

  *majorVersion = static_cast<uint16_t>(majorVersionInt);
  *minorVersion = static_cast<uint16_t>(minorVersionInt);
}

/**
 * @brief Add a list of HTTP headers to the #Azure::Core::Http::RawResponse.
 *
 * @remark The \p headers must contain valid header name characters (RFC 7230).
 * @remark Header name, value and delimiter are expected to be in \p headers.
 *
 * @param headers The complete list of headers to be added, in the form "name:value\0",
 * terminated by "\0".
 *
 * @throw if \p headers has an invalid header name or if the delimiter is missing.
 */
void SetHeaders(std::string const& headers, std::unique_ptr<RawResponse>& rawResponse)
{
  auto begin = headers.data();
  auto end = begin + headers.size();

  while (begin < end)
  {
    auto delimiter = std::find(begin, end, '\0');
    if (delimiter < end)
    {
      Azure::Core::Http::_detail::RawResponseHelpers::SetHeader(
          *rawResponse,
          reinterpret_cast<uint8_t const*>(begin),
          reinterpret_cast<uint8_t const*>(delimiter));
    }
    else
    {
      break;
    }
    begin = delimiter + 1;
  }
}

std::string GetHeadersAsString(Azure::Core::Http::Request const& request)
{
  std::string requestHeaderString;

  for (auto const& header : request.GetHeaders())
  {
    requestHeaderString += header.first; // string (key)
    requestHeaderString += ": ";
    requestHeaderString += header.second; // string's value
    requestHeaderString += "\r\n";
  }
  requestHeaderString += "\r\n";

  return requestHeaderString;
}
} // namespace

// For each certificate specified in trustedCertificate, add to certificateStore.
bool WinHttpTransport::AddCertificatesToStore(
    std::vector<std::string> const& trustedCertificates,
    HCERTSTORE certificateStore)
{
  for (auto const& trustedCertificate : trustedCertificates)
  {
    auto derCertificate = Azure::Core::Convert::Base64Decode(trustedCertificate);

    if (!CertAddEncodedCertificateToStore(
            certificateStore,
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            derCertificate.data(),
            static_cast<DWORD>(derCertificate.size()),
            CERT_STORE_ADD_NEW,
            NULL))
    {
      GetErrorAndThrow("CertAddEncodedCertificateToStore failed");
    }
  }
  return true;
}

// VerifyCertificateInChain determines whether the certificate in serverCertificate
// chains up to one of the certificates represented by trustedCertificate or not.
bool WinHttpTransport::VerifyCertificatesInChain(
    std::vector<std::string> const& trustedCertificates,
    PCCERT_CONTEXT serverCertificate)
{
  if ((trustedCertificates.empty()) || !serverCertificate)
  {
    return false;
  }

  // Creates an in-memory certificate store that is destroyed at end of this function.
  wil::unique_hcertstore certificateStore(CertOpenStore(
      CERT_STORE_PROV_MEMORY,
      X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
      0,
      CERT_STORE_CREATE_NEW_FLAG,
      nullptr));
  if (!certificateStore)
  {
    GetErrorAndThrow("CertOpenStore failed");
  }

  // Add the trusted certificates to that store.
  if (!AddCertificatesToStore(trustedCertificates, certificateStore.get()))
  {
    Log::Write(Logger::Level::Error, "Cannot add certificates to store");
    return false;
  }

  // WIL doesn't declare a convenient wrapper for a HCERTCHAINENGINE, so we define a custom one.
  wil::unique_any<
      HCERTCHAINENGINE,
      decltype(CertFreeCertificateChainEngine),
      CertFreeCertificateChainEngine>
      certificateChainEngine;
  {
    CERT_CHAIN_ENGINE_CONFIG EngineConfig{};
    EngineConfig.cbSize = sizeof(EngineConfig);
    EngineConfig.dwFlags = CERT_CHAIN_ENABLE_CACHE_AUTO_UPDATE | CERT_CHAIN_ENABLE_SHARE_STORE;
    EngineConfig.hExclusiveRoot = certificateStore.get();

    if (!CertCreateCertificateChainEngine(&EngineConfig, certificateChainEngine.addressof()))
    {
      GetErrorAndThrow("CertCreateCertificateChainEngine failed");
    }
  }

  // Generate a certificate chain using the local chain engine and the certificate store containing
  // the trusted certificates.
  wil::unique_cert_chain_context chainContextToVerify;
  {
    CERT_CHAIN_PARA ChainPara{};
    ChainPara.cbSize = sizeof(ChainPara);
    if (!CertGetCertificateChain(
            certificateChainEngine.get(),
            serverCertificate,
            nullptr,
            certificateStore.get(),
            &ChainPara,
            0,
            nullptr,
            chainContextToVerify.addressof()))
    {
      GetErrorAndThrow("CertGetCertificateChain failed");
    }
  }

  // And make sure that the certificate chain which was created matches the SSL chain.
  {
    CERT_CHAIN_POLICY_PARA PolicyPara{};
    PolicyPara.cbSize = sizeof(PolicyPara);

    CERT_CHAIN_POLICY_STATUS PolicyStatus{};
    PolicyStatus.cbSize = sizeof(PolicyStatus);

    if (!CertVerifyCertificateChainPolicy(
            CERT_CHAIN_POLICY_SSL, chainContextToVerify.get(), &PolicyPara, &PolicyStatus))
    {
      GetErrorAndThrow("CertVerifyCertificateChainPolicy");
    }
    if (PolicyStatus.dwError != 0)
    {
      Log::Write(
          Logger::Level::Error,
          "CertVerifyCertificateChainPolicy sets certificateStatus "
              + std::to_string(PolicyStatus.dwError));
      return false;
    }
  }
  return true;
}

/**
 * Called by WinHTTP when sending a request to the server. This callback allows us to inspect the
 * TLS certificate before sending it to the server.
 */
void WinHttpTransport::StatusCallback(
    HINTERNET hInternet,
    DWORD_PTR dwContext,
    DWORD dwInternetStatus,
    LPVOID,
    DWORD) noexcept
{
  // If we're called before our context has been set (on Open and Close callbacks), ignore the
  // status callback.
  if (dwContext == 0)
  {
    return;
  }

  try
  {
    WinHttpTransport* httpTransport = reinterpret_cast<WinHttpTransport*>(dwContext);
    httpTransport->OnHttpStatusOperation(hInternet, dwInternetStatus);
  }
  catch (Azure::Core::RequestFailedException& rfe)
  {
    // If an exception is thrown in the handler, log the error and terminate the connection.
    Log::Write(
        Logger::Level::Error,
        "Request Failed Exception Thrown: " + std::string(rfe.what()) + rfe.Message);
    WinHttpCloseHandle(hInternet);
  }
  catch (std::exception& ex)
  {
    // If an exception is thrown in the handler, log the error and terminate the connection.
    Log::Write(Logger::Level::Error, "Exception Thrown: " + std::string(ex.what()));
  }
}

/**
 * @brief HTTP Callback to enable private certificate checks.
 *
 * This method is called by WinHTTP when a certificate is received. This method is called multiple
 * times based on the state of the TLS connection. We are only interested in
 * WINHTTP_CALLBACK_STATUS_SENDING_REQUEST, which is called during the TLS handshake.
 *
 * When called, we verify that the certificate chain sent from the server contains the certificate
 * the HTTP client was configured with. If it is, we accept the connection, if it is not,
 * we abort the connection, closing the incoming request handle.
 */
void WinHttpTransport::OnHttpStatusOperation(HINTERNET hInternet, DWORD dwInternetStatus)
{
  if (dwInternetStatus != WINHTTP_CALLBACK_STATUS_SENDING_REQUEST)
  {
    if (dwInternetStatus == WINHTTP_CALLBACK_STATUS_SECURE_FAILURE)
    {
      Log::Write(Logger::Level::Error, "Security failure. :(");
    }
    // Silently ignore if there's any statuses we get that we can't handle
    return;
  }

  // We will only set the Status callback if a root certificate has been set.
  AZURE_ASSERT(!m_options.ExpectedTlsRootCertificates.empty());

  // Ask WinHTTP for the server certificate - this won't be valid outside a status callback.
  wil::unique_cert_context serverCertificate;
  {
    DWORD bufferLength = sizeof(PCCERT_CONTEXT);
    if (!WinHttpQueryOption(
            hInternet,
            WINHTTP_OPTION_SERVER_CERT_CONTEXT,
            reinterpret_cast<void*>(serverCertificate.addressof()),
            &bufferLength))
    {
      GetErrorAndThrow("Could not retrieve TLS server certificate.");
    }
  }

  if (!VerifyCertificatesInChain(m_options.ExpectedTlsRootCertificates, serverCertificate.get()))
  {
    Log::Write(Logger::Level::Error, "Server certificate is not trusted.  Aborting HTTP request");

    // To signal to caller that the request is to be terminated, the callback closes the handle.
    // This ensures that no message is sent to the server.
    WinHttpCloseHandle(hInternet);

    // To avoid a double free of this handle record that we've
    // already closed the handle.
    m_requestHandleClosed = true;
  }
}

void WinHttpTransport::GetErrorAndThrow(const std::string& exceptionMessage, DWORD error)
{
  std::string errorMessage = exceptionMessage + " Error Code: " + std::to_string(error);

  char* errorMsg = nullptr;
  if (FormatMessage(
          FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER,
          GetModuleHandle("winhttp.dll"),
          error,
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          reinterpret_cast<LPSTR>(&errorMsg),
          0,
          nullptr)
      != 0)
  {
    // Use a unique_ptr to manage the lifetime of errorMsg.
    std::unique_ptr<char, decltype(&LocalFree)> errorString(errorMsg, &LocalFree);
    errorMsg = nullptr;

    errorMessage += ": ";
    errorMessage += errorString.get();
  }
  errorMessage += '.';

  throw Azure::Core::Http::TransportException(errorMessage);
}

Azure::Core::_internal::UniqueHandle<HINTERNET> WinHttpTransport::CreateSessionHandle()
{
  // Use WinHttpOpen to obtain a session handle.
  // The dwFlags is set to 0 - all WinHTTP functions are performed synchronously.
  Azure::Core::_internal::UniqueHandle<HINTERNET> sessionHandle(WinHttpOpen(
      NULL, // Do not use a fallback user-agent string, and only rely on the header within the
            // request itself.
      // If the customer asks for it, enable use of the system default HTTP proxy.
      (m_options.EnableSystemDefaultProxy ? WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY
                                          : WINHTTP_ACCESS_TYPE_NO_PROXY),
      WINHTTP_NO_PROXY_NAME,
      WINHTTP_NO_PROXY_BYPASS,
      0));

  if (!sessionHandle)
  {
    // Errors include:
    // ERROR_WINHTTP_INTERNAL_ERROR
    // ERROR_NOT_ENOUGH_MEMORY
    GetErrorAndThrow("Error while getting a session handle.");
  }

// These options are only available starting from Windows 10 Version 2004, starting 06/09/2020.
// These are primarily round trip time (RTT) performance optimizations, and hence if they don't get
// set successfully, we shouldn't fail the request and continue as if the options don't exist.
// Therefore, we just ignore the error and move on.
#ifdef WINHTTP_OPTION_TCP_FAST_OPEN
  BOOL tcp_fast_open = TRUE;
  WinHttpSetOption(
      sessionHandle.get(), WINHTTP_OPTION_TCP_FAST_OPEN, &tcp_fast_open, sizeof(tcp_fast_open));
#endif

#ifdef WINHTTP_OPTION_TLS_FALSE_START
  BOOL tls_false_start = TRUE;
  WinHttpSetOption(
      sessionHandle.get(),
      WINHTTP_OPTION_TLS_FALSE_START,
      &tls_false_start,
      sizeof(tls_false_start));
#endif

  // Enforce TLS version 1.2
  auto tlsOption = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
  if (!WinHttpSetOption(
          sessionHandle.get(), WINHTTP_OPTION_SECURE_PROTOCOLS, &tlsOption, sizeof(tlsOption)))
  {
    GetErrorAndThrow("Error while enforcing TLS 1.2 for connection request.");
  }

  if (!m_options.ExpectedTlsRootCertificates.empty())
  {

    // Set the callback function to be called when a server certificate is received.
    if (WinHttpSetStatusCallback(
            sessionHandle.get(),
            &WinHttpTransport::StatusCallback,
            WINHTTP_CALLBACK_FLAG_SEND_REQUEST /* WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS*/,
            0)
        == WINHTTP_INVALID_STATUS_CALLBACK)
    {
      GetErrorAndThrow("Error while setting up the status callback.");
    }
  }

  return sessionHandle;
}

namespace {
WinHttpTransportOptions WinHttpTransportOptionsFromTransportOptions(
    Azure::Core::Http::Policies::TransportOptions const& transportOptions)
{
  WinHttpTransportOptions httpOptions;
  if (transportOptions.HttpProxy.HasValue())
  {
    // WinHTTP proxy strings are semicolon separated elements, each of which
    // has the following format:
    //  ([<scheme>=][<scheme>"://"]<server>[":"<port>])
    std::string proxyString;
    proxyString = "http=" + transportOptions.HttpProxy.Value();
    proxyString += ";";
    proxyString += "https=" + transportOptions.HttpProxy.Value();
    httpOptions.ProxyInformation = proxyString;
  }
  httpOptions.ProxyUserName = transportOptions.ProxyUserName;
  httpOptions.ProxyPassword = transportOptions.ProxyPassword;
  // Note that WinHTTP accepts a set of root certificates, even though transportOptions only
  // specifies a single one.
  if (!transportOptions.ExpectedTlsRootCertificate.empty())
  {
    httpOptions.ExpectedTlsRootCertificates.push_back(transportOptions.ExpectedTlsRootCertificate);
  }
  if (transportOptions.EnableCertificateRevocationListCheck)
  {
    httpOptions.EnableCertificateRevocationListCheck;
  }
  // If you specify an expected TLS root certificate, you also need to enable ignoring unknown
  // CAs.
  if (!transportOptions.ExpectedTlsRootCertificate.empty())
  {
    httpOptions.IgnoreUnknownCertificateAuthority;
  }

  return httpOptions;
}
} // namespace

WinHttpTransport::WinHttpTransport(WinHttpTransportOptions const& options)
    : m_options(options), m_sessionHandle(CreateSessionHandle())
{
}

WinHttpTransport::WinHttpTransport(
    Azure::Core::Http::Policies::TransportOptions const& transportOptions)
    : WinHttpTransport(WinHttpTransportOptionsFromTransportOptions(transportOptions))
{
}

Azure::Core::_internal::UniqueHandle<HINTERNET> WinHttpTransport::CreateConnectionHandle(
    Azure::Core::Url const& url,
    Azure::Core::Context const& context)
{
  // If port is 0, i.e. INTERNET_DEFAULT_PORT, it uses port 80 for HTTP and port 443 for HTTPS.
  uint16_t port = url.GetPort();

  context.ThrowIfCancelled();

  // Specify an HTTP server.
  // This function always operates synchronously.
  Azure::Core::_internal::UniqueHandle<HINTERNET> rv(WinHttpConnect(
      m_sessionHandle.get(),
      StringToWideString(url.GetHost()).c_str(),
      port == 0 ? INTERNET_DEFAULT_PORT : port,
      0));

  if (!rv)
  {
    // Errors include:
    // ERROR_WINHTTP_INCORRECT_HANDLE_TYPE
    // ERROR_WINHTTP_INTERNAL_ERROR
    // ERROR_WINHTTP_INVALID_URL
    // ERROR_WINHTTP_OPERATION_CANCELLED
    // ERROR_WINHTTP_UNRECOGNIZED_SCHEME
    // ERROR_WINHTTP_SHUTDOWN
    // ERROR_NOT_ENOUGH_MEMORY
    GetErrorAndThrow("Error while getting a connection handle.");
  }
  return rv;
}

Azure::Core::_internal::UniqueHandle<HINTERNET> WinHttpTransport::CreateRequestHandle(
    Azure::Core::_internal::UniqueHandle<HINTERNET> const& connectionHandle,
    Azure::Core::Url const& url,
    Azure::Core::Http::HttpMethod const& method)
{
  const std::string& path = url.GetRelativeUrl();
  HttpMethod requestMethod = method;
  bool const requestSecureHttp(
      !Azure::Core::_internal::StringExtensions::LocaleInvariantCaseInsensitiveEqual(
          url.GetScheme(), HttpScheme)
      && !Azure::Core::_internal::StringExtensions::LocaleInvariantCaseInsensitiveEqual(
          url.GetScheme(), WebSocketScheme));

  // Create an HTTP request handle.
  Azure::Core::_internal::UniqueHandle<HINTERNET> request(WinHttpOpenRequest(
      connectionHandle.get(),
      HttpMethodToWideString(requestMethod).c_str(),
      path.empty() ? NULL : StringToWideString(path).c_str(), // Name of the target resource of
                                                              // the specified HTTP verb
      NULL, // Use HTTP/1.1
      WINHTTP_NO_REFERER,
      WINHTTP_DEFAULT_ACCEPT_TYPES, // No media types are accepted by the client
      requestSecureHttp ? WINHTTP_FLAG_SECURE : 0)); // Uses secure transaction semantics (SSL/TLS)
  if (!request)
  {
    // Errors include:
    // ERROR_WINHTTP_INCORRECT_HANDLE_TYPE
    // ERROR_WINHTTP_INTERNAL_ERROR
    // ERROR_WINHTTP_INVALID_URL
    // ERROR_WINHTTP_OPERATION_CANCELLED
    // ERROR_WINHTTP_UNRECOGNIZED_SCHEME
    // ERROR_NOT_ENOUGH_MEMORY
    GetErrorAndThrow("Error while getting a request handle.");
  }

  if (requestSecureHttp)
  {
    // If the service requests TLS client certificates, we want to let the WinHTTP APIs know that
    // it's ok to initiate the request without a client certificate.
    //
    // Note: If/When TLS client certificate support is added to the pipeline, this line may need to
    // be revisited.
    if (!WinHttpSetOption(
            request.get(), WINHTTP_OPTION_CLIENT_CERT_CONTEXT, WINHTTP_NO_CLIENT_CERT_CONTEXT, 0))
    {
      GetErrorAndThrow("Error while setting client cert context to ignore.");
    }
  }

  if (!m_options.ProxyInformation.empty())
  {
    WINHTTP_PROXY_INFO proxyInfo{};
    std::wstring proxyWide{StringToWideString(m_options.ProxyInformation)};
    proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
    proxyInfo.lpszProxy = const_cast<LPWSTR>(proxyWide.c_str());
    proxyInfo.lpszProxyBypass = WINHTTP_NO_PROXY_BYPASS;
    if (!WinHttpSetOption(request.get(), WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo)))
    {
      GetErrorAndThrow("Error while setting Proxy information.");
    }
  }
  if (m_options.ProxyUserName.HasValue() || m_options.ProxyPassword.HasValue())
  {
    if (!WinHttpSetCredentials(
            request.get(),
            WINHTTP_AUTH_TARGET_PROXY,
            WINHTTP_AUTH_SCHEME_BASIC,
            StringToWideString(m_options.ProxyUserName.Value()).c_str(),
            StringToWideString(m_options.ProxyPassword.Value()).c_str(),
            0))
    {
      GetErrorAndThrow("Error while setting Proxy credentials.");
    }
  }

  if (m_options.IgnoreUnknownCertificateAuthority || !m_options.ExpectedTlsRootCertificates.empty())
  {
    auto option = SECURITY_FLAG_IGNORE_UNKNOWN_CA;
    if (!WinHttpSetOption(request.get(), WINHTTP_OPTION_SECURITY_FLAGS, &option, sizeof(option)))
    {
      GetErrorAndThrow("Error while setting ignore unknown server certificate.");
    }
  }

  if (m_options.EnableCertificateRevocationListCheck)
  {
    DWORD value = WINHTTP_ENABLE_SSL_REVOCATION;
    if (!WinHttpSetOption(request.get(), WINHTTP_OPTION_ENABLE_FEATURE, &value, sizeof(value)))
    {
      GetErrorAndThrow("Error while enabling CRL validation.");
    }
  }

  // If we are supporting WebSockets, then let WinHTTP know that it should
  // prepare to upgrade the HttpRequest to a WebSocket.
#pragma warning(push)
// warning C6387: _Param_(3) could be '0'.
#pragma warning(disable : 6387)
  if (HasWebSocketSupport()
      && !WinHttpSetOption(request.get(), WINHTTP_OPTION_UPGRADE_TO_WEB_SOCKET, nullptr, 0))
#pragma warning(pop)
  {
    GetErrorAndThrow("Error while Enabling WebSocket upgrade.");
  }
  return request;
}

// For PUT/POST requests, send additional data using WinHttpWriteData.
void WinHttpTransport::Upload(
    Azure::Core::_internal::UniqueHandle<HINTERNET> const& requestHandle,
    Azure::Core::Http::Request& request,
    Azure::Core::Context const& context)
{
  auto streamBody = request.GetBodyStream();
  int64_t streamLength = streamBody->Length();

  // Consider using `MaximumUploadChunkSize` here, after some perf measurements
  size_t uploadChunkSize = _detail::DefaultUploadChunkSize;
  if (streamLength < _detail::MaximumUploadChunkSize)
  {
    uploadChunkSize = static_cast<size_t>(streamLength);
  }
  auto unique_buffer = std::make_unique<uint8_t[]>(uploadChunkSize);

  while (true)
  {
    size_t rawRequestLen = streamBody->Read(unique_buffer.get(), uploadChunkSize, context);
    if (rawRequestLen == 0)
    {
      break;
    }

    DWORD dwBytesWritten = 0;

    context.ThrowIfCancelled();

    // Write data to the server.
    if (!WinHttpWriteData(
            requestHandle.get(),
            unique_buffer.get(),
            static_cast<DWORD>(rawRequestLen),
            &dwBytesWritten))
    {
      GetErrorAndThrow("Error while uploading/sending data.");
    }
  }
}

void WinHttpTransport::SendRequest(
    Azure::Core::_internal::UniqueHandle<HINTERNET> const& requestHandle,
    Azure::Core::Http::Request& request,
    Azure::Core::Context const& context)
{
  std::wstring encodedHeaders;
  int encodedHeadersLength = 0;

  auto requestHeaders = request.GetHeaders();
  if (requestHeaders.size() != 0)
  {
    // The encodedHeaders will be null-terminated and the length is calculated.
    encodedHeadersLength = -1;
    std::string requestHeaderString = GetHeadersAsString(request);
    requestHeaderString.append("\0");

    encodedHeaders = StringToWideString(requestHeaderString);
  }

  int64_t streamLength = request.GetBodyStream()->Length();

  context.ThrowIfCancelled();

  // Send a request.
  if (!WinHttpSendRequest(
          requestHandle.get(),
          requestHeaders.size() == 0 ? WINHTTP_NO_ADDITIONAL_HEADERS : encodedHeaders.c_str(),
          encodedHeadersLength,
          WINHTTP_NO_REQUEST_DATA,
          0,
          streamLength > 0 ? static_cast<DWORD>(streamLength) : 0,
          reinterpret_cast<DWORD_PTR>(this)))
  {
    // Errors include:
    // ERROR_WINHTTP_CANNOT_CONNECT
    // ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED
    // ERROR_WINHTTP_CONNECTION_ERROR
    // ERROR_WINHTTP_INCORRECT_HANDLE_STATE
    // ERROR_WINHTTP_INCORRECT_HANDLE_TYPE
    // ERROR_WINHTTP_INTERNAL_ERROR
    // ERROR_WINHTTP_INVALID_URL
    // ERROR_WINHTTP_LOGIN_FAILURE
    // ERROR_WINHTTP_NAME_NOT_RESOLVED
    // ERROR_WINHTTP_OPERATION_CANCELLED
    // ERROR_WINHTTP_RESPONSE_DRAIN_OVERFLOW
    // ERROR_WINHTTP_SECURE_FAILURE
    // ERROR_WINHTTP_SHUTDOWN
    // ERROR_WINHTTP_TIMEOUT
    // ERROR_WINHTTP_UNRECOGNIZED_SCHEME
    // ERROR_NOT_ENOUGH_MEMORY
    // ERROR_INVALID_PARAMETER
    // ERROR_WINHTTP_RESEND_REQUEST
    GetErrorAndThrow("Error while sending a request.");
  }

  // Chunked transfer encoding is not supported and the content length needs to be known up front.
  if (streamLength == -1)
  {
    throw Azure::Core::Http::TransportException(
        "When uploading data, the body stream must have a known length.");
  }

  if (streamLength > 0)
  {
    Upload(requestHandle, request, context);
  }
}

void WinHttpTransport::ReceiveResponse(
    Azure::Core::_internal::UniqueHandle<HINTERNET> const& requestHandle,
    Azure::Core::Context const& context)
{
  context.ThrowIfCancelled();

  // Wait to receive the response to the HTTP request initiated by WinHttpSendRequest.
  // When WinHttpReceiveResponse completes successfully, the status code and response headers have
  // been received.
  if (!WinHttpReceiveResponse(requestHandle.get(), NULL))
  {
    // Errors include:
    // ERROR_WINHTTP_CANNOT_CONNECT
    // ERROR_WINHTTP_CHUNKED_ENCODING_HEADER_SIZE_OVERFLOW
    // ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED
    // ...
    // ERROR_WINHTTP_TIMEOUT
    // ERROR_WINHTTP_UNRECOGNIZED_SCHEME
    // ERROR_NOT_ENOUGH_MEMORY
    GetErrorAndThrow("Error while receiving a response.");
  }
}

int64_t WinHttpTransport::GetContentLength(
    Azure::Core::_internal::UniqueHandle<HINTERNET> const& requestHandle,
    HttpMethod requestMethod,
    HttpStatusCode responseStatusCode)
{
  DWORD dwContentLength = 0;
  DWORD dwSize = sizeof(dwContentLength);

  // For Head request, set the length of body response to 0.
  // Response will give us content-length as if we were not doing Head saying what would be the
  // length of the body. However, server won't send any body.
  // For NoContent status code, also need to set contentLength to 0.
  int64_t contentLength = 0;

  // Get the content length as a number.
  if (requestMethod != HttpMethod::Head && responseStatusCode != HttpStatusCode::NoContent)
  {
    if (!WinHttpQueryHeaders(
            requestHandle.get(),
            WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX,
            &dwContentLength,
            &dwSize,
            WINHTTP_NO_HEADER_INDEX))
    {
      contentLength = -1;
    }
    else
    {
      contentLength = static_cast<int64_t>(dwContentLength);
    }
  }

  return contentLength;
}

std::unique_ptr<RawResponse> WinHttpTransport::SendRequestAndGetResponse(
    Azure::Core::_internal::UniqueHandle<HINTERNET>& requestHandle,
    HttpMethod requestMethod)
{
  // First, use WinHttpQueryHeaders to obtain the size of the buffer.
  // The call is expected to fail since no destination buffer is provided.
  DWORD sizeOfHeaders = 0;
  if (WinHttpQueryHeaders(
          requestHandle.get(),
          WINHTTP_QUERY_RAW_HEADERS,
          WINHTTP_HEADER_NAME_BY_INDEX,
          NULL,
          &sizeOfHeaders,
          WINHTTP_NO_HEADER_INDEX))
  {
    // WinHttpQueryHeaders was expected to fail.
    throw Azure::Core::Http::TransportException("Error while querying response headers.");
  }

  {
    DWORD error = GetLastError();
    if (error != ERROR_INSUFFICIENT_BUFFER)
    {
      GetErrorAndThrow("Error while querying response headers.", error);
    }
  }

  // Allocate memory for the buffer.
  std::vector<WCHAR> outputBuffer(sizeOfHeaders / sizeof(WCHAR), 0);

  // Now, use WinHttpQueryHeaders to retrieve all the headers.
  // Each header is terminated by "\0". An additional "\0" terminates the list of headers.
  if (!WinHttpQueryHeaders(
          requestHandle.get(),
          WINHTTP_QUERY_RAW_HEADERS,
          WINHTTP_HEADER_NAME_BY_INDEX,
          outputBuffer.data(),
          &sizeOfHeaders,
          WINHTTP_NO_HEADER_INDEX))
  {
    GetErrorAndThrow("Error while querying response headers.");
  }

  auto start = outputBuffer.begin();
  auto last = start + sizeOfHeaders / sizeof(WCHAR);
  auto statusLineEnd = std::find(start, last, '\0');
  start = statusLineEnd + 1; // start of headers
  std::string responseHeaders = WideStringToString(std::wstring(start, last));

  DWORD sizeOfHttp = sizeOfHeaders;

  // Get the HTTP version.
  if (!WinHttpQueryHeaders(
          requestHandle.get(),
          WINHTTP_QUERY_VERSION,
          WINHTTP_HEADER_NAME_BY_INDEX,
          outputBuffer.data(),
          &sizeOfHttp,
          WINHTTP_NO_HEADER_INDEX))
  {
    GetErrorAndThrow("Error while querying response headers.");
  }

  start = outputBuffer.begin();
  // Assuming ASCII here is OK since the input is expected to be an HTTP version string.
  std::string httpVersion = WideStringToStringASCII(start, start + sizeOfHttp / sizeof(WCHAR));

  uint16_t majorVersion = 0;
  uint16_t minorVersion = 0;
  ParseHttpVersion(httpVersion, &majorVersion, &minorVersion);

  DWORD statusCode = 0;
  DWORD dwSize = sizeof(statusCode);

  // Get the status code as a number.
  if (!WinHttpQueryHeaders(
          requestHandle.get(),
          WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
          WINHTTP_HEADER_NAME_BY_INDEX,
          &statusCode,
          &dwSize,
          WINHTTP_NO_HEADER_INDEX))
  {
    GetErrorAndThrow("Error while querying response headers.");
  }

  HttpStatusCode httpStatusCode = static_cast<HttpStatusCode>(statusCode);

  // Get the optional reason phrase.
  std::string reasonPhrase;
  DWORD sizeOfReasonPhrase = sizeOfHeaders;

  // HTTP/2 does not support reason phrase, refer to
  // https://www.rfc-editor.org/rfc/rfc7540#section-8.1.2.4.
  if (majorVersion == 1)
  {
    if (WinHttpQueryHeaders(
            requestHandle.get(),
            WINHTTP_QUERY_STATUS_TEXT,
            WINHTTP_HEADER_NAME_BY_INDEX,
            outputBuffer.data(),
            &sizeOfReasonPhrase,
            WINHTTP_NO_HEADER_INDEX))
    {
      // even with HTTP/1.1, we cannot assume that reason phrase is set since it is optional
      // according to https://www.rfc-editor.org/rfc/rfc2616.html#section-6.1.1.
      if (sizeOfReasonPhrase > 0)
      {
        start = outputBuffer.begin();
        reasonPhrase
            = WideStringToString(std::wstring(start, start + sizeOfReasonPhrase / sizeof(WCHAR)));
      }
    }
  }

  // Allocate the instance of the response on the heap with a shared ptr so this memory gets
  // delegated outside the transport and will be eventually released.
  auto rawResponse
      = std::make_unique<RawResponse>(majorVersion, minorVersion, httpStatusCode, reasonPhrase);

  SetHeaders(responseHeaders, rawResponse);

  if (HasWebSocketSupport() && (httpStatusCode == HttpStatusCode::SwitchingProtocols))
  {
    OnUpgradedConnection(requestHandle);
  }
  else
  {
    int64_t contentLength
        = GetContentLength(requestHandle, requestMethod, rawResponse->GetStatusCode());

    rawResponse->SetBodyStream(
        std::make_unique<_detail::WinHttpStream>(requestHandle, contentLength));
  }
  return rawResponse;
}

std::unique_ptr<RawResponse> WinHttpTransport::Send(Request& request, Context const& context)
{
  Azure::Core::_internal::UniqueHandle<HINTERNET> connectionHandle
      = CreateConnectionHandle(request.GetUrl(), context);
  Azure::Core::_internal::UniqueHandle<HINTERNET> requestHandle
      = CreateRequestHandle(connectionHandle, request.GetUrl(), request.GetMethod());
  try
  {
    SendRequest(requestHandle, request, context);
  }
  catch (TransportException&)
  {
    // If there was a TLS validation error, then we will have closed the request handle
    // during the TLS validation callback. So if an exception was thrown, if we force closed the
    // request handle, clear the handle in the requestHandle to prevent a double free.
    if (m_requestHandleClosed)
    {
      requestHandle.release();
    }

    throw;
  }

  ReceiveResponse(requestHandle, context);

  return SendRequestAndGetResponse(requestHandle, request.GetMethod());
}

// Read the response from the sent request.
size_t _detail::WinHttpStream::OnRead(uint8_t* buffer, size_t count, Context const& context)
{
  if (count == 0 || this->m_isEOF)
  {
    return 0;
  }

  // No need to check for context cancellation before the first I/O because the base class
  // BodyStream::Read already does that.
  (void)context;

  DWORD numberOfBytesRead = 0;

  if (!WinHttpReadData(
          this->m_requestHandle.get(),
          (LPVOID)(buffer),
          static_cast<DWORD>(count),
          &numberOfBytesRead))
  {
    // Errors include:
    // ERROR_WINHTTP_CONNECTION_ERROR
    // ERROR_WINHTTP_INCORRECT_HANDLE_STATE
    // ERROR_WINHTTP_INCORRECT_HANDLE_TYPE
    // ERROR_WINHTTP_INTERNAL_ERROR
    // ERROR_WINHTTP_OPERATION_CANCELLED
    // ERROR_WINHTTP_RESPONSE_DRAIN_OVERFLOW
    // ERROR_WINHTTP_TIMEOUT
    // ERROR_NOT_ENOUGH_MEMORY

    DWORD error = GetLastError();
    throw Azure::Core::Http::TransportException(
        "Error while reading available data from the wire. Error Code: " + std::to_string(error)
        + ".");
  }

  this->m_streamTotalRead += numberOfBytesRead;

  if (numberOfBytesRead == 0
      || (this->m_contentLength != -1 && this->m_streamTotalRead == this->m_contentLength))
  {
    this->m_isEOF = true;
  }
  return numberOfBytesRead;
}
