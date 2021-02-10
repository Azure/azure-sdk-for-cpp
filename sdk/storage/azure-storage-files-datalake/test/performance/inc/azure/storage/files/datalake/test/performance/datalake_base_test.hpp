// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Define the base bahavior of the tests using a data lake client.
 *
 */

#pragma once

#include <azure/performance_framework.hpp>

#include <azure/storage/files/datalake.hpp>

#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Files { namespace DataLake { namespace Test {
  namespace Performance {

    /**
     * @brief A base test that set up a data lake performance test.
     *
     */
    class DataLakeTest : public Azure::PerformanceStress::PerformanceTest {
    protected:
      std::string m_connectionString;
      std::string m_fileSystemName;
      std::string m_directoryName;
      std::string m_fileName;
      std::unique_ptr<Azure::Storage::Files::DataLake::DataLakeServiceClient> m_serviceClient;
      std::unique_ptr<Azure::Storage::Files::DataLake::DataLakeFileSystemClient> m_fileSystemClient;
      std::unique_ptr<Azure::Storage::Files::DataLake::DataLakeDirectoryClient> m_directoryClient;
      std::unique_ptr<Azure::Storage::Files::DataLake::DataLakeFileClient> m_fileClient;

    public:
      /**
       * @brief Creat the data lake clients.
       *
       */
      void Setup() override
      {
        m_connectionString = m_options.GetMandatoryOption<std::string>("connectionString");
        m_fileSystemName = m_options.GetMandatoryOption<std::string>("FileSystemName");
        m_directoryName = m_options.GetMandatoryOption<std::string>("DirectoryName");
        m_fileName = m_options.GetMandatoryOption<std::string>("FileName");

        m_serviceClient = std::make_unique<Azure::Storage::Files::DataLake::DataLakeServiceClient>(
            Azure::Storage::Files::DataLake::DataLakeServiceClient::CreateFromConnectionString(
                m_connectionString));

        m_fileSystemClient = std::make_unique<
            Azure::Storage::Files::DataLake::DataLakeFileSystemClient>(
            Azure::Storage::Files::DataLake::DataLakeFileSystemClient::CreateFromConnectionString(
                m_connectionString, m_fileSystemName));
        // Create file systems and ignore the already exist error.
        try
        {
          m_fileSystemClient->Create();
        }
        catch (Azure::Storage::StorageException& e)
        {
          if (e.ErrorCode != "ContainerAlreadyExists")
          {
            throw;
          }
        }
        m_directoryClient
            = std::make_unique<Azure::Storage::Files::DataLake::DataLakeDirectoryClient>(
                m_fileSystemClient->GetDirectoryClient(m_directoryName));
        m_directoryClient->Create();
        m_fileClient = std::make_unique<Azure::Storage::Files::DataLake::DataLakeFileClient>(
            m_directoryClient->GetFileClient(m_fileName));
        m_fileClient->Create();
      }

      /**
       * @brief Construct a new DataLakeTest test.
       *
       * @param options The test options.
       */
      DataLakeTest(Azure::PerformanceStress::TestOptions options) : PerformanceTest(options) {}

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
            {"FileSystemName", {"--fileSystemName"}, "The name of the file system", 1, true},
            {"DirectoryName", {"--directoryName"}, "The name of the directory name.", 1, true},
            {"FileName", {"--fileName"}, "The name of the file name.", 1, true}};
      }
    };

}}}}}} // namespace Azure::Storage::Files::DataLake::Test::Performance
