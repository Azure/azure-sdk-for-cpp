
#include "http/curl/curl.hpp"

#include "azure.hpp"
#include "http/http.hpp"

#include <string>

using namespace Azure::Core::Http;

std::unique_ptr<Response> CurlTransport::Send(Context& context, Request& request)
{
  // Create CurlSession in heap. We will point to it from response's stream to keep it alive
  CurlSession* session = new CurlSession(request);

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

  return session->GetResponse();
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

  settingUp = SetConnectOnly();
  if (settingUp != CURLE_OK)
  {
    return settingUp;
  }

  // stablish connection only (won't send or receive nothing yet)
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
  settingUp = HttpRawSend();
  if (settingUp != CURLE_OK)
  {
    return settingUp;
  }
  this->m_rawResponseEOF = false; // Control EOF for response;
  return ReadStatusLineAndHeadersFromRawResponse();
}

// Creates an HTTP Response with specific bodyType
static std::unique_ptr<Response> CreateHTTPResponse(std::string const& header)
{
  // set response code, http version and reason phrase (i.e. HTTP/1.1 200 OK)
  auto start = header.begin() + 5; // HTTP = 4, / = 1, moving to 5th place for version
  auto end = std::find(start, header.end(), '.');
  auto majorVersion = std::stoi(std::string(start, end));

  start = end + 1; // start of minor version
  end = std::find(start, header.end(), ' ');
  auto minorVersion = std::stoi(std::string(start, end));

  start = end + 1; // start of status code
  end = std::find(start, header.end(), ' ');
  auto statusCode = std::stoi(std::string(start, end));

  start = end + 1; // start of reason phrase
  end = std::find(start, header.end(), '\r');
  auto reasonPhrase = std::string(start, end); // remove \r

  // allocate the instance of response to heap with shared ptr
  // So this memory gets delegated outside Curl Transport as a shared ptr so memory will be
  // eventually released
  return std::make_unique<Response>(
      (uint16_t)majorVersion, (uint16_t)minorVersion, HttpStatusCode(statusCode), reasonPhrase);
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
CURLcode CurlSession::SendBuffer(uint8_t* buffer, size_t bufferSize)
{
  if (bufferSize <= 0)
  {
    return CURLE_OK;
  }

  size_t sentBytesTotal = 0;
  CURLcode sendResult;

  do
  {
    size_t sentBytesPerRequest;
    do
    {
      sentBytesPerRequest = 0;
      auto sendFrom = buffer + sentBytesTotal;
      auto remainingBytes = bufferSize - sentBytesTotal;

      sendResult = curl_easy_send(this->m_pCurl, sendFrom, remainingBytes, &sentBytesPerRequest);
      sentBytesTotal += sentBytesPerRequest;

      if (sendResult == CURLE_AGAIN && !WaitForSocketReady(this->m_curlSocket, 0, 60000L))
      {
        throw;
      }
    } while (sendResult == CURLE_AGAIN);

    if (sendResult != CURLE_OK)
    {
      return sendResult;
    }

  } while (sentBytesTotal < bufferSize);

  return CURLE_OK;
}

// custom sending to wire an http request
CURLcode CurlSession::HttpRawSend()
{
  auto rawRequest = this->m_request.GetHTTPMessagePreBody();
  uint64_t rawRequestLen = rawRequest.size();

  CURLcode sendResult = SendBuffer((uint8_t*)rawRequest.data(), (size_t)rawRequestLen);
  if (this->m_request.GetMethod() == HttpMethod::Get)
  {
    uint8_t endOfRequest[] = "0";
    return SendBuffer(endOfRequest, 1); // need one more byte to end request
  }

  auto streamBody = this->m_request.GetBodyStream();
  // Send body 64k at a time (libcurl default)
  // NOTE: if stream is on top a contiguous memory, we can avoid allocating this copying buffer
  std::unique_ptr<uint8_t[]> unique_buffer(new uint8_t[UploadSstreamPageSize]);
  auto buffer = unique_buffer.get();
  while (rawRequestLen > 0)
  {
    rawRequestLen = streamBody->Read(buffer, UploadSstreamPageSize);
    sendResult = SendBuffer(buffer, (size_t)rawRequestLen);
  }
  return sendResult;
}

// Read status line plus headers to create a response with no body
CURLcode CurlSession::ReadStatusLineAndHeadersFromRawResponse()
{
  auto parser = ResponseBufferParser();

  // Keep reading until all headers were read
  while (!parser.IsParseCompleted())
  {
    // Try to fill internal buffer from socket.
    // If response is smaller than buffer, we will get back the size of the response
    auto bufferSize = ReadSocketToBuffer(this->m_readBuffer, LibcurlReaderSize);

    // parse from buffer to create response
    auto bytesParsed = parser.Parse(this->m_readBuffer, (size_t)bufferSize);
    // if end of headers is reach before the end of response, that's where body start
    if (bytesParsed < bufferSize)
    {
      this->m_bodyStartInBuffer = bytesParsed + 1; // Set the start of body (skip \n)
    }
  }

  this->m_response = parser.GetResponse();
  // TODO: tolower ContentLength
  auto headers = this->m_response->GetHeaders();
  auto bodySize = atoi(headers.at("Content-Length").data());
  // content-length is used later by session and session won't have access to the response any more
  // (unique_ptr), so we save this value
  this->m_contentLength = bodySize;
  // Move session to live inside the stream from response.
  this->m_response->SetBodyStream(new CurlBodyStream(bodySize, this));

  return CURLE_OK;
}

uint64_t CurlSession::ReadWithOffset(uint8_t* buffer, uint64_t bufferSize, uint64_t offset)
{
  if (bufferSize <= 0)
  {
    return 0;
  }

  // calculate where to start reading from inner buffer
  auto fixedOffset
      = offset == 0 ? offset + 1 : offset; // advance the last '\n' from headers end on first read
  auto innerBufferStart = this->m_bodyStartInBuffer + fixedOffset;
  // total size from content-length header less any bytes already read
  auto remainingBodySize = this->m_contentLength - fixedOffset;

  // set ptr for writting
  auto writePosition = buffer;
  // Set the max to be written as the size of remaining body size
  auto bytesToWrite = std::min<>(bufferSize, remainingBodySize);

  // total of bytes read (any from inner buffer plus any from socket)
  uint64_t bytesRead = uint64_t();

  // If bodyStartInBuffer is set and while innerBufferStart is less than the buffer, it means there
  // is still data at innerbuffer for the body that is not yet read
  if (this->m_bodyStartInBuffer > 0 && LibcurlReaderSize > innerBufferStart)
  {
    // Calculate the right size of innerBuffer:
    // It can be smaller than the total body, in that case we will take as much as the size of
    // buffer
    // It can be bugger than the total body, in that case we will take as much as the size of the
    // body
    auto innerBufferWithBodyContent = LibcurlReaderSize - innerBufferStart;
    auto innerbufferSize = remainingBodySize < innerBufferWithBodyContent
        ? remainingBodySize
        : innerBufferWithBodyContent;

    // Requested less data than what we have at inner buffer, take it from innerBuffer
    if (bufferSize <= innerbufferSize)
    {
      std::memcpy(writePosition, this->m_readBuffer + innerBufferStart, (size_t)bytesToWrite);
      return bytesToWrite;
    }
    // Requested more data than what we have at innerbuffer. Take all from inner buffer and continue
    std::memcpy(writePosition, this->m_readBuffer + innerBufferStart, (size_t)innerbufferSize);

    // Return if all body was read and theres not need to read socket
    if (innerbufferSize == remainingBodySize)
    {
      // libcurl handle can be clean now. We won't request more data
      curl_easy_cleanup(this->m_pCurl);
      return innerbufferSize;
    }

    // next write will be done after reading from socket, move ptr to where to write and how many
    // to write
    writePosition += innerbufferSize;
    bytesToWrite -= innerbufferSize;
    bytesRead += innerbufferSize;
  }

  // read from socket the remaining requested bytes
  bytesRead += ReadSocketToBuffer(writePosition, (size_t)bytesToWrite);
  if (remainingBodySize - bytesRead == 0)
  {
    // No more to read from socket
    curl_easy_cleanup(this->m_pCurl);
  }

  return bytesRead;
}

// Read from socket until buffer is full or until socket has no more data
uint64_t CurlSession::ReadSocketToBuffer(uint8_t* buffer, size_t bufferSize)
{
  CURLcode readResult;
  size_t readBytes = 0;
  size_t totalRead = 0;
  auto pendingToRead = bufferSize;

  while (!this->m_rawResponseEOF && pendingToRead > 0)
  {
    do // try to read from socket until response is OK
    {
      readResult = curl_easy_recv(this->m_pCurl, buffer + totalRead, pendingToRead, &readBytes);

      // socket not ready. Wait or fail on timeout
      if (readResult == CURLE_AGAIN && !WaitForSocketReady(this->m_curlSocket, 1, 60000L))
      {
        throw;
      }
    } while (readResult == CURLE_AGAIN); // Keep trying to read until result is not CURLE_AGAIN

    // At this point we read, update counters
    totalRead += readBytes;
    pendingToRead -= readBytes;

    if (readBytes == 0)
    {
      // set socket as nothing more to read
      this->m_rawResponseEOF = true;
      break;
    }
  }

  return totalRead;
}

std::unique_ptr<Azure::Core::Http::Response> CurlSession::GetResponse()
{
  if (this->m_response != nullptr)
  {
    return std::move(this->m_response);
  }
  return nullptr;
}

size_t CurlSession::ResponseBufferParser::Parse(
    uint8_t const* const buffer,
    size_t const bufferSize)
{
  if (this->m_parseCompleted)
  {
    return 0;
  }

  switch (this->state)
  {
    case ResponseParserState::StatusLine: {
      auto parsedBytes = BuildStatusCode(buffer, bufferSize);
      if (parsedBytes < bufferSize) // status code is built and buffer can be still parsed
      {
        // Can keep parsing, Control have moved to headers
        return parsedBytes + Parse(buffer + parsedBytes, bufferSize - parsedBytes);
      }
      return parsedBytes;
    }
    case ResponseParserState::Headers: {
      auto parsedBytes = BuildHeader(buffer, bufferSize);
      if (!this->m_parseCompleted
          && parsedBytes < bufferSize) // status code is built and buffer can be still parsed
      {
        // Can keep parsing, Control have moved to headers
        return parsedBytes + Parse(buffer + parsedBytes, bufferSize - parsedBytes);
      }
      return parsedBytes;
    }
    case ResponseParserState::EndOfHeaders:
    default: {
      return 0;
    }
  }
}

// Finds delimiter '\r' as the end of the
size_t CurlSession::ResponseBufferParser::BuildStatusCode(
    uint8_t const* const buffer,
    size_t const bufferSize)
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
size_t CurlSession::ResponseBufferParser::BuildHeader(
    uint8_t const* const buffer,
    size_t const bufferSize)
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
