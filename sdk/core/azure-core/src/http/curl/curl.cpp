
#include "http/curl/curl.hpp"

#include "azure.hpp"
#include "http/http.hpp"

#include <string>

using namespace Azure::Core::Http;

std::unique_ptr<Response> CurlTransport::Send(Context& context, Request& request)
{
  // Create CurlSession to perform request
  auto session = std::make_unique<CurlSession>(request);

  auto performing = session->Perform(context);

  if (performing != CURLE_OK)
  {
    switch (performing)
    {
      case CURLE_COULDNT_RESOLVE_HOST: {
        throw Azure::Core::Http::CouldNotResolveHostException();
      }
      case CURLE_WRITE_ERROR: {
        throw Azure::Core::Http::ErrorWhileWrittingResponse();
      }
      default: {
        throw Azure::Core::Http::TransportException();
      }
    }
  }

  // Move Response out of the session
  auto response = session->GetResponse();
  // Move the ownership of the CurlSession (bodyStream) to the response
  response->SetBodyStream(std::move(session));
  return response;
}

CURLcode CurlSession::Perform(Context& context)
{
  AZURE_UNREFERENCED_PARAMETER(context);

  // Working with Body Buffer. let Libcurl use the classic callback to read/write
  auto settingUp = SetUrl();
  if (settingUp != CURLE_OK)
  {
    return settingUp;
  }

  // Make sure host is set
  // TODO-> use isEqualNoCase here once it is merged
  {
    auto headers = this->m_request.GetHeaders();
    auto hostHeader = headers.find("Host");
    if (hostHeader == headers.end())
    {
      this->m_request.AddHeader("Host", this->m_request.GetHost());
    }
  }

  settingUp = SetConnectOnly();
  if (settingUp != CURLE_OK)
  {
    return settingUp;
  }

  // establish connection only (won't send or receive anything yet)
  settingUp = curl_easy_perform(this->m_pCurl);
  if (settingUp != CURLE_OK)
  {
    return settingUp;
  }
  // Record socket to be used
  settingUp = curl_easy_getinfo(this->m_pCurl, CURLINFO_ACTIVESOCKET, &this->m_curlSocket);
  if (settingUp != CURLE_OK)
  {
    return settingUp;
  }

  // Send request
  settingUp = HttpRawSend(context);
  if (settingUp != CURLE_OK)
  {
    return settingUp;
  }
  return ReadStatusLineAndHeadersFromRawResponse();
}

// Creates an HTTP Response with specific bodyType
static std::unique_ptr<Response> CreateHTTPResponse(
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
  return std::make_unique<Response>(
      (uint16_t)majorVersion, (uint16_t)minorVersion, HttpStatusCode(statusCode), reasonPhrase);
}

// Creates an HTTP Response with specific bodyType
static std::unique_ptr<Response> CreateHTTPResponse(std::string const& header)
{
  return CreateHTTPResponse(
      reinterpret_cast<const uint8_t* const>(header.begin().base()),
      reinterpret_cast<const uint8_t* const>(header.end().base()));
}

// To wait for a socket to be ready to be read/write
static int WaitForSocketReady(curl_socket_t sockfd, int for_recv, long timeout_ms)
{
  struct timeval tv;
  fd_set infd, outfd, errfd;
  int res;

  tv.tv_sec = timeout_ms / 1000;
  tv.tv_usec = (timeout_ms % 1000) * 1000;

  FD_ZERO(&infd);
  FD_ZERO(&outfd);
  FD_ZERO(&errfd);

  FD_SET(sockfd, &errfd); /* always check for error */

  if (for_recv)
  {
    FD_SET(sockfd, &infd);
  }
  else
  {
    FD_SET(sockfd, &outfd);
  }

  /* select() returns the number of signalled sockets or -1 */
  res = select((int)sockfd + 1, &infd, &outfd, &errfd, &tv);
  return res;
}

bool CurlSession::isUploadRequest()
{
  return this->m_request.GetMethod() == HttpMethod::Put
      || this->m_request.GetMethod() == HttpMethod::Post;
}

CURLcode CurlSession::SetUrl()
{
  return curl_easy_setopt(this->m_pCurl, CURLOPT_URL, this->m_request.GetEncodedUrl().c_str());
}

CURLcode CurlSession::SetConnectOnly()
{
  return curl_easy_setopt(this->m_pCurl, CURLOPT_CONNECT_ONLY, 1L);
}

// Send buffer thru the wire
CURLcode CurlSession::SendBuffer(uint8_t const* buffer, size_t bufferSize)
{

  for (size_t sentBytesTotal = 0; sentBytesTotal < bufferSize;)
  {
    for (CURLcode sendResult = CURLE_AGAIN; sendResult == CURLE_AGAIN;)
    {
      size_t sentBytesPerRequest = 0;
      sendResult = curl_easy_send(
          this->m_pCurl,
          buffer + sentBytesTotal,
          bufferSize - sentBytesTotal,
          &sentBytesPerRequest);

      switch (sendResult)
      {
        case CURLE_OK:
          sentBytesTotal += sentBytesPerRequest;
          break;
        case CURLE_AGAIN:
          if (!WaitForSocketReady(this->m_curlSocket, 0, 60000L))
          {
            // TODO: Change this to somehing more relevant
            throw;
          }
          break;
        default:
          return sendResult;
      }
    };
  }

  return CURLE_OK;
}

// custom sending to wire an http request
CURLcode CurlSession::HttpRawSend(Context& context)
{
  // something like GET /path HTTP1.0 \r\nheaders\r\n
  auto rawRequest = this->m_request.GetHTTPMessagePreBody();
  int64_t rawRequestLen = rawRequest.size();

  CURLcode sendResult = SendBuffer(
      reinterpret_cast<uint8_t const*>(rawRequest.data()), static_cast<size_t>(rawRequestLen));

  auto streamBody = this->m_request.GetBodyStream();
  if (streamBody->Length() == 0)
  {
    // Finish request with no body
    uint8_t const endOfRequest = 0;
    return SendBuffer(&endOfRequest, 1); // need one more byte to end request
  }

  // Send body UploadStreamPageSize at a time (libcurl default)
  // NOTE: if stream is on top a contiguous memory, we can avoid allocating this copying buffer
  auto unique_buffer = std::make_unique<uint8_t[]>(UploadStreamPageSize);
  // reusing rawRequestLen variable to read
  while (true)
  {
    rawRequestLen = streamBody->Read(context, unique_buffer.get(), UploadStreamPageSize);
    if (rawRequestLen == 0)
    {
      break;
    }
    sendResult = SendBuffer(unique_buffer.get(), static_cast<size_t>(rawRequestLen));
  }
  return sendResult;
}

// Read status line plus headers to create a response with no body
CURLcode CurlSession::ReadStatusLineAndHeadersFromRawResponse()
{
  auto parser = ResponseBufferParser();
  auto bufferSize = int64_t();
  // Select a default reading strategy. // No content-length or Transfer-Encoding
  this->m_bodyLengthType = ResponseBodyLengthType::ReadToCloseConnection;

  // Keep reading until all headers were read
  while (!parser.IsParseCompleted())
  {
    // Try to fill internal buffer from socket.
    // If response is smaller than buffer, we will get back the size of the response
    bufferSize = ReadSocketToBuffer(this->m_readBuffer, LibcurlReaderSize);

    // returns the number of bytes parsed
    auto bytesParsed = parser.Parse(this->m_readBuffer, static_cast<size_t>(bufferSize));
    // if end of headers is reach before the end of response, that's where body start
    // bytesParsed returns the index of \r
    // +2 adds \r and \n to the index and check if it is bigger then internal buffer
    if (bytesParsed + 2 < bufferSize)
    {
      this->m_bodyStartInBuffer = bytesParsed + 1; // Set the start of body (skip \r)
    }
  }

  this->m_response = parser.GetResponse();
  this->m_innerBufferSize = static_cast<size_t>(bufferSize);

  // For Head request, set the length of body response to 0.
  // Response will give us content-length as if we were not doing Head saying what would it be the
  // length of the body. However, Server won't send body
  if (this->m_request.GetMethod() == HttpMethod::Head)
  {
    this->m_contentLength = 0;
    return CURLE_OK;
  }

  // TODO: tolower ContentLength
  auto headers = this->m_response->GetHeaders();

  auto isContentLengthHeaderInResponse = headers.find("Content-Length");
  if (isContentLengthHeaderInResponse != headers.end())
  {
    this->m_contentLength
        = static_cast<int64_t>(std::stoull(isContentLengthHeaderInResponse->second.data()));
    this->m_bodyLengthType = ResponseBodyLengthType::ContentLength;
    return CURLE_OK;
  }

  this->m_contentLength = -1;
  auto isTransferEncodingHeaderInResponse = headers.find("Transfer-Encoding");
  if (isTransferEncodingHeaderInResponse != headers.end())
  {
    auto headerValue = isTransferEncodingHeaderInResponse->second;
    auto isChunked = headerValue.find("chunked");

    if (isChunked != std::string::npos)
    {
      // set curl session to know response is chunked
      // This will be used to remove chunked info while reading
      this->m_bodyLengthType = ResponseBodyLengthType::Chunked;
      return CURLE_OK;
    }
  }

  /*
  https://tools.ietf.org/html/rfc7230#section-3.3.3
   7.  Otherwise, this is a response message without a declared message
       body length, so the message body length is determined by the
       number of octets received prior to the server closing the
       connection.
  */

  // Use unknown size CurlBodyStream. CurlSession will use the ResponseBodyLengthType to select a
  // reading strategy
  this->m_bodyLengthType = ResponseBodyLengthType::ReadToCloseConnection;
  return CURLE_OK;
}

// Read from curl session
int64_t CurlSession::Read(Azure::Core::Context& context, uint8_t* buffer, int64_t count)
{
  context.ThrowIfCanceled();
  auto totalRead = int64_t();

  // Take data from inner buffer if any
  if (this->m_bodyStartInBuffer > 0)
  {
    if (this->m_readBuffer[this->m_bodyStartInBuffer] == '\n' && this->m_sessionTotalRead == 0)
    {
      // first read. Need to move to next position
      if (this->m_bodyLengthType == ResponseBodyLengthType::Chunked)
      {
        // For chunked, first advance until next `\r` after chunked size (\nsomeNumber\r\n)
        auto nextPosition = std::find(
            this->m_readBuffer + this->m_bodyStartInBuffer,
            this->m_readBuffer + this->m_innerBufferSize,
            '\r');
        if (nextPosition != this->m_readBuffer + this->m_innerBufferSize)
        {
          // Found possition of next '\r', +1 jumps the \r
          this->m_bodyStartInBuffer
              += std::distance(this->m_readBuffer + this->m_bodyStartInBuffer, nextPosition) + 1;

          // Check if the end of body is also at inner buffer
          auto endOfChunk = std::find(
              this->m_readBuffer + this->m_bodyStartInBuffer,
              this->m_readBuffer + this->m_innerBufferSize,
              '\r');
          if (endOfChunk != this->m_readBuffer + this->m_innerBufferSize)
          {
            this->m_innerBufferSize
                -= std::distance(endOfChunk, this->m_readBuffer + this->m_innerBufferSize);
          }
        } // TODO: else read from wire until next \r
      }
      this->m_bodyStartInBuffer += 1;
    }
    if (this->m_bodyStartInBuffer < this->m_innerBufferSize)
    {
      // still have data to take from innerbuffer
      MemoryBodyStream innerBufferMemoryStream(
          this->m_readBuffer + this->m_bodyStartInBuffer,
          this->m_innerBufferSize - this->m_bodyStartInBuffer);
      totalRead = innerBufferMemoryStream.Read(context, buffer, count);
      this->m_bodyStartInBuffer += totalRead;
      this->m_sessionTotalRead += totalRead;
      if (this->m_bodyStartInBuffer == this->m_innerBufferSize)
      {
        this->m_bodyStartInBuffer = 0; // read everyting from inner buffer already
      }
      return totalRead;
    }
    // After moving the reading start we reached the end
    this->m_bodyStartInBuffer = 0;
  }

  // if the last position in inner buffer is `\r` it means the next
  // thing we read from wire is `\n`. (usually this is when reading 1byte per time from wire)
  if (this->m_readBuffer[this->m_innerBufferSize - 1] == '\r')
  {
    // Read one possition from socket on same user buffer, We wil override the value after this
    ReadSocketToBuffer(buffer, 1);
  }

  // Head request have contentLength = 0, so we won't read more, just return 0
  // Also if we have already read all contentLength
  if (this->m_sessionTotalRead == this->m_contentLength)
  {
    // Read everything already
    return 0;
  }

  // Read from socket
  // [status + header\r\n\r]
  // [\nbody]
  totalRead = ReadSocketToBuffer(buffer, static_cast<size_t>(count));
  this->m_sessionTotalRead += totalRead;

  if (this->m_bodyLengthType == ResponseBodyLengthType::Chunked && totalRead > 0)
  {
    // Check if the end of chunked is part of the body
    auto endOfBody = std::find(buffer, buffer + totalRead, '\r');
    if (endOfBody != buffer + totalRead)
    {
      // End of stream is when chunk is 0 (\r\n0\r\n)
      if (buffer[0] == '0' && buffer + 1 == endOfBody)
      {
        // got already the end
        return 0;
      }
      // Read all remaining to close connection
      {
        constexpr int64_t finalRead = 50; // usually only 5 more bytes are gotten "0\r\n\r\n"
        uint8_t b[finalRead];
        ReadSocketToBuffer(b, finalRead);
      }
      totalRead -= std::distance(endOfBody, buffer + totalRead);
      this->m_bodyLengthType = ResponseBodyLengthType::ContentLength;
      this->m_contentLength = this->m_sessionTotalRead;
    }
  }
  return totalRead;
}

// Read from socket and return the number of bytes taken from socket
int64_t CurlSession::ReadSocketToBuffer(uint8_t* buffer, int64_t bufferSize)
{
  // loop until read result is not CURLE_AGAIN
  size_t readBytes = 0;
  for (CURLcode readResult = CURLE_AGAIN; readResult == CURLE_AGAIN;)
  {
    readResult = curl_easy_recv(this->m_pCurl, buffer, static_cast<size_t>(bufferSize), &readBytes);

    switch (readResult)
    {
      case CURLE_AGAIN:
        if (!WaitForSocketReady(this->m_curlSocket, 0, 60000L))
        {
          // TODO: Change this to somehing more relevant
          throw;
        }
        break;
      case CURLE_OK:
        break;
      default:
        // Error code while reading from socket
        readBytes = -1;
        break;
    }
  }
  return readBytes;
}

std::unique_ptr<Azure::Core::Http::Response> CurlSession::GetResponse()
{
  return std::move(this->m_response);
}

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
          // Add header. TODO: Do toLower so all headers are lowerCase
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

          // Add header. TODO: Do toLower so all headers are lowerCase
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
    // internalBuffer containst the status line and we don't need to add anything else
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
    // '\r' then next delimeter is '\n' and we don't care
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
    // internalBuffer containst the status line and we don't need to add anything else
    if (indexOfEndOfStatusLine > buffer)
    {
      // Append and build response minus the delimiter
      this->m_internalBuffer.append(start, indexOfEndOfStatusLine);
    }
    this->m_response->AddHeader(this->m_internalBuffer);
  }
  else
  {
    // Internal Buffer was not required, create response directly from buffer
    this->m_response->AddHeader(std::string(start, indexOfEndOfStatusLine));
  }

  // reuse buffer
  this->m_internalBuffer.clear();

  // Return the index of the next char to read after delimiter
  // No need to advance one more char ('\n') (since we might be at the end of the array)
  // Parsing Headers will make sure to move one possition
  return indexOfEndOfStatusLine + 1 - buffer;
}
