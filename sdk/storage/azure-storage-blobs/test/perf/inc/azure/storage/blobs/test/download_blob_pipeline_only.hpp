// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Test the performance of downloading a block blob using SaS token and with the http core
 * pipeline directly.
 *
 */

#pragma once

#include <azure/core/http/curl_transport.hpp>
#include <azure/core/io/body_stream.hpp>
#include <azure/perf.hpp>

#include "azure/storage/blobs/test/blob_base_test.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Blobs { namespace Test {

  /**
   * @brief A test to measure downloading a blob using SaS token and with the http core pipeline
   * directly.
   *
   */
  class DownloadBlobWithPipelineOnly : public Azure::Storage::Blobs::Test::BlobsTest {
  private:
    std::unique_ptr<std::vector<uint8_t>> m_downloadBuffer;
    std::unique_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;
    std::unique_ptr<Azure::Core::Http::Request> m_request;

  public:
    /**
     * @brief Construct a new DownloadBlobWithPipelineOnly test.
     *
     * @param options The test options.
     */
    DownloadBlobWithPipelineOnly(Azure::Perf::TestOptions options) : BlobsTest(options) {}

    /**
     * @brief The size to upload on setup is defined by a mandatory parameter.
     *
     */
    void Setup() override
    {
      // Call base to create blob client
      BlobsTest::Setup();

      long size = m_options.GetMandatoryOption<long>("Size");
      bool bufferResponse = m_options.GetMandatoryOption<bool>("Buffer");

      m_downloadBuffer = std::make_unique<std::vector<uint8_t>>(size);

      auto rawData = std::make_unique<std::vector<uint8_t>>(size);
      auto content = Azure::Core::IO::MemoryBodyStream(*rawData);
      m_blobClient->Upload(content);

      auto requestUrl = m_blobClient->GetUrl() + GetSasToken();

      m_request = std::make_unique<Azure::Core::Http::Request>(
          Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(requestUrl), bufferResponse);

      Azure::Core::_internal::ClientOptions options;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetry;
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperation;
      m_pipeline = std::make_unique<Azure::Core::Http::_internal::HttpPipeline>(
          options, "perfTest", "x.x", std::move(perRetry), std::move(perOperation));
    }

    /**
     * @brief Define the test
     *
     */
    void Run(Azure::Core::Context const& context) override
    {
      // Transport policy resolved the buffer option. If buffer is ON on the request, the response
      // will contain the payload directly. When it is OFF, the response will contain the stream to
      // the network.
      auto response = m_pipeline->Send(*m_request, context);
    }

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::Perf::TestOption> GetTestOptions() override
    {
      // TODO: Merge with base options
      return {
          {"Size", {"--size"}, "Size of payload (in bytes)", 1, true},
          {"Buffer", {"--buffer"}, "Whether to buffer the response", 1, true}};
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::Perf::TestMetadata describing the test.
     */
    static Azure::Perf::TestMetadata GetTestMetadata()
    {
      return {
          "DownloadBlobWithPipelineOnly",
          "Download a blob using the curl transport adapter directly. No SDK layer.",
          [](Azure::Perf::TestOptions options) {
            return std::make_unique<Azure::Storage::Blobs::Test::DownloadBlobWithPipelineOnly>(
                options);
          }};
    }
  };

}}}} // namespace Azure::Storage::Blobs::Test
