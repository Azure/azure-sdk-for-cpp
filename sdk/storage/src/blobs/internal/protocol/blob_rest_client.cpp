// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/internal/protocol/blob_rest_client.hpp"

namespace Azure
{
namespace Storage
{
namespace Blobs
{

const std::string BlobRestClient::k_HEADER_X_MS_BLOB_TYPE = "x-ms-blob-type";
const std::string BlobRestClient::k_HEADER_BLOCK_BLOB = "BlockBlob";
const std::string BlobRestClient::k_HEADER_PAGE_BLOB = "PageBlob";
const std::string BlobRestClient::k_HEADER_APPEND_BLOB = "AppendBlob";
const std::string BlobRestClient::k_HEADER_X_MS_BLOB_SEQUENCE_NUMBER = "x-ms-blob-sequence-number";
const std::string BlobRestClient::k_QUERY_CONTAINER = "container";
const std::string BlobRestClient::k_QUERY_BLOB = "blob";
const std::string BlobRestClient::k_QUERY_BLOCK_ID = "blockid";
const std::string BlobRestClient::k_QUERY_BLOCK_LIST = "blocklist";
const std::string BlobRestClient::k_QUERY_BLOCK = "block";
const std::string BlobRestClient::Container::k_HEADER_MS_BLOB_PUBLIC_ACCESS = "x-ms-blob-public-access";
const std::string BlobRestClient::BlockBlob::k_XML_TAG_BLOCK_LIST = "BlockList";
const std::string BlobRestClient::BlockBlob::k_XML_TAG_COMMITTED = "Committed";
const std::string BlobRestClient::BlockBlob::k_XML_TAG_UNCOMMITTED = "Uncommitted";
const std::string BlobRestClient::BlockBlob::k_XML_TAG_LATEST = "Latest";

} // namespace Blobs
} // namespace Storage
} // namespace Azure

