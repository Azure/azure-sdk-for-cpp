// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/storage/blobs.hpp>

#include "azure/storage/datamovement/task.hpp"

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {

  struct AsyncCopyBlobTask final : public Storage::_internal::TaskBase
  {
    explicit AsyncCopyBlobTask(
        _internal::TaskType type,
        Blobs::BlobClient source,
        Blobs::BlobClient destination)
        : TaskBase(type), Source(std::move(source)), Destination(std::move(destination))
    {
    }

    Blobs::BlobClient Source;
    Blobs::BlobClient Destination;

    void Execute() noexcept override;
  };

  struct WaitAsyncCopyToFinishTask final : public Storage::_internal::TaskBase
  {
    explicit WaitAsyncCopyToFinishTask(
        _internal::TaskType type,
        Blobs::BlobClient source,
        Blobs::BlobClient destination)
        : TaskBase(type), Source(std::move(source)), Destination(std::move(destination))
    {
    }

    Blobs::BlobClient Source;
    Blobs::BlobClient Destination;

    void Execute() noexcept override;
  };

}}}} // namespace Azure::Storage::Blobs::_detail
