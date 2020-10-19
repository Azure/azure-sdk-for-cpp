// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/http.hpp"
#include "azure/core/strings.hpp"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <vector>

using namespace Azure::Core::Http;

Url::Url(const std::string& url)
{
  std::string::const_iterator pos = url.begin();

  const std::string schemeEnd = "://";
  auto schemeIter = url.find(schemeEnd);
  if (schemeIter != std::string::npos)
  {
    std::transform(url.begin(), url.begin() + schemeIter, std::back_inserter(m_scheme), [](char c) {
      return static_cast<char>(Azure::Core::Strings::ToLower(static_cast<unsigned char>(c)));
    });

    pos = url.begin() + schemeIter + schemeEnd.length();
  }

  auto hostIter
      = std::find_if(pos, url.end(), [](char c) { return c == '/' || c == '?' || c == ':'; });
  m_host = std::string(pos, hostIter);
  pos = hostIter;

  if (pos != url.end() && *pos == ':')
  {
    auto port_ite = std::find_if_not(
        pos + 1, url.end(), [](char c) { return std::isdigit(static_cast<unsigned char>(c)); });
    m_port = std::stoi(std::string(pos + 1, port_ite));
    pos = port_ite;
  }

  if (pos != url.end() && *pos == '/')
  {
    auto pathIter = std::find(pos + 1, url.end(), '?');
    m_encodedPath = std::string(pos + 1, pathIter);
    pos = pathIter;
  }

  if (pos != url.end() && *pos == '?')
  {
    auto queryIter = std::find(pos + 1, url.end(), '#');
    AppendQueryParameters(std::string(pos + 1, queryIter));
    pos = queryIter;
  }
}

std::string Url::Decode(const std::string& value)
{
  const static std::vector<int> hexTable = []() {
    std::vector<int> t(256, -1);
    for (int i = 0; i < 10; ++i)
    {
      t[static_cast<std::size_t>('0') + i] = i;
    }
    for (int i = 10; i < 16; ++i)
    {
      t[static_cast<std::size_t>('A') + i - 10] = i;
      t[static_cast<std::size_t>('a') + i - 10] = i;
    }
    return t;
  }();

  std::string decodedValue;
  for (std::size_t i = 0; i < value.size();)
  {
    char c = value[i];
    if (c == '+')
    {
      decodedValue += ' ';
      ++i;
    }
    else if (c == '%')
    {
      if (i + 2 >= value.size() || hexTable[value[i + 1]] < 0 || hexTable[value[i + 2]] < 0)
      {
        throw std::runtime_error("failed when decoding url component");
      }
      int v = (hexTable[value[i + 1]] << 4) + hexTable[value[i + 2]];
      decodedValue += static_cast<std::string::value_type>(v);
      i += 3;
    }
    else
    {
      decodedValue += value[i];
      ++i;
    }
  }
  return decodedValue;
}

std::string Url::Encode(const std::string& value, const std::string& doNotEncodeSymbols)
{
  const char* hex = "0123456789ABCDEF";
  std::unordered_set<unsigned char> noEncodingSymbolsSet(
      doNotEncodeSymbols.begin(), doNotEncodeSymbols.end());

  std::string encoded;
  for (char c : value)
  {
    unsigned char uc = c;
    // encode if char is not in the default non-encoding set AND if it is NOT in chars to ignore
    // from user input
    if (defaultNonUrlEncodeChars.find(uc) == defaultNonUrlEncodeChars.end()
        && noEncodingSymbolsSet.find(uc) == noEncodingSymbolsSet.end())
    {
      encoded += '%';
      encoded += hex[(uc >> 4) & 0x0f];
      encoded += hex[uc & 0x0f];
    }
    else
    {
      encoded += c;
    }
  }
  return encoded;
}

void Url::AppendQueryParameters(const std::string& query)
{
  std::string::const_iterator cur = query.begin();
  if (cur != query.end() && *cur == '?')
  {
    ++cur;
  }

  while (cur != query.end())
  {
    auto key_end = std::find(cur, query.end(), '=');
    std::string query_key = std::string(cur, key_end);

    cur = key_end;
    if (cur != query.end())
    {
      ++cur;
    }

    auto value_end = std::find(cur, query.end(), '&');
    std::string query_value = std::string(cur, value_end);

    cur = value_end;
    if (cur != query.end())
    {
      ++cur;
    }
    m_encodedQueryParameters[std::move(query_key)] = std::move(query_value);
  }
}

std::string Url::GetRelativeUrl() const
{
  std::string relative_url;
  if (!m_encodedPath.empty())
  {
    relative_url += m_encodedPath;
  }
  {
    relative_url += '?';
    for (const auto& q : m_encodedQueryParameters)
    {
      relative_url += q.first + '=' + q.second + '&';
    }
    relative_url.pop_back();
  }

  return relative_url;
}

std::string Url::GetAbsoluteUrl() const
{
  std::string full_url;
  if (!m_scheme.empty())
  {
    full_url += m_scheme + "://";
  }
  full_url += m_host;
  if (m_port != -1)
  {
    full_url += ":" + std::to_string(m_port);
  }
  if (!m_encodedPath.empty())
  {
    full_url += "/";
  }
  full_url += GetRelativeUrl();
  return full_url;
}

std::unordered_set<unsigned char> Url::defaultNonUrlEncodeChars
    = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q',
       'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
       'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
       'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '.', '_', '~'};
