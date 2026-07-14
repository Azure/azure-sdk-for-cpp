// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Test the performance of listing blobs.
 *
 */

#pragma once

#include "azure/storage/blobs/test/blob_base_test.hpp"

#include <azure/core/io/body_stream.hpp>
#include <azure/core/uuid.hpp>
#include <azure/perf.hpp>

#include <algorithm>
#include <atomic>
#include <exception>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace Azure { namespace Storage { namespace Blobs { namespace Test {

  /**
   * @brief A test to measure listing a blob.
   *
   */
  class ListBlob : public Azure::Storage::Blobs::Test::BlobsTest {
  private:
    int m_pageSize = 0;

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
      // --num-blobs is the canonical name (matches the Go perf harness); --count is kept
      // for backward compatibility with existing test definitions.
      long count = 0;
      if (m_options.HasOption("NumBlobs"))
      {
        count = m_options.GetMandatoryOption<long>("NumBlobs");
      }
      else
      {
        count = m_options.GetMandatoryOption<long>("Count");
      }
      m_pageSize = m_options.GetOptionOrDefault<int>("PageSize", 0);

      // Upload the number of blobs to be listed later in the test, using multiple
      // threads to upload the blobs in parallel to speed up setup.
      unsigned int hardwareThreads = std::thread::hardware_concurrency();
      if (hardwareThreads == 0)
      {
        hardwareThreads = 4;
      }
      const unsigned int threadCount = static_cast<unsigned int>(
          std::min<long>(static_cast<long>(hardwareThreads), std::max<long>(count, 1)));

      std::atomic<long> nextBlobIndex(0);
      std::mutex exceptionMutex;
      std::exception_ptr firstException;
      auto uploadWorker = [this, count, &nextBlobIndex, &exceptionMutex, &firstException]() {
        try
        {
          std::vector<uint8_t> rawData(1);
          for (long blobCount = nextBlobIndex.fetch_add(1); blobCount < count;
               blobCount = nextBlobIndex.fetch_add(1))
          {
            auto content = Azure::Core::IO::MemoryBodyStream(rawData);
            auto blobName = "Azure.Storage.Blobs.Perf.Scenarios.ListBlob-"
                + Azure::Core::Uuid::CreateUuid().ToString();
            m_containerClient->GetBlockBlobClient(blobName).Upload(content);
          }
        }
        catch (...)
        {
          // Capture the first exception thrown by any worker so it can be rethrown after
          // joining, avoiding std::terminate from an exception escaping the thread function.
          std::lock_guard<std::mutex> lock(exceptionMutex);
          if (!firstException)
          {
            firstException = std::current_exception();
          }
        }
      };

      std::vector<std::thread> threads;
      threads.reserve(threadCount);
      for (unsigned int i = 0; i < threadCount; i++)
      {
        threads.emplace_back(uploadWorker);
      }
      for (auto& thread : threads)
      {
        thread.join();
      }

      if (firstException)
      {
        std::rethrow_exception(firstException);
      }
    }

    /**
     * @brief Define the test
     *
     */
    void Run(Azure::Core::Context const& context) override
    {
      Azure::Storage::Blobs::ListBlobsOptions opts;
      if (m_pageSize > 0)
      {
        opts.PageSizeHint = m_pageSize;
      }
      // Loop each page
      auto page = m_containerClient->ListBlobs(opts, context);
      for (; page.HasPage(); page.MoveToNextPage(context))
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
      return {
          {"TokenCredential",
           {"--token-credential"},
           "Use a token credential to run the test. By default, a connection string is used.",
           0},
          {"Count", {"--count"}, "Number of blobs to list (legacy alias of --num-blobs).", 1},
          {"NumBlobs", {"--num-blobs"}, "Number of blobs to list.", 1},
          {"PageSize",
           {"--page-size"},
           "Server page size hint for ListBlobs. Default: server default.",
           1}};
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
