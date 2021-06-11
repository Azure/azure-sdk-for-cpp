// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

namespace Azure { namespace Storage { namespace _internal {
  constexpr static const char* BlobServicePackageName = "storage-blobs";
  constexpr static const char* DatalakeServicePackageName = "storage-files-datalake";
  constexpr static const char* FileServicePackageName = "storage-files-shares";
  constexpr static const char* QueueServicePackageName = "storage-queues";
  constexpr static const char* HttpQuerySnapshot = "snapshot";
  constexpr static const char* HttpQueryVersionId = "versionid";
  constexpr static const char* StorageScope = "https://storage.azure.com/.default";
  constexpr static const char* HttpHeaderDate = "date";
  constexpr static const char* HttpHeaderXMsVersion = "x-ms-version";
  constexpr static const char* HttpHeaderRequestId = "x-ms-request-id";
  constexpr static const char* HttpHeaderClientRequestId = "x-ms-client-request-id";
  constexpr static const char* HttpHeaderContentType = "content-type";
  constexpr static const char* DefaultSasVersion = "2020-02-10";

  constexpr int ReliableStreamRetryCount = 3;
}}} // namespace Azure::Storage::_internal
