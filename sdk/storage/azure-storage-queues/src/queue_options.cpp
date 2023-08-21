// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/queues/queue_options.hpp"

namespace Azure { namespace Storage { namespace Queues {

  const ServiceVersion ServiceVersion::V2019_12_12(std::string("2019-12-12"));
  const std::chrono::seconds EnqueueMessageOptions::MessageNeverExpires{-1};

}}} // namespace Azure::Storage::Queues
