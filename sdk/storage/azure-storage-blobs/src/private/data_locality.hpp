// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/storage/blobs/blob_responses.hpp>

#include <chrono>
#include <cstdint>
#include <exception>
#include <functional>
#include <future>
#include <mutex>
#include <string>
#include <vector>

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {

  constexpr std::chrono::minutes DataLocalityLayoutValidity{5};
  constexpr std::chrono::minutes DataLocalityLayoutExtendedExpiry{5};

  struct DataLocalityLayout final
  {
    std::vector<Models::BlobLayoutRange> Ranges;
    int64_t BlobSize = 0;
    Azure::ETag ETag;
  };

  std::string GetIdealDataLocalityEndpoint(
      int64_t offset,
      int64_t length,
      const DataLocalityLayout& layout);

  class DataLocalityLayoutState final {
  public:
    explicit DataLocalityLayoutState(std::function<DataLocalityLayout()> refresh);

    ~DataLocalityLayoutState() { WaitForRefreshTask(); }

    DataLocalityLayoutState(const DataLocalityLayoutState&) = delete;
    DataLocalityLayoutState& operator=(const DataLocalityLayoutState&) = delete;

    std::string GetEndpoint(int64_t offset, int64_t length);

    void WaitForInitialLayout();

    bool HasLayout() const
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      return !m_layout.Ranges.empty();
    }

    int64_t GetBlobSize() const
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      return m_layout.BlobSize;
    }

    Azure::ETag GetETag() const
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      return m_layout.ETag;
    }

  private:
    void WaitForRefreshTask();

    void StartRefresh();

    DataLocalityLayout m_layout;
    std::function<DataLocalityLayout()> m_refresh;
    std::chrono::steady_clock::time_point m_lastRefresh;
    std::chrono::steady_clock::time_point m_lastRefreshAttempt;
    mutable std::mutex m_mutex;
    std::future<void> m_refreshFuture;
    std::exception_ptr m_initialRefreshException;
    bool m_initialRefresh = false;
  };

}}}} // namespace Azure::Storage::Blobs::_detail
