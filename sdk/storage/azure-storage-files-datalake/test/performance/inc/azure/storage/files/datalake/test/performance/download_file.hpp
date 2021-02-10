// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Download a Data Lake file.
 *
 */

#pragma once

#include <azure/performance_framework.hpp>

#include "azure/storage/files/datalake/test/performance/datalake_base_test.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Files { namespace DataLake { namespace Test {
  namespace Performance {

    /**
     * @brief A test to measure downloading a data lake file.
     *
     */
    class DownloadFile : public Azure::Storage::Files::DataLake::Test::Performance::DataLakeTest {

    public:
      /**
       * @brief Construct a new DownloadFile test.
       *
       * @param options The test options.
       */
      DownloadFile(Azure::PerformanceStress::TestOptions options) : DataLakeTest(options) {}

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
        return Azure::Storage::Files::DataLake::Test::Performance::DataLakeTest::GetTestOptions();
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
            "Download a data lake file.",
            [](Azure::PerformanceStress::TestOptions options) {
              return std::make_unique<
                  Azure::Storage::Files::DataLake::Test::Performance::DownloadFile>(options);
            }};
      }
    };

}}}}}} // namespace Azure::Storage::Files::DataLake::Test::Performance
