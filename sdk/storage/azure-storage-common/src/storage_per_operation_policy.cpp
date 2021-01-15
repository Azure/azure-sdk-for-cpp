// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/storage_per_operation_policy.hpp"

#include "azure/storage/common/constants.hpp"

namespace Azure { namespace Storage { namespace Details {

  std::unique_ptr<Core::Http::RawResponse> StoragePerOperationPolicy::Send(
      Core::Context const& ctx,
      Core::Http::Request& request,
      Core::Http::NextHttpPolicy nextHttpPolicy) const
  {
    request.AddHeader(Details::HttpHeaderXMsVersion, m_apiVersion);
    return nextHttpPolicy.Send(ctx, request);
  }

}}} // namespace Azure::Storage::Details
