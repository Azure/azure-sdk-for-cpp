
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

namespace Azure { namespace Storage { namespace Details {
  constexpr static const char* c_ConnectionStringTagAccountName = "AccountName";
  constexpr static const char* c_ConnectionStringTagAccountKey = "AccountKey";
  constexpr static const char* c_ConnectionStringTagBlobEndpoint = "BlobEndpoint";
  constexpr static const char* c_ConnectionStringTagDataLakeEndpoint = "AdlsEndpoint";
  constexpr static const char* c_ConnectionStringTagEndpointSuffix = "EndpointSuffix";
  constexpr static const char* c_ConnectionStringTagDefaultEndpointsProtocol
      = "DefaultEndpointsProtocol";
  constexpr static const char* c_DfsEndpointIdentifier = "dfs";
}}} // namespace Azure::Storage::Details
