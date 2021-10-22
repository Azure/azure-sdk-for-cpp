// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define the base bahavior of the tests using a blobs client.
 *
 */

#pragma once

#include <azure/core/uuid.hpp>
#include <azure/perf.hpp>

#include <azure/storage/blobs.hpp>

#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Blobs { namespace Test {

  /**
   * @brief A base test that set up a blobs performance test.
   *
   */
  class BlobsTest : public Azure::Perf::PerfTest {
  protected:
    std::string m_containerName;
    std::string m_blobName;
    std::string m_connectionString;
    std::unique_ptr<Azure::Storage::Blobs::BlobServiceClient> m_serviceClient;
    std::unique_ptr<Azure::Storage::Blobs::BlobContainerClient> m_containerClient;
    std::unique_ptr<Azure::Storage::Blobs::BlockBlobClient> m_blobClient;

  public:
    /**
     * @brief Creat the container client
     *
     */
    void Setup() override
    {
      // Get connection string from env
      const static std::string envConnectionString = std::getenv("STORAGE_CONNECTION_STRING");
      m_connectionString = envConnectionString;

      // Generate random container and blob names.
      m_containerName = "container" + Azure::Core::Uuid::CreateUuid().ToString();
      m_blobName = "blob" + Azure::Core::Uuid::CreateUuid().ToString();

      // Create client, container and blobClient
      m_serviceClient = std::make_unique<Azure::Storage::Blobs::BlobServiceClient>(
          Azure::Storage::Blobs::BlobServiceClient::CreateFromConnectionString(m_connectionString));
      m_containerClient = std::make_unique<Azure::Storage::Blobs::BlobContainerClient>(
          m_serviceClient->GetBlobContainerClient(m_containerName));
      m_containerClient->CreateIfNotExists();
      m_blobClient = std::make_unique<Azure::Storage::Blobs::BlockBlobClient>(
          m_containerClient->GetBlockBlobClient(m_blobName));
    }

    void Cleanup() override { m_containerClient->DeleteIfExists(); }

    /**
     * @brief Construct a new BlobsTest test.
     *
     * @param options The test options.
     */
    BlobsTest(Azure::Perf::TestOptions options) : PerfTest(options) {}

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::Perf::TestOption> GetTestOptions() override { return {}; }
  };

}}}} // namespace Azure::Storage::Blobs::Test
