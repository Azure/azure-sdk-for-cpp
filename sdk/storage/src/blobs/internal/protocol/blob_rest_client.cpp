// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/internal/protocol/blob_rest_client.hpp"

namespace Azure
{
namespace Storage
{
namespace Blobs
{

const std::string BlobRestClient::k_CONTAINER = "container";
const std::string BlobRestClient::k_BLOB = "blob";
const std::string BlobRestClient::k_BLOCK_ID = "blockid";
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

