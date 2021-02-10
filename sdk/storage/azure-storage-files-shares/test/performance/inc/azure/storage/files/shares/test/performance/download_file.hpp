// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Download a share file.
 *
 */

#pragma once

#include <azure/performance_framework.hpp>

#include "azure/storage/files/shares/test/performance/shares_base_test.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Files { namespace Shares { namespace Test {
  namespace Performance {

    /**
     * @brief A test to measure downloading a share file.
     *
     */
    class DownloadFile : public Azure::Storage::Files::Shares::Test::Performance::FileSharesTest {

    public:
      /**
       * @brief Construct a new DownloadFile test.
       *
       * @param options The test options.
       */
      DownloadFile(Azure::PerformanceStress::TestOptions options) : FileSharesTest(options) {}

      /**
       * @brief Define the test
       *
       * @param ctx The cancellation token.
       */
      void Run(Azure::Core::Context const&) override { auto file = m_fileClient->Download(); }

      /**
       * @brief Define the test options for the test.
       *
       * @return The list of test options.
       */
      std::vector<Azure::PerformanceStress::TestOption> GetTestOptions() override
      {
        return Azure::Storage::Files::Shares::Test::Performance::FileSharesTest::GetTestOptions();
      }

      /**
       * @brief Get the static Test Metadata for the test.
       *
       * @return Azure::PerformanceStress::TestMetadata describing the test.
       */
      static Azure::PerformanceStress::TestMetadata GetTestMetadata()
      {
        return {
            "DownloadFile",
            "Download a share file.",
            [](Azure::PerformanceStress::TestOptions options) {
              return std::make_unique<
                  Azure::Storage::Files::Shares::Test::Performance::DownloadFile>(options);
            }};
      }
    };

}}}}}} // namespace Azure::Storage::Files::Shares::Test::Performance
