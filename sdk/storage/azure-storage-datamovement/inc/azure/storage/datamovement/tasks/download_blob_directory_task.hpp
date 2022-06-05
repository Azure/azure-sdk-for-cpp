// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/storage/datamovement/blob_folder.hpp"
#include "azure/storage/datamovement/task.hpp"

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {

  struct DownloadBlobDirectoryTask final : public Storage::_internal::TaskBase
  {
    DownloadBlobDirectoryTask(
        _internal::TaskType type,
        const Blobs::BlobFolder& source,
        const std::string& destination) noexcept
        : TaskBase(type), Context(std::make_shared<TaskContext>(source, destination))
    {
    }

    struct TaskContext final
    {
      explicit TaskContext(Blobs::BlobFolder source, std::string destination)
          : Source(std::move(source)), Destination(std::move(destination))
      {
      }
      Blobs::BlobFolder Source;
      std::string Destination;
      Azure::Nullable<std::string> ContinuationToken;
    };
    std::shared_ptr<TaskContext> Context;

    void Execute() noexcept override;
  };

}}}} // namespace Azure::Storage::Blobs::_detail
