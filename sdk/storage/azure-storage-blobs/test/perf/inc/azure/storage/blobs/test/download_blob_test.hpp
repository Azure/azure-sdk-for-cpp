// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Test the performance of downloading a block blob.
 *
 */

#pragma once

#include "azure/storage/blobs/test/blob_base_test.hpp"

#include <azure/core/base64.hpp>
#include <azure/core/io/body_stream.hpp>
#include <azure/perf.hpp>
#include <azure/perf/random_stream.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Blobs { namespace Test {

  /**
   * @brief A test to measure downloading a blob.
   *
   * @details `--download-method` chooses between:
   *  - `buffer` (default, preserves existing behavior): allocate a contiguous buffer and
   *    call `DownloadTo(buffer, size)`.
   *  - `stream`: stream the response with `Download()` and drain its body stream without
   *    materializing the payload in RAM. Use for multi-GiB sizes.
   *
   * `--block-size` and `--concurrency` are forwarded to `DownloadBlobToOptions` for the
   * `buffer` method.
   */
  class DownloadBlob : public Azure::Storage::Blobs::Test::BlobsTest {
  private:
    std::unique_ptr<std::vector<uint8_t>> m_downloadBuffer;
    int64_t m_size = 0;
    std::string m_downloadMethod = "buffer";
    int64_t m_blockSize = 0;
    int m_concurrency = 0;

    static constexpr size_t StreamDrainBufferSize = 1024 * 1024;
    static constexpr size_t StageBlockSize = 256 * 1024 * 1024;

  public:
    /**
     * @brief Construct a new DownloadBlob test.
     *
     * @param options The test options.
     */
    DownloadBlob(Azure::Perf::TestOptions options) : BlobsTest(options) {}

    /**
     * @brief The size to upload on setup is defined by a mandatory parameter.
     *
     */
    void Setup() override
    {
      // Call base to create blob client
      BlobsTest::Setup();

      m_size = m_options.GetMandatoryOption<int64_t>("Size");
      m_downloadMethod = m_options.GetOptionOrDefault<std::string>("DownloadMethod", "buffer");
      m_blockSize = m_options.GetOptionOrDefault<int64_t>("BlockSize", 0);
      m_concurrency = m_options.GetOptionOrDefault<int>("Concurrency", 0);

      if (m_downloadMethod == "buffer")
      {
        m_downloadBuffer = std::make_unique<std::vector<uint8_t>>(m_size);
      }
      else if (m_downloadMethod != "stream")
      {
        throw std::runtime_error(
            "Invalid --download-method '" + m_downloadMethod
            + "'. Expected one of: buffer, stream.");
      }

      StageInBlocks();
    }

    void StageInBlocks()
    {
      auto staging = Azure::Perf::RandomStream::Create(m_size);

      std::vector<uint8_t> blockBuffer(StageBlockSize);
      std::vector<std::string> blockIds;
      uint64_t blockIndex = 0;

      while (true)
      {
        size_t offset = 0;
        while (offset < blockBuffer.size())
        {
          auto read = staging->Read(
              blockBuffer.data() + offset, blockBuffer.size() - offset, Azure::Core::Context());
          if (read == 0)
          {
            break;
          }
          offset += read;
        }

        if (offset == 0)
        {
          break;
        }

        // Block IDs must all be the same length before Base64 encoding; encode the index as
        // an 8-byte big-endian value to guarantee a fixed length.
        uint8_t rawBlockId[8];
        for (int i = 0; i < 8; ++i)
        {
          rawBlockId[7 - i] = static_cast<uint8_t>((blockIndex >> (8 * i)) & 0xFF);
        }
        std::string blockId = Azure::Core::Convert::Base64Encode(
            std::vector<uint8_t>(rawBlockId, rawBlockId + sizeof(rawBlockId)));

        Azure::Core::IO::MemoryBodyStream blockContent(blockBuffer.data(), offset);
        m_blobClient->StageBlock(blockId, blockContent);
        blockIds.push_back(std::move(blockId));
        ++blockIndex;

        if (offset < blockBuffer.size())
        {
          break;
        }
      }

      m_blobClient->CommitBlockList(blockIds);
    }

    /**
     * @brief Define the test
     *
     */
    void Run(Azure::Core::Context const& context) override
    {
      if (m_downloadMethod == "stream")
      {
        auto response = m_blobClient->Download({}, context);
        auto& bodyStream = response.Value.BodyStream;
        if (bodyStream)
        {
          // Drain into a thread-local heap buffer; a stack buffer this large would
          // overflow the default Windows thread stack (1 MiB) under high --parallel.
          static thread_local std::vector<uint8_t> buffer(StreamDrainBufferSize);
          while (true)
          {
            auto read = bodyStream->Read(buffer.data(), buffer.size(), context);
            if (read == 0)
            {
              break;
            }
          }
        }
        return;
      }
      Azure::Storage::Blobs::DownloadBlobToOptions opts;
      if (m_blockSize > 0)
      {
        opts.TransferOptions.ChunkSize = m_blockSize;
      }
      if (m_concurrency > 0)
      {
        opts.TransferOptions.Concurrency = m_concurrency;
      }
      m_blobClient->DownloadTo(m_downloadBuffer->data(), m_downloadBuffer->size(), opts);
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
          {"Size", {"--size"}, "Size of payload (in bytes)", 1, true},
          {"DownloadMethod",
           {"--download-method"},
           "Download method: 'buffer' (default, contiguous buffer via DownloadTo) or "
           "'stream' (drain the response BodyStream, no contiguous buffer).",
           1},
          {"BlockSize",
           {"--block-size"},
           "Chunk size (bytes) for buffer-mode DownloadTo. Default: client default.",
           1},
          {"Concurrency",
           {"--concurrency"},
           "Per-operation concurrency for buffer-mode DownloadTo. Default: client default.",
           1}};
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
