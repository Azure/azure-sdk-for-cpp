// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/files/datalake/datalake_lease_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  const std::chrono::seconds DataLakeLeaseClient::InfiniteLeaseDuration{-1};

}}}} // namespace Azure::Storage::Files::DataLake
