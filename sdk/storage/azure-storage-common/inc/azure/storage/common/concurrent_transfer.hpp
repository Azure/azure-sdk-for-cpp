// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <functional>
#include <future>
#include <stdexcept>
#include <vector>

namespace Azure { namespace Storage { namespace Details {

  inline void ConcurrentTransfer(
      int64_t offset,
      int64_t length,
      int64_t chunkSize,
      int concurrency,
      // offset, length, chunk id, number of chunks
      std::function<void(int64_t, int64_t, int64_t, int64_t)> transferFunc)
  {
    std::atomic<int> numWorkingThreads{concurrency};
    std::atomic<int> nextChunkId{0};
    std::atomic<bool> failed{false};

    const auto numChunks = (length + chunkSize - 1) / chunkSize;

    auto threadFunc = [&]() {
      while (true)
      {
        int chunkId = nextChunkId.fetch_add(1);
        if (chunkId >= numChunks || failed)
        {
          break;
        }
        int64_t chunkOffset = offset + chunkSize * chunkId;
        int64_t chunkLength = (std::min)(length - chunkSize * chunkId, chunkSize);
        try
        {
          transferFunc(chunkOffset, chunkLength, chunkId, numChunks);
        }
        catch (std::exception&)
        {
          if (failed.exchange(true) == false)
          {
            numWorkingThreads.fetch_sub(1);
            throw;
          }
        }
      }
      numWorkingThreads.fetch_sub(1);
    };

    std::vector<std::future<void>> threadHandles;
    for (int i = 0; i < concurrency - 1; ++i)
    {
      threadHandles.emplace_back(std::async(std::launch::async, threadFunc));
    }
    threadFunc();
    for (auto& handle : threadHandles)
    {
      handle.get();
    }

    if (numWorkingThreads != 0)
    {
      std::abort();
    }
  }

}}} // namespace Azure::Storage::Details
