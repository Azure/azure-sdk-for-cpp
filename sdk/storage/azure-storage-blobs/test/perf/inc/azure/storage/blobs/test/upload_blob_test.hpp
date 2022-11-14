//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Test the performance of uploading a block blob.
 *
 */

#pragma once

#include <azure/core/io/body_stream.hpp>
#include <azure/perf.hpp>
#include <azure/perf/random_stream.hpp>

#include "azure/storage/blobs/test/blob_base_test.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Blobs { namespace Test {

  /**
   * @brief A test to measure uploading a blob.
   *
   */
  class UploadBlob : public Azure::Storage::Blobs::Test::BlobsTest {
  private:
    // C++ can upload and download from contiguos memory or file only
    std::vector<uint8_t> m_uploadBuffer;

  public:
    /**
     * @brief Construct a new UploadBlob test.
     *
     * @param options The test options.
     */
    UploadBlob(Azure::Perf::TestOptions options) : BlobsTest(options) {}

    /**
     * @brief The size to upload on setup is defined by a mandatory parameter.
     *
     */
    void Setup() override
    {
      // Call base to create blob client
      BlobsTest::Setup();

      long size = m_options.GetMandatoryOption<long>("Size");
      m_uploadBuffer = Azure::Perf::RandomStream::Create(size)->ReadToEnd(
          Azure::Core::Context::ApplicationContext);
    }

    /**
     * @brief Define the test
     *
     */
    void Run(Azure::Core::Context const&) override
    {
      m_blobClient->UploadFrom(m_uploadBuffer.data(), m_uploadBuffer.size());
    }

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::Perf::TestOption> GetTestOptions() override
    {
      // TODO: Merge with base options
      return {{"Size", {"--size", "-s"}, "Size of payload (in bytes)", 1, true}};
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::Perf::TestMetadata describing the test.
     */
    static Azure::Perf::TestMetadata GetTestMetadata()
    {
      return {"UploadBlob", "Upload a blob.", [](Azure::Perf::TestOptions options) {
                return std::make_unique<Azure::Storage::Blobs::Test::UploadBlob>(options);
              }};
    }
  };

}}}} // namespace Azure::Storage::Blobs::Test
