// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/common/internal/storage_url.hpp"

#include <azure/core/internal/strings.hpp>

namespace {

std::string StripAccountSuffix(std::string accountName)
{
  while (true)
  {
    const auto originalSize = accountName.size();
    for (const std::string suffix :
         {"-ipv6", "-dualstack", "-secondary", "-internetrouting", "-microsoftrouting"})
    {
      if (accountName.size() > suffix.size()
          && accountName.compare(accountName.size() - suffix.size(), suffix.size(), suffix) == 0)
      {
        accountName.resize(accountName.size() - suffix.size());
        break;
      }
    }
    if (accountName.size() == originalSize)
    {
      break;
    }
  }
  return accountName;
}

} // namespace

namespace Azure { namespace Storage { namespace _internal {

  Azure::Nullable<StorageUrlParts> ParseStorageUrl(const Azure::Core::Url& url)
  {
    const auto hostname = Azure::Core::_internal::StringExtensions::ToLower(url.GetHost());
    std::string endpoint;
    for (const std::string candidate :
         {".preprod.core.windows.net",
          ".core.windows.net",
          ".core.chinacloudapi.cn",
          ".core.usgovcloudapi.net"})
    {
      if (hostname.find(candidate) != std::string::npos)
      {
        endpoint = candidate;
        break;
      }
    }
    std::string service;
    std::string accountName;
    if (!endpoint.empty())
    {
      for (const std::string candidate : {"blob", "dfs", "file", "queue", "table", "web"})
      {
        const auto suffix = "." + candidate + endpoint;
        if (hostname.size() > suffix.size()
            && hostname.compare(hostname.size() - suffix.size(), suffix.size(), suffix) == 0)
        {
          service = candidate;
          accountName = hostname.substr(0, hostname.size() - suffix.size());
          break;
        }
      }
    }
    else
    {
      for (const std::string candidate : {"blob", "dfs", "file", "queue", "table", "web"})
      {
        const auto suffix = "." + candidate + ".storage.azure.net";
        if (hostname.size() <= suffix.size()
            || hostname.compare(hostname.size() - suffix.size(), suffix.size(), suffix) != 0)
        {
          continue;
        }

        service = candidate;
        accountName = hostname.substr(0, hostname.find('.'));
        break;
      }
    }

    accountName = StripAccountSuffix(std::move(accountName));
    if (service.empty() || accountName.empty())
    {
      return {};
    }

    StorageUrlParts result;
    result.Service = std::move(service);
    result.AccountName = std::move(accountName);

    auto serviceUrl = url;
    serviceUrl.SetPath({});
    serviceUrl.SetQueryParameters({});
    result.ServiceUrl = serviceUrl.GetAbsoluteUrl();

    const auto& path = url.GetPath();
    if (!path.empty())
    {
      const auto containerNameEnd = path.find('/', 1);
      result.ContainerName
          = containerNameEnd == std::string::npos ? path : path.substr(0, containerNameEnd);

      auto containerUrl = serviceUrl;
      containerUrl.SetPath(result.ContainerName.Value());
      result.ContainerUrl = containerUrl.GetAbsoluteUrl();
    }
    return result;
  }

}}} // namespace Azure::Storage::_internal
