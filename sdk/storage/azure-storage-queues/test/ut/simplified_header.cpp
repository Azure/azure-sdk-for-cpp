// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/storage/queues.hpp>

#include <gtest/gtest.h>

namespace Azure { namespace Storage { namespace Test {

  TEST(SimplifiedHeader, StorageQueues)
  {
    Azure::Storage::Queues::QueueServiceClient serviceClient(
        "https://account.blob.core.windows.net");
    Azure::Storage::Queues::QueueClient containerClient(
        "https://account.queue.core.windows.net/queue");

    Azure::Storage::Sas::QueueSasBuilder sasBuilder;

    StorageSharedKeyCredential keyCredential("account", "key");

    try
    {
    }
    catch (Azure::Storage::StorageException& e)
    {
      std::cout << e.what() << std::endl;
    }
  }

}}} // namespace Azure::Storage::Test