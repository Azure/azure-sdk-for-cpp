// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/perf/process_stats.hpp"

#include <azure/core/platform.hpp>

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
// psapi.h must follow windows.h
#include <psapi.h>
#elif defined(AZ_PLATFORM_LINUX)
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#elif defined(AZ_PLATFORM_MAC)
#include <mach/mach.h>
#include <sys/resource.h>
#endif

#include <algorithm>

namespace Azure { namespace Perf {

  ProcessStatsSampler::ProcessStatsSampler(std::chrono::milliseconds interval)
      : m_interval(interval)
  {
  }

  ProcessStatsSampler::~ProcessStatsSampler() { Stop(); }

  void ProcessStatsSampler::Start()
  {
    if (m_thread.joinable())
    {
      return;
    }
    m_stop.store(false);
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_startTime = std::chrono::steady_clock::now();
      m_baselineCpuSeconds = SampleCpuSeconds();
      m_haveBaseline = true;
      m_lastCpuSeconds = m_baselineCpuSeconds;
      m_sampleCount = 0;
      m_memoryBytesSum = 0.0;
      m_latest = Snapshot{};
    }
    m_thread = std::thread(&ProcessStatsSampler::Run, this);
  }

  void ProcessStatsSampler::Stop()
  {
    if (!m_thread.joinable())
    {
      return;
    }
    m_stop.store(true);
    m_thread.join();
  }

  void ProcessStatsSampler::Run()
  {
    auto previousCpuSeconds = m_baselineCpuSeconds;
    auto previousTime = m_startTime;
    while (!m_stop.load())
    {
      std::this_thread::sleep_for(m_interval);
      auto now = std::chrono::steady_clock::now();
      double cpuSeconds = SampleCpuSeconds();
      uint64_t mem = SampleResidentMemoryBytes();

      double wall = std::chrono::duration<double>(now - previousTime).count();
      double cpuDelta = cpuSeconds - previousCpuSeconds;
      // Clamp to avoid negative readings if a counter is non-monotonic on some platforms.
      double cpuPct = (wall > 0) ? (std::max)(0.0, (cpuDelta / wall) * 100.0) : 0.0;

      {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_latest.CpuPercent = cpuPct;
        m_latest.MemoryBytes = mem;
        m_sampleCount += 1;
        m_memoryBytesSum += static_cast<double>(mem);
        m_lastCpuSeconds = cpuSeconds;
      }
      previousCpuSeconds = cpuSeconds;
      previousTime = now;
    }
  }

  ProcessStatsSampler::Snapshot ProcessStatsSampler::Latest() const
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_latest;
  }

  ProcessStatsSampler::Snapshot ProcessStatsSampler::Average() const
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    Snapshot avg;
    auto now = std::chrono::steady_clock::now();
    double wall = std::chrono::duration<double>(now - m_startTime).count();
    if (m_haveBaseline && wall > 0)
    {
      double cpuDelta = m_lastCpuSeconds - m_baselineCpuSeconds;
      avg.CpuPercent = (std::max)(0.0, (cpuDelta / wall) * 100.0);
    }
    if (m_sampleCount > 0)
    {
      avg.MemoryBytes
          = static_cast<uint64_t>(m_memoryBytesSum / static_cast<double>(m_sampleCount));
    }
    return avg;
  }

  void ProcessStatsSampler::Reset()
  {
    // Stop the sampler thread first so we can re-prime baselines without racing the
    // Run() loop, which caches previous* in locals at thread start.
    Stop();
    Start();
  }

#if defined(AZ_PLATFORM_WINDOWS)
  double ProcessStatsSampler::SampleCpuSeconds()
  {
    FILETIME creation, exitTime, kernel, user;
    if (!GetProcessTimes(GetCurrentProcess(), &creation, &exitTime, &kernel, &user))
    {
      return 0.0;
    }
    auto toSeconds = [](FILETIME const& ft) {
      ULARGE_INTEGER u;
      u.LowPart = ft.dwLowDateTime;
      u.HighPart = ft.dwHighDateTime;
      // FILETIME is in 100-ns units.
      return static_cast<double>(u.QuadPart) / 1.0e7;
    };
    return toSeconds(kernel) + toSeconds(user);
  }

  uint64_t ProcessStatsSampler::SampleResidentMemoryBytes()
  {
    PROCESS_MEMORY_COUNTERS pmc;
    if (!GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
    {
      return 0;
    }
    return static_cast<uint64_t>(pmc.WorkingSetSize);
  }
#elif defined(AZ_PLATFORM_LINUX)
  double ProcessStatsSampler::SampleCpuSeconds()
  {
    std::ifstream stat("/proc/self/stat");
    if (!stat.is_open())
    {
      return 0.0;
    }
    std::string content;
    std::getline(stat, content);
    // /proc/self/stat fields are space-separated, but the second field (comm) may contain
    // spaces and is wrapped in parentheses. Skip past it before splitting.
    auto rp = content.rfind(')');
    if (rp == std::string::npos)
    {
      return 0.0;
    }
    std::istringstream iss(content.substr(rp + 1));
    std::string token;
    // After the ')', the next field is field 3 ('state'); CPU times are fields 14 (utime)
    // and 15 (stime), i.e. tokens 12 and 13 (0-indexed) of the remainder.
    unsigned long utime = 0, stime = 0;
    for (int i = 0; i < 14; ++i)
    {
      if (!(iss >> token))
      {
        return 0.0;
      }
      if (i == 11)
      {
        utime = std::stoul(token);
      }
      else if (i == 12)
      {
        stime = std::stoul(token);
      }
    }
    long hz = sysconf(_SC_CLK_TCK);
    if (hz <= 0)
    {
      hz = 100;
    }
    return static_cast<double>(utime + stime) / static_cast<double>(hz);
  }

  uint64_t ProcessStatsSampler::SampleResidentMemoryBytes()
  {
    std::ifstream status("/proc/self/status");
    if (!status.is_open())
    {
      return 0;
    }
    std::string line;
    while (std::getline(status, line))
    {
      if (line.rfind("VmRSS:", 0) == 0)
      {
        std::istringstream iss(line.substr(6));
        unsigned long kb = 0;
        std::string unit;
        iss >> kb >> unit;
        return static_cast<uint64_t>(kb) * 1024ULL;
      }
    }
    return 0;
  }
#elif defined(AZ_PLATFORM_MAC)
  double ProcessStatsSampler::SampleCpuSeconds()
  {
    struct rusage ru;
    if (getrusage(RUSAGE_SELF, &ru) != 0)
    {
      return 0.0;
    }
    double user = static_cast<double>(ru.ru_utime.tv_sec)
        + static_cast<double>(ru.ru_utime.tv_usec) / 1.0e6;
    double sys = static_cast<double>(ru.ru_stime.tv_sec)
        + static_cast<double>(ru.ru_stime.tv_usec) / 1.0e6;
    return user + sys;
  }

  uint64_t ProcessStatsSampler::SampleResidentMemoryBytes()
  {
    mach_task_basic_info info;
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(
            mach_task_self(), MACH_TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&info), &count)
        != KERN_SUCCESS)
    {
      return 0;
    }
    return static_cast<uint64_t>(info.resident_size);
  }
#else
  double ProcessStatsSampler::SampleCpuSeconds() { return 0.0; }
  uint64_t ProcessStatsSampler::SampleResidentMemoryBytes() { return 0; }
#endif

}} // namespace Azure::Perf
