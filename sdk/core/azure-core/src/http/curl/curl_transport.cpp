
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

  // TODO:  If working with streams, set request to use send and receive as customs HTTP protocol

  // Working with Body Buffer. let Libcurl use the classic callback to read/write
  auto settingUp = SetUrl();
  if (settingUp != CURLE_OK)
  {
    return settingUp;
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

// Creates an HTTP Response
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
  auto reasonPhrase = std::string(start, header.end() - 2); // remove \r and \n from the end

  // allocate the instance of response to heap with shared ptr
  // So this memory gets delegated outside Curl Transport as a shared ptr so memory will be
  // eventually released
  return std::make_unique<Response>(
      (uint16_t)majorVersion, (uint16_t)minorVersion, HttpStatusCode(statusCode), reasonPhrase);
}

void CurlSession::ParseHeader(std::string const& header)
{
  // get name and value from header
  auto start = header.begin();
  auto end = std::find(start, header.end(), ':');

  if (end == header.end())
  {
    return; // not a valid header or end of headers symbol reached
  }

  auto headerName = std::string(start, end);
  start = end + 1; // start value
  while (start < header.end() && (*start == ' ' || *start == '\t'))
  {
    ++start;
  }

  auto headerValue = std::string(start, header.end() - 2); // remove \r and \n from the end

  this->m_response->AddHeader(headerName, headerValue);
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
    session->ParseHeader(curlResponse);
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

  // TODO: if body_size > dst_size, set some control to resume data copy from there on next
  // callback
  auto body = session->m_request.GetBodyBuffer();
  auto uploadedBytes = session->uploadedBytes;
  auto remainingBodySize = body.size() - uploadedBytes;
  auto bytesToCopy = std::min(dst_size, remainingBodySize); // take the smallest to copy

  std::memcpy(dst, body.data() + uploadedBytes, bytesToCopy);
  session->uploadedBytes += bytesToCopy;
  return bytesToCopy;
}
