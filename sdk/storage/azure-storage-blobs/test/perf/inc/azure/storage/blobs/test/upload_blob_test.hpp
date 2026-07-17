// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Test the performance of uploading a block blob.
 *
 */

#pragma once

#include "azure/storage/blobs/test/blob_base_test.hpp"

#include <azure/core/io/body_stream.hpp>
#include <azure/perf.hpp>
#include <azure/perf/random_stream.hpp>

#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Blobs { namespace Test {

  /**
   * @brief A test to measure uploading a blob.
   *
   * @details Supports three upload methods selected via `--upload-method`:
   *  - `buffer` (default, preserves existing behavior): build a contiguous in-memory
   *    payload and call `BlockBlobClient::UploadFrom(buffer, size)`. Guarded by a
   *    `size * parallel` memory-budget check to avoid OOM kills.
   *  - `stream`: do not materialize the payload; stream a circular `RandomStream` into
   *    `BlockBlobClient::Upload(BodyStream)`. Use for multi-GiB sizes.
   *  - `single`: same as `buffer` but uses the single-shot `Upload(BodyStream)` for the
   *    in-memory buffer (no chunked staging). Useful to compare buffered vs. chunked
   *    upload paths.
   *
   * `--block-size` and `--concurrency` are forwarded to `UploadBlockBlobFromOptions` for
   * the `buffer` method.
   */
  class UploadBlob : public Azure::Storage::Blobs::Test::BlobsTest {
  private:
    // C++ can upload and download from contiguous memory or file only
    std::vector<uint8_t> m_uploadBuffer;
    // 64-bit: on MSVC `long` is 32-bit, so sizes above ~2.14 GB overflow when parsed.
    int64_t m_size = 0;
    std::string m_uploadMethod = "buffer";
    int64_t m_blockSize = 0;
    int m_concurrency = 0;

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

      m_size = m_options.GetMandatoryOption<int64_t>("Size");
      m_uploadMethod = m_options.GetOptionOrDefault<std::string>("UploadMethod", "buffer");
      m_blockSize = m_options.GetOptionOrDefault<int64_t>("BlockSize", 0);
      m_concurrency = m_options.GetOptionOrDefault<int>("Concurrency", 0);

      if (m_uploadMethod == "buffer" || m_uploadMethod == "single")
      {
        m_uploadBuffer
            = Azure::Perf::RandomStream::Create(m_size)->ReadToEnd(Azure::Core::Context{});
      }
      else if (m_uploadMethod != "stream")
      {
        throw std::runtime_error(
            "Invalid --upload-method '" + m_uploadMethod
            + "'. Expected one of: buffer, stream, single.");
      }
    }

    /**
     * @brief Define the test
     *
     */
    void Run(Azure::Core::Context const&) override
    {
      if (m_uploadMethod == "stream")
      {
        auto stream = Azure::Perf::RandomStream::Create(m_size);
        m_blobClient->Upload(*stream);
        return;
      }
      if (m_uploadMethod == "single")
      {
        auto stream = Azure::Core::IO::MemoryBodyStream(m_uploadBuffer);
        m_blobClient->Upload(stream);
        return;
      }
      // Default: buffer (chunked via UploadFrom).
      Azure::Storage::Blobs::UploadBlockBlobFromOptions opts;
      if (m_blockSize > 0)
      {
        opts.TransferOptions.ChunkSize = m_blockSize;
      }
      if (m_concurrency > 0)
      {
        opts.TransferOptions.Concurrency = m_concurrency;
      }
      m_blobClient->UploadFrom(m_uploadBuffer.data(), m_uploadBuffer.size(), opts);
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
          {"Size", {"--size", "-s"}, "Size of payload (in bytes)", 1, true},
          {"UploadMethod",
           {"--upload-method"},
           "Upload method: 'buffer' (default, chunked UploadFrom), 'stream' (Upload "
           "BodyStream from a circular RandomStream, no contiguous buffer), or 'single' "
           "(single-shot Upload of an in-memory buffer).",
           1},
          {"BlockSize",
           {"--block-size"},
           "Chunk size (bytes) for buffer-mode UploadFrom. Default: client default.",
           1},
          {"Concurrency",
           {"--concurrency"},
           "Per-operation concurrency for buffer-mode UploadFrom. Default: client default.",
           1}};
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
