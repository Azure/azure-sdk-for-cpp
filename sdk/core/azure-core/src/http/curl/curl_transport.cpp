
#include "azure.hpp"
#include "http/curl/curl.hpp"
#include "http/http.hpp"

#include <string>

using namespace Azure::Core::Http;

CurlTransport::CurlTransport() : HttpTransport() { m_pCurl = curl_easy_init(); }

CurlTransport::~CurlTransport() { curl_easy_cleanup(m_pCurl); }

std::unique_ptr<Response> CurlTransport::Send(Context& context, Request& request)
{
  auto performing = Perform(context, request);

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
  return std::move(m_response);
}

CURLcode CurlTransport::Perform(Context& context, Request& request)
{
  AZURE_UNREFERENCED_PARAMETER(context);

  m_firstHeader = true;

  auto settingUp = SetUrl(request);
  if (settingUp != CURLE_OK)
  {
    return settingUp;
  }

  settingUp = SetHeaders(request);
  if (settingUp != CURLE_OK)
  {
    return settingUp;
  }

  settingUp = SetWriteResponse();
  if (settingUp != CURLE_OK)
  {
    return settingUp;
  }

  return curl_easy_perform(m_pCurl);
}

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

static void ParseHeader(std::string const& header, std::unique_ptr<Response>& response)
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

  response->AddHeader(headerName, headerValue);
}

// Callback function for curl. This is called for every header that curl get from network
size_t CurlTransport::WriteHeadersCallBack(void* contents, size_t size, size_t nmemb, void* userp)
{
  // No need to check for overflow, Curl already allocated this size internally for contents
  size_t const expected_size = size * nmemb;

  // cast client
  CurlTransport* client = static_cast<CurlTransport*>(userp);
  // convert response to standard string
  std::string const& response = std::string((char*)contents, expected_size);

  if (client->m_firstHeader)
  {
    // first header is expected to be the status code, version and reasonPhrase
    client->m_response = ParseAndSetFirstHeader(response);
    client->m_firstHeader = false;
    return expected_size;
  }

  if (client->m_response != nullptr) // only if a response has been created
  {
    // parse all next headers and add them
    ParseHeader(response, client->m_response);
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

  // cast client
  CurlTransport* client = static_cast<CurlTransport*>(userp);

  if (client->m_response != nullptr) // only if a response has been created
  {
    // TODO: check if response is to be written to buffer or to Stream
    client->m_response->AppendBody((uint8_t*)contents, expected_size);
  }

  // This callback needs to return the response size or curl will consider it as it failed
  return expected_size;
}
