// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "protocol/share_rest_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  // ServiceClient models:

  using ListSharesSegmentResult = ServiceListSharesSegmentResponse;
  using SetServicePropertiesInfo = ServiceSetPropertiesResponse;

  // ShareClient models:
  using ShareInfo = ShareCreateResponse;
  using ShareDeleteInfo = ShareDeleteResponse;
  using ShareSnapshotInfo = ShareCreateSnapshotResponse;
  using FileShareProperties = ShareGetPropertiesResponse;
  using SetShareQuotaInfo = ShareSetQuotaResponse;
  using SetShareMetadataInfo = ShareSetMetadataResponse;
  using SetAccessPolicyInfo = ShareSetAccessPolicyResponse;
  using ShareStatistics = ShareGetStatisticsResponse;
  using SharePermissionInfo = ShareCreatePermissionResponse;
  using GetShareAccessPolicyResult = ShareGetAccessPolicyResponse;

}}}} // namespace Azure::Storage::Files::Shares
