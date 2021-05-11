// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Test the performance of downloading a block blob.
 *
 */

#pragma once

#include <azure/core/io/body_stream.hpp>
#include <azure/perf.hpp>

#include "azure/storage/blobs/test/blob_base_test.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Blobs { namespace Test {

  /**
   * @brief A test to measure downloading a blob.
   *
   */
  class DownloadBlob : public Azure::Storage::Blobs::Test::BlobsTest {

  public:
    /**
     * @brief Construct a new DownloadBlob test.
     *
     * @param options The test options.
     */
    DownloadBlob(Azure::Perf::TestOptions options) : BlobsTest(options) {}

    /**
     * @brief Upload 5Mb to be downloaded in the test
     *
     */
    void Setup() override
    {
      // Call base to create blob client
      BlobsTest::Setup();
      auto rawData = std::make_unique<std::vector<uint8_t>>(1024 * 1024 * 5);
      auto content = Azure::Core::IO::MemoryBodyStream(*rawData);
      m_blobClient->Upload(content);
    }

    /**
     * @brief Define the test
     *
     */
    void Run(Azure::Core::Context const&) override { auto blob = m_blobClient->Download(); }

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::Perf::TestOption> GetTestOptions() override
    {
      return Azure::Storage::Blobs::Test::BlobsTest::GetTestOptions();
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::Perf::TestMetadata describing the test.
     */
    static Azure::Perf::TestMetadata GetTestMetadata()
    {
      return {"DownloadBlob", "Download a blob.", [](Azure::Perf::TestOptions options) {
                return std::make_unique<Azure::Storage::Blobs::Test::DownloadBlob>(options);
              }};
    }
  };

}}}} // namespace Azure::Storage::Blobs::Test
