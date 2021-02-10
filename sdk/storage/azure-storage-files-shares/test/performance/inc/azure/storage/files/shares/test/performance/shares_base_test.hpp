// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define the base bahavior of the tests using a files shares.
 *
 */

#pragma once

#include <azure/performance_framework.hpp>

#include <azure/storage/files/shares.hpp>

#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Files { namespace Shares { namespace Test {
  namespace Performance {

    /**
     * @brief A base test that set up a file shares performance test.
     *
     */
    class FileSharesTest : public Azure::PerformanceStress::PerformanceTest {
    protected:
      std::string m_connectionString;
      std::string m_shareName;
      std::string m_fileName;
      std::unique_ptr<Azure::Storage::Files::Shares::ShareClient> m_sharesClient;
      std::unique_ptr<Azure::Storage::Files::Shares::ShareFileClient> m_fileClient;

    public:
      /**
       * @brief Creat the base clients
       *
       */
      void Setup() override
      {
        m_connectionString = m_options.GetMandatoryOption<std::string>("connectionString");
        m_shareName = m_options.GetMandatoryOption<std::string>("ShareName");
        m_fileName = m_options.GetMandatoryOption<std::string>("FileName");

        m_sharesClient = std::make_unique<Azure::Storage::Files::Shares::ShareClient>(
            Azure::Storage::Files::Shares::ShareClient::CreateFromConnectionString(
                m_connectionString, m_shareName));

        // Create file systems and ignore the already exist error.
        m_sharesClient->Create();

        m_fileClient = std::make_unique<Azure::Storage::Files::Shares::ShareFileClient>(
            m_sharesClient->GetRootDirectoryClient().GetFileClient(m_fileName));
      }

      /**
       * @brief Construct a new FileSharesTest test.
       *
       * @param options The test options.
       */
      FileSharesTest(Azure::PerformanceStress::TestOptions options) : PerformanceTest(options) {}

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
            {"ShareName", {"--shareName"}, "The name of the share file", 1, true},
            {"FileName", {"--fileName"}, "The name of the file.", 1, true}};
      }
    };

}}}}}} // namespace Azure::Storage::Files::Shares::Test::Performance
