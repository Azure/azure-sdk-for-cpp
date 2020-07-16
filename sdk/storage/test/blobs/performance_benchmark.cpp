// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "block_blob_client_test.hpp"

#include <chrono>

namespace Azure { namespace Storage { namespace Test {

  TEST_F(BlockBlobClientTest, DISABLED_SingleThreadPerf)
  {
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, RandomString());

    constexpr std::size_t bufferSize = static_cast<std::size_t>(1_GB);
    std::vector<uint8_t> buffer = RandomBuffer(bufferSize);
    {
      auto timer_start = std::chrono::steady_clock::now();
      auto res = blockBlobClient.UploadFromBuffer(buffer.data(), buffer.size());
      auto timer_end = std::chrono::steady_clock::now();

      double speed = static_cast<double>(bufferSize) / 1_MB
          / std::chrono::duration_cast<std::chrono::milliseconds>(timer_end - timer_start).count()
          * 1000;
      std::cout << "Upload speed: " << speed << "MiB/s" << std::endl;
    }
    {
      auto timer_start = std::chrono::steady_clock::now();
      auto res = blockBlobClient.DownloadToBuffer(buffer.data(), buffer.size());
      auto timer_end = std::chrono::steady_clock::now();

      double speed = static_cast<double>(bufferSize) / 1_MB
          / std::chrono::duration_cast<std::chrono::milliseconds>(timer_end - timer_start).count()
          * 1000;
      std::cout << "Download speed: " << speed << "MiB/s" << std::endl;
    }
  }

}}} // namespace Azure::Storage::Test
