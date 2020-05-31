// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <map>
#include <string>
#include <functional>

namespace Azure { namespace Storage {

  class UrlBuilder {
  public:
    UrlBuilder()
    {
    }

    // url must be url-encoded
    explicit UrlBuilder(const std::string& url);

    void SetScheme(const std::string& scheme)
    {
      m_scheme = scheme;
    }

    void SetHost(const std::string& host, bool do_encoding = false)
    {
      m_host = do_encoding ? encode_host(host) : host;
    }

    void SetPort(uint16_t port)
    {
      m_port = port;
    }

    void SetPath(const std::string& path, bool do_encoding = false)
    {
      m_path = do_encoding ? encode_path(path) : path;
    }

    void AppendPath(const std::string& path, bool do_encoding = false)
    {
      if (!m_path.empty() && m_path.back() != '/')
      {
        m_path += '/';
      }
      m_path += do_encoding ? encode_path(path) : path;
    }

    // query must be encoded
    void SetQuery(const std::string& query);

    void AppendQuery(const std::string& key, const std::string& value, bool do_encoding = false)
    {
      if (do_encoding)
      {
        m_query[encode_query(key)] = encode_query(value);
      }
      else
      {
        m_query[key] = value;
      }
    }

    void RemoveQuery(const std::string& key)
    {
      m_query.erase(key);
    }

    void SetFragment(const std::string& fragment, bool do_encoding = false)
    {
      m_fragment = do_encoding ? encode_fragment(fragment) : fragment;
    }

    std::string to_string() const;

  private:
    static std::string encode_host(const std::string& host);
    static std::string encode_path(const std::string& path);
    static std::string encode_query(const std::string& query);
    static std::string encode_fragment(const std::string& fragment);
    static std::string encode_impl(const std::string& source, const std::function<bool(int)>& should_encode);

    std::string m_scheme;
    std::string m_host;
    int m_port{-1};
    std::string m_path; // encoded
    std::map<std::string, std::string> m_query; // encoded
    std::string m_fragment;
  };

}} // namespace Azure::Storage
