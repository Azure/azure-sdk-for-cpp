// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "protocol/share_rest_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  // ServiceClient models:

  using ListSharesSegmentResult = ServiceListSharesSegmentResult;
  using SetServicePropertiesResult = ServiceSetPropertiesResult;
  using GetServicePropertiesResult = StorageServiceProperties;

  // ShareClient models:
  using CreateShareResult = ShareCreateResult;
  using DeleteShareResult = ShareDeleteResult;
  using CreateShareSnapshotResult = ShareCreateSnapshotResult;
  using GetSharePropertiesResult = ShareGetPropertiesResult;
  using SetShareQuotaResult = ShareSetQuotaResult;
  using SetShareMetadataResult = ShareSetMetadataResult;
  using SetShareAccessPolicyResult = ShareSetAccessPolicyResult;
  using GetShareStatisticsResult = ShareGetStatisticsResult;
  using CreateSharePermissionResult = ShareCreatePermissionResult;
  using GetShareAccessPolicyResult = ShareGetAccessPolicyResult;
  using GetSharePermissionResult = ShareGetPermissionResult;

}}}} // namespace Azure::Storage::Files::Shares
