// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

namespace Azure { namespace Storage { namespace _internal {
  constexpr static const char* BlobServicePackageName = "storage-blobs";
  constexpr static const char* DatalakeServicePackageName = "storage-files-datalake";
  constexpr static const char* FileServicePackageName = "storage-files-shares";
  constexpr static const char* QueueServicePackageName = "storage-queues";
  constexpr static const char* HttpQuerySnapshot = "snapshot";
  constexpr static const char* HttpQueryVersionId = "versionid";
  constexpr static const char* HttpQueryTimeout = "timeout";
  constexpr static const char* StorageScope = "https://storage.azure.com/.default";
  constexpr static const char* StorageDefaultAudience = "https://storage.azure.com";
  constexpr static const char* HttpHeaderDate = "date";
  constexpr static const char* HttpHeaderXMsDate = "x-ms-date";
  constexpr static const char* HttpHeaderXMsVersion = "x-ms-version";
  constexpr static const char* HttpHeaderRequestId = "x-ms-request-id";
  constexpr static const char* HttpHeaderClientRequestId = "x-ms-client-request-id";
  constexpr static const char* HttpHeaderContentType = "content-type";
  constexpr static const char* HttpHeaderContentLength = "content-length";
  constexpr static const char* HttpHeaderContentRange = "content-range";
  constexpr static const char* InvalidHeaderValueErrorCode = "InvalidHeaderValue";
  constexpr static const char* InvalidVersionHeaderMessage
      = "The provided service version is not enabled on this storage account.  Please see "
        "https://learn.microsoft.com/rest/api/storageservices/"
        "versioning-for-the-azure-storage-services for additional information.";

  constexpr int ReliableStreamRetryCount = 3;
}}} // namespace Azure::Storage::_internal
