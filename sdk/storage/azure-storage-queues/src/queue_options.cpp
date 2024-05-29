// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/queues/queue_options.hpp"

namespace Azure { namespace Storage { namespace Queues {

  const QueueAudience QueueAudience::DefaultAudience(_internal::StorageDefaultAudience);

  const ServiceVersion ServiceVersion::V2018_03_28(std::string("2018-03-28"));
  const ServiceVersion ServiceVersion::V2019_12_12(std::string("2019-12-12"));
  const ServiceVersion ServiceVersion::V2024_08_04(std::string("2024-08-04"));
  const std::chrono::seconds EnqueueMessageOptions::MessageNeverExpires{-1};

}}} // namespace Azure::Storage::Queues
