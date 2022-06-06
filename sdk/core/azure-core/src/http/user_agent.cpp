// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief HTTP pipeline is a stack of HTTP policies.
 * @remark See #policy.hpp
 */

#include <azure/core/internal/http/user_agent.hpp>

#include "azure/core/context.hpp"
#include "azure/core/http/policies/policy.hpp"
#include "azure/core/internal/tracing/service_tracing.hpp"
#include "azure/core/platform.hpp"
#include "azure/core/tracing/tracing.hpp"
#include <cctype>
#include <sstream>

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#include <windows.h>

#if !defined(WINAPI_PARTITION_DESKTOP) \
    || WINAPI_PARTITION_DESKTOP // See azure/core/platform.hpp for explanation.

namespace {

/**
 * @brief HkeyHolder ensures native handle resource is released.
 *
 */
class HkeyHolder final {
private:
  HKEY m_value = nullptr;

public:
  explicit HkeyHolder() noexcept : m_value(nullptr) {}

  ~HkeyHolder() noexcept
  {
    if (m_value != nullptr)
    {
      ::RegCloseKey(m_value);
    }
  }

  void operator=(HKEY p) noexcept
  {
    if (p != nullptr)
    {
      m_value = p;
    }
  }

  operator HKEY() noexcept { return m_value; }

  operator HKEY*() noexcept { return &m_value; }

  HKEY* operator&() noexcept { return &m_value; }
};

} // namespace

#endif

#elif defined(AZ_PLATFORM_POSIX)
#include <sys/utsname.h>
#endif

namespace {
std::string GetOSVersion()
{
  std::ostringstream osVersionInfo;

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WINAPI_PARTITION_DESKTOP) \
    || WINAPI_PARTITION_DESKTOP // See azure/core/platform.hpp for explanation.
  {
    HkeyHolder regKey;
    if (RegOpenKeyExA(
            HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
            0,
            KEY_READ,
            &regKey)
        == ERROR_SUCCESS)
    {
      auto first = true;
      static constexpr char const* regValues[]{
          "ProductName", "CurrentVersion", "CurrentBuildNumber", "BuildLabEx"};
      for (auto regValue : regValues)
      {
        char valueBuf[200] = {};
        DWORD valueBufSize = sizeof(valueBuf);

        if (RegQueryValueExA(regKey, regValue, NULL, NULL, (LPBYTE)valueBuf, &valueBufSize)
            == ERROR_SUCCESS)
        {
          if (valueBufSize > 0)
          {
            osVersionInfo << (first ? "" : " ")
                          << std::string(valueBuf, valueBuf + (valueBufSize - 1));
            first = false;
          }
        }
      }
    }
  }
#else
  {
    osVersionInfo << "UWP";
  }
#endif
#elif defined(AZ_PLATFORM_POSIX)
  {
    utsname sysInfo{};
    if (uname(&sysInfo) == 0)
    {
      osVersionInfo << sysInfo.sysname << " " << sysInfo.release << " " << sysInfo.machine << " "
                    << sysInfo.version;
    }
  }
#endif

  return osVersionInfo.str();
}

std::string TrimString(std::string s)
{
  auto const isSpace = [](int c) { return !std::isspace(c); };

  s.erase(s.begin(), std::find_if(s.begin(), s.end(), isSpace));
  s.erase(std::find_if(s.rbegin(), s.rend(), isSpace).base(), s.end());

  return s;
}
} // namespace

namespace Azure { namespace Core { namespace Http { namespace _detail {

  std::string UserAgentGenerator::GenerateUserAgent(
      std::string const& componentName,
      std::string const& componentVersion,
      std::string const& applicationId)
  {
    // Spec: https://azure.github.io/azure-sdk/general_azurecore.html#telemetry-policy
    std::ostringstream telemetryId;

    if (!applicationId.empty())
    {
      telemetryId << TrimString(applicationId).substr(0, 24) << " ";
    }

    static std::string const osVer = GetOSVersion();
    telemetryId << "azsdk-cpp-" << componentName << "/" << componentVersion << " (" << osVer << ")";

    return telemetryId.str();
  }
}}}} // namespace Azure::Core::Http::_detail
