// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "common/storage_common.hpp"

#include <uuid.hpp>

namespace Azure { namespace Storage {

  std::string CreateUniqueLeaseId()
  {
    auto uuid = Azure::Core::Uuid::CreateUuid();
    return uuid.GetUuidString();
  }

}} // namespace Azure::Storage
