// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <map>
#include <ctime>

#include "http/http.hpp"

namespace Azure
{
namespace Storage
{
namespace Common
{

extern const std::string k_HEADER_METADATA_PREFIX;
extern const std::string k_HEADER_DATE;
extern const std::string k_HEADER_X_MS_VERSION;
extern const std::string k_HEADER_AUTHORIZATION;
extern const std::string k_HEADER_CLIENT_REQUEST_ID;
extern const std::string k_HEADER_ETAG;
extern const std::string k_HEADER_LAST_MODIFIED;
extern const std::string k_HEADER_X_MS_CLIENT_REQUEST_ID;
extern const std::string k_HEADER_X_MS_REQUEST_ID;
extern const std::string k_HEADER_CONTENT_MD5;
extern const std::string k_HEADER_X_MS_CONTENT_CRC64;
extern const std::string k_HEADER_X_MS_ACCESS_TIER;
extern const std::string k_HEADER_X_MS_SERVER_ENCRYPTED;
extern const std::string k_HEADER_X_MS_ENCRYPTION_KEY_SHA256;
extern const std::string k_QUERY_RESTYPE;
extern const std::string k_QUERY_COMP;

struct RequestOptions
{
  std::string Version = "2019-02-02";
  std::string ClientRequestID = std::string();
  std::string Date = std::string();
};

struct BodiedRequestOptions : public RequestOptions
{
  azure::core::http::BodyBuffer* BodyBuffer = nullptr;
  azure::core::http::BodyStream* BodyStream = nullptr;
};

struct ResponseInfo
{
  std::string RequestId = std::string();
  std::string Date = std::string();
  std::string Version = std::string();
  std::string ClientRequestID = std::string();
};

inline void AddMetadata(
  const std::map<std::string, std::string>& metadata,
  azure::core::http::Request& request)
{
  for (auto pair : metadata)
  {
    request.addHeader(k_HEADER_METADATA_PREFIX + pair.first, pair.second);
  }
}

inline std::string GetDateString()
{
  // TODO: how to suppress this warning
  char buf[30];
  std::time_t t = std::time(nullptr);
  std::tm* pm;
  pm = std::gmtime(&t);
  size_t s = std::strftime(buf, 30, "%a, %d %b %Y %H:%M:%S GMT", pm);
  return std::string(buf, s);
}

inline void ApplyBasicHeaders(
    const RequestOptions& options,
    azure::core::http::Request& request)
{
  request.addHeader(k_HEADER_X_MS_VERSION, options.Version);
  request.addHeader(k_HEADER_CLIENT_REQUEST_ID, options.ClientRequestID);
  request.addHeader(k_HEADER_DATE, options.Date.empty() ? GetDateString() : options.Date);
}

inline std::string GetHeaderValue(
    const std::map<std::string, std::string>& headers,
    const std::string& key)
{
  if (headers.find(key) != headers.end())
  {
    return headers.at(key);
  }
  return std::string();
}

inline void ParseBasicResponseHeaders(
    const std::map<std::string, std::string>& headers,
    ResponseInfo& info)
{
  info.RequestId = GetHeaderValue(headers, k_HEADER_X_MS_REQUEST_ID);
  info.ClientRequestID = GetHeaderValue(headers, k_HEADER_X_MS_CLIENT_REQUEST_ID);
  info.Version = GetHeaderValue(headers, k_HEADER_X_MS_VERSION);
  info.Date = GetHeaderValue(headers, k_HEADER_DATE);
}

} // namespace Common
} // namespace Storage
} // namespace Azure