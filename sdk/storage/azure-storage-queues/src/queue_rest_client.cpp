// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/queues/protocol/queue_rest_client.hpp"

namespace Azure { namespace Storage { namespace Queues { namespace Models {

  const GeoReplicationStatus GeoReplicationStatus::Live("live");
  const GeoReplicationStatus GeoReplicationStatus::Bootstrap("bootstrap");
  const GeoReplicationStatus GeoReplicationStatus::Unavailable("unavailable");

}}}} // namespace Azure::Storage::Queues::Models
