// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Test the performance of downloading a block blob.
 *
 */

#pragma once

#include <azure/performance_framework.hpp>

#include "azure/storage/blobs/test/performance/blob_base_test.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Blobs { namespace Test { namespace Performance {

  /**
   * @brief A test to measure downloading a blob.
   *
   */
  class DownloadBlob : public Azure::Storage::Blobs::Test::Performance::BlobsTest {

  public:
    /**
     * @brief Construct a new DownloadBlob test.
     *
     * @param options The test options.
     */
    DownloadBlob(Azure::PerformanceStress::TestOptions options) : BlobsTest(options) {}

    /**
     * @brief Define the test
     *
     * @param ctx The cancellation token.
     */
    void Run(Azure::Core::Context const&) override { auto blob = m_blobClient->Download(); }

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::PerformanceStress::TestOption> GetTestOptions() override
    {
      return Azure::Storage::Blobs::Test::Performance::BlobsTest::GetTestOptions();
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::PerformanceStress::TestMetadata describing the test.
     */
    static Azure::PerformanceStress::TestMetadata GetTestMetadata()
    {
      return {
          "DownloadBlob", "Download a blob.", [](Azure::PerformanceStress::TestOptions options) {
            return std::make_unique<Azure::Storage::Blobs::Test::Performance::DownloadBlob>(
                options);
          }};
    }
  };

}}}}} // namespace Azure::Storage::Blobs::Test::Performance
