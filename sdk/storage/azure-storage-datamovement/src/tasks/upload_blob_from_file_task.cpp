#include "azure/storage/datamovement/tasks/upload_blob_from_file_task.hpp"

#include <algorithm>

#include <azure/core/azure_assert.hpp>
#include <azure/core/base64.hpp>

#include "azure/storage/datamovement/scheduler.hpp"

namespace Azure { namespace Storage { namespace DataMovement { namespace _internal {

  namespace {
    constexpr uint64_t SingleUploadThreshold = 4 * 1024 * 1024;
    constexpr uint64_t ChunkSize = 8 * 1024 * 1024;

    std::string GetBlockId(int64_t id)
    {
      constexpr size_t BlockIdLength = 64;
      std::string blockId = std::to_string(id);
      blockId = std::string(BlockIdLength - blockId.length(), '0') + blockId;
      return Azure::Core::Convert::Base64Encode(
          std::vector<uint8_t>(blockId.begin(), blockId.end()));
    }

  } // namespace

  void UploadBlobFromFileTask::Execute()
  {
    if (!Context->FileReader)
    {
      Context->FileReader = std::make_unique<Storage::_internal::FileReader>(Context->Source);
    }
    const uint64_t fileSize = Context->FileReader->GetFileSize();
    Context->FileSize = fileSize;

    if (fileSize == 0)
    {
      Core::IO::MemoryBodyStream emptyStream(nullptr, 0);
      Context->Destination.AsBlockBlobClient().Upload(emptyStream);
      return;
    }

    // TOOD: if file is small enough

    Context->NumBlocks = static_cast<int>((fileSize + ChunkSize - 1) / ChunkSize);
    std::vector<Task> subtasks;
    for (int blockId = 0; blockId < Context->NumBlocks; ++blockId)
    {
      auto readFileRangeTask
          = std::make_unique<ReadFileRangeToMemoryTask>(TaskType::DiskIO, m_scheduler);
      readFileRangeTask->Context = Context;
      readFileRangeTask->BlockId = blockId;
      readFileRangeTask->Offset = blockId * ChunkSize;
      readFileRangeTask->Length = std::min<uint64_t>(ChunkSize, fileSize - blockId * ChunkSize);
      readFileRangeTask->MemoryCost = readFileRangeTask->Length;
      subtasks.push_back(std::move(readFileRangeTask));
    }
    m_scheduler->AddTasks(std::move(subtasks));
  }

  void ReadFileRangeToMemoryTask::Execute()
  {
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(Length);
    size_t bytesRead = Context->FileReader->Read(buffer.get(), Length, Offset);
    AZURE_ASSERT(bytesRead == Length);

    auto stageBlockTask = std::make_unique<StageBlockTask>(TaskType::NetworkUpload, m_scheduler);
    stageBlockTask->Context = Context;
    stageBlockTask->BlockId = BlockId;
    stageBlockTask->Buffer = std::move(buffer);
    stageBlockTask->Length = Length;
    stageBlockTask->MemoryGiveBack = MemoryCost;

    m_scheduler->AddTask(std::move(stageBlockTask));
  }

  void StageBlockTask::Execute()
  {
    const std::string blockId = GetBlockId(BlockId);
    Core::IO::MemoryBodyStream contentStream(Buffer.get(), Length);
    auto blockBlobClient = Context->Destination.AsBlockBlobClient();
    blockBlobClient.StageBlock(blockId, contentStream);
    Buffer.reset();
    int numStagedBlocks = Context->NumStagedBlocks.fetch_add(1, std::memory_order_relaxed) + 1;
    if (numStagedBlocks == Context->NumBlocks)
    {
      std::vector<std::string> blockIds;
      for (int i = 0; i < Context->NumBlocks; ++i)
      {
        blockIds.push_back(GetBlockId(i));
      }
      blockBlobClient.CommitBlockList(blockIds);
    }
  }
}}}} // namespace Azure::Storage::DataMovement::_internal