//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Test the performance of listing blobs.
 *
 */

#pragma once

#include <azure/core/io/body_stream.hpp>
#include <azure/core/uuid.hpp>
#include <azure/perf.hpp>

#include "azure/storage/blobs/test/blob_base_test.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Blobs { namespace Test {

  /**
   * @brief A test to measure listing a blob.
   *
   */
  class ListBlob : public Azure::Storage::Blobs::Test::BlobsTest {
  public:
    /**
     * @brief Construct a new ListBlob test.
     *
     * @param options The test options.
     */
    ListBlob(Azure::Perf::TestOptions options) : BlobsTest(options) {}

    /**
     * @brief The size to upload on setup is defined by a mandatory parameter.
     *
     */
    void Setup() override
    {
      // Call base to create blob client
      BlobsTest::Setup();
      long count = m_options.GetMandatoryOption<long>("Count");

      auto rawData = std::make_unique<std::vector<uint8_t>>(1);
      auto content = Azure::Core::IO::MemoryBodyStream(*rawData);

      // Upload the number of blobs to be listed later in the test
      for (auto blobCount = 0; blobCount < count; blobCount++)
      {
        auto blobName = "Azure.Storage.Blobs.Perf.Scenarios.DownloadBlob-"
            + Azure::Core::Uuid::CreateUuid().ToString();
        m_containerClient->GetBlockBlobClient(blobName).Upload(content);
      }
    }

    /**
     * @brief Define the test
     *
     */
    void Run(Azure::Core::Context const& context) override
    {
      // Loop each page
      for (auto page = m_containerClient->ListBlobs({}, context); page.HasPage();
           page.MoveToNextPage(context))
      {
        // loop each blob
        for (auto blob : page.Blobs)
        {
          (void)blob;
        }
      }
    }

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::Perf::TestOption> GetTestOptions() override
    {
      // TODO: Merge with base options
      return {{"Count", {"--count"}, "Number of blobs to list", 1, true}};
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::Perf::TestMetadata describing the test.
     */
    static Azure::Perf::TestMetadata GetTestMetadata()
    {
      return {"ListBlob", "List blobs.", [](Azure::Perf::TestOptions options) {
                return std::make_unique<Azure::Storage::Blobs::Test::ListBlob>(options);
              }};
    }
  };

}}}} // namespace Azure::Storage::Blobs::Test
