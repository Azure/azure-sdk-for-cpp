// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/tasks/async_copy_blob_task.hpp"

#include "azure/storage/datamovement/task_shared_status.hpp"
#include "azure/storage/datamovement/transfer_engine.hpp"
#include "azure/storage/datamovement/utilities.hpp"

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {

  void AsyncCopyBlobTask::Execute() noexcept
  {
    enum class Action
    {
      StartCopy,
      WaitToFinish,
    };
    auto action = Action::StartCopy;

    try
    {
      Models::BlobProperties destinationProperties = Destination.GetProperties().Value;
      if (destinationProperties.CopySource.HasValue()
          && _internal::RemoveSasToken(destinationProperties.CopySource.Value())
              == _internal::RemoveSasToken(Source.GetUrl())
          && destinationProperties.CopyStatus.Value() == Models::CopyStatus::Pending)
      {
        action = Action::WaitToFinish;
      }
      // TODO: if src url matches and status is successful and copy completion time is later than
      // job start time, we don't need to copy again.
    }
    catch (StorageException&)
    {
    }

    if (action == Action::StartCopy)
    {
      try
      {
        auto operation = Destination.StartCopyFromUri(Source.GetUrl());
        Models::CopyStatus copyStatus(
            operation.GetRawResponse().GetHeaders().at("x-ms-copy-status"));
        if (copyStatus == Models::CopyStatus::Success)
        {
          // TODO: it tasks one extra HTTP request to get real copy size.
          TransferSucceeded(0);
          return;
        }
        else if (copyStatus == Models::CopyStatus::Pending)
        {
          action = Action::WaitToFinish;
        }
        else
        {
          TransferFailed(Source.GetUrl(), Destination.GetUrl());
          return;
        }
      }
      catch (std::exception&)
      {
        TransferFailed(Source.GetUrl(), Destination.GetUrl());
        return;
      }
    }
    if (action == Action::WaitToFinish)
    {
      auto waitTask = CreateTask<WaitAsyncCopyToFinishTask>(
          _internal::TaskType::NetworkUpload, std::move(Source), std::move(Destination));
      std::swap(waitTask->MemoryGiveBack, this->MemoryGiveBack);
      waitTask->JournalContext = std::move(JournalContext);
      // TODO: tune the wait time
      SharedStatus->TransferEngine->AddTimedWaitTask(5000, std::move(waitTask));
      return;
    }
    AZURE_UNREACHABLE_CODE();
  }

  void WaitAsyncCopyToFinishTask::Execute() noexcept
  {
    try
    {
      auto properties = Destination.GetProperties().Value;
      if (!properties.CopyStatus.HasValue())
      {
        TransferFailed(Source.GetUrl(), Destination.GetUrl());
        return;
      }
      if (properties.CopyStatus.Value() == Models::CopyStatus::Success)
      {
        TransferSucceeded(properties.BlobSize);
        return;
      }
      else if (properties.CopyStatus.Value() == Models::CopyStatus::Pending)
      {
        auto waitTask = CreateTask<WaitAsyncCopyToFinishTask>(
            _internal::TaskType::NetworkUpload, std::move(Source), std::move(Destination));
        std::swap(waitTask->MemoryGiveBack, this->MemoryGiveBack);
        waitTask->JournalContext = std::move(JournalContext);
        // TODO: tune the wait time
        SharedStatus->TransferEngine->AddTimedWaitTask(5000, std::move(waitTask));
        return;
      }
      else
      {
        TransferFailed(Source.GetUrl(), Destination.GetUrl());
        return;
      }
    }
    catch (StorageException&)
    {
      TransferFailed(Source.GetUrl(), Destination.GetUrl());
      return;
    }
  }

}}}} // namespace Azure::Storage::Blobs::_detail
