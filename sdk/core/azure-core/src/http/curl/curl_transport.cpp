
#include "azure.hpp"
#include "http/curl/curl.hpp"
#include "http/http.hpp"

#include <string>

using namespace Azure::Core::Http;

CurlTransport::CurlTransport() : HttpTransport() {}

CurlTransport::~CurlTransport() {}

std::unique_ptr<Response> CurlTransport::Send(Context& context, Request& request)
{
  // Create a new CurlSession. This is shared ptr because it will be referenced within the
  // CurlBodyStream inside the response when working with streams
  std::shared_ptr<CurlSession> session = std::make_shared<CurlSession>(request);

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

  // When response uses Stream, the stream will contain a ref to the session to keep reading from
  // socket using the same session.
  // If not working with streams, response will contain just the bodyBuffer and ptr to session will
  // be deleted
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

  // If working with streams, set request to use send and receive as customs HTTP protocol
  if (this->m_request.GetResponseBodyType() == BodyType::Stream)
  {
    settingUp = SetConnectOnly();
    if (settingUp != CURLE_OK)
    {
      return settingUp;
    }

    // stablish connection only (won't send or receive nothing yet)
    settingUp = curl_easy_perform(m_pCurl);
    if (settingUp != CURLE_OK)
    {
      return settingUp;
    }
    // Record socket to be used
    settingUp = curl_easy_getinfo(m_pCurl, CURLINFO_ACTIVESOCKET, &this->m_curlSocket);
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

  settingUp = SetMethod();
  if (settingUp != CURLE_OK)
  {
    return settingUp;
  }

  settingUp = SetHeaders();
  if (settingUp != CURLE_OK)
  {
    return settingUp;
  }

  settingUp = SetWriteResponse();
  if (settingUp != CURLE_OK)
  {
    return settingUp;
  }

  // Set ReadCallback for POST and PUT
  if (isUploadRequest())
  {
    settingUp = SetReadRequest();
    if (settingUp != CURLE_OK)
    {
      return settingUp;
    }
  }

  return curl_easy_perform(m_pCurl);
}

// Creates an HTTP Response with specific bodyType
static std::unique_ptr<Response> CreateHTTPResponse(std::string const& header, BodyType bodyType)
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
      (uint16_t)majorVersion,
      (uint16_t)minorVersion,
      HttpStatusCode(statusCode),
      reasonPhrase,
      bodyType);
}

// Creates an HTTP Response default to BodyBuffer type
static std::unique_ptr<Response> CreateHTTPResponse(std::string const& header)
{
  return CreateHTTPResponse(header, BodyType::Buffer);
}

// Callback function for curl. This is called for every header that curl get from network.
// This is only used when working with body buffer
size_t CurlSession::WriteHeadersCallBack(void* contents, size_t size, size_t nmemb, void* userp)
{
  // No need to check for overflow, Curl already allocated this size internally for contents
  size_t const expected_size = size * nmemb;

  // cast transport
  CurlSession* session = static_cast<CurlSession*>(userp);
  // convert curlResponse to standard string
  std::string const& curlResponse = std::string((char*)contents, expected_size);

  // Check if httpResponse was already created bases on the first header
  if (session->m_response == nullptr)
  {
    // first header is expected to be the status code, version and reasonPhrase
    session->m_response = CreateHTTPResponse(curlResponse);
    return expected_size;
  }
  else
  {
    // parse all next headers and add them. Response lives inside the transport
    session->m_response->AddHeader(curlResponse);
  }

  // This callback needs to return the response size or curl will consider it as it failed
  return expected_size;
}

// callback function for libcurl. It would be called as many times as need to ready a body from
// network
size_t CurlSession::WriteBodyCallBack(void* contents, size_t size, size_t nmemb, void* userp)
{
  // No need to check for overflow, Curl already allocated this size internally for contents
  size_t const expected_size = size * nmemb;

  // cast transport
  CurlSession* session = static_cast<CurlSession*>(userp);

  // use buffer body
  session->m_response->AppendBody((uint8_t*)contents, expected_size);

  // This callback needs to return the response size or curl will consider it as it failed
  return expected_size;
}

// Read body and put it into wire
size_t CurlSession::ReadBodyCallBack(void* dst, size_t size, size_t nmemb, void* userdata)
{
  // Calculate the size of the *dst buffer (libcurl buffer to be sent to wire)
  size_t const dst_size = size * nmemb;

  // cast transport
  CurlSession* session = static_cast<CurlSession*>(userdata);

  // check Working with Streams
  auto bodyStream = session->m_request.GetBodyStream();
  if (bodyStream != nullptr)
  {
    auto copiedBytes = bodyStream->Read((uint8_t*)dst, dst_size);
    if (copiedBytes == 0)
    {
      // nothing else to copy
      return CURLE_OK;
    }
    return copiedBytes;
  }

  // upload a chunk of data from body buffer
  auto body = session->m_request.GetBodyBuffer();
  auto uploadedBytes = session->uploadedBytes;
  auto remainingBodySize = body.size() - uploadedBytes;
  auto bytesToCopy = std::min(dst_size, remainingBodySize); // take the smallest to copy

  std::memcpy(dst, body.data() + uploadedBytes, bytesToCopy);
  session->uploadedBytes += bytesToCopy;
  return bytesToCopy;
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
  // head request
  auto rawRequest = this->m_request.ToString();
  auto rawRequestLen = rawRequest.size();

  // Send head request
  CURLcode sendResult = SendBuffer((uint8_t*)rawRequest.data(), rawRequestLen);
  auto streamBody = this->m_request.GetBodyStream();

  if (streamBody == nullptr)
  {
    auto bodyBuffer = this->m_request.GetBodyBuffer();
    return SendBuffer(bodyBuffer.data(), bodyBuffer.size());
  }

  // Send body 1k at a time (TODO: define size of upload buffer)
  // TODO: Can we get the ptr to the start of stream and length and use it directly with no
  // additional buffer? [wonder if stream is in non-contiguos mem...]
  uint8_t buffer[1024];
  streamBody->Rewind();
  while (rawRequestLen > 0)
  {
    rawRequestLen = streamBody->Read(buffer, 1023);
    sendResult = SendBuffer(buffer, rawRequestLen);
  }
  return sendResult;
}

// Read status line plus headers to create a response with no body
CURLcode CurlSession::ReadStatusLineAndHeadersFromRawResponse()
{
  auto parser = Http::ResponseBufferParser();

  // Keep reading until all headers were read
  while (!parser.IsParseCompleted())
  {
    // Try to fill internal buffer from socket.
    // If response is smaller than buffer, we will get back the size of the response
    auto bufferSize = ReadSocketToBuffer(this->readBuffer, LIBCURL_READER_SIZE);

    // parse from buffer to create response
    auto bytesParsed = parser.Parse(this->readBuffer, bufferSize);
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
  // size for curlStream is only size of body, so contentlength less what we have already read
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
  auto innerBufferStart = this->m_bodyStartInBuffer + offset;
  // total size from content-length header less any bytes already read
  auto remainingBodySize = this->m_request.GetBodyStream()->Length() - offset;

  if (offset >= remainingBodySize)
  {
    // Can read beyond bodySize
    return 0;
  }

  // set ptr for writting
  auto writePosition = buffer;
  auto bytesToWrite = bufferSize;

  // If bodyStartInBuffer is set and while innerBufferStart is less than the buffer, it means there
  // is still data at innerbuffer for the body that is not yet read
  if (this->m_bodyStartInBuffer > 0 && LIBCURL_READER_SIZE > innerBufferStart)
  {
    // Calculate the right size of innerBuffer:
    // It can be smaller than the total body, in that case we will take as much as the size of
    // buffer
    // It can be bugger than the total body, in that case we will take as much as the size of the
    // body
    auto innerBufferWithBodyContent = LIBCURL_READER_SIZE - innerBufferStart;
    auto innerbufferSize = remainingBodySize < innerBufferWithBodyContent
        ? remainingBodySize
        : innerBufferWithBodyContent;

    // Requested less data than what we have at inner buffer, take it from innerBuffer
    if (bufferSize <= innerbufferSize)
    {
      std::memcpy(writePosition, this->readBuffer + innerBufferStart, bytesToWrite);
      return bytesToWrite;
    }
    // Requested more data than what we have at innerbuffer. Take all from inner buffer and continue
    std::memcpy(writePosition, this->readBuffer + innerBufferStart, innerbufferSize);
    // next write will be done after reading from socket, move ptr to where to write and how many to
    // write
    writePosition += innerbufferSize;
    bytesToWrite -= innerbufferSize;
  }

  // read from socket the remaining requested bytes
  return ReadSocketToBuffer(writePosition, bytesToWrite);
}

// Read from socket until buffer is full or until socket has no more data
uint64_t CurlSession::ReadSocketToBuffer(uint8_t* buffer, size_t bufferSize)
{
  CURLcode readResult;
  size_t readBytes = 0;
  auto totalRead = 0;
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
    }
  }

  return totalRead;
}

size_t ResponseBufferParser::Parse(uint8_t const* const buffer, size_t const bufferSize)
{
  if (this->parseCompleted)
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
      if (!parseCompleted
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
  return 0;
}

// Finds delimiter '\r' as the end of the
size_t ResponseBufferParser::BuildStatusCode(uint8_t const* const buffer, size_t const bufferSize)
{
  uint8_t endOfStatusLine = '\r';
  auto endOfBuffer = buffer + bufferSize;

  // Look for the end of status line in buffer
  auto indexOfEndOfStatusLine = std::find(buffer, endOfBuffer, endOfStatusLine);

  if (indexOfEndOfStatusLine == endOfBuffer)
  {
    // did not find the delimiter yet, copy to internal buffer
    this->internalBuffer.append(buffer, endOfBuffer);
    return bufferSize; // all buffer read and requesting for more
  }

  // Delimiter found, check if there is data in the internal buffer
  if (this->internalBuffer.size() > 0)
  {
    // If the index is same as buffer it means delimiter is at position 0, meaning that
    // internalBuffer containst the status line and we don't need to add anything else
    if (indexOfEndOfStatusLine > buffer)
    {
      // Append and build response minus the delimiter
      this->internalBuffer.append(buffer, indexOfEndOfStatusLine);
    }
    this->m_response = CreateHTTPResponse(this->internalBuffer, BodyType::Stream);
  }
  else
  {
    // Internal Buffer was not required, create response directly from buffer
    this->m_response
        = CreateHTTPResponse(std::string(buffer, indexOfEndOfStatusLine), BodyType::Stream);
  }

  // update control
  this->state = ResponseParserState::Headers;
  this->internalBuffer.clear();

  // Return the index of the next char to read after delimiter
  // No need to advance one more char ('\n') (since we might be at the end of the array)
  // Parsing Headers will make sure to move one possition
  return indexOfEndOfStatusLine + 1 - buffer;
}

// Finds delimiter '\r' as the end of the
size_t ResponseBufferParser::BuildHeader(uint8_t const* const buffer, size_t const bufferSize)
{
  uint8_t delimiter = '\r';
  auto start = buffer;
  auto endOfBuffer = buffer + bufferSize;

  if (bufferSize == 1 && buffer[0] == '\n')
  {
    // rare case of using buffer of size 1 to read. In this case, if the char is next value after
    // headers or previous header, just consider it as read and return
    return bufferSize;
  }
  else if (bufferSize > 1 && this->internalBuffer.size() == 0) // only if nothing in buffer,
                                                               // advance
  {
    // move offset one possition. This is because readStatusLine and readHeader will read up to
    // '\r' then next delimeter is '\n' and we don't care
    start = buffer + 1;
  }

  // Look for the end of status line in buffer
  auto indexOfEndOfStatusLine = std::find(start, endOfBuffer, delimiter);

  if (indexOfEndOfStatusLine == start)
  {
    // \r found at the start means the end of headers
    this->internalBuffer.clear();
    this->parseCompleted = true;
    return 1; // can't return more than the found delimiter. On read remaining we need to also
              // remove first char
  }

  if (indexOfEndOfStatusLine == endOfBuffer)
  {
    // did not find the delimiter yet, copy to internal buffer
    this->internalBuffer.append(start, endOfBuffer);
    return bufferSize; // all buffer read and requesting for more
  }

  // Delimiter found, check if there is data in the internal buffer
  if (this->internalBuffer.size() > 0)
  {
    // If the index is same as buffer it means delimiter is at position 0, meaning that
    // internalBuffer containst the status line and we don't need to add anything else
    if (indexOfEndOfStatusLine > buffer)
    {
      // Append and build response minus the delimiter
      this->internalBuffer.append(start, indexOfEndOfStatusLine);
    }
    this->m_response->AddHeader(this->internalBuffer);
  }
  else
  {
    // Internal Buffer was not required, create response directly from buffer
    this->m_response->AddHeader(std::string(start, indexOfEndOfStatusLine));
  }

  // reuse buffer
  this->internalBuffer.clear();

  // Return the index of the next char to read after delimiter
  // No need to advance one more char ('\n') (since we might be at the end of the array)
  // Parsing Headers will make sure to move one possition
  return indexOfEndOfStatusLine + 1 - buffer;
}
