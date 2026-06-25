// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/common/internal/concurrent_transfer.hpp"

#include <thread>

namespace Azure { namespace Storage { namespace _internal {

  int GetHardwareConcurrency()
  {
    static int c = static_cast<int>(std::thread::hardware_concurrency());
    return c;
  }

}}} // namespace Azure::Storage::_internal
