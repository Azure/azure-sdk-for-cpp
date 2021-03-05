// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/common/storage_retry_policy.hpp"

#include <thread>

#include "azure/storage/common/constants.hpp"

namespace Azure { namespace Storage { namespace Details {

  std::unique_ptr<Azure::Core::Http::RawResponse> StorageSwitchToSecondaryPolicy::Send(
      const Azure::Core::Context& ctx,
      Azure::Core::Http::Request& request,
      Azure::Core::Http::NextHttpPolicy nextHttpPolicy) const
  {
    return nextHttpPolicy.Send(ctx, request);
  }

}}} // namespace Azure::Storage::Details
