// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Contains the user agent string generator implementation.
 */

#include "azure/core/http/http.hpp"
#include "azure/core/internal/strings.hpp"
#include "azure/core/platform.hpp"

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

// cspell: ignore Hkey

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
} // namespace

namespace Azure { namespace Core { namespace Http { namespace _internal {

  std::string HttpShared::GenerateUserAgent(
      std::string const& componentName,
      std::string const& componentVersion,
      std::string const& applicationId,
      long cplusplusValue)
  {
    using Azure::Core::_internal::StringExtensions;

    // Spec: https://azure.github.io/azure-sdk/general_azurecore.html#telemetry-policy
    std::ostringstream telemetryId;

    if (!applicationId.empty())
    {
      telemetryId << StringExtensions::Trim(applicationId).substr(0, 24) << " ";
    }

    static std::string const osVer = GetOSVersion();
    telemetryId << "azsdk-cpp-" << componentName << "/" << componentVersion << " (" << osVer << " "
                << "Cpp/" << cplusplusValue << ")";

    return telemetryId.str();
  }
}}}} // namespace Azure::Core::Http::_internal
