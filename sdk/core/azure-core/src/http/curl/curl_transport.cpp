
#include "http/http.hpp"
#include "http/curl/curl.hpp"

#include <string>

using namespace Azure::Core::Http;

CurlTransport::CurlTransport() : 
    Transport()
{
  m_p_curl = curl_easy_init();
}

CurlTransport::~CurlTransport() {
  curl_easy_cleanup(m_p_curl);
}


Response CurlTransport::Send(Context& context, Request& request)
{
  reinterpret_cast<const int&>(context);

  auto performing = perform(request);

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

  return Response(200, "OK\n");
}
