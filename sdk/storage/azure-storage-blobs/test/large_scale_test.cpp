// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs.hpp"
#include "azure/storage/common/file_io.hpp"
#include "test_base.hpp"

#include <chrono>

namespace Azure { namespace Storage { namespace Test {

  TEST(BlobsLargeScaleTest, DISABLED_LargeScaleUpload)
  {
    const std::string containerName = "large-scale-test";
    auto containerClient = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
        StandardStorageConnectionString(), containerName);
    try
    {
      containerClient.Create();
    }
    catch (StorageError& e)
    {
      if (e.StatusCode != Azure::Core::Http::HttpStatusCode::Conflict)
      {
        throw;
      }
    }
    auto blockBlobClient = Azure::Storage::Blobs::BlockBlobClient::CreateFromConnectionString(
        StandardStorageConnectionString(), containerName, "LargeScale" + RandomString());

    const std::string sourceFile = "";
    std::size_t fileSize = 0;
    constexpr std::size_t concurrency = 16;

    ASSERT_FALSE(sourceFile.empty());
    {
      Details::FileReader reader(sourceFile);
      fileSize = static_cast<std::size_t>(reader.GetFileSize());
    }

    Blobs::UploadBlockBlobFromOptions options;
    options.Concurrency = concurrency;
    auto timer_start = std::chrono::steady_clock::now();
    try
    {
      auto res = blockBlobClient.UploadFrom(sourceFile, options);
    }
    catch (std::exception& e)
    {
      std::cout << e.what() << std::endl;
    }
    auto timer_end = std::chrono::steady_clock::now();

    double speed = static_cast<double>(fileSize) / 1_MB
        / std::chrono::duration_cast<std::chrono::milliseconds>(timer_end - timer_start).count()
        * 1000;
    std::cout << "Upload " << static_cast<double>(fileSize) / 1_GB << "GiB, speed " << speed
              << "MiB/s" << std::endl;
  }

}}} // namespace Azure::Storage::Test
