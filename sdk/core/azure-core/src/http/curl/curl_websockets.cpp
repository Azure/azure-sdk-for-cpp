// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/http.hpp"
#include "azure/core/http/policies/policy.hpp"
#include "azure/core/http/transport.hpp"
#include "azure/core/http/websockets/curl_websockets_transport.hpp"
#include "azure/core/internal/diagnostics/log.hpp"
#include "azure/core/platform.hpp"

// Private include
#include "curl_connection_private.hpp"

#if defined(AZ_PLATFORM_POSIX)
#include <poll.h> // for poll()
#include <sys/socket.h> // for socket shutdown
#elif defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <winapifamily.h>
#include <winsock2.h> // for WSAPoll();
#endif

namespace Azure { namespace Core { namespace Http { namespace WebSockets {

  void CurlWebSocketTransport::CompleteUpgrade() {}

  void CurlWebSocketTransport::Close() { m_upgradedConnection->Shutdown(); }

  // Send an HTTP request to the remote server.
  std::unique_ptr<RawResponse> CurlWebSocketTransport::Send(
      Request& request,
      Context const& context)
  {
    // CURL doesn't understand the ws and wss protocols, so change the URL to be http based.
    std::string requestScheme(request.GetUrl().GetScheme());
    if (requestScheme == "wss" || requestScheme == "ws")
    {
      if (requestScheme == "wss")
      {
        request.GetUrl().SetScheme("https");
      }
      else
      {
        request.GetUrl().SetScheme("http");
      }
    }
    return CurlTransport::Send(request, context);
  }

  size_t CurlWebSocketTransport::ReadFromSocket(
      uint8_t* buffer,
      size_t bufferSize,
      Context const& context)
  {
    return m_upgradedConnection->ReadFromSocket(buffer, bufferSize, context);
  }

  /**
   * @brief This method will use libcurl socket to write all the bytes from buffer.
   *
   */
  int CurlWebSocketTransport::SendBuffer(
      uint8_t const* buffer,
      size_t bufferSize,
      Context const& context)
  {
    return m_upgradedConnection->SendBuffer(buffer, bufferSize, context);
  }

  void CurlWebSocketTransport::OnUpgradedConnection(
      std::unique_ptr<CurlNetworkConnection>& upgradedConnection)
  {
    CurlTransport::OnUpgradedConnection(upgradedConnection);
    // Note that m_upgradedConnection is a std::shared_ptr. We define it as a std::shared_ptr
    // because a std::shared_ptr can be declared on an incomplete type, while a std::unique_ptr
    // cannot.
    m_upgradedConnection = std::move(upgradedConnection);
  }

}}}} // namespace Azure::Core::Http::WebSockets
