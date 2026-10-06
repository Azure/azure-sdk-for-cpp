// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "data_locality.hpp"

#include <algorithm>
#include <exception>
#include <limits>
#include <unordered_map>
#include <utility>

namespace Azure { namespace Storage { namespace Blobs { namespace _detail {

  std::string GetIdealDataLocalityEndpoint(
      int64_t offset,
      int64_t length,
      const DataLocalityLayout& layout)
  {
    if (offset < 0 || length <= 0)
    {
      return std::string();
    }

    const int64_t requestEnd = length - 1 > (std::numeric_limits<int64_t>::max)() - offset
        ? (std::numeric_limits<int64_t>::max)()
        : offset + (length - 1);
    auto range = std::lower_bound(
        layout.Ranges.begin(),
        layout.Ranges.end(),
        offset,
        [](const Models::BlobLayoutRange& value, int64_t target) {
          return value.Range.Offset <= target
              && value.Range.Length.Value() <= target - value.Range.Offset;
        });
    std::unordered_map<std::string, uint64_t> endpointOverlaps;
    for (; range != layout.Ranges.end() && range->Range.Offset <= requestEnd; ++range)
    {
      const int64_t rangeEnd = range->Range.Offset + (range->Range.Length.Value() - 1);
      const int64_t overlapStart = (std::max)(offset, range->Range.Offset);
      const int64_t overlapEnd = (std::min)(requestEnd, rangeEnd);
      if (overlapStart > overlapEnd)
      {
        continue;
      }
      auto endpoint = endpointOverlaps.emplace(range->Endpoint, 0);
      endpoint.first->second += static_cast<uint64_t>(overlapEnd - overlapStart) + 1;
    }

    std::string idealEndpoint;
    uint64_t largestOverlap = 0;
    for (const auto& endpoint : endpointOverlaps)
    {
      if (endpoint.second > largestOverlap)
      {
        idealEndpoint = endpoint.first;
        largestOverlap = endpoint.second;
      }
    }
    return idealEndpoint;
  }

  DataLocalityLayoutState::DataLocalityLayoutState(std::function<DataLocalityLayout()> refresh)
      : m_refresh(std::move(refresh)), m_lastRefresh(std::chrono::steady_clock::now()),
        m_lastRefreshAttempt(m_lastRefresh), m_initialRefresh(true)
  {
    StartRefresh();
  }

  std::string DataLocalityLayoutState::GetEndpoint(int64_t offset, int64_t length)
  {
    const auto now = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_refreshFuture.valid()
        && m_refreshFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
    {
      m_refreshFuture.get();
    }
    const auto age = now - m_lastRefresh;
    if (age > DataLocalityLayoutValidity && !m_refreshFuture.valid()
        && now - m_lastRefreshAttempt > DataLocalityLayoutValidity)
    {
      StartRefresh();
    }
    if (age > DataLocalityLayoutValidity + DataLocalityLayoutExtendedExpiry)
    {
      return std::string();
    }
    return GetIdealDataLocalityEndpoint(offset, length, m_layout);
  }

  void DataLocalityLayoutState::WaitForInitialLayout()
  {
    WaitForRefreshTask();

    std::exception_ptr initialRefreshException;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if (m_initialRefresh)
      {
        m_initialRefresh = false;
        initialRefreshException = std::move(m_initialRefreshException);
      }
    }
    if (initialRefreshException)
    {
      std::rethrow_exception(initialRefreshException);
    }
  }

  void DataLocalityLayoutState::WaitForRefreshTask()
  {
    std::future<void> refreshFuture;
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if (m_refreshFuture.valid())
      {
        refreshFuture = std::move(m_refreshFuture);
      }
    }
    if (refreshFuture.valid())
    {
      refreshFuture.get();
    }
  }

  void DataLocalityLayoutState::StartRefresh()
  {
    m_lastRefreshAttempt = std::chrono::steady_clock::now();
    try
    {
      m_refreshFuture = std::async(std::launch::async, [this]() {
        DataLocalityLayout refreshedLayout;
        bool refreshed = false;
        std::exception_ptr refreshException;
        try
        {
          refreshedLayout = m_refresh();
          refreshed = !refreshedLayout.Ranges.empty();
        }
        catch (...)
        {
          refreshException = std::current_exception();
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        if (refreshed)
        {
          m_layout = std::move(refreshedLayout);
          m_lastRefresh = std::chrono::steady_clock::now();
        }
        else if (m_initialRefresh)
        {
          m_initialRefreshException = std::move(refreshException);
        }
      });
    }
    catch (...)
    {
      // If a refresh task cannot be created, continue with stale or primary routing.
    }
  }

}}}} // namespace Azure::Storage::Blobs::_detail
