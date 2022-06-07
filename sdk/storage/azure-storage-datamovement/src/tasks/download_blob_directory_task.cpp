// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/tasks/download_blob_directory_task.hpp"
#include "azure/storage/datamovement/scheduler.hpp"
#include "azure/storage/datamovement/tasks/download_blob_to_file_task.hpp"
#include "azure/storage/datamovement/job_properties.hpp"
#include "azure/storage/datamovement/local_utils.hpp"
#include "azure/storage/datamovement/utilities.hpp"

namespace Azure { namespace Storage { namespace _internal {
  extern const char FolderDelimiter;
}}} // namespace Azure::Storage::_internal

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {
  void DownloadBlobDirectoryTask::Execute() noexcept
  {
    try
    {
      Azure::Storage::_internal::Local_Utils::create_directory(Context->Destination);
    }
    catch (std::exception&)
    {
      SharedStatus->TaskFailedCallback(1,  Context->Source.GetUrl(), _internal::GetFileUrl(Context->Destination));
      return;
    }
    ListBlobsOptions options;
    options.Prefix = Context->Source.m_folderPath;
    options.PageSizeHint = 250;
    options.ContinuationToken = std::move(Context->ContinuationToken);

    std::vector<_internal::Task> subtasks;
    try
    {
      auto result = Context->Source.m_blobContainerClient.ListBlobsByHierarchy("/", options);
      for (auto blobItem : result.Blobs)
      {
        auto downloadBlobTask
            = CreateTask<DownloadBlobToFileTask>(Storage::_internal::TaskType::NetworkDownload,
            Context->Source.m_blobContainerClient.GetBlobClient(
                blobItem.Name), 
              Context->Destination + Azure::Storage::_internal::FolderDelimiter + blobItem.Name.substr(options.Prefix->length()));
        subtasks.push_back(std::move(downloadBlobTask));
      }

      for (auto blobPrefix : result.BlobPrefixes)
      {
        //TODO: local file system delimiter: "/" on Linux and "\\" on Windows
        auto downloadBlobDirectoryTask
            = CreateTask<DownloadBlobDirectoryTask>(Storage::_internal::TaskType::NetworkDownload,
              BlobFolder(Context->Source.m_blobContainerClient, blobPrefix),
            Context->Destination + Azure::Storage::_internal::FolderDelimiter
                + blobPrefix.substr(
                    options.Prefix->length(), blobPrefix.length() - options.Prefix->length() - 1));
        subtasks.push_back(std::move(downloadBlobDirectoryTask));
      }

      if (result.NextPageToken.HasValue())
      {
        Context->ContinuationToken = std::move(result.NextPageToken);
        auto task = std::make_unique<DownloadBlobDirectoryTask>(std::move(*this));
        subtasks.push_back(std::move(task));
      }

      if (!subtasks.empty())
      {
        SharedStatus->Scheduler->AddTasks(std::move(subtasks));
      }
    }
    catch (std::exception&)
    {
      SharedStatus->TaskFailedCallback(
          1, Context->Source.GetUrl(), _internal::GetFileUrl(Context->Destination));
      return; 
    }
  }

}}}} // namespace Azure::Storage::Blobs::_detail
