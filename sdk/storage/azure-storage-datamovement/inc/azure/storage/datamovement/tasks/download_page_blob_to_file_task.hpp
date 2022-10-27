// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <azure/storage/blobs/page_blob_client.hpp>
#include <azure/storage/common/internal/file_io.hpp>

#include "azure/storage/datamovement/datamovement_options.hpp"
#include "azure/storage/datamovement/task.hpp"
#include "azure/storage/datamovement/tasks/download_blob_to_file_task.hpp"

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {

  struct DownloadPageBlobRangeToMemoryTask final : public Storage::_internal::TaskBase
  {
    using TaskBase::TaskBase;

    struct TaskContext final
    {
      explicit TaskContext(Blobs::PageBlobClient source, std::string destination)
          : Source(std::move(source)), Destination(std::move(destination))
      {
      }

      Blobs::PageBlobClient Source;
      std::string Destination;

      std::mutex FileWriterMutex;
      std::unique_ptr<Storage::_internal::FileWriter> FileWriter;
      uint64_t FileSize{0};
      int NumChunks{0};
      std::atomic<int> NumDownloadedChunks{0};
      std::atomic<bool> Failed{false};
    };

    std::shared_ptr<TaskContext> Context;
    std::vector<Azure::Core::Http::HttpRange> Ranges;

    void Execute() noexcept override;
  };

  struct WritePageBlobRangesToSparseFileTask final : public Storage::_internal::TaskBase
  {
    using TaskBase::TaskBase;

    std::shared_ptr<DownloadPageBlobRangeToMemoryTask::TaskContext> Context;
    std::vector<Azure::Core::Http::HttpRange> Ranges;
    std::unique_ptr<uint8_t[]> Buffer;

    void Execute() noexcept override;
  };

}}}} // namespace Azure::Storage::Blobs::_detail
