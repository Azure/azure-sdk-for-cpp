// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policy.hpp"
#include "azure/core/platform.hpp"

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
    HKEY regKey{};
    auto regKeyOpened = false;
    try
    {
      if (RegOpenKeyExA(
              HKEY_LOCAL_MACHINE,
              "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
              0,
              KEY_READ,
              &regKey)
          == ERROR_SUCCESS)
      {
        regKeyOpened = true;

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

        RegCloseKey(regKey);
      }
    }
    catch (...)
    {
      if (regKeyOpened)
      {
        RegCloseKey(regKey);
      }

      throw;
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

using Azure::Core::Context;
using namespace Azure::Core::Http;

std::string TelemetryPolicy::BuildTelemetryId(
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

std::unique_ptr<RawResponse> TelemetryPolicy::Send(
    Context const& ctx,
    Request& request,
    NextHttpPolicy nextHttpPolicy) const
{
  request.AddHeader("User-Agent", m_telemetryId);
  return nextHttpPolicy.Send(ctx, request);
}
