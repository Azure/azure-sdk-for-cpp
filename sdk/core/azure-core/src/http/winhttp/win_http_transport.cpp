// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/http.hpp"
#include "azure/core/http/winhttp/win_http_client.hpp"

#include <Windows.h>
#include <algorithm>
#include <string>
#include <winhttp.h>

using namespace Azure::Core::Http;

namespace {

inline std::wstring HttpMethodToWideString(HttpMethod method)
{
  // This string should be all uppercase.
  // Many servers treat HTTP verbs as case-sensitive, and the Internet Engineering Task Force (IETF)
  // Requests for Comments (RFCs) spell these verbs using uppercase characters only.
  switch (method)
  {
    case HttpMethod::Get:
      return L"GET";
    case HttpMethod::Head:
      return L"HEAD";
    case HttpMethod::Post:
      return L"POST";
    case HttpMethod::Put:
      return L"PUT";
    case HttpMethod::Delete:
      return L"DELETE";
    case HttpMethod::Patch:
      return L"PATCH";
    default:
      throw Azure::Core::Http::TransportException("Invalid or unsupported HTTP method.");
  }
}

// Convert a UTF-8 string to a wide Unicode string.
std::wstring StringToWideString(const std::string& str)
{
  size_t stringSize = str.size();
  if (stringSize > INT_MAX)
  {
    throw Azure::Core::Http::TransportException(
        "Input string is too large to fit within a 32-bit int.");
  }

  int strLength = static_cast<int>(stringSize);
  int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), strLength, 0, 0);
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
  if (MultiByteToWideChar(CP_UTF8, 0, str.c_str(), strLength, &wideStr[0], sizeNeeded) == 0)
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
  size_t wideStrSize = wideString.size();
  if (wideStrSize > INT_MAX)
  {
    throw Azure::Core::Http::TransportException(
        "Input wide string is too large to fit within a 32-bit int.");
  }

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
    wchar_t const* const wideStringStart,
    wchar_t const* const wideStringEnd)
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
  auto httpVersionEnd = httpVersion.data() + httpVersion.size();

  // Set response code and HTTP version (i.e. HTTP/1.1)
  auto majorVersionStart
      = httpVersion.data() + 5; // HTTP = 4, / = 1, moving to 5th place for version
  auto majorVersionEnd = std::find(majorVersionStart, httpVersionEnd, '.');
  auto majorVersionInt = std::stoi(std::string(majorVersionStart, majorVersionEnd));

  auto minorVersionStart = majorVersionEnd + 1; // start of minor version
  auto minorVersionInt = std::stoi(std::string(minorVersionStart, httpVersionEnd));

  *majorVersion = (uint16_t)majorVersionInt;
  *minorVersion = (uint16_t)minorVersionInt;
}

} // namespace

void GetErrorAndThrow(const std::string& exceptionMessage)
{
  DWORD error = GetLastError();
  throw Azure::Core::Http::TransportException(
      exceptionMessage + " Error Code: " + std::to_string(error) + ".");
}

void WinHttpTransport::GetSessionHandle(std::unique_ptr<Details::HandleManager>& handleManager)
{
  // Use WinHttpOpen to obtain a session handle.
  // The dwFlags is set to 0 - all WinHTTP functions are performed synchronously.
  // TODO: Use specific user-agent or application name.
  handleManager->m_sessionHandle = WinHttpOpen(
      L"WinHTTP Azure SDK",
      WINHTTP_ACCESS_TYPE_NO_PROXY,
      WINHTTP_NO_PROXY_NAME,
      WINHTTP_NO_PROXY_BYPASS,
      0);

  if (!handleManager->m_sessionHandle)
  {
    // Errors include:
    // ERROR_WINHTTP_INTERNAL_ERROR
    // ERROR_NOT_ENOUGH_MEMORY
    GetErrorAndThrow("Error while getting a session handle.");
  }
}

void WinHttpTransport::GetConnectionHandle(std::unique_ptr<Details::HandleManager>& handleManager)
{
  // TODO: Get port from Url
  // Specify an HTTP server.
  // Uses port 80 for HTTP and port 443 for HTTPS.
  // This function always operates synchronously.
  handleManager->m_connectionHandle = WinHttpConnect(
      handleManager->m_sessionHandle,
      StringToWideString(handleManager->m_request.GetUrl().GetHost()).c_str(),
      INTERNET_DEFAULT_PORT,
      0);

  if (!handleManager->m_connectionHandle)
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
}

void WinHttpTransport::GetRequestHandle(std::unique_ptr<Details::HandleManager>& handleManager)
{
  const std::string& path = handleManager->m_request.GetUrl().GetRelativeUrl();
  HttpMethod requestMethod = handleManager->m_request.GetMethod();

  // Create an HTTP request handle.
  handleManager->m_requestHandle = WinHttpOpenRequest(
      handleManager->m_connectionHandle,
      HttpMethodToWideString(requestMethod).c_str(),
      path.empty() ? NULL
                   : StringToWideString(path)
                         .c_str(), // Name of the target resource of the specified HTTP verb
      NULL, // Use HTTP/1.1
      WINHTTP_NO_REFERER,
      WINHTTP_DEFAULT_ACCEPT_TYPES, // No media types are accepted by the client
      WINHTTP_FLAG_SECURE); // Uses secure transaction semantics (SSL/TLS)

  if (!handleManager->m_requestHandle)
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
}

void WinHttpTransport::Upload(std::unique_ptr<Details::HandleManager>& handleManager)
{
  auto streamBody = handleManager->m_request.GetBodyStream();
  int64_t streamLength = streamBody->Length();

  int64_t uploadChunkSize = handleManager->m_request.GetUploadChunkSize();
  if (uploadChunkSize <= 0)
  {
    // use default size
    if (streamLength < Details::MaximumUploadChunkSize)
    {
      uploadChunkSize = streamLength;
    }
    else
    {
      uploadChunkSize = Details::DefaultUploadChunkSize;
    }
  }

  auto unique_buffer = std::make_unique<uint8_t[]>(static_cast<size_t>(uploadChunkSize));

  while (true)
  {
    auto rawRequestLen
        = streamBody->Read(handleManager->m_context, unique_buffer.get(), uploadChunkSize);
    if (rawRequestLen == 0)
    {
      break;
    }

    DWORD dwBytesWritten = 0;

    // Write data to the server.
    if (!WinHttpWriteData(
            handleManager->m_requestHandle,
            unique_buffer.get(),
            static_cast<DWORD>(rawRequestLen),
            &dwBytesWritten))
    {
      GetErrorAndThrow("Error while writing data.");
    }
  }
}

void WinHttpTransport::SendRequest(std::unique_ptr<Details::HandleManager>& handleManager)
{
  std::wstring encodedHeaders;
  int encodedHeadersLength = 0;

  // TODO: Consider saving this to a field to avoid multiple request processing on retry.
  auto requestHeaders = handleManager->m_request.GetHeaders();
  if (requestHeaders.size() != 0)
  {
    // The encodedHeaders will be null-terminated and the length is calculated.
    encodedHeadersLength = -1;
    std::string requestHeaderString = handleManager->m_request.GetHeadersAsString();
    requestHeaderString.append("\0");

    encodedHeaders = StringToWideString(requestHeaderString);
  }

  int64_t streamLength = handleManager->m_request.GetBodyStream()->Length();

  // Send a request.
  // TODO: For PUT/POST requests, send additional data using WinHttpWriteData.
  // TODO: Support chunked transfer encoding and missing content-length header.
  if (!WinHttpSendRequest(
          handleManager->m_requestHandle,
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

  if (streamLength > 0 || streamLength == -1)
  {
    Upload(handleManager);
  }
}

void WinHttpTransport::ReceiveResponse(std::unique_ptr<Details::HandleManager>& handleManager)
{
  // Wait to receive the response to the HTTP request initiated by WinHttpSendRequest.
  // When WinHttpReceiveResponse completes successfully, the status code and response headers have
  // been received.
  if (!WinHttpReceiveResponse(handleManager->m_requestHandle, NULL))
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
    std::unique_ptr<Details::HandleManager>& handleManager,
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
            handleManager->m_requestHandle,
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

std::unique_ptr<Details::WinHttpStream> WinHttpTransport::GetBodyStream(
    std::unique_ptr<Details::HandleManager>& handleManager,
    int64_t contentLength)
{
  return std::make_unique<Details::WinHttpStream>(handleManager, contentLength);
}

std::unique_ptr<RawResponse> WinHttpTransport::GetRawResponse(
    std::unique_ptr<Details::HandleManager>& handleManager,
    HttpMethod requestMethod)
{
  // First, use WinHttpQueryHeaders to obtain the size of the buffer.
  // The call is expected to fail since no destination buffer is provided.
  DWORD sizeOfHeaders = 0;
  if (WinHttpQueryHeaders(
          handleManager->m_requestHandle,
          WINHTTP_QUERY_RAW_HEADERS_CRLF,
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
      throw Azure::Core::Http::TransportException(
          "Error while querying response headers. Error Code: " + std::to_string(error) + ".");
    }
  }

  // Allocate memory for the buffer.
  std::vector<WCHAR> outputBuffer(sizeOfHeaders / sizeof(WCHAR), 0);

  // Now, use WinHttpQueryHeaders to retrieve all the headers.
  if (!WinHttpQueryHeaders(
          handleManager->m_requestHandle,
          WINHTTP_QUERY_RAW_HEADERS_CRLF,
          WINHTTP_HEADER_NAME_BY_INDEX,
          outputBuffer.data(),
          &sizeOfHeaders,
          WINHTTP_NO_HEADER_INDEX))
  {
    GetErrorAndThrow("Error while querying response headers.");
  }

  // TODO: This check isn't really necessary - need a debug time assert or testing before removing.
  if (outputBuffer.size() < sizeOfHeaders / sizeof(WCHAR))
  {
    GetErrorAndThrow("Unexpected error - buffer size not consistent with expected header size.");
  }

  auto start = outputBuffer.data();
  auto last = start + sizeOfHeaders / sizeof(WCHAR);
  auto statusLineEnd = std::find(start, last, '\n');
  start = statusLineEnd + 1; // start of headers
  std::string responseHeaders = WideStringToString(std::wstring(start, last - (statusLineEnd + 1)));

  DWORD sizeOfHttp = sizeOfHeaders;

  // Get the HTTP version.
  if (!WinHttpQueryHeaders(
          handleManager->m_requestHandle,
          WINHTTP_QUERY_VERSION,
          WINHTTP_HEADER_NAME_BY_INDEX,
          outputBuffer.data(),
          &sizeOfHttp,
          WINHTTP_NO_HEADER_INDEX))
  {
    GetErrorAndThrow("Error while querying response headers.");
  }

  start = outputBuffer.data();
  // Assuming ASCII here is OK since the input is expected to be an HTTP version string.
  std::string httpVersion = WideStringToStringASCII(start, start + sizeOfHttp / sizeof(WCHAR));

  uint16_t majorVersion = 0;
  uint16_t minorVersion = 0;
  ParseHttpVersion(httpVersion, &majorVersion, &minorVersion);

  DWORD statusCode = 0;
  DWORD dwSize = sizeof(statusCode);

  // Get the status code as a number.
  if (!WinHttpQueryHeaders(
          handleManager->m_requestHandle,
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
          handleManager->m_requestHandle,
          WINHTTP_QUERY_STATUS_TEXT,
          WINHTTP_HEADER_NAME_BY_INDEX,
          outputBuffer.data(),
          &sizeOfReasonPhrase,
          WINHTTP_NO_HEADER_INDEX))
  {
    start = outputBuffer.data();
    reasonPhrase
        = WideStringToString(std::wstring(start, start + sizeOfReasonPhrase / sizeof(WCHAR)));
  }

  // Allocate the instance of the response on the heap with a shared ptr so this memory gets
  // delegated outside the transport and will be eventually released.
  auto rawResponse
      = std::make_unique<RawResponse>(majorVersion, minorVersion, httpStatusCode, reasonPhrase);
  rawResponse->AddHeaders(responseHeaders);

  int64_t contentLength
      = GetContentLength(handleManager, requestMethod, rawResponse->GetStatusCode());

  rawResponse->SetBodyStream(std::move(GetBodyStream(handleManager, contentLength)));
  return rawResponse;
}

std::unique_ptr<RawResponse> WinHttpTransport::Send(Context const& context, Request& request)
{
  // TODO: Need to test if context has been canceled and cleanup, can't call ThrowIfCanceled.
  (void)(context);

  auto handleManager = std::make_unique<Details::HandleManager>(context, request);

  GetSessionHandle(handleManager);
  GetConnectionHandle(handleManager);
  GetRequestHandle(handleManager);

  SendRequest(handleManager);

  ReceiveResponse(handleManager);

  return GetRawResponse(handleManager, request.GetMethod());
}

// Read the response from the sent request.
int64_t Details::WinHttpStream::Read(Context const& context, uint8_t* buffer, int64_t count)
{
  context.ThrowIfCanceled();

  if (count <= 0 || this->m_isEOF)
  {
    return 0;
  }

  int64_t totalNumberOfBytesRead = 0;
  DWORD numberOfBytesRead = 0;

  // Keep checking for data until there is nothing left.
  do
  {
    context.ThrowIfCanceled();

    // Check for available data.
    DWORD numberOfBytesAvailable = 0;
    if (!WinHttpQueryDataAvailable(this->m_handleManager->m_requestHandle, &numberOfBytesAvailable))
    {
      // Errors include:
      // ERROR_WINHTTP_CONNECTION_ERROR
      // ERROR_WINHTTP_INCORRECT_HANDLE_STATE
      // ERROR_WINHTTP_INCORRECT_HANDLE_TYPE
      // ERROR_WINHTTP_INTERNAL_ERROR
      // ERROR_WINHTTP_OPERATION_CANCELLED
      // ERROR_WINHTTP_TIMEOUT
      // ERROR_NOT_ENOUGH_MEMORY

      DWORD error = GetLastError();
      throw Azure::Core::Http::TransportException(
          "Error while querying how much data is available to read. Error Code: "
          + std::to_string(error) + ".");
    }

    context.ThrowIfCanceled();

    DWORD numberOfBytesToRead = numberOfBytesAvailable;
    if (numberOfBytesAvailable > count)
    {
      numberOfBytesToRead = static_cast<DWORD>(count);
    }

    if (!WinHttpReadData(
            this->m_handleManager->m_requestHandle,
            (LPVOID)(buffer + totalNumberOfBytesRead),
            numberOfBytesToRead,
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
          "Error while querying how much data is available to read. Error Code: "
          + std::to_string(error) + ".");
    }

    totalNumberOfBytesRead += numberOfBytesRead;
    count -= numberOfBytesRead;

    if (numberOfBytesRead == 0
        || (this->m_contentLength != -1
            && this->m_streamTotalRead == this->m_contentLength - totalNumberOfBytesRead))
    {
      this->m_isEOF = true;
      break;
    }

  } while (count > 0);

  this->m_streamTotalRead += totalNumberOfBytesRead;
  return totalNumberOfBytesRead;
}
