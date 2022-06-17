// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/tasks/upload_blob_from_file_task.hpp"

#include <atomic>
#include <limits>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include <azure/core/azure_assert.hpp>
#include <azure/core/base64.hpp>

#include "azure/storage/datamovement/task_shared_status.hpp"
#include "azure/storage/datamovement/transfer_engine.hpp"
#include "azure/storage/datamovement/utilities.hpp"

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {

  namespace {
    std::string GetBlockId(int64_t id)
    {
      // TODO: we want to add identifier in block ID, so that we resuming this job, we can pick up
      // from where we left off. The identifier may include az-storage-dm and job id and file size
      constexpr size_t BlockIdLength = 64;
      std::string blockId = std::to_string(id);
      blockId = std::string(BlockIdLength - blockId.length(), '0') + blockId;
      return Azure::Core::Convert::Base64Encode(
          std::vector<uint8_t>(blockId.begin(), blockId.end()));
    }
  } // namespace

  void UploadBlobFromFileTask::Execute() noexcept
  {
    std::unique_ptr<uint8_t[]> buffer;
    int64_t fileSize = -1;
    try
    {
      Storage::_internal::FileReader fileReader(Source);
      fileSize = fileReader.GetFileSize();
      buffer = std::make_unique<uint8_t[]>(static_cast<size_t>(fileSize));
      size_t bytesRead = fileReader.Read(buffer.get(), static_cast<size_t>(fileSize), 0);
      if (static_cast<int64_t>(bytesRead) != fileSize)
      {
        throw std::runtime_error("Failed to read file.");
      }
    }
    catch (std::exception&)
    {
      TransferFailed(_internal::GetPathUrl(Source), Destination.GetUrl());
      return;
    }

    Core::IO::MemoryBodyStream bodyStream(buffer.get(), static_cast<size_t>(fileSize));
    auto blockBlobClient = Destination.AsBlockBlobClient();
    try
    {
      blockBlobClient.Upload(bodyStream);
    }
    catch (std::exception&)
    {
      TransferFailed(_internal::GetPathUrl(Source), Destination.GetUrl());
      return;
    }
    TransferSucceeded(fileSize);
  }

  void ReadFileRangeToMemoryTask::Execute() noexcept
  {
    if (Context->Failed.load(std::memory_order_relaxed))
    {
      return;
    }

    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(Length);

    try
    {
      {
        std::lock_guard<std::mutex> guard(Context->FileReaderMutex);
        if (!Context->FileReader)
        {
          Context->FileReader = std::make_unique<Storage::_internal::FileReader>(Context->Source);
        }
      }
      size_t bytesRead = Context->FileReader->Read(buffer.get(), Length, Offset);
      if (bytesRead != Length)
      {
        throw std::runtime_error("Failed to read file.");
      }
    }
    catch (std::exception&)
    {
      bool firstFailure = !Context->Failed.exchange(true, std::memory_order_relaxed);
      if (firstFailure)
      {
        TransferFailed(_internal::GetPathUrl(Context->Source), Context->Destination.GetUrl());
      }
      return;
    }

    auto stageBlockTask = CreateTask<StageBlockTask>(_internal::TaskType::NetworkUpload);
    stageBlockTask->Context = Context;
    stageBlockTask->BlockId = BlockId;
    stageBlockTask->Buffer = std::move(buffer);
    stageBlockTask->Length = Length;
    std::swap(stageBlockTask->MemoryGiveBack, this->MemoryGiveBack);
    stageBlockTask->JournalContext = std::move(JournalContext);

    SharedStatus->TransferEngine->AddTask(std::move(stageBlockTask));
  }

  void StageBlockTask::Execute() noexcept
  {
    if (Context->Failed.load(std::memory_order_relaxed))
    {
      return;
    }

    const std::string blockId = GetBlockId(BlockId);
    Core::IO::MemoryBodyStream contentStream(Buffer.get(), Length);
    auto blockBlobClient = Context->Destination.AsBlockBlobClient();
    try
    {
      blockBlobClient.StageBlock(blockId, contentStream);
      TransferSucceeded(Length, 0);
    }
    catch (std::exception&)
    {
      Buffer.reset();
      bool firstFailure = !Context->Failed.exchange(true, std::memory_order_relaxed);
      if (firstFailure)
      {
        TransferFailed(_internal::GetPathUrl(Context->Source), Context->Destination.GetUrl());
      }
      return;
    }

    Buffer.reset();

    int numStagedBlocks = Context->NumStagedBlocks.fetch_add(1, std::memory_order_relaxed) + 1;
    if (numStagedBlocks != Context->NumBlocks)
    {
      return;
    }
    std::vector<std::string> blockIds;
    for (int i = 0; i < Context->NumBlocks; ++i)
    {
      blockIds.push_back(GetBlockId(i));
    }
    try
    {
      blockBlobClient.CommitBlockList(blockIds);
    }
    catch (std::exception&)
    {
      bool firstFailure = !Context->Failed.exchange(true, std::memory_order_relaxed);
      if (firstFailure)
      {
        TransferFailed(_internal::GetPathUrl(Context->Source), Context->Destination.GetUrl());
      }
      return;
    }
    TransferSucceeded(0, 1);
  }
}}}} // namespace Azure::Storage::Blobs::_detail
