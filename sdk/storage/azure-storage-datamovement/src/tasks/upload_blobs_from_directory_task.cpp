#include "azure/storage/datamovement/tasks/upload_blobs_from_directory_task.hpp"

#include <azure/storage/datamovement/tasks/upload_blob_from_file_task.hpp>

#include "azure/storage/datamovement/scheduler.hpp"

namespace Azure { namespace Storage { namespace DataMovement { namespace _internal {
  namespace {
    constexpr int ListBatchSize = 1000;
  }
  void UploadBlobsFromDirectoryTask::Execute()
  {
    std::vector<Task> subtasks;

    bool hasMoreEntries = true;
    while (subtasks.size() < ListBatchSize)
    {
      auto entry = Iterator.Next();
      if (entry.Name.empty())
      {
        hasMoreEntries = false;
        break;
      }
      if (entry.IsDirectory)
      {
        auto task = std::make_unique<UploadBlobsFromDirectoryTask>(
            TaskType::NetworkUpload,
            m_scheduler,
            Source + '/' + entry.Name,
            Destination.GetBlobFolder(entry.Name));
        subtasks.push_back(std::move(task));
      }
      else
      {
        auto task = std::make_unique<UploadBlobFromFileTask>(
            TaskType::NetworkUpload,
            m_scheduler,
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
      m_scheduler->AddTasks(std::move(subtasks));
    }
  }
}}}} // namespace Azure::Storage::DataMovement::_internal
