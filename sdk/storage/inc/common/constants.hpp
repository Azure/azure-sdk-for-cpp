
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

namespace Azure { namespace Storage { namespace Details {
  constexpr static const char* c_HttpQuerySnapshot = "snapshot";
  constexpr static const char* c_HttpQueryVersionId = "versionid";
  constexpr static const char* c_StorageScope = "https://storage.azure.com/.default";
  constexpr static const char* c_HttpHeaderDate = "date";
  constexpr static const char* c_HttpHeaderXMsVersion = "x-ms-version";
  constexpr static const char* c_HttpHeaderRequestId = "x-ms-request-id";
  constexpr static const char* c_HttpHeaderClientRequestId = "x-ms-client-request-id";
  constexpr static const char* c_HttpHeaderContentType = "content-type";
}}} // namespace Azure::Storage::Details
