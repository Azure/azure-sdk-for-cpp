// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "common/storage_uri_builder.hpp"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <vector>

namespace Azure { namespace Storage {

  UriBuilder::UriBuilder(const std::string& url)
  {
    std::string::const_iterator pos = url.begin();

    const std::string schemeEnd = "://";
    auto schemeIter = url.find(schemeEnd);
    if (schemeIter != std::string::npos)
    {
      std::transform(
          url.begin(), url.begin() + schemeIter, std::back_inserter(m_scheme), [](char c) {
            return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
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
      m_path = std::string(pos + 1, pathIter);
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

  static const char* unreserved
      = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-._~";
  static const char* subdelimiters = "!$&'()*+,;=";

  std::string UriBuilder::EncodeHost(const std::string& host)
  {
    return EncodeImpl(host, [](int c) { return c > 127; });
  }

  std::string UriBuilder::EncodePath(const std::string& path)
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

  std::string UriBuilder::EncodeQuery(const std::string& query)
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

  std::string UriBuilder::EncodeFragment(const std::string& fragment)
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

  std::string UriBuilder::EncodeImpl(
      const std::string& source,
      const std::function<bool(int)>& shouldEncode)
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

  void UriBuilder::AppendQueries(const std::string& query)
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
      m_query[std::move(query_key)] = std::move(query_value);
    }
  }

  std::string UriBuilder::ToString() const
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
    if (!m_path.empty())
    {
      full_url += "/" + m_path;
    }
    if (!m_query.empty())
    {
      bool first_query = true;
      for (const auto& q : m_query)
      {
        full_url += first_query ? "?" : "&";
        first_query = false;
        full_url += q.first + "=" + q.second;
      }
    }
    if (!m_fragment.empty())
    {
      full_url += "#" + m_fragment;
    }
    return full_url;
  }

}} // namespace Azure::Storage
