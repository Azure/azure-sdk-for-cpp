// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief System-memory budget guard used by buffer-mode upload/download perf tests.
 *
 * @remark Mirrors the memory-budget guard in the Go perf framework so buffer-mode tests
 * fail fast with a clear message when `--size x --parallel` would exceed a fraction of
 * the host's available memory, instead of triggering an OOM kill.
 */

#pragma once

#include <azure/core/platform.hpp>

#include <cstdint>
#include <stdexcept>
#include <string>

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(AZ_PLATFORM_LINUX)
#include <unistd.h>
#elif defined(AZ_PLATFORM_MAC)
#include <sys/sysctl.h>
#include <sys/types.h>
#endif

namespace Azure { namespace Storage { namespace Blobs { namespace Test {

  /**
   * @brief Get the total system memory in bytes. Returns 0 if it cannot be determined.
   */
  inline uint64_t GetSystemMemoryBytes()
  {
#if defined(AZ_PLATFORM_WINDOWS)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status))
    {
      return static_cast<uint64_t>(status.ullTotalPhys);
    }
    return 0;
#elif defined(AZ_PLATFORM_LINUX)
    long pages = sysconf(_SC_PHYS_PAGES);
    long pageSize = sysconf(_SC_PAGE_SIZE);
    if (pages > 0 && pageSize > 0)
    {
      return static_cast<uint64_t>(pages) * static_cast<uint64_t>(pageSize);
    }
    return 0;
#elif defined(AZ_PLATFORM_MAC)
    int mib[2] = {CTL_HW, HW_MEMSIZE};
    uint64_t mem = 0;
    size_t len = sizeof(mem);
    if (sysctl(mib, 2, &mem, &len, nullptr, 0) == 0)
    {
      return mem;
    }
    return 0;
#else
    return 0;
#endif
  }

  /**
   * @brief Throw `std::runtime_error` when `sizeBytes * parallel` would exceed
   * `budgetFraction` of system memory.
   *
   * @param sizeBytes Per-task buffer size requested by the test.
   * @param parallel Number of parallel tasks.
   * @param budgetFraction Fraction of system memory we are willing to consume (default
   * 0.8, matching the Go perf framework).
   */
  inline void CheckMemoryBudget(uint64_t sizeBytes, int parallel, double budgetFraction = 0.8)
  {
    uint64_t systemBytes = GetSystemMemoryBytes();
    if (systemBytes == 0 || parallel <= 0)
    {
      return;
    }
    uint64_t requested = sizeBytes * static_cast<uint64_t>(parallel);
    uint64_t budget = static_cast<uint64_t>(static_cast<double>(systemBytes) * budgetFraction);
    if (requested > budget)
    {
      throw std::runtime_error(
          "Requested buffer footprint " + std::to_string(requested) + " bytes (size "
          + std::to_string(sizeBytes) + " x parallel " + std::to_string(parallel)
          + ") exceeds " + std::to_string(static_cast<int>(budgetFraction * 100))
          + "% of system memory (" + std::to_string(systemBytes)
          + " bytes). Use --upload-method stream / --download-method stream, lower --size, "
            "or lower --parallel.");
    }
  }

}}}} // namespace Azure::Storage::Blobs::Test
