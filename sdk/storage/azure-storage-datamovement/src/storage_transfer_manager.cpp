#include "azure/storage/datamovement/storage_transfer_manager.hpp"

#include <azure/core/uuid.hpp>

#include "azure/storage/datamovement/tasks/upload_blob_from_file_task.hpp"

namespace Azure { namespace Storage { namespace DataMovement {

  namespace {
    constexpr static const char* FileUrlScheme = "file://";

    std::string GetFullPath(const std::string& relativePath)
    {
      // TODO: implement this
      (void)relativePath;
      return std::string();
    }
  } // namespace

  StorageTransferManager::StorageTransferManager(const StorageTransferManagerOptions& options)
      : m_options(options), m_scheduler(_internal::SchedulerOptions{})
  {
  }

  JobProperties StorageTransferManager::ScheduleUpload(
      const std::string& sourceLocalPath,
      const Blobs::BlobClient& destinationBlob,
      const UploadBlobOptions& options)
  {
    (void)options;
    auto jobProperties = JobProperties();
    jobProperties.JobId = Core::Uuid::CreateUuid().ToString();
    jobProperties.SourceUrl = FileUrlScheme + sourceLocalPath;
    jobProperties.DestinationUrl = destinationBlob.GetUrl();
    jobProperties.Type = TransferType::SingleUpload;

    auto task = std::make_unique<_internal::UploadBlobFromFileTask>(
        _internal::TaskType::NetworkUpload, &m_scheduler, sourceLocalPath, destinationBlob);
    m_scheduler.AddTask(std::move(task));

    return jobProperties;
  }

}}} // namespace Azure::Storage::DataMovement
