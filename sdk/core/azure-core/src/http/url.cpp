// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/azure.hpp"
#include "azure/core/http/http.hpp"

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
      return static_cast<char>(Azure::Core::Details::ToLower(static_cast<unsigned char>(c)));
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
    AppendQueries(std::string(pos + 1, queryIter));
    pos = queryIter;
  }

  if (pos != url.end() && *pos == '#')
  {
    m_fragment = std::string(pos + 1, url.end());
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

static const char* unreserved
    = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~";
static const char* subdelimiters = "!$&'()*+,;=";

std::string Url::EncodeHost(const std::string& host)
{
  return EncodeImpl(host, [](int c) { return c > 127; });
}

std::string Url::EncodePath(const std::string& path)
{
  const static std::vector<bool> shouldEncodeTable = []() {
    const std::string pathCharacters
        = std::string(unreserved) + std::string(subdelimiters) + "%/:@";

    std::vector<bool> ret(256, true);
    for (char c : pathCharacters)
    {
      ret[c] = false;
    }
    // we also encode % and +
    ret['%'] = true;
    ret['+'] = true;
    return ret;
  }();

  return EncodeImpl(path, [](int c) { return shouldEncodeTable[c]; });
}

std::string Url::EncodeQuery(const std::string& query)
{
  const static std::vector<bool> shouldEncodeTable = []() {
    std::string queryCharacters = std::string(unreserved) + std::string(subdelimiters) + "%/:@?";

    std::vector<bool> ret(256, true);
    for (char c : queryCharacters)
    {
      ret[c] = false;
    }
    // we also encode % and +
    ret['%'] = true;
    ret['+'] = true;
    // Surprisingly, '=' also needs to be encoded because Azure Storage server side is so strict.
    // We are applying this function to query key and value respectively, so this won't affect
    // that = used to separate key and query.
    ret['='] = true;
    return ret;
  }();

  return EncodeImpl(query, [](int c) { return shouldEncodeTable[c]; });
}

std::string Url::EncodeFragment(const std::string& fragment)
{
  const static std::vector<bool> shouldEncodeTable = []() {
    std::string queryCharacters = std::string(unreserved) + std::string(subdelimiters) + "%/:@?";

    std::vector<bool> ret(256, true);
    for (char c : queryCharacters)
    {
      ret[c] = false;
    }
    // we also encode % and +
    ret['%'] = true;
    ret['+'] = true;
    return ret;
  }();

  return EncodeImpl(fragment, [](int c) { return shouldEncodeTable[c]; });
}

std::string Url::EncodeImpl(const std::string& source, const std::function<bool(int)>& shouldEncode)
{
  const char* hex = "0123456789ABCDEF";

  std::string encoded;
  for (char c : source)
  {
    unsigned char uc = c;
    if (shouldEncode(uc))
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

void Url::AppendQueries(const std::string& query)
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
    std::string query_value = Decode(std::string(cur, value_end));

    cur = value_end;
    if (cur != query.end())
    {
      ++cur;
    }
    m_queryParameters[std::move(query_key)] = std::move(query_value);
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
    auto queryParameters = m_retryModeEnabled
        ? Details::MergeMaps(m_retryQueryParameters, m_queryParameters)
        : m_queryParameters;
    relative_url += '?';
    for (const auto& q : queryParameters)
    {
      relative_url += q.first + '=' + EncodeQuery(q.second) + '&';
    }
    relative_url.pop_back();
  }
  if (!m_fragment.empty())
  {
    relative_url += "#" + m_fragment;
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
