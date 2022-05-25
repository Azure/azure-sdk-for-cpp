// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>

#include <azure/storage/blobs.hpp>

#include "azure/storage/datamovement/blob_folder.hpp"
#include "azure/storage/datamovement/directory_iterator.hpp"
#include "azure/storage/datamovement/task.hpp"

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {

  struct UploadBlobsFromDirectoryTask final : public Storage::_internal::TaskBase
  {
    explicit UploadBlobsFromDirectoryTask(
        _internal::TaskType type,
        const std::string& source,
        const BlobFolder& destination) noexcept
        : TaskBase(type), Source(source), Destination(destination)
    {
    }
    UploadBlobsFromDirectoryTask(UploadBlobsFromDirectoryTask&& other) noexcept
        : TaskBase(std::move(other)), Source(std::move(other.Source)),
          Destination(std::move(other.Destination)), Iterator(std::move(other.Iterator))
    {
    }

    std::string Source;
    BlobFolder Destination;
    std::unique_ptr<Storage::_internal::DirectoryIterator> Iterator;

    void Execute() noexcept override;
  };

}}}} // namespace Azure::Storage::Blobs::_detail
