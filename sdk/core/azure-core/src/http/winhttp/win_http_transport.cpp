
#include "http/http.hpp"
#include "http/winhttp/win_http_client.hpp"

#include <string>

using namespace Azure::Core::Http;

WinHttpTansport::WinHttpTansport() : 
    Transport()
{
}

WinHttpTansport::~WinHttpTansport() {}


Response WinHttpTansport::Send(Context& context, Request& request)
{
  reinterpret_cast<const int&>(context);
  reinterpret_cast<const int&>(request);

  return Response(200, "OK\n");
}
