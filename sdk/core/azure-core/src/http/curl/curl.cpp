// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/curl/curl.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/internal/log.hpp"

#ifdef POSIX
#include <poll.h> // for poll()
#endif
#ifdef WINDOWS
#include <winsock2.h> // for WSAPoll();
#endif

#include <algorithm>
#include <string>
#include <thread>

namespace {
// Can be used from anywhere a little simpler
inline void LogThis(std::string const& msg)
{
  if (Azure::Core::Logging::Details::ShouldWrite(
          Azure::Core::Http::LogClassification::HttpTransportAdapter))
  {
    Azure::Core::Logging::Details::Write(
        Azure::Core::Http::LogClassification::HttpTransportAdapter,
        "[CURL Transport Adapter]: " + msg);
  }
}

template <typename T>
inline void SetLibcurlOption(
    CURL* handle,
    CURLoption option,
    T value,
    std::string const& errorMessage)
{
  auto result = curl_easy_setopt(handle, option, value);
  if (result != CURLE_OK)
  {
    throw Azure::Core::Http::TransportException(
        errorMessage + ". " + std::string(curl_easy_strerror(result)));
  }
}

enum class PollSocketDirection
{
  Read = 1,
  Write = 2,
};

/**
 * @brief Use poll from OS to check if socket is ready to be read or written.
 *
 * @param socketFileDescriptor socket descriptor.
 * @param direction poll events for read or write socket.
 * @param timeout  return if polling for more than \p timeout
 * @param context The context while polling that can be use to cancel waiting for socket.
 *
 * @return int with negative 1 upon any error, 0 on timeout or greater than zero if events were
 * detected (socket ready to be written/read)
 */
int pollSocketUntilEventOrTimeout(
    Azure::Core::Context const& context,
    curl_socket_t socketFileDescriptor,
    PollSocketDirection direction,
    long timeout)
{

#ifndef POSIX
#ifndef WINDOWS
  // platform does not support Poll().
  throw TransportException("Error while sending request. Platform does not support Poll()");
#endif
#endif

  struct pollfd poller;
  poller.fd = socketFileDescriptor;

  // set direction
  if (direction == PollSocketDirection::Read)
  {
    poller.events = POLLIN;
  }
  else
  {
    poller.events = POLLOUT;
  }

  // Call poll with the poller struct. Poll can handle multiple file descriptors by making an
  // pollfd array and passing the size of it as the second arg. Since we are only passing one fd,
  // we use 1 as arg.

  // Cancelation is possible by calling poll() with small time intervals instead of using the
  // requested timeout. Default interval for calling poll() is 1 sec whenever arg timeout is
  // greater than 1 sec. Otherwise the interval is set to timeout
  long interval = 1000; // 1 second
  if (timeout < interval)
  {
    interval = timeout;
  }
  int result = 0;
  for (long counter = 0; counter < timeout && result == 0; counter = counter + interval)
  {
    // check cancelation
    context.ThrowIfCanceled();
#ifdef POSIX
    result = poll(&poller, 1, interval);
#endif
#ifdef WINDOWS
    result = WSAPoll(&poller, 1, interval);
#endif
  }
  // result can be either 0 (timeout) or > 1 (socket ready)
  return result;
}

#ifdef WINDOWS
// Windows needs this after every write to socket or performance would be reduced to 1/4 for
// uploading operation.
// https://github.com/Azure/azure-sdk-for-cpp/issues/644
void WinSocketSetBuffSize(curl_socket_t socket)
{
  ULONG ideal;
  DWORD ideallen;
  // WSAloctl would get the ideal size for the socket buffer.
  if (WSAIoctl(socket, SIO_IDEAL_SEND_BACKLOG_QUERY, 0, 0, &ideal, sizeof(ideal), &ideallen, 0, 0)
      == 0)
  {
    // if WSAloctl succeeded (returned 0), set the socket buffer size.
    // Specifies the total per-socket buffer space reserved for sends.
    // https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-setsockopt
    auto result = setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (const char*)&ideal, sizeof(ideal));
    LogThis(
        "Windows - calling setsockopt after uploading chunk. ideal = " + std::to_string(ideal)
        + " result = " + std::to_string(result));
  }
}
#endif // WINDOWS
} // namespace

using Azure::Core::Http::CurlConnection;
using Azure::Core::Http::CurlConnectionPool;
using Azure::Core::Http::CurlSession;
using Azure::Core::Http::CurlTransport;
using Azure::Core::Http::HttpStatusCode;
using Azure::Core::Http::LogClassification;
using Azure::Core::Http::RawResponse;
using Azure::Core::Http::Request;
using Azure::Core::Http::TransportException;

std::unique_ptr<RawResponse> CurlTransport::Send(Context const& context, Request& request)
{
  // Create CurlSession to perform request
  LogThis("Creating a new session.");
  auto session = std::make_unique<CurlSession>(request);
  CURLcode performing;

  // Try to send the request. If we get CURLE_UNSUPPORTED_PROTOCOL back, it means the connection is
  // either closed or the socket is not usable any more. In that case, let the session be destroyed
  // and create a new session to get another connection from connection pool.
  // Prevent from trying forever by using c_DefaultMaxOpenNewConnectionIntentsAllowed.
  for (auto getConnectionOpenIntent = 0;
       getConnectionOpenIntent < Details::c_DefaultMaxOpenNewConnectionIntentsAllowed;
       getConnectionOpenIntent++)
  {
    performing = session->Perform(context);
    if (performing != CURLE_UNSUPPORTED_PROTOCOL)
    {
      break;
    }
    // Let session be destroyed and create a new one to get a new connection
    session = std::make_unique<CurlSession>(request);
  }

  if (performing != CURLE_OK)
  {
    throw Azure::Core::Http::TransportException(
        "Error while sending request. " + std::string(curl_easy_strerror(performing)));
  }

  LogThis("Request completed. Moving response out of session and session to response.");
  // Move Response out of the session
  auto response = session->GetResponse();
  // Move the ownership of the CurlSession (bodyStream) to the response
  response->SetBodyStream(std::move(session));
  return response;
}

CURLcode CurlSession::Perform(Context const& context)
{

  // Set the session state
  m_sessionState = SessionState::PERFORM;

  // Get the socket that libcurl is using from handle. Will use this to wait while reading/writing
  // into wire
  auto result = curl_easy_getinfo(
      this->m_connection->GetHandle(), CURLINFO_ACTIVESOCKET, &this->m_curlSocket);
  if (result != CURLE_OK)
  {
    return result;
  }

  // LibCurl settings after connection is open (headers)
  {
    auto headers = this->m_request.GetHeaders();
    auto hostHeader = headers.find("Host");
    if (hostHeader == headers.end())
    {
      LogThis("No Host in request headers. Adding it");
      this->m_request.AddHeader("Host", this->m_request.GetUrl().GetHost());
    }
    auto isContentLengthHeaderInRequest = headers.find("content-length");
    if (isContentLengthHeaderInRequest == headers.end())
    {
      LogThis("No content-length in headers. Adding it");
      this->m_request.AddHeader(
          "content-length", std::to_string(this->m_request.GetBodyStream()->Length()));
    }
  }

  // use expect:100 for PUT requests. Server will decide if it can take our request
  if (this->m_request.GetMethod() == HttpMethod::Put)
  {
    LogThis("Using 100-continue for PUT request");
    this->m_request.AddHeader("expect", "100-continue");
  }

  // Send request. If the connection assigned to this curlSession is closed or the socket is
  // somehow lost, libcurl will return CURLE_UNSUPPORTED_PROTOCOL
  // (https://curl.haxx.se/libcurl/c/curl_easy_send.html). Return the error back.
  LogThis("Send request without payload");
  result = SendRawHttp(context);
  if (result != CURLE_OK)
  {
    return result;
  }

  LogThis("Parse server response");
  ReadStatusLineAndHeadersFromRawResponse(context);

  // non-PUT request are ready to be stream at this point. Only PUT request would start an uploading
  // transfer where we want to maintain the `PERFORM` state.
  if (this->m_request.GetMethod() != HttpMethod::Put)
  {
    m_sessionState = SessionState::STREAMING;
    return result;
  }

  LogThis("Check server response before upload starts");

  // Check server response from Expect:100-continue for PUT;
  // This help to prevent us from start uploading data when Server can't handle it
  if (this->m_lastStatusCode != HttpStatusCode::Continue)
  {
    LogThis("Server rejected the upload request");
    return result; // Won't upload.
  }

  LogThis("Upload payload");
  if (this->m_bodyStartInBuffer > 0)
  {
    // If internal buffer has more data after the 100-continue means Server return an error.
    // We don't need to upload body, just parse the response from Server and return
    ReadStatusLineAndHeadersFromRawResponse(context, true);
    return result;
  }

  // Start upload
  result = this->UploadBody(context);
  if (result != CURLE_OK)
  {
    return result; // will throw transport exception before trying to read
  }

  LogThis("Upload completed. Parse server response");
  ReadStatusLineAndHeadersFromRawResponse(context);
  // If no throw at this point, the request is ready to stream.
  // If any throw happened before this point, the state will remain as PERFORM.
  m_sessionState = SessionState::STREAMING;
  return result;
}

// Creates an HTTP Response with specific bodyType
static std::unique_ptr<RawResponse> CreateHTTPResponse(
    uint8_t const* const begin,
    uint8_t const* const last)
{
  // set response code, http version and reason phrase (i.e. HTTP/1.1 200 OK)
  auto start = begin + 5; // HTTP = 4, / = 1, moving to 5th place for version
  auto end = std::find(start, last, '.');
  auto majorVersion = std::stoi(std::string(start, end));

  start = end + 1; // start of minor version
  end = std::find(start, last, ' ');
  auto minorVersion = std::stoi(std::string(start, end));

  start = end + 1; // start of status code
  end = std::find(start, last, ' ');
  auto statusCode = std::stoi(std::string(start, end));

  start = end + 1; // start of reason phrase
  end = std::find(start, last, '\r');
  auto reasonPhrase = std::string(start, end); // remove \r

  // allocate the instance of response to heap with shared ptr
  // So this memory gets delegated outside Curl Transport as a shared ptr so memory will be
  // eventually released
  return std::make_unique<RawResponse>(
      (uint16_t)majorVersion, (uint16_t)minorVersion, HttpStatusCode(statusCode), reasonPhrase);
}

// Creates an HTTP Response with specific bodyType
static std::unique_ptr<RawResponse> CreateHTTPResponse(std::string const& header)
{
  return CreateHTTPResponse(
      reinterpret_cast<const uint8_t*>(header.data()),
      reinterpret_cast<const uint8_t*>(header.data() + header.size()));
}

bool CurlSession::isUploadRequest()
{
  return this->m_request.GetMethod() == HttpMethod::Put
      || this->m_request.GetMethod() == HttpMethod::Post;
}

// Send buffer thru the wire
CURLcode CurlSession::SendBuffer(Context const& context, uint8_t const* buffer, size_t bufferSize)
{
  for (size_t sentBytesTotal = 0; sentBytesTotal < bufferSize;)
  {
    // check cancelation for each chunk of data.
    // Next loop is expected to be called at most 2 times:
    // The first time we call `curl_easy_send()`, if it return CURLE_AGAIN it would call
    // `pollSocketUntilEventOrTimeout` to wait for socket to be ready to write.
    // `pollSocketUntilEventOrTimeout` will then handle cancelation token.
    // If socket is not ready before the timeout, Exception is thrown.
    // When socket is ready, it calls curl_easy_send() again (second loop iteration). It is not
    // expected to return CURLE_AGAIN (since socket is ready), so, a chuck of data will be uploaded
    // and result will be CURLE_OK which breaks the loop. Also, getting other than CURLE_OK or
    // CURLE_AGAIN throws.
    context.ThrowIfCanceled();
    for (CURLcode sendResult = CURLE_AGAIN; sendResult == CURLE_AGAIN;)
    {
      size_t sentBytesPerRequest = 0;
      sendResult = curl_easy_send(
          this->m_connection->GetHandle(),
          buffer + sentBytesTotal,
          bufferSize - sentBytesTotal,
          &sentBytesPerRequest);

      switch (sendResult)
      {
        case CURLE_OK:
        {
          sentBytesTotal += sentBytesPerRequest;
          break;
        }
        case CURLE_AGAIN:
        {
          // start polling operation with 1 min timeout
          auto pollUntilSocketIsReady = pollSocketUntilEventOrTimeout(
              context, this->m_curlSocket, PollSocketDirection::Write, 60000L);

          if (pollUntilSocketIsReady == 0)
          {
            throw TransportException("Timeout waiting for socket to upload.");
          }
          else if (pollUntilSocketIsReady < 0)
          { // negative value, error while polling
            throw TransportException("Error while polling for socket ready write");
          }

          // Ready to continue download.
          break;
        }
        default:
        {
          return sendResult;
        }
      }
    };
  }
#ifdef WINDOWS
  WinSocketSetBuffSize(this->m_curlSocket);
#endif // WINDOWS
  return CURLE_OK;
}

CURLcode CurlSession::UploadBody(Context const& context)
{
  // Send body UploadStreamPageSize at a time (libcurl default)
  // NOTE: if stream is on top a contiguous memory, we can avoid allocating this copying buffer
  auto streamBody = this->m_request.GetBodyStream();
  CURLcode sendResult = CURLE_OK;

  int64_t uploadChunkSize = this->m_request.GetUploadChunkSize();
  if (uploadChunkSize <= 0)
  {
    // use default size
    uploadChunkSize = Details::c_DefaultUploadChunkSize;
  }
  auto unique_buffer = std::make_unique<uint8_t[]>(static_cast<size_t>(uploadChunkSize));

  while (true)
  {
    auto rawRequestLen = streamBody->Read(context, unique_buffer.get(), uploadChunkSize);
    if (rawRequestLen == 0)
    {
      break;
    }
    sendResult = SendBuffer(context, unique_buffer.get(), static_cast<size_t>(rawRequestLen));
    if (sendResult != CURLE_OK)
    {
      return sendResult;
    }
  }
  return sendResult;
}

// custom sending to wire an http request
CURLcode CurlSession::SendRawHttp(Context const& context)
{
  // something like GET /path HTTP1.0 \r\nheaders\r\n
  auto rawRequest = this->m_request.GetHTTPMessagePreBody();
  int64_t rawRequestLen = rawRequest.size();

  CURLcode sendResult = SendBuffer(
      context,
      reinterpret_cast<uint8_t const*>(rawRequest.data()),
      static_cast<size_t>(rawRequestLen));

  if (sendResult != CURLE_OK || this->m_request.GetMethod() == HttpMethod::Put)
  {
    return sendResult;
  }

  return this->UploadBody(context);
}

void CurlSession::ParseChunkSize(Context const& context)
{
  // Use this string to construct the chunk size. This is because we could have an internal
  // buffer like [headers...\r\n123], where 123 is chunk size but we still need to pull more
  // data fro wire to get the full chunkSize. Next data could be just [\r\n] or [456\r\n]
  auto strChunkSize = std::string();

  // Move to after chunk size
  for (bool keepPolling = true; keepPolling;)
  {
    for (int64_t index = this->m_bodyStartInBuffer, i = 0; index < this->m_innerBufferSize;
         index++, i++)
    {
      strChunkSize.append(reinterpret_cast<char*>(&this->m_readBuffer[index]), 1);
      if (i > 1 && this->m_readBuffer[index] == '\n')
      {
        // get chunk size. Chunk size comes in Hex value
        this->m_chunkSize = static_cast<int64_t>(std::stoull(strChunkSize, nullptr, 16));

        if (this->m_chunkSize == 0)
        { // Response with no content. end of chunk
          keepPolling = false;
          break;
        }

        if (index + 1 == this->m_innerBufferSize)
        { // on last index. Whatever we read is the BodyStart here
          this->m_innerBufferSize
              = ReadFromSocket(context, this->m_readBuffer, Details::c_DefaultLibcurlReaderSize);
          this->m_bodyStartInBuffer = 0;
        }
        else
        { // not at the end, buffer like [999 \r\nBody...]
          this->m_bodyStartInBuffer = index + 1;
        }

        keepPolling = false;
        break;
      }
    }
    if (keepPolling)
    { // Read all internal buffer and \n was not found, pull from wire
      this->m_innerBufferSize
          = ReadFromSocket(context, this->m_readBuffer, Details::c_DefaultLibcurlReaderSize);
      this->m_bodyStartInBuffer = 0;
    }
  }
  return;
}

// Read status line plus headers to create a response with no body
void CurlSession::ReadStatusLineAndHeadersFromRawResponse(
    Context const& context,
    bool reuseInternalBuffer)
{
  auto parser = ResponseBufferParser();
  auto bufferSize = int64_t();

  // Keep reading until all headers were read
  while (!parser.IsParseCompleted())
  {
    int64_t bytesParsed = 0;
    if (reuseInternalBuffer)
    {
      // parse from internal buffer. This means previous read from server got more than one
      // response. This happens when Server returns a 100-continue plus an error code
      bufferSize = this->m_innerBufferSize - this->m_bodyStartInBuffer;
      bytesParsed = parser.Parse(
          this->m_readBuffer + this->m_bodyStartInBuffer, static_cast<size_t>(bufferSize));
      // if parsing from internal buffer is not enough, do next read from wire
      reuseInternalBuffer = false;
      // reset body start
      this->m_bodyStartInBuffer = -1;
    }
    else
    {
      // Try to fill internal buffer from socket.
      // If response is smaller than buffer, we will get back the size of the response
      bufferSize = ReadFromSocket(context, this->m_readBuffer, Details::c_DefaultLibcurlReaderSize);
      if (bufferSize == 0)
      {
        // closed connection, prevent application from keep trying to pull more bytes from the wire
        throw TransportException(
            "Connection was closed by the server while trying to read a response");
      }
      // returns the number of bytes parsed up to the body Start
      bytesParsed = parser.Parse(this->m_readBuffer, static_cast<size_t>(bufferSize));
    }

    if (bytesParsed < bufferSize)
    {
      this->m_bodyStartInBuffer = bytesParsed; // Body Start
    }
  }

  this->m_response = parser.GetResponse();
  this->m_innerBufferSize = static_cast<size_t>(bufferSize);
  this->m_lastStatusCode = this->m_response->GetStatusCode();

  // For Head request, set the length of body response to 0.
  // Response will give us content-length as if we were not doing Head saying what would it be the
  // length of the body. However, Server won't send body
  // For NoContent status code, also need to set contentLength to 0.
  // https://github.com/Azure/azure-sdk-for-cpp/issues/406
  if (this->m_request.GetMethod() == HttpMethod::Head
      || this->m_lastStatusCode == HttpStatusCode::NoContent)
  {
    this->m_contentLength = 0;
    this->m_bodyStartInBuffer = -1;
    return;
  }

  // headers are already lowerCase at this point
  auto headers = this->m_response->GetHeaders();

  auto isContentLengthHeaderInResponse = headers.find("content-length");
  if (isContentLengthHeaderInResponse != headers.end())
  {
    this->m_contentLength
        = static_cast<int64_t>(std::stoull(isContentLengthHeaderInResponse->second.data()));
    return;
  }

  this->m_contentLength = -1;
  auto isTransferEncodingHeaderInResponse = headers.find("transfer-encoding");
  if (isTransferEncodingHeaderInResponse != headers.end())
  {
    auto headerValue = isTransferEncodingHeaderInResponse->second;
    auto isChunked = headerValue.find("chunked");

    if (isChunked != std::string::npos)
    {
      // set curl session to know response is chunked
      // This will be used to remove chunked info while reading
      this->m_isChunkedResponseType = true;

      // Need to move body start after chunk size
      if (this->m_bodyStartInBuffer == -1)
      { // if nothing on inner buffer, pull from wire
        this->m_innerBufferSize
            = ReadFromSocket(context, this->m_readBuffer, Details::c_DefaultLibcurlReaderSize);
        this->m_bodyStartInBuffer = 0;
      }

      ParseChunkSize(context);
      return;
    }
  }
  /*
  https://tools.ietf.org/html/rfc7230#section-3.3.3
   7.  Otherwise, this is a response message without a declared message
       body length, so the message body length is determined by the
       number of octets received prior to the server closing the
       connection.
  */
}

// Read from curl session
int64_t CurlSession::Read(Context const& context, uint8_t* buffer, int64_t count)
{
  context.ThrowIfCanceled();

  if (count <= 0 || this->IsEOF())
  {
    return 0;
  }

  // check if all chunked is all read already
  if (this->m_isChunkedResponseType && this->m_chunkSize == this->m_sessionTotalRead)
  {
    // Need to read CRLF after all chunk was read
    for (int8_t i = 0; i < 2; i++)
    {
      if (this->m_bodyStartInBuffer > 0 && this->m_bodyStartInBuffer < this->m_innerBufferSize)
      {
        this->m_bodyStartInBuffer += 1;
      }
      else
      { // end of buffer, pull data from wire
        this->m_innerBufferSize
            = ReadFromSocket(context, this->m_readBuffer, Details::c_DefaultLibcurlReaderSize);
        this->m_bodyStartInBuffer = 1; // jump first char (could be \r or \n)
      }
    }
    // Reset session read counter for next chunk
    this->m_sessionTotalRead = 0;
    // get the size of next chunk
    ParseChunkSize(context);

    if (this->IsEOF())
    { // after parsing next chunk, check if it is zero
      return 0;
    }
  }

  auto totalRead = int64_t();
  auto readRequestLength = this->m_isChunkedResponseType
      ? std::min(this->m_chunkSize - this->m_sessionTotalRead, count)
      : count;

  // For responses with content-length, avoid trying to read beyond Content-length or
  // libcurl could return a second response as BadRequest.
  // https://github.com/Azure/azure-sdk-for-cpp/issues/306
  if (this->m_contentLength > 0)
  {
    auto remainingBodyContent = this->m_contentLength - this->m_sessionTotalRead;
    readRequestLength = std::min(readRequestLength, remainingBodyContent);
  }

  // Take data from inner buffer if any
  if (this->m_bodyStartInBuffer >= 0)
  {
    // still have data to take from innerbuffer
    MemoryBodyStream innerBufferMemoryStream(
        this->m_readBuffer + this->m_bodyStartInBuffer,
        this->m_innerBufferSize - this->m_bodyStartInBuffer);

    totalRead = innerBufferMemoryStream.Read(context, buffer, readRequestLength);
    this->m_bodyStartInBuffer += totalRead;
    this->m_sessionTotalRead += totalRead;

    if (this->m_bodyStartInBuffer == this->m_innerBufferSize)
    {
      this->m_bodyStartInBuffer = -1; // read everything from inner buffer already
    }
    return totalRead;
  }

  // Head request have contentLength = 0, so we won't read more, just return 0
  // Also if we have already read all contentLength
  if (this->m_sessionTotalRead == this->m_contentLength || this->IsEOF())
  {
    return 0;
  }

  // Read from socket when no more data on internal buffer
  // For chunk request, read a chunk based on chunk size
  totalRead = ReadFromSocket(context, buffer, static_cast<size_t>(readRequestLength));
  this->m_sessionTotalRead += totalRead;

  // Reading 0 bytes means closed connection.
  // For known content length and chunked response, this means there is nothing else to read from
  // server or lost connection before getting full response.
  // For unknown response size, it means the end of response and it's fine.
  if (totalRead == 0 && (this->m_contentLength > 0 || this->m_isChunkedResponseType))
  {
    auto expectedToRead = this->m_isChunkedResponseType ? this->m_chunkSize : this->m_contentLength;
    if (this->m_sessionTotalRead < expectedToRead)
    {
      throw TransportException(
          "Connection closed before getting full response or response is less than expected. "
          "Expected response length = "
          + std::to_string(expectedToRead)
          + ". Read until now = " + std::to_string(this->m_sessionTotalRead));
    }
  }

  return totalRead;
}

// Read from socket and return the number of bytes taken from socket
int64_t CurlSession::ReadFromSocket(Context const& context, uint8_t* buffer, int64_t bufferSize)
{
  // loop until read result is not CURLE_AGAIN
  // Next loop is expected to be called at most 2 times:
  // The first time it calls `curl_easy_recv()`, if it returns CURLE_AGAIN it would call
  // `pollSocketUntilEventOrTimeout` and wait for socket to be ready to read.
  // `pollSocketUntilEventOrTimeout` will then handle cancelation token.
  // If socket is not ready before the timeout, Exception is thrown.
  // When socket is ready, it calls curl_easy_recv() again (second loop iteration). It is not
  // expected to return CURLE_AGAIN (since socket is ready), so, a chuck of data will be downloaded
  // and result will be CURLE_OK which breaks the loop. Also, getting other than CURLE_OK or
  // CURLE_AGAIN throws.
  size_t readBytes = 0;
  for (CURLcode readResult = CURLE_AGAIN; readResult == CURLE_AGAIN;)
  {
    readResult = curl_easy_recv(
        this->m_connection->GetHandle(), buffer, static_cast<size_t>(bufferSize), &readBytes);

    switch (readResult)
    {
      case CURLE_AGAIN:
      {
        // start polling operation
        auto pollUntilSocketIsReady = pollSocketUntilEventOrTimeout(
            context, this->m_curlSocket, PollSocketDirection::Read, 60000L);

        if (pollUntilSocketIsReady == 0)
        {
          throw TransportException("Timeout waiting for socket to read.");
        }
        else if (pollUntilSocketIsReady < 0)
        { // negative value, error while polling
          throw TransportException("Error while polling for socket ready read");
        }

        // Ready to continue download.
        break;
      }
      case CURLE_OK:
      {
        break;
      }
      default:
      {
        // Error reading from socket
        throw TransportException(
            "Error while reading from network socket. CURLE code: " + std::to_string(readResult)
            + ". " + std::string(curl_easy_strerror(readResult)));
      }
    }
  }
#ifdef WINDOWS
  WinSocketSetBuffSize(this->m_curlSocket);
#endif // WINDOWS
  return readBytes;
}

std::unique_ptr<RawResponse> CurlSession::GetResponse() { return std::move(this->m_response); }

int64_t CurlSession::ResponseBufferParser::Parse(
    uint8_t const* const buffer,
    int64_t const bufferSize)
{
  if (this->m_parseCompleted)
  {
    return 0;
  }

  // Read all buffer until \r\n is found
  int64_t start = 0, index = 0;
  for (; index < bufferSize; index++)
  {
    if (buffer[index] == '\r')
    {
      this->m_delimiterStartInPrevPosition = true;
      continue;
    }

    if (buffer[index] == '\n' && this->m_delimiterStartInPrevPosition)
    {
      // found end of delimiter
      if (this->m_internalBuffer.size() > 0) // Check internal buffer
      {
        // At this point, we are reading to append more to internal buffer.
        // Only append more if index is greater than 1, meaning not when buffer is [\r\nxxx]
        // only on buffer like [xxx\r\n yyyy], append xxx
        if (index > 1)
        {
          // Previously appended something
          this->m_internalBuffer.append(buffer + start, buffer + index - 1); // minus 1 to remove \r
        }
        if (this->state == ResponseParserState::StatusLine)
        {
          // Create Response
          this->m_response = CreateHTTPResponse(this->m_internalBuffer);
          // Set state to headers
          this->state = ResponseParserState::Headers;
          this->m_delimiterStartInPrevPosition = false;
          start = index + 1; // jump \n
        }
        else if (this->state == ResponseParserState::Headers)
        {
          // will throw if header is invalid
          this->m_response->AddHeader(this->m_internalBuffer);
          this->m_delimiterStartInPrevPosition = false;
          start = index + 1; // jump \n
        }
        else
        {
          // Should never happen that parser is not statusLIne or Headers and we still try to parse
          // more.
          throw;
        }
        // clean internal buffer
        this->m_internalBuffer.clear();
      }
      else
      {
        // Nothing at internal buffer. Add directly from internal buffer
        if (this->state == ResponseParserState::StatusLine)
        {
          // Create Response
          this->m_response = CreateHTTPResponse(buffer + start, buffer + index - 1);
          // Set state to headers
          this->state = ResponseParserState::Headers;
          this->m_delimiterStartInPrevPosition = false;
          start = index + 1; // jump \n
        }
        else if (this->state == ResponseParserState::Headers)
        {
          // Check if this is end of headers delimiter
          // 1) internal buffer is empty and \n is the first char on buffer [\nBody...]
          // 2) index == start + 1. No header data after last \r\n [header\r\n\r\n]
          if (index == 0 || index == start + 1)
          {
            this->m_parseCompleted = true;
            return index + 1; // plus 1 to advance the \n. If we were at buffer end.
          }

          // will throw if header is invalid
          this->m_response->AddHeader(buffer + start, buffer + index - 1);
          this->m_delimiterStartInPrevPosition = false;
          start = index + 1; // jump \n
        }
        else
        {
          // Should never happen that parser is not statusLIne or Headers and we still try to parse
          // more.
          throw;
        }
      }
    }
    else
    {
      if (index == 0 && this->m_internalBuffer.size() > 0 && this->m_delimiterStartInPrevPosition)
      {
        // unlikely. But this means a case with buffers like [xx\r], [xxxx]
        // \r is not delimiter and in previous loop it was omitted, so adding it now
        this->m_internalBuffer.append("\r");
      }
      // \r in the response without \n after it. keep parsing
      this->m_delimiterStartInPrevPosition = false;
    }
  }

  if (start < bufferSize)
  {
    // didn't find the end of delimiter yet, save at internal buffer
    // If this->m_delimiterStartInPrevPosition is true, buffer ends in \r [xxxx\r]
    // Don't add \r. IF next char is not \n, we will append \r then on next loop
    this->m_internalBuffer.append(
        buffer + start, buffer + bufferSize - (this->m_delimiterStartInPrevPosition ? 1 : 0));
  }

  return index;
}

// Finds delimiter '\r' as the end of the
int64_t CurlSession::ResponseBufferParser::BuildStatusCode(
    uint8_t const* const buffer,
    int64_t const bufferSize)
{
  if (this->state != ResponseParserState::StatusLine)
  {
    return 0; // Wrong internal state to call this method.
  }

  uint8_t endOfStatusLine = '\r';
  auto endOfBuffer = buffer + bufferSize;

  // Look for the end of status line in buffer
  auto indexOfEndOfStatusLine = std::find(buffer, endOfBuffer, endOfStatusLine);

  if (indexOfEndOfStatusLine == endOfBuffer)
  {
    // did not find the delimiter yet, copy to internal buffer
    this->m_internalBuffer.append(buffer, endOfBuffer);
    return bufferSize; // all buffer read and requesting for more
  }

  // Delimiter found, check if there is data in the internal buffer
  if (this->m_internalBuffer.size() > 0)
  {
    // If the index is same as buffer it means delimiter is at position 0, meaning that
    // internalBuffer contains the status line and we don't need to add anything else
    if (indexOfEndOfStatusLine > buffer)
    {
      // Append and build response minus the delimiter
      this->m_internalBuffer.append(buffer, indexOfEndOfStatusLine);
    }
    this->m_response = CreateHTTPResponse(this->m_internalBuffer);
  }
  else
  {
    // Internal Buffer was not required, create response directly from buffer
    this->m_response = CreateHTTPResponse(std::string(buffer, indexOfEndOfStatusLine));
  }

  // update control
  this->state = ResponseParserState::Headers;
  this->m_internalBuffer.clear();

  // Return the index of the next char to read after delimiter
  // No need to advance one more char ('\n') (since we might be at the end of the array)
  // Parsing Headers will make sure to move one possition
  return indexOfEndOfStatusLine + 1 - buffer;
}

// Finds delimiter '\r' as the end of the
int64_t CurlSession::ResponseBufferParser::BuildHeader(
    uint8_t const* const buffer,
    int64_t const bufferSize)
{
  if (this->state != ResponseParserState::Headers)
  {
    return 0; // can't run this if state is not Headers.
  }

  uint8_t delimiter = '\r';
  auto start = buffer;
  auto endOfBuffer = buffer + bufferSize;

  if (bufferSize == 1 && buffer[0] == '\n')
  {
    // rare case of using buffer of size 1 to read. In this case, if the char is next value after
    // headers or previous header, just consider it as read and return
    return bufferSize;
  }
  else if (bufferSize > 1 && this->m_internalBuffer.size() == 0) // only if nothing in buffer,
                                                                 // advance
  {
    // move offset one possition. This is because readStatusLine and readHeader will read up to
    // '\r' then next delimiter is '\n' and we don't care
    start = buffer + 1;
  }

  // Look for the end of status line in buffer
  auto indexOfEndOfStatusLine = std::find(start, endOfBuffer, delimiter);

  if (indexOfEndOfStatusLine == start && this->m_internalBuffer.size() == 0)
  {
    // \r found at the start means the end of headers
    this->m_internalBuffer.clear();
    this->m_parseCompleted = true;
    return 1; // can't return more than the found delimiter. On read remaining we need to also
              // remove first char
  }

  if (indexOfEndOfStatusLine == endOfBuffer)
  {
    // did not find the delimiter yet, copy to internal buffer
    this->m_internalBuffer.append(start, endOfBuffer);
    return bufferSize; // all buffer read and requesting for more
  }

  // Delimiter found, check if there is data in the internal buffer
  if (this->m_internalBuffer.size() > 0)
  {
    // If the index is same as buffer it means delimiter is at position 0, meaning that
    // internalBuffer contains the status line and we don't need to add anything else
    if (indexOfEndOfStatusLine > buffer)
    {
      // Append and build response minus the delimiter
      this->m_internalBuffer.append(start, indexOfEndOfStatusLine);
    }
    // will throw if header is invalid
    m_response->AddHeader(this->m_internalBuffer);
  }
  else
  {
    // Internal Buffer was not required, create response directly from buffer
    std::string header(std::string(start, indexOfEndOfStatusLine));
    // will throw if header is invalid
    this->m_response->AddHeader(header);
  }

  // reuse buffer
  this->m_internalBuffer.clear();

  // Return the index of the next char to read after delimiter
  // No need to advance one more char ('\n') (since we might be at the end of the array)
  // Parsing Headers will make sure to move one position
  return indexOfEndOfStatusLine + 1 - buffer;
}

std::mutex CurlConnectionPool::s_connectionPoolMutex;
std::map<std::string, std::list<std::unique_ptr<CurlConnection>>>
    CurlConnectionPool::s_connectionPoolIndex;
int32_t CurlConnectionPool::s_connectionCounter = 0;
bool CurlConnectionPool::s_isCleanConnectionsRunning = false;

std::unique_ptr<CurlConnection> CurlConnectionPool::GetCurlConnection(Request& request)
{
  std::string const& host = request.GetUrl().GetHost();

  {
    // Critical section. Needs to own s_connectionPoolMutex before executing
    // Lock mutex to access connection pool. mutex is unlock as soon as lock is out of scope
    std::lock_guard<std::mutex> lock(CurlConnectionPool::s_connectionPoolMutex);

    // get a ref to the pool from the map of pools
    auto hostPoolIndex = CurlConnectionPool::s_connectionPoolIndex.find(host);
    if (hostPoolIndex != CurlConnectionPool::s_connectionPoolIndex.end()
        && hostPoolIndex->second.size() > 0)
    {
      // get ref to first connection
      auto fistConnectionIterator = hostPoolIndex->second.begin();
      // move the connection ref to temp ref
      auto connection = std::move(*fistConnectionIterator);
      // Remove the connection ref from list
      hostPoolIndex->second.erase(fistConnectionIterator);
      // reduce number of connections on the pool
      CurlConnectionPool::s_connectionCounter -= 1;

      // Remove index if there are no more connections
      if (hostPoolIndex->second.size() == 0)
      {
        CurlConnectionPool::s_connectionPoolIndex.erase(hostPoolIndex);
      }

      // return connection ref
      return connection;
    }
  }

  // Creating a new connection is thread safe. No need to lock mutex here.
  // No available connection for the pool for the required host. Create one
  auto newConnection = std::make_unique<CurlConnection>(host);

  // Libcurl setup before open connection (url, connect_only, timeout)
  SetLibcurlOption(
      newConnection->GetHandle(),
      CURLOPT_URL,
      request.GetUrl().GetAbsoluteUrl().data(),
      Details::c_DefaultFailedToGetNewConnectionTemplate + host);

  SetLibcurlOption(
      newConnection->GetHandle(),
      CURLOPT_CONNECT_ONLY,
      1L,
      Details::c_DefaultFailedToGetNewConnectionTemplate + host);

  // curl_easy_setopt(newConnection->GetHandle(), CURLOPT_VERBOSE, 1L);
  // Set timeout to 24h. Libcurl will fail uploading on windows if timeout is:
  // timeout >= 25 days. Fails as soon as trying to upload any data
  // 25 days < timeout > 1 days. Fail on huge uploads ( > 1GB)
  SetLibcurlOption(
      newConnection->GetHandle(),
      CURLOPT_TIMEOUT,
      60L * 60L * 24L,
      Details::c_DefaultFailedToGetNewConnectionTemplate + host);

  auto result = curl_easy_perform(newConnection->GetHandle());
  if (result != CURLE_OK)
  {
    throw Http::TransportException(
        Details::c_DefaultFailedToGetNewConnectionTemplate + host + ". "
        + std::string(curl_easy_strerror(result)));
  }
  return newConnection;
}

// Move the connection back to the connection pool. Push it to the front so it becomes the first
// connection to be picked next time some one ask for a connection to the pool (LIFO)
void CurlConnectionPool::MoveConnectionBackToPool(
    std::unique_ptr<CurlConnection> connection,
    Http::HttpStatusCode lastStatusCode)
{
  auto code = static_cast<std::underlying_type<Http::HttpStatusCode>::type>(lastStatusCode);
  // laststatusCode = 0
  if (code < 200 || code >= 300)
  {
    // A handler with previous response with Error can't be re-use.
    return;
  }

  // Lock mutex to access connection pool. mutex is unlock as soon as lock is out of scope
  std::lock_guard<std::mutex> lock(CurlConnectionPool::s_connectionPoolMutex);
  auto& hostPool = CurlConnectionPool::s_connectionPoolIndex[connection->GetHost()];
  // update the time when connection was moved back to pool
  connection->updateLastUsageTime();
  hostPool.push_front(std::move(connection));
  CurlConnectionPool::s_connectionCounter += 1;
  // Check if there's no cleaner running and started
  if (!CurlConnectionPool::s_isCleanConnectionsRunning)
  {
    CurlConnectionPool::s_isCleanConnectionsRunning = true;
    CurlConnectionPool::CleanUp();
  }
}

// spawn a thread for cleaning old connections.
// Thread will keep running while there are at least one connection in the pool
void CurlConnectionPool::CleanUp()
{
  std::thread backgroundCleanerThread([]() {
    for (;;)
    {
      // wait before trying to clean
      std::this_thread::sleep_for(
          std::chrono::milliseconds(Details::c_DefaultCleanerIntervalMilliseconds));

      {
        // take mutex for reading the pool
        std::lock_guard<std::mutex> lock(CurlConnectionPool::s_connectionPoolMutex);

        if (CurlConnectionPool::s_connectionCounter == 0)
        {
          // stop the cleaner since there are no connections
          CurlConnectionPool::s_isCleanConnectionsRunning = false;
          return;
        }

        // loop the connection pool index
        for (auto index = CurlConnectionPool::s_connectionPoolIndex.begin();
             index != CurlConnectionPool::s_connectionPoolIndex.end();
             index++)
        {
          if (index->second.size() == 0)
          {
            // Move the next pool index
            continue;
          }

          // Pool index with waiting connections. Loop the connection pool backwards until
          // a connection that is not expired is found or until all connections are removed.
          for (auto connection = index->second.end();;)
          {
            // loop starts at end(), go back to previous possition. We know the list is size() > 0
            // so we are safe to go end() - 1 and find the last element in the list
            connection--;
            if (connection->get()->isExpired())
            {
              // remove connection from the pool and update the connection to the next one which
              // is going to be list.end()
              connection = index->second.erase(connection);
              CurlConnectionPool::s_connectionCounter -= 1;

              // Connection removed, break if there are no more connections to check
              if (index->second.size() == 0)
              {
                break;
              }
            }
            else
            {
              // Got a non-expired connection, all connections before this one are not expired.
              // Break the loop and continue looping the Pool index
              break;
            }
          }
        }
      }
    }
  });

  // let thread run independent. It will be done once ther is not connections in the pool
  backgroundCleanerThread.detach();
}
