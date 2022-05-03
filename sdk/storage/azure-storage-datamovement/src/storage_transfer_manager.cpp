#include "azure/storage/datamovement/storage_transfer_manager.hpp"

#include <azure/core/platform.hpp>

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <climits>
#include <cstdlib>
#endif

#include <azure/core/uuid.hpp>

#include "azure/storage/datamovement/tasks/upload_blob_from_file_task.hpp"

#if defined(AZ_PLATFORM_WINDOWS)
namespace Azure { namespace Storage { namespace _internal {
  std::wstring Utf8ToWide(const std::string& narrow);
  std::string Utf8ToNarrow(const std::wstring& wide);
}}} // namespace Azure::Storage::_internal
#endif

namespace Azure { namespace Storage { namespace DataMovement {

  namespace {
    constexpr static const char* FileUrlScheme = "file://";

    std::string GetFullPath(const std::string& relativePath)
    {
#if defined(AZ_PLATFORM_WINDOWS)
      const std::wstring relativePathW = Storage::_internal::Utf8ToWide(relativePath);
      wchar_t absPathW[MAX_PATH];
      DWORD absPathWLength = GetFullPathNameW(relativePathW.data(), MAX_PATH, absPathW, nullptr);
      if (absPathWLength == 0)
      {
        throw std::runtime_error("Failed to get absolute path.");
      }
      std::replace(absPathW, absPathW + absPathWLength, L'\\', L'/');
      return Storage::_internal::Utf8ToNarrow(absPathW);
#else
      std::string absPath(PATH_MAX + 1, '\0');
      if (realpath(relativePath.data(), &absPath[0]) == nullptr)
      {
        throw std::runtime_error("Invalid filename");
      }
      return absPath;
#endif
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
    jobProperties.SourceUrl = FileUrlScheme + GetFullPath(sourceLocalPath);
    jobProperties.DestinationUrl = destinationBlob.GetUrl();
    jobProperties.Type = TransferType::SingleUpload;

    auto task = std::make_unique<_internal::UploadBlobFromFileTask>(
        _internal::TaskType::NetworkUpload, &m_scheduler, sourceLocalPath, destinationBlob);
    m_scheduler.AddTask(std::move(task));

    return jobProperties;
  }

}}} // namespace Azure::Storage::DataMovement
