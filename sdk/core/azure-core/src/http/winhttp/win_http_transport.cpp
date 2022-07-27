// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/http.hpp"

#include "azure/core/internal/strings.hpp"

#if defined(BUILD_TRANSPORT_WINHTTP_ADAPTER)
#include "azure/core/http/win_http_transport.hpp"
#endif

#include <Windows.h>
#include <algorithm>
#include <string>
#include <winhttp.h>

using Azure::Core::Context;
using namespace Azure::Core::Http;

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
    throw Azure::Core::Http::WinHttpTransportException(
        error,
        "Unable to get the required transcoded size for the input string. Error Code: "
            + std::to_string(error) + ".");
  }

  std::wstring wideStr(sizeNeeded, L'\0');
  if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, str.c_str(), -1, &wideStr[0], sizeNeeded)
      == 0)
  {
    DWORD error = GetLastError();
    throw Azure::Core::Http::WinHttpTransportException(
        error,
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
    throw Azure::Core::Http::WinHttpTransportException(
        error,
        "Unable to get the required transcoded size for the input wide string. Error Code: "
            + std::to_string(error) + ".");
  }

  std::string str(sizeNeeded, 0);
  if (WideCharToMultiByte(
          CP_UTF8, 0, wideString.c_str(), wideStrLength, &str[0], sizeNeeded, NULL, NULL)
      == 0)
  {
    DWORD error = GetLastError();
    throw Azure::Core::Http::WinHttpTransportException(
        error,
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

  throw Azure::Core::Http::WinHttpTransportException(error, errorMessage);
}

_detail::unique_HINTERNET WinHttpTransport::CreateSessionHandle()
{
  // Use WinHttpOpen to obtain a session handle.
  // The dwFlags is set to 0 - all WinHTTP functions are performed synchronously.
  _detail::unique_HINTERNET sessionHandle(
      WinHttpOpen(
          NULL, // Do not use a fallback user-agent string, and only rely on the header within the
                // request itself.
          // If the customer asks for it, enable use of the system default HTTP proxy.
          (m_options.EnableSystemDefaultProxy ? WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY
                                              : WINHTTP_ACCESS_TYPE_NO_PROXY),
          WINHTTP_NO_PROXY_NAME,
          WINHTTP_NO_PROXY_BYPASS,
          0),
      _detail::HINTERNET_deleter{});

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

  return sessionHandle;
}

WinHttpTransport::WinHttpTransport(WinHttpTransportOptions const& options)
    : m_options(options), m_sessionHandle(CreateSessionHandle())
{
}

_detail::unique_HINTERNET WinHttpTransport::CreateConnectionHandle(
    Azure::Core::Url const& url,
    Azure::Core::Context const& context)
{
  // If port is 0, i.e. INTERNET_DEFAULT_PORT, it uses port 80 for HTTP and port 443 for HTTPS.
  uint16_t port = url.GetPort();

  context.ThrowIfCancelled();

  // Specify an HTTP server.
  // This function always operates synchronously.
  _detail::unique_HINTERNET rv(
      WinHttpConnect(
          m_sessionHandle.get(),
          StringToWideString(url.GetHost()).c_str(),
          port == 0 ? INTERNET_DEFAULT_PORT : port,
          0),
      _detail::HINTERNET_deleter{});

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

_detail::unique_HINTERNET WinHttpTransport::CreateRequestHandle(
    _detail::unique_HINTERNET const& connectionHandle,
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
  _detail::unique_HINTERNET request(
      WinHttpOpenRequest(
          connectionHandle.get(),
          HttpMethodToWideString(requestMethod).c_str(),
          path.empty() ? NULL : StringToWideString(path).c_str(), // Name of the target resource of
                                                                  // the specified HTTP verb
          NULL, // Use HTTP/1.1
          WINHTTP_NO_REFERER,
          WINHTTP_DEFAULT_ACCEPT_TYPES, // No media types are accepted by the client
          requestSecureHttp ? WINHTTP_FLAG_SECURE : 0),
      _detail::HINTERNET_deleter{}); // Uses secure transaction semantics (SSL/TLS)
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
#if 0
  if (!m_options.ProxyUserName.empty() || !m_options.ProxyPassword.empty())
  {
    DWORD proxyAuthSchemes = 0;
    DWORD proxyAuthSchemeSize = sizeof(proxyAuthSchemes);
    if (!WinHttpQueryOption(
            request.get(),
            WINHTTP_OPTION_DISABLE_PROXY_AUTH_SCHEMES,
            &proxyAuthSchemes,
            &proxyAuthSchemeSize))
    {
      GetErrorAndThrow("Error while getting disabled proxy auth schemes.");
    }
    proxyAuthSchemes &= ~WINHTTP_PROXY_DISABLE_SCHEME_BASIC;
    if (!WinHttpSetOption(
            request.get(),
            WINHTTP_OPTION_DISABLE_PROXY_AUTH_SCHEMES,
            &proxyAuthSchemes,
            proxyAuthSchemeSize))
    {
      GetErrorAndThrow("Error while setting disabled proxy auth schemes.");
    }
  }

  if (!m_options.ProxyUserName.empty())
  {
    std::wstring userNameWide{StringToWideString(m_options.ProxyUserName)};
    if (!WinHttpSetOption(
            request.get(),
            WINHTTP_OPTION_PROXY_USERNAME,
            reinterpret_cast<LPVOID>(const_cast<wchar_t*>(userNameWide.c_str())),
            static_cast<DWORD>(m_options.ProxyUserName.size())))
    {
      GetErrorAndThrow("Error while setting Proxy UserName information.");
    }
  }

  if (!m_options.ProxyPassword.empty())
  {
    std::wstring passwordWide{StringToWideString(m_options.ProxyPassword)};
    if (!WinHttpSetOption(
            request.get(),
            WINHTTP_OPTION_PROXY_PASSWORD,
            reinterpret_cast<LPVOID>(const_cast<wchar_t*>(passwordWide.c_str())),
            static_cast<DWORD>(m_options.ProxyPassword.size())))
    {
      GetErrorAndThrow("Error while setting Proxy Password information.");
    }
  }
#endif
  if (!m_options.ProxyUserName.empty() || !m_options.ProxyPassword.empty())
  {
    if (!WinHttpSetCredentials(
            request.get(),
            WINHTTP_AUTH_TARGET_PROXY,
            WINHTTP_AUTH_SCHEME_BASIC,
            StringToWideString(m_options.ProxyUserName).c_str(),
            StringToWideString(m_options.ProxyPassword).c_str(),
            0))
    {
      GetErrorAndThrow("Error while setting Proxy credentials.");
    }
  }

  if (m_options.IgnoreUnknownCertificateAuthority)
  {
    auto option = SECURITY_FLAG_IGNORE_UNKNOWN_CA;
    if (!WinHttpSetOption(request.get(), WINHTTP_OPTION_SECURITY_FLAGS, &option, sizeof(option)))
    {
      GetErrorAndThrow("Error while setting ignore unknown server certificate.");
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
    _detail::unique_HINTERNET const& requestHandle,
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
    _detail::unique_HINTERNET const& requestHandle,
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
          0))
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
    _detail::unique_HINTERNET const& requestHandle,
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
    _detail::unique_HINTERNET const& requestHandle,
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
    _detail::unique_HINTERNET& requestHandle,
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

  if (WinHttpQueryHeaders(
          requestHandle.get(),
          WINHTTP_QUERY_STATUS_TEXT,
          WINHTTP_HEADER_NAME_BY_INDEX,
          outputBuffer.data(),
          &sizeOfReasonPhrase,
          WINHTTP_NO_HEADER_INDEX))
  {
    start = outputBuffer.begin();
    reasonPhrase
        = WideStringToString(std::wstring(start, start + sizeOfReasonPhrase / sizeof(WCHAR)));
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
  _detail::unique_HINTERNET connectionHandle = CreateConnectionHandle(request.GetUrl(), context);
  _detail::unique_HINTERNET requestHandle
      = CreateRequestHandle(connectionHandle, request.GetUrl(), request.GetMethod());

  SendRequest(requestHandle, request, context);

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
    throw Azure::Core::Http::WinHttpTransportException(
        error,
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
