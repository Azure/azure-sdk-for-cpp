
#include "azure.hpp"
#include "http/curl/curl.hpp"
#include "http/http.hpp"

#include <string>

using namespace Azure::Core::Http;

CurlTransport::CurlTransport() : Transport() { m_pCurl = curl_easy_init(); }

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

  throw;
  // allocate the instance of response to heap with shared ptr
  // So this memory gets delegated outside Curl Transport as a shared ptr so memory will be
  // eventually released
  // return std::make_unique<Response>(
  //    majorVersion, minorVersion, HttpStatusCode(statusCode), reasonPhrase);
}

CURLcode CurlTransport::Perform(Context& context, Request& request)
{
  AZURE_UNREFERENCED_PARAMETER(context);
  auto settingUp = setUrl(request);
  if (settingUp != CURLE_OK)
  {
    return settingUp;
  }
  return curl_easy_perform(m_pCurl);
}
