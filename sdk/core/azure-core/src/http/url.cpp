// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <http/http.hpp>

using namespace Azure::Core::Http;

URL::URL(std::string const& url)
{
  if (url.size() == 0)
  {
    return; // nothing to set
  }

  auto endOfUrl = url.end();
  auto start = url.begin();
  // Protocol
  auto protocolEnd = std::find(start, endOfUrl, ':');
  if (protocolEnd != endOfUrl)
  {
    auto protocolDelimiter = std::string(protocolEnd, endOfUrl);
    // Check protocol delimiter is there ://, otherwise it can be a port
    if (protocolDelimiter.size() >= 3 && protocolDelimiter[1] == '/' && protocolDelimiter[2] == '/')
    {
      this->m_protocol = std::string(start, protocolEnd);
      start = protocolEnd + 3;
    }
  }

  // Host
  auto endOfHost = std::find(start, endOfUrl, '/');
  auto startOfPort = std::find(start, endOfUrl, ':');
  if (startOfPort < endOfHost)
  {
    this->m_port = std::string(startOfPort + 1, endOfHost);
  }
  this->m_host = std::string(start, endOfHost);

  // finish if there is nothing more ahead
  if (endOfHost == endOfUrl)
  {
    return;
  }

  // Advance to path
  start = endOfHost + 1;

  // Path
  this->m_path = std::string(start, endOfUrl);
}
