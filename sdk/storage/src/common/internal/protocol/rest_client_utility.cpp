// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "common/internal/protocol/rest_client_utility.hpp"

namespace Azure
{
namespace Storage
{
namespace Common
{

const std::string k_HEADER_METADATA_PREFIX = "x-ms-meta-";
const std::string k_HEADER_DATE = "date";
const std::string k_HEADER_X_MS_VERSION = "x-ms-version";
const std::string k_HEADER_AUTHORIZATION = "Authorization";
const std::string k_HEADER_CLIENT_REQUEST_ID = "x-ms-client-request-id";
const std::string k_HEADER_ETAG = "ETag";
const std::string k_HEADER_LAST_MODIFIED = "Last-Modified";
const std::string k_HEADER_X_MS_REQUEST_ID = "x-ms-request-id";
const std::string k_HEADER_X_MS_CLIENT_REQUEST_ID = "x-ms-client-request-id";
const std::string k_HEADER_CONTENT_MD5 = "Content-MD5";
const std::string k_HEADER_X_MS_CONTENT_CRC64 = "x-ms-content-crc64";
const std::string k_HEADER_X_MS_ACCESS_TIER = "x-ms-access_tier";
const std::string k_HEADER_X_MS_SERVER_ENCRYPTED = "x-ms-server-encrypted";
const std::string k_HEADER_X_MS_ENCRYPTION_KEY_SHA256 = "x-ms-server-encryption-key-sha256";
const std::string k_QUERY_RESTYPE = "restype";
const std::string k_QUERY_COMP = "comp";

} // namespace Common
} // namespace Storage
} // namespace Azure