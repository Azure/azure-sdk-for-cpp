// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Test the performance of downloading a block blob using SaS token.
 *
 */

#pragma once

#include <azure/core/io/body_stream.hpp>
#include <azure/perf.hpp>

#include "azure/storage/blobs/test/blob_base_test.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Blobs { namespace Test {

  /**
   * @brief A test to measure downloading a blob using SaS token.
   *
   */
  class DownloadBlobSas : public Azure::Storage::Blobs::Test::BlobsTest {
  private:
    std::unique_ptr<std::vector<uint8_t>> m_downloadBuffer;
    std::unique_ptr<Azure::Storage::Blobs::BlockBlobClient> m_blobClientSas;

  public:
    /**
     * @brief Construct a new DownloadBlobSas test.
     *
     * @param options The test options.
     */
    DownloadBlobSas(Azure::Perf::TestOptions options) : BlobsTest(options) {}

    /**
     * @brief The size to upload on setup is defined by a mandatory parameter.
     *
     */
    void Setup() override
    {
      // Call base to create blob client
      BlobsTest::Setup();

      long size = m_options.GetMandatoryOption<long>("Size");

      m_downloadBuffer = std::make_unique<std::vector<uint8_t>>(size);

      auto rawData = std::make_unique<std::vector<uint8_t>>(size);
      auto content = Azure::Core::IO::MemoryBodyStream(*rawData);
      m_blobClient->Upload(content);

      m_blobClientSas = std::make_unique<Azure::Storage::Blobs::BlockBlobClient>(
          m_blobClient->GetUrl() + GetSasToken());
    }

    /**
     * @brief Define the test
     *
     */
    void Run(Azure::Core::Context const&) override
    {
      m_blobClientSas->DownloadTo(m_downloadBuffer->data(), m_downloadBuffer->size());
    }

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::Perf::TestOption> GetTestOptions() override
    {
      // TODO: Merge with base options
      return {{"Size", {"--size"}, "Size of payload (in bytes)", 1, true}};
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::Perf::TestMetadata describing the test.
     */
    static Azure::Perf::TestMetadata GetTestMetadata()
    {
      return {"DownloadBlobSas", "Download a blob.", [](Azure::Perf::TestOptions options) {
                return std::make_unique<Azure::Storage::Blobs::Test::DownloadBlobSas>(options);
              }};
    }
  };

}}}} // namespace Azure::Storage::Blobs::Test
