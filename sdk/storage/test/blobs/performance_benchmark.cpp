// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blob_container_client_test.hpp"

#include <chrono>
#include <future>
#include <vector>

namespace Azure { namespace Storage { namespace Test {

  TEST_F(BlobContainerClientTest, DISABLED_SingleThreadPerf)
  {
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_containerName, "SingleThreadPerf" + RandomString());

    constexpr std::size_t bufferSize = static_cast<std::size_t>(1_GB);
    std::vector<uint8_t> buffer = RandomBuffer(bufferSize);
    {
      Blobs::UploadBlockBlobFromOptions options;
      options.ChunkSize = 8_MB;
      auto timer_start = std::chrono::steady_clock::now();
      auto res = blockBlobClient.UploadFrom(buffer.data(), buffer.size(), options);
      auto timer_end = std::chrono::steady_clock::now();

      double speed = static_cast<double>(bufferSize) / 1_MB
          / std::chrono::duration_cast<std::chrono::milliseconds>(timer_end - timer_start).count()
          * 1000;
      std::cout << "Upload speed: " << speed << "MiB/s" << std::endl;
    }
    {
      Blobs::DownloadBlobToOptions options;
      options.InitialChunkSize = 8_MB;
      options.ChunkSize = 8_MB;
      auto timer_start = std::chrono::steady_clock::now();
      auto res = blockBlobClient.DownloadTo(buffer.data(), buffer.size(), options);
      auto timer_end = std::chrono::steady_clock::now();

      double speed = static_cast<double>(bufferSize) / 1_MB
          / std::chrono::duration_cast<std::chrono::milliseconds>(timer_end - timer_start).count()
          * 1000;
      std::cout << "Download speed: " << speed << "MiB/s" << std::endl;
    }
  }

  TEST_F(BlobContainerClientTest, DISABLED_MultiThreadPerf)
  {
    constexpr int concurrency = 64;
    std::vector<Blobs::BlockBlobClient> blockBlobClients;
    for (int i = 0; i < concurrency; ++i)
    {
      blockBlobClients.emplace_back(
          Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
              StandardStorageConnectionString(),
              m_containerName,
              "MultiThreadPerf" + RandomString()));
    }

    constexpr std::size_t bufferSize = static_cast<std::size_t>(1_GB);
    std::vector<uint8_t> buffer = RandomBuffer(bufferSize);

    {
      std::vector<std::future<void>> futures;
      Blobs::UploadBlockBlobFromOptions options;
      options.ChunkSize = 8_MB;
      auto timer_start = std::chrono::steady_clock::now();
      for (int i = 0; i < concurrency; ++i)
      {
        futures.emplace_back(
            std::async(std::launch::async, [&blockBlobClients, &buffer, &options, i]() {
              auto res = blockBlobClients[i].UploadFrom(buffer.data(), buffer.size(), options);
            }));
      }
      for (auto& f : futures)
      {
        f.get();
      }
      auto timer_end = std::chrono::steady_clock::now();
      double speed = static_cast<double>(bufferSize) * concurrency / 1_MB
          / std::chrono::duration_cast<std::chrono::milliseconds>(timer_end - timer_start).count()
          * 1000;
      std::cout << "Upload speed: " << speed << "MiB/s" << std::endl;
    }
    {
      std::vector<std::future<void>> futures;
      Blobs::DownloadBlobToOptions options;
      options.InitialChunkSize = 8_MB;
      options.ChunkSize = 8_MB;
      auto timer_start = std::chrono::steady_clock::now();
      for (int i = 0; i < concurrency; ++i)
      {
        futures.emplace_back(
            std::async(std::launch::async, [&blockBlobClients, &buffer, &options, i]() {
              auto res = blockBlobClients[i].DownloadTo(buffer.data(), buffer.size(), options);
            }));
      }
      for (auto& f : futures)
      {
        f.get();
      }
      auto timer_end = std::chrono::steady_clock::now();
      double speed = static_cast<double>(bufferSize) * concurrency / 1_MB
          / std::chrono::duration_cast<std::chrono::milliseconds>(timer_end - timer_start).count()
          * 1000;
      std::cout << "Download speed: " << speed << "MiB/s" << std::endl;
    }
  }

}}} // namespace Azure::Storage::Test
