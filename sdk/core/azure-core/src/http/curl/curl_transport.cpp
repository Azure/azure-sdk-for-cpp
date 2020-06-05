
#include "azure.hpp"
#include "http/curl/curl.hpp"
#include "http/http.hpp"

#include <string>

using namespace Azure::Core::Http;

CurlTransport::CurlTransport() : HttpTransport() { m_pCurl = curl_easy_init(); }

CurlTransport::~CurlTransport() { curl_easy_cleanup(m_pCurl); }

std::unique_ptr<Response> CurlTransport::Send(Context& context, Request& request)
{
  // create inner ref to the request
  this->m_request = &request;

  // If request uses streamBody, set transport to return response with stream
  if (request.GetBodyStream() != nullptr)
  {
    this->m_isStreamRequest = true;
  }

  auto performing = Perform(context);

  if (performing != CURLE_OK)
  {
    switch (performing)
    {
      case CURLE_COULDNT_RESOLVE_HOST:
      {
        throw Azure::Core::Http::CouldNotResolveHostException();
      }
      case CURLE_WRITE_ERROR:
      {
        throw Azure::Core::Http::ErrorWhileWrittingResponse();
      }
      default:
      {
        throw Azure::Core::Http::TransportException();
      }
    }
  }
  return std::move(m_response);
}

CURLcode CurlTransport::Perform(Context& context)
{
  AZURE_UNREFERENCED_PARAMETER(context);

  m_isFirstHeader = true;

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

  curl_easy_setopt(m_pCurl, CURLOPT_NOPROGRESS, 0L);
  return curl_easy_perform(m_pCurl);
}

// Creates an HTTP Response
static std::unique_ptr<Response> ParseAndSetFirstHeader(std::string const& header)
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

void CurlTransport::ParseHeader(std::string const& header)
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

// Callback function for curl. This is called for every header that curl get from network
size_t CurlTransport::WriteHeadersCallBack(void* contents, size_t size, size_t nmemb, void* userp)
{
  // No need to check for overflow, Curl already allocated this size internally for contents
  size_t const expected_size = size * nmemb;

  // cast transport
  CurlTransport* transport = static_cast<CurlTransport*>(userp);
  // convert response to standard string
  std::string const& response = std::string((char*)contents, expected_size);

  if (transport->m_isFirstHeader)
  {
    // first header is expected to be the status code, version and reasonPhrase
    transport->m_response = ParseAndSetFirstHeader(response);
    transport->m_isFirstHeader = false;

    // set response to work with stream if required. TODO: ask jeff how he wants to handle setting
    // up stream usage on response/request
    if (transport->m_isStreamRequest)
    {
      // Setting an empty Stream will make response to know we are working with streams
      // Then, when adding content-length header, we will set the length
      transport->m_response->SetBodyStream(new MemoryBodyStream(nullptr, 0));
    }
    return expected_size;
  }

  if (transport->m_response != nullptr) // only if a response has been created
  {
    // parse all next headers and add them. Response lives inside the transport
    transport->ParseHeader(response);
  }

  // This callback needs to return the response size or curl will consider it as it failed
  return expected_size;
}

// callback function for libcurl. It would be called as many times as need to ready a body from
// network
size_t CurlTransport::WriteBodyCallBack(void* contents, size_t size, size_t nmemb, void* userp)
{
  // No need to check for overflow, Curl already allocated this size internally for contents
  size_t const expected_size = size * nmemb;

  // cast transport
  CurlTransport* transport = static_cast<CurlTransport*>(userp);

  // check Working with Streams
  if (transport->m_isStreamRequest)
  {
    // Create the curlBodyStream the first time
    if (transport->m_isFirstBodyCallBack)
    {
      uint64_t bodySize = transport->m_response->GetBodyStream()->Length();
      // Set curl body stream
      transport->m_response->SetBodyStream(new CurlBodyStream(bodySize, transport));
      transport->m_isFirstBodyCallBack = false;

      // Curl will hold data until handle gests un-paused
      return CURL_WRITEFUNC_PAUSE;
    }
  }

  if (transport->m_response != nullptr) // only if a response has been created
  {
    // If request uses bodyBuffer, response will
    if (transport->m_isStreamRequest)
    {
      std::memcpy(transport->m_responseUserBuffer, contents, expected_size);
      transport->m_isPullCompleted = true;
    }
    else
    {
      // use buffer body
      transport->m_response->AppendBody((uint8_t*)contents, expected_size);
    }
  }

  // This callback needs to return the response size or curl will consider it as it failed
  return expected_size;
}

// Read body and put it into wire
size_t CurlTransport::ReadBodyCallBack(void* dst, size_t size, size_t nmemb, void* userdata)
{
  // Calculate the size of the *dst buffer (libcurl buffer to be sent to wire)
  size_t const dst_size = size * nmemb;

  // cast transport
  CurlTransport* transport = static_cast<CurlTransport*>(userdata);

  // check Working with Streams
  if (transport->m_isStreamRequest)
  {
    // pull from request stream into libcurl buffer
    auto bodyStream = transport->m_request->GetBodyStream();
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
  auto body = transport->m_request->GetBodyBuffer();
  auto body_size = body.capacity();
  std::memcpy(dst, body.data(), body_size);
  return CURLE_OK;
}

int CurlTransport::progressCallback(
    void* clientp,
    curl_off_t dltotal,
    curl_off_t dlnow,
    curl_off_t ultotal,
    curl_off_t ulnow)
{
  (void)dltotal;
  (void)dlnow;
  (void)ultotal;
  (void)ulnow;

  CurlTransport* transport = static_cast<CurlTransport*>(clientp);
  if (transport->m_isPausedRead)
  {
    curl_easy_pause(transport->m_pCurl, CURLPAUSE_CONT);
    transport->m_isPausedRead = false;
  }

  return CURL_PROGRESSFUNC_CONTINUE;
}