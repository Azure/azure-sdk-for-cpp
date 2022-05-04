// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <memory>

#include <azure/storage/blobs.hpp>

#include "azure/storage/datamovement/blob_folder.hpp"
#include "azure/storage/datamovement/directory_iterator.hpp"
#include "azure/storage/datamovement/task.hpp"

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {

  struct UploadBlobsFromDirectoryTask final : public Storage::_internal::TaskBase
  {
    explicit UploadBlobsFromDirectoryTask(
        Storage::_internal::TaskType type,
        Storage::_internal::Scheduler* scheduler,
        const std::string& source,
        const BlobFolder& destination)
        : TaskBase(type, scheduler), Source(source), Destination(destination), Iterator(source)
    {
    }
    UploadBlobsFromDirectoryTask(UploadBlobsFromDirectoryTask&& other) noexcept
        : TaskBase(std::move(other)), Source(std::move(other.Source)),
          Destination(std::move(other.Destination)), Iterator(std::move(other.Iterator))
    {
    }

    std::string Source;
    BlobFolder Destination;
    Storage::_internal::DirectoryIterator Iterator;

    void Execute() override;
  };

}}}} // namespace Azure::Storage::Blobs::_detail
