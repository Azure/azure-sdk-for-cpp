// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define the base bahavior of the tests using a blobs client.
 *
 */

#pragma once

#include <azure/performance_framework.hpp>

#include <azure/storage/blobs.hpp>

#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Blobs { namespace Test { namespace Performance {

  /**
   * @brief A base test that set up a blobs performance test.
   *
   */
  class BlobsTest : public Azure::PerformanceStress::PerformanceTest {
  protected:
    std::string m_containerName;
    std::string m_blobName;
    std::string m_connectionString;
    std::unique_ptr<Azure::Storage::Blobs::BlobContainerClient> m_containerClient;
    std::unique_ptr<Azure::Storage::Blobs::BlockBlobClient> m_blobClient;

  public:
    /**
     * @brief Creat the container client
     *
     */
    void Setup() override
    {
      m_connectionString = m_options.GetMandatoryOption<std::string>("connectionString");
      m_containerName = m_options.GetMandatoryOption<std::string>("ContainerName");
      m_blobName = m_options.GetMandatoryOption<std::string>("BlobName");
      m_containerClient = std::make_unique<Azure::Storage::Blobs::BlobContainerClient>(
          Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
              m_connectionString, m_containerName));
      m_containerClient->CreateIfNotExists();
      m_blobClient = std::make_unique<Azure::Storage::Blobs::BlockBlobClient>(
          m_containerClient->GetBlockBlobClient(m_blobName));
    }

    /**
     * @brief Construct a new BlobsTest test.
     *
     * @param options The test options.
     */
    BlobsTest(Azure::PerformanceStress::TestOptions options) : PerformanceTest(options) {}

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::PerformanceStress::TestOption> GetTestOptions() override
    {
      return {
          {"connectionString",
           {"--connectionString"},
           "The Storage account connection string.",
           1,
           true,
           true},
          {"ContainerName", {"--containerName"}, "The name of a blob container", 1, true},
          {"BlobName", {"--blobName"}, "The name of a blob.", 1, true}};
    }
  };

}}}}} // namespace Azure::Storage::Blobs::Test::Performance
