// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/curl/curl.hpp"
#include "azure/core/http/http.hpp"
#include "azure/core/http/policy.hpp"
#include "azure/core/http/transport.hpp"
#include "azure/core/internal/log.hpp"
#include "azure/core/platform.hpp"

// Private incude
#include "curl_connection_pool_private.hpp"
#include "curl_connection_private.hpp"
#include "curl_session_private.hpp"

#if defined(AZ_PLATFORM_POSIX)
#include <poll.h> // for poll()
#elif defined(AZ_PLATFORM_WINDOWS)
#include <winsock2.h> // for WSAPoll();
#endif

#include <algorithm>
#include <curl/curl.h>
#include <string>
#include <thread>

namespace {
std::string const LogMsgPrefix = "[CURL Transport Adapter]: ";

template <typename T>
#if defined(_MSC_VER)
#pragma warning(push)
// C26812: The enum type 'CURLoption' is unscoped. Prefer 'enum class' over 'enum' (Enum.3)
#pragma warning(disable : 26812)
#endif
inline bool SetLibcurlOption(CURL* handle, CURLoption option, T value, CURLcode* outError)
{
  *outError = curl_easy_setopt(handle, option, value);
  return *outError == CURLE_OK;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

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
#if !defined(AZ_PLATFORM_WINDOWS) && !defined(AZ_PLATFORM_POSIX)
  // platform does not support Poll().
  throw TransportException("Error while sending request. Platform does not support Poll()");
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
    context.ThrowIfCancelled();
#if defined(AZ_PLATFORM_POSIX)
    result = poll(&poller, 1, interval);
#elif defined(AZ_PLATFORM_WINDOWS)
    result = WSAPoll(&poller, 1, interval);
#endif
  }
  // result can be either 0 (timeout) or > 1 (socket ready)
  return result;
}

#if defined(AZ_PLATFORM_WINDOWS)
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

    {
      using namespace Azure::Core::Logging;
      using namespace Azure::Core::Logging::Internal;
      if (ShouldLog(LogLevel::Verbose))
      {
        Log(LogLevel::Verbose,
            LogMsgPrefix + "Windows - calling setsockopt after uploading chunk. ideal = "
                + std::to_string(ideal) + " result = " + std::to_string(result));
      }
    }
  }
}
#endif
} // namespace

using Azure::Core::Context;
using Azure::Core::Http::CurlConnection;
using Azure::Core::Http::CurlConnectionPool;
using Azure::Core::Http::CurlNetworkConnection;
using Azure::Core::Http::CurlSession;
using Azure::Core::Http::CurlTransport;
using Azure::Core::Http::CurlTransportOptions;
using Azure::Core::Http::HttpStatusCode;
using Azure::Core::Http::RawResponse;
using Azure::Core::Http::Request;
using Azure::Core::Http::TransportException;

std::unique_ptr<RawResponse> CurlTransport::Send(Context const& context, Request& request)
{
  using namespace Azure::Core::Logging;
  using namespace Azure::Core::Logging::Internal;

  // Create CurlSession to perform request
  Log(LogLevel::Verbose, LogMsgPrefix + "Creating a new session.");

  auto session = std::make_unique<CurlSession>(
      request, CurlConnectionPool::GetCurlConnection(request, m_options), m_options.HttpKeepAlive);
  CURLcode performing;

  // Try to send the request. If we get CURLE_UNSUPPORTED_PROTOCOL back, it means the connection is
  // either closed or the socket is not usable any more. In that case, let the session be destroyed
  // and create a new session to get another connection from connection pool.
  // Prevent from trying forever by using DefaultMaxOpenNewConnectionIntentsAllowed.
  for (auto getConnectionOpenIntent = 0;
       getConnectionOpenIntent < Details::DefaultMaxOpenNewConnectionIntentsAllowed;
       getConnectionOpenIntent++)
  {
    performing = session->Perform(context);
    if (performing != CURLE_UNSUPPORTED_PROTOCOL)
    {
      break;
    }
    // Let session be destroyed and create a new one to get a new connection
    session = std::make_unique<CurlSession>(
        request,
        CurlConnectionPool::GetCurlConnection(request, m_options),
        m_options.HttpKeepAlive);
  }

  if (performing != CURLE_OK)
  {
    throw Azure::Core::Http::TransportException(
        "Error while sending request. " + std::string(curl_easy_strerror(performing)));
  }

  Log(LogLevel::Verbose,
      LogMsgPrefix + "Request completed. Moving response out of session and session to response.");

  // Move Response out of the session
  auto response = session->GetResponse();
  // Move the ownership of the CurlSession (bodyStream) to the response
  response->SetBodyStream(std::move(session));
  return response;
}

CURLcode CurlSession::Perform(Context const& context)
{
  using namespace Azure::Core::Logging;
  using namespace Azure::Core::Logging::Internal;

  // Set the session state
  m_sessionState = SessionState::PERFORM;

  // LibCurl settings after connection is open (headers)
  {
    auto headers = this->m_request.GetHeaders();
    auto hostHeader = headers.find("Host");
    if (hostHeader == headers.end())
    {
      Log(LogLevel::Verbose, LogMsgPrefix + "No Host in request headers. Adding it");
      this->m_request.AddHeader("Host", this->m_request.GetUrl().GetHost());
    }
    auto isContentLengthHeaderInRequest = headers.find("content-length");
    if (isContentLengthHeaderInRequest == headers.end())
    {
      Log(LogLevel::Verbose, LogMsgPrefix + "No content-length in headers. Adding it");
      this->m_request.AddHeader(
          "content-length", std::to_string(this->m_request.GetBodyStream()->Length()));
    }
  }

  // use expect:100 for PUT requests. Server will decide if it can take our request
  if (this->m_request.GetMethod() == HttpMethod::Put)
  {
    Log(LogLevel::Verbose, LogMsgPrefix + "Using 100-continue for PUT request");
    this->m_request.AddHeader("expect", "100-continue");
  }

  // Send request. If the connection assigned to this curlSession is closed or the socket is
  // somehow lost, libcurl will return CURLE_UNSUPPORTED_PROTOCOL
  // (https://curl.haxx.se/libcurl/c/curl_easy_send.html). Return the error back.
  Log(LogLevel::Verbose, LogMsgPrefix + "Send request without payload");

  auto result = SendRawHttp(context);
  if (result != CURLE_OK)
  {
    return result;
  }

  Log(LogLevel::Verbose, LogMsgPrefix + "Parse server response");
  ReadStatusLineAndHeadersFromRawResponse(context);

  // non-PUT request are ready to be stream at this point. Only PUT request would start an uploading
  // transfer where we want to maintain the `PERFORM` state.
  if (this->m_request.GetMethod() != HttpMethod::Put)
  {
    m_sessionState = SessionState::STREAMING;
    return result;
  }

  Log(LogLevel::Verbose, LogMsgPrefix + "Check server response before upload starts");
  // Check server response from Expect:100-continue for PUT;
  // This help to prevent us from start uploading data when Server can't handle it
  if (this->m_lastStatusCode != HttpStatusCode::Continue)
  {
    Log(LogLevel::Verbose, LogMsgPrefix + "Server rejected the upload request");
    m_sessionState = SessionState::STREAMING;
    return result; // Won't upload.
  }

  Log(LogLevel::Verbose, LogMsgPrefix + "Upload payload");
  if (this->m_bodyStartInBuffer > 0)
  {
    // If internal buffer has more data after the 100-continue means Server return an error.
    // We don't need to upload body, just parse the response from Server and return
    ReadStatusLineAndHeadersFromRawResponse(context, true);
    m_sessionState = SessionState::STREAMING;
    return result;
  }

  // Start upload
  result = this->UploadBody(context);
  if (result != CURLE_OK)
  {
    m_sessionState = SessionState::STREAMING;
    return result; // will throw transport exception before trying to read
  }

  Log(LogLevel::Verbose, LogMsgPrefix + "Upload completed. Parse server response");
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
      static_cast<uint16_t>(majorVersion),
      static_cast<uint16_t>(minorVersion),
      HttpStatusCode(statusCode),
      reasonPhrase);
}

// Creates an HTTP Response with specific bodyType
static std::unique_ptr<RawResponse> CreateHTTPResponse(std::string const& header)
{
  return CreateHTTPResponse(
      reinterpret_cast<const uint8_t*>(header.data()),
      reinterpret_cast<const uint8_t*>(header.data() + header.size()));
}

// Send buffer thru the wire
CURLcode CurlConnection::SendBuffer(
    Context const& context,
    uint8_t const* buffer,
    size_t bufferSize)
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
    context.ThrowIfCancelled();
    for (CURLcode sendResult = CURLE_AGAIN; sendResult == CURLE_AGAIN;)
    {
      size_t sentBytesPerRequest = 0;
      sendResult = curl_easy_send(
          m_handle, buffer + sentBytesTotal, bufferSize - sentBytesTotal, &sentBytesPerRequest);

      switch (sendResult)
      {
        case CURLE_OK: {
          sentBytesTotal += sentBytesPerRequest;
          break;
        }
        case CURLE_AGAIN: {
          // start polling operation with 1 min timeout
          auto pollUntilSocketIsReady = pollSocketUntilEventOrTimeout(
              context, m_curlSocket, PollSocketDirection::Write, 60000L);

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
        default: {
          return sendResult;
        }
      }
    };
  }
#if defined(AZ_PLATFORM_WINDOWS)
  WinSocketSetBuffSize(m_curlSocket);
#endif
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
    uploadChunkSize = Details::DefaultUploadChunkSize;
  }
  auto unique_buffer = std::make_unique<uint8_t[]>(static_cast<size_t>(uploadChunkSize));

  while (true)
  {
    auto rawRequestLen = streamBody->Read(context, unique_buffer.get(), uploadChunkSize);
    if (rawRequestLen == 0)
    {
      break;
    }
    sendResult = m_connection->SendBuffer(
        context, unique_buffer.get(), static_cast<size_t>(rawRequestLen));
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

  CURLcode sendResult = m_connection->SendBuffer(
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
        try
        {
          this->m_chunkSize = static_cast<int64_t>(std::stoull(strChunkSize, nullptr, 16));
        }
        catch (std::invalid_argument& err)
        {
          (void)err;
          // Server can return something like `\n\r\n` for a chunk of zero length data. This is
          // allowed by RFC. `stoull` will throw invalid_argument if there is not at least one hex
          // digit to be parsed. For those cases, we consider the response as zero-length.
          this->m_chunkSize = 0;
        }

        if (this->m_chunkSize == 0)
        { // Response with no content. end of chunk
          keepPolling = false;
          /*
           * The index represents the current position while reading.
           * When the chunkSize is 0, the index should have already read up to the next CRLF.
           * When reading again, we want to start reading from the next position, so we need to add
           * 1 to the index.
           */
          this->m_bodyStartInBuffer = index + 1;
          break;
        }

        if (index + 1 == this->m_innerBufferSize)
        {
          /*
           * index + 1 represents the next possition to Read. If that's equal to the inner buffer
           * size it means that there is no more data and we need to fetch more from network. And
           * whatever we fetch will be the start of the chunk data. The bodyStart is set to 0 to
           * indicate the the next read call should read from the inner buffer start.
           */
          this->m_innerBufferSize = m_connection->ReadFromSocket(
              context, this->m_readBuffer, Details::DefaultLibcurlReaderSize);
          this->m_bodyStartInBuffer = 0;
        }
        else
        {
          /*
           * index + 1 represents the next position to Read. If that's NOT equal to the inner
           * buffer size, it means that there is chunk data in the inner buffer. So, we set the
           * start to the next position to read.
           */
          this->m_bodyStartInBuffer = index + 1;
        }

        keepPolling = false;
        break;
      }
    }
    if (keepPolling)
    { // Read all internal buffer and \n was not found, pull from wire
      this->m_innerBufferSize = m_connection->ReadFromSocket(
          context, this->m_readBuffer, Details::DefaultLibcurlReaderSize);
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
      bufferSize = m_connection->ReadFromSocket(
          context, this->m_readBuffer, Details::DefaultLibcurlReaderSize);
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
        this->m_innerBufferSize = m_connection->ReadFromSocket(
            context, this->m_readBuffer, Details::DefaultLibcurlReaderSize);
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

void CurlSession::ReadExpected(Context const& context, uint8_t expected)
{
  if (this->m_bodyStartInBuffer == -1 || this->m_bodyStartInBuffer == this->m_innerBufferSize)
  {
    // end of buffer, pull data from wire
    this->m_innerBufferSize = m_connection->ReadFromSocket(
        context, this->m_readBuffer, Details::DefaultLibcurlReaderSize);
    this->m_bodyStartInBuffer = 0;
  }
  auto data = this->m_readBuffer[this->m_bodyStartInBuffer];
  if (data != expected)
  {
    throw TransportException(
        "Unexpected format in HTTP response. Expecting: " + std::to_string(expected)
        + ", but found: " + std::to_string(data) + ".");
  }
  this->m_bodyStartInBuffer += 1;
}

void CurlSession::ReadCRLF(Context const& context)
{
  ReadExpected(context, '\r');
  ReadExpected(context, '\n');
}

// Read from curl session
int64_t CurlSession::OnRead(Context const& context, uint8_t* buffer, int64_t count)
{
  if (count <= 0 || this->IsEOF())
  {
    return 0;
  }

  // check if all chunked is all read already
  if (this->m_isChunkedResponseType && this->m_chunkSize == this->m_sessionTotalRead)
  {
    ReadCRLF(context);
    // Reset session read counter for next chunk
    this->m_sessionTotalRead = 0;
    // get the size of next chunk
    ParseChunkSize(context);

    if (this->IsEOF())
    {
      /* For a chunk response, EOF means that the last chunk was found.
       *  As per RFC, after the last chunk, there should be one last CRLF
       */
      ReadCRLF(context);
      // after parsing next chunk, check if it is zero
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
  totalRead = m_connection->ReadFromSocket(context, buffer, static_cast<size_t>(readRequestLength));
  this->m_sessionTotalRead += totalRead;

  // Reading 0 bytes means closed connection.
  // For known content length and chunked response, this means there is nothing else to read
  // from server or lost connection before getting full response. For unknown response size,
  // it means the end of response and it's fine.
  if (totalRead == 0 && (this->m_contentLength > 0 || this->m_isChunkedResponseType))
  {
    auto expectedToRead = this->m_isChunkedResponseType ? this->m_chunkSize : this->m_contentLength;
    if (this->m_sessionTotalRead < expectedToRead)
    {
      throw TransportException(
          "Connection closed before getting full response or response is less than "
          "expected. "
          "Expected response length = "
          + std::to_string(expectedToRead)
          + ". Read until now = " + std::to_string(this->m_sessionTotalRead));
    }
  }

  return totalRead;
}

// Read from socket and return the number of bytes taken from socket
int64_t CurlConnection::ReadFromSocket(Context const& context, uint8_t* buffer, int64_t bufferSize)
{
  // loop until read result is not CURLE_AGAIN
  // Next loop is expected to be called at most 2 times:
  // The first time it calls `curl_easy_recv()`, if it returns CURLE_AGAIN it would call
  // `pollSocketUntilEventOrTimeout` and wait for socket to be ready to read.
  // `pollSocketUntilEventOrTimeout` will then handle cancelation token.
  // If socket is not ready before the timeout, Exception is thrown.
  // When socket is ready, it calls curl_easy_recv() again (second loop iteration). It is
  // not expected to return CURLE_AGAIN (since socket is ready), so, a chuck of data will be
  // downloaded and result will be CURLE_OK which breaks the loop. Also, getting other than
  // CURLE_OK or CURLE_AGAIN throws.
  size_t readBytes = 0;
  for (CURLcode readResult = CURLE_AGAIN; readResult == CURLE_AGAIN;)
  {
    readResult = curl_easy_recv(m_handle, buffer, static_cast<size_t>(bufferSize), &readBytes);

    switch (readResult)
    {
      case CURLE_AGAIN: {
        // start polling operation
        auto pollUntilSocketIsReady = pollSocketUntilEventOrTimeout(
            context, m_curlSocket, PollSocketDirection::Read, 60000L);

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
      case CURLE_OK: {
        break;
      }
      default: {
        // Error reading from socket
        throw TransportException(
            "Error while reading from network socket. CURLE code: " + std::to_string(readResult)
            + ". " + std::string(curl_easy_strerror(readResult)));
      }
    }
  }
#if defined(AZ_PLATFORM_WINDOWS)
  WinSocketSetBuffSize(m_curlSocket);
#endif
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
          // Should never happen that parser is not statusLIne or Headers and we still try
          // to parse more.
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
          // Should never happen that parser is not statusLIne or Headers and we still try
          // to parse more.
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
    // rare case of using buffer of size 1 to read. In this case, if the char is next value
    // after headers or previous header, just consider it as read and return
    return bufferSize;
  }
  else if (bufferSize > 1 && this->m_internalBuffer.size() == 0) // only if nothing in
                                                                 // buffer, advance
  {
    // move offset one possition. This is because readStatusLine and readHeader will read up
    // to
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
    return 1; // can't return more than the found delimiter. On read remaining we need to
              // also remove first char
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

std::mutex CurlConnectionPool::ConnectionPoolMutex;
std::map<std::string, std::list<std::unique_ptr<CurlNetworkConnection>>>
    CurlConnectionPool::ConnectionPoolIndex;
int32_t CurlConnectionPool::s_connectionCounter = 0;
bool CurlConnectionPool::s_isCleanConnectionsRunning = false;

namespace {
inline std::string GetConnectionKey(std::string const& host, CurlTransportOptions const& options)
{
  std::string key(host);
  if (!options.CAInfo.empty())
  {
    key.append(options.CAInfo);
  }
  else
  {
    key.append("0");
  }
  if (!options.Proxy.empty())
  {
    key.append(options.Proxy);
  }
  else
  {
    key.append("0");
  }
  if (!options.SSLOptions.EnableCertificateRevocationListCheck)
  {
    key.append("1");
  }
  else
  {
    key.append("0");
  }
  if (options.SSLVerifyPeer)
  {
    key.append("1");
  }
  else
  {
    key.append("0");
  }
  return key;
}
} // namespace

std::unique_ptr<CurlNetworkConnection> CurlConnectionPool::GetCurlConnection(
    Request& request,
    CurlTransportOptions const& options)
{
  std::string const& host = request.GetUrl().GetHost();
  std::string const connectionKey = GetConnectionKey(host, options);

  {
    // Critical section. Needs to own ConnectionPoolMutex before executing
    // Lock mutex to access connection pool. mutex is unlock as soon as lock is out of scope
    std::lock_guard<std::mutex> lock(CurlConnectionPool::ConnectionPoolMutex);

    // get a ref to the pool from the map of pools
    auto hostPoolIndex = CurlConnectionPool::ConnectionPoolIndex.find(connectionKey);
    if (hostPoolIndex != CurlConnectionPool::ConnectionPoolIndex.end()
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
        CurlConnectionPool::ConnectionPoolIndex.erase(hostPoolIndex);
      }

      // return connection ref
      return connection;
    }
  }

  // Creating a new connection is thread safe. No need to lock mutex here.
  // No available connection for the pool for the required host. Create one
  CURL* newHandle = curl_easy_init();
  if (!newHandle)
  {
    throw Azure::Core::Http::TransportException(
        Details::DefaultFailedToGetNewConnectionTemplate + host + ". "
        + std::string("curl_easy_init returned Null"));
  }
  CURLcode result;

  // Libcurl setup before open connection (url, connect_only, timeout)
  if (!SetLibcurlOption(newHandle, CURLOPT_URL, request.GetUrl().GetAbsoluteUrl().data(), &result))
  {
    throw Azure::Core::Http::TransportException(
        Details::DefaultFailedToGetNewConnectionTemplate + host + ". "
        + std::string(curl_easy_strerror(result)));
  }

  uint16_t port = request.GetUrl().GetPort();
  if (port != 0 && !SetLibcurlOption(newHandle, CURLOPT_PORT, port, &result))
  {
    throw Azure::Core::Http::TransportException(
        Details::DefaultFailedToGetNewConnectionTemplate + host + ". "
        + std::string(curl_easy_strerror(result)));
  }

  if (!SetLibcurlOption(newHandle, CURLOPT_CONNECT_ONLY, 1L, &result))
  {
    throw Azure::Core::Http::TransportException(
        Details::DefaultFailedToGetNewConnectionTemplate + host + ". "
        + std::string(curl_easy_strerror(result)));
  }

  // Set timeout to 24h. Libcurl will fail uploading on windows if timeout is:
  // timeout >= 25 days. Fails as soon as trying to upload any data
  // 25 days < timeout > 1 days. Fail on huge uploads ( > 1GB)
  if (!SetLibcurlOption(newHandle, CURLOPT_TIMEOUT, 60L * 60L * 24L, &result))
  {
    throw Azure::Core::Http::TransportException(
        Details::DefaultFailedToGetNewConnectionTemplate + host + ". "
        + std::string(curl_easy_strerror(result)));
  }

  /******************** Curl handle options apply to all connections created
   * The keepAlive option is managed by the session directly.
   */
  if (!options.Proxy.empty())
  {
    if (!SetLibcurlOption(newHandle, CURLOPT_PROXY, options.Proxy.c_str(), &result))
    {
      throw Azure::Core::Http::TransportException(
          Details::DefaultFailedToGetNewConnectionTemplate + host + ". Failed to set proxy to:"
          + options.Proxy + ". " + std::string(curl_easy_strerror(result)));
    }
  }

  if (!options.CAInfo.empty())
  {
    if (!SetLibcurlOption(newHandle, CURLOPT_CAINFO, options.CAInfo.c_str(), &result))
    {
      throw Azure::Core::Http::TransportException(
          Details::DefaultFailedToGetNewConnectionTemplate + host + ". Failed to set CA cert to:"
          + options.CAInfo + ". " + std::string(curl_easy_strerror(result)));
    }
  }

  long sslOption = 0;
  if (!options.SSLOptions.EnableCertificateRevocationListCheck)
  {
    sslOption |= CURLSSLOPT_NO_REVOKE;
  }

  if (!SetLibcurlOption(newHandle, CURLOPT_SSL_OPTIONS, sslOption, &result))
  {
    throw Azure::Core::Http::TransportException(
        Details::DefaultFailedToGetNewConnectionTemplate + host
        + ". Failed to set ssl options to long bitmask:" + std::to_string(sslOption) + ". "
        + std::string(curl_easy_strerror(result)));
  }

  if (!options.SSLVerifyPeer)
  {
    if (!SetLibcurlOption(newHandle, CURLOPT_SSL_VERIFYPEER, 0L, &result))
    {
      throw Azure::Core::Http::TransportException(
          Details::DefaultFailedToGetNewConnectionTemplate + host
          + ". Failed to disable ssl verify peer." + ". "
          + std::string(curl_easy_strerror(result)));
    }
  }

  auto performResult = curl_easy_perform(newHandle);
  if (performResult != CURLE_OK)
  {
    throw Http::TransportException(
        Details::DefaultFailedToGetNewConnectionTemplate + host + ". "
        + std::string(curl_easy_strerror(performResult)));
  }

  return std::make_unique<CurlConnection>(newHandle, std::move(connectionKey));
}

// Move the connection back to the connection pool. Push it to the front so it becomes the
// first connection to be picked next time some one ask for a connection to the pool (LIFO)
void CurlConnectionPool::MoveConnectionBackToPool(
    std::unique_ptr<CurlNetworkConnection> connection,
    HttpStatusCode lastStatusCode)
{
  auto code = static_cast<std::underlying_type<Http::HttpStatusCode>::type>(lastStatusCode);
  // laststatusCode = 0
  if (code < 200 || code >= 300)
  {
    // A handler with previous response with Error can't be re-use.
    return;
  }

  // Lock mutex to access connection pool. mutex is unlock as soon as lock is out of scope
  std::lock_guard<std::mutex> lock(CurlConnectionPool::ConnectionPoolMutex);
  auto& poolId = connection->GetConnectionKey();
  auto& hostPool = CurlConnectionPool::ConnectionPoolIndex[poolId];
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
          std::chrono::milliseconds(Details::DefaultCleanerIntervalMilliseconds));

      {
        // take mutex for reading the pool
        std::lock_guard<std::mutex> lock(CurlConnectionPool::ConnectionPoolMutex);

        if (CurlConnectionPool::s_connectionCounter == 0)
        {
          // stop the cleaner since there are no connections
          CurlConnectionPool::s_isCleanConnectionsRunning = false;
          return;
        }

        // loop the connection pool index
        for (auto index = CurlConnectionPool::ConnectionPoolIndex.begin();
             index != CurlConnectionPool::ConnectionPoolIndex.end();
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
            // loop starts at end(), go back to previous possition. We know the list is
            // size() > 0 so we are safe to go end() - 1 and find the last element in the
            // list
            connection--;
            if (connection->get()->isExpired())
            {
              // remove connection from the pool and update the connection to the next one
              // which is going to be list.end()
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
              // Got a non-expired connection, all connections before this one are not
              // expired. Break the loop and continue looping the Pool index
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
