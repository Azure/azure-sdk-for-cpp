
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

namespace Azure { namespace Storage { namespace Details {
  constexpr static const char* c_BlobServicePackageName = "storageblob";
  constexpr static const char* c_DatalakeServicePackageName = "storagedatalake";
  constexpr static const char* c_FileServicePackageName = "storagefile";
  constexpr static const char* c_QueueServicePackageName = "storagequeue";
  constexpr static const char* c_HttpQuerySnapshot = "snapshot";
  constexpr static const char* c_HttpQueryVersionId = "versionid";
  constexpr static const char* c_StorageScope = "https://storage.azure.com/.default";
  constexpr static const char* c_HttpHeaderDate = "date";
  constexpr static const char* c_HttpHeaderXMsVersion = "x-ms-version";
  constexpr static const char* c_HttpHeaderRequestId = "x-ms-request-id";
  constexpr static const char* c_HttpHeaderClientRequestId = "x-ms-client-request-id";
  constexpr static const char* c_HttpHeaderContentType = "content-type";
  constexpr static const char* c_defaultSasVersion = "2019-12-12";

  constexpr int c_reliableStreamRetryCount = 3;
  constexpr int64_t c_FileUploadDefaultChunkSize = 4 * 1024 * 1024;
  constexpr int64_t c_DownloadDefaultChunkSize = 4 * 1024 * 1024;

}}} // namespace Azure::Storage::Details
