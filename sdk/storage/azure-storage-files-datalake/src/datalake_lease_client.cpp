// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_lease_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  const std::chrono::seconds DataLakeLeaseClient::InfiniteLeaseDuration{
      Blobs::BlobLeaseClient::InfiniteLeaseDuration};

}}}} // namespace Azure::Storage::Files::DataLake
