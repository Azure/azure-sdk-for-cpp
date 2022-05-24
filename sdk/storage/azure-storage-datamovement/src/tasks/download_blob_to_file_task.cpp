#include "azure/storage/datamovement/tasks/download_blob_to_file_task.hpp"

#include "azure/storage/datamovement/scheduler.hpp"

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {
  namespace {
    constexpr uint64_t ChunkSize = 4 * 1024 * 1024;
    static_assert(ChunkSize < static_cast<uint64_t>(std::numeric_limits<size_t>::max()), "");

  } // namespace

  // TODO: Should not allow expection thrown out from the method, should add some error handling.
  void DownloadBlobToFileTask::Execute()
  {
    // TODO Error handling here
    auto properties = Context->Source.GetProperties().Value;
    const uint64_t fileSize = properties.BlobSize;
    Context->FileSize = fileSize;

    if (!Context->FileWriter)
    {
      // TODO before creation, check file existence.
      // TODO: error handling here, when failed on opening the file.
      Context->FileWriter = std::make_unique<Storage::_internal::FileWriter>(Context->Destination);
    }

    if (fileSize == 0)
    {
      // TODO: completed
      return;
    }

    // TODO: if file is small enough

    Context->NumChunks = static_cast<int>((fileSize + ChunkSize - 1) / ChunkSize);
    std::vector<Storage::_internal::Task> subtasks;
    for (int index = 0; index < Context->NumChunks; ++index)
    {
      auto downloadRangeTask = std::make_unique<DownloadRangeToMemoryTask>(
          Storage::_internal::TaskType::NetworkDownload, m_scheduler);
      downloadRangeTask->Context = Context;
      downloadRangeTask->Offset = index * ChunkSize;
      downloadRangeTask->Length
          = static_cast<size_t>(std::min(ChunkSize, fileSize - index * ChunkSize));
      downloadRangeTask->MemoryCost = downloadRangeTask->Length;
      subtasks.push_back(std::move(downloadRangeTask));
    }
    m_scheduler->AddTasks(std::move(subtasks));
  }

  void DownloadRangeToMemoryTask::Execute()
  {
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(Length);
    Azure::Storage::Blobs::DownloadBlobOptions options;
    options.Range = Core::Http::HttpRange();
    options.Range->Offset = this->Offset;
    options.Range->Length = this->Length;
    // TODO: error handling & retry
    // TODO: when error happens, memory should be given back, handling MemoryGiveBack.
    // TODO: should check for source blob changing to avoid data corruption.
    auto downloadResult = Context->Source.Download(options).Value;
    size_t bytesRead = downloadResult.BodyStream->ReadToCount(buffer.get(), this->Length);
    if (bytesRead != this->Length)
    {
      // TODO: error handling
    }

    auto writeToFileTask
        = std::make_unique<WriteToFileTask>(_internal::TaskType::DiskIO, m_scheduler);
    writeToFileTask->Context = Context;
    writeToFileTask->Buffer = std::move(buffer);
    writeToFileTask->Offset = this->Offset;
    writeToFileTask->Length = Length;
    writeToFileTask->MemoryGiveBack = MemoryCost;

    m_scheduler->AddTask(std::move(writeToFileTask));
  }

  void WriteToFileTask::Execute()
  {
    Context->FileWriter->Write(this->Buffer.get(), this->Length, this->Offset);
    Buffer.reset();
    int numDownloadedBlocks
        = Context->NumDownloadedChunks.fetch_add(1, std::memory_order_relaxed) + 1;
    if (numDownloadedBlocks == Context->NumChunks)
    {
      // TODO handling complete
    }
  }
}}}} // namespace Azure::Storage::Blobs::_detail
