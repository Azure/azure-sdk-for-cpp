// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/tasks/upload_blobs_from_directory_task.hpp"

#include <vector>

#include <azure/storage/datamovement/tasks/upload_blob_from_file_task.hpp>

#include "azure/storage/datamovement/job_properties.hpp"
#include "azure/storage/datamovement/scheduler.hpp"
#include "azure/storage/datamovement/utilities.hpp"

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {
  namespace {
    constexpr int ListBatchSize = 1000;
  }

  void UploadBlobsFromDirectoryTask::Execute() noexcept
  {
    if (!Iterator)
    {
      try
      {
        Iterator = std::make_unique<_internal::DirectoryIterator>(Source);
      }
      catch (std::exception&)
      {
        SharedStatus->TaskFailedCallback(1, _internal::GetFileUrl(Source), Destination.GetUrl());
        return;
      }
    }
    std::vector<_internal::Task> subtasks;

    bool hasMoreEntries = true;
    while (subtasks.size() < ListBatchSize)
    {
      _internal::DirectoryIterator::DirectoryEntry entry;
      try
      {
        entry = Iterator->Next();
      }
      catch (std::exception&)
      {
        SharedStatus->TaskFailedCallback(1, _internal::GetFileUrl(Source), Destination.GetUrl());
        hasMoreEntries = false;
        break;
      }

      if (entry.Name.empty())
      {
        hasMoreEntries = false;
        break;
      }
      if (entry.IsDirectory)
      {
        auto task = CreateTask<UploadBlobsFromDirectoryTask>(
            _internal::TaskType::NetworkUpload,
            Source + '/' + entry.Name,
            Destination.GetBlobFolder(entry.Name));
        subtasks.push_back(std::move(task));
      }
      else
      {
        auto task = CreateTask<UploadBlobFromFileTask>(
            _internal::TaskType::NetworkUpload,
            Source + '/' + entry.Name,
            Destination.GetBlobClient(entry.Name));
        subtasks.push_back(std::move(task));
      }
    }

    if (hasMoreEntries)
    {
      auto task = std::make_unique<UploadBlobsFromDirectoryTask>(std::move(*this));
      subtasks.push_back(std::move(task));
    }

    if (!subtasks.empty())
    {
      SharedStatus->Scheduler->AddTasks(std::move(subtasks));
    }
  }
}}}} // namespace Azure::Storage::Blobs::_detail
