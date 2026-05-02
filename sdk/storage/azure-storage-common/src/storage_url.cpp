// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/storage/common/internal/storage_url.hpp"

#include <azure/core/url.hpp>

namespace Azure { namespace Storage { namespace _internal {
  std::string StripAccountSuffix(std::string accountName)
  {
    for (const std::string& suffix : {"-ipv6", "-dualstack", "-secondary"})
    {
      if (accountName.length() > suffix.length()
          && accountName.compare(accountName.length() - suffix.length(), suffix.length(), suffix) == 0)
      {
        accountName = accountName.substr(0, accountName.length() - suffix.length());
      }
    }
    return accountName;
  }

  Azure::Nullable<StorageUrlParts> ParseStorageUrl(const Azure::Core::Url& url)
  {
    const auto& hostname = url.GetHost();
    std::string endpoint;
    for (const std::string& e : {".preprod.core.windows.net", ".core.windows.net"})
    {
      if (hostname.find(e) != std::string::npos)
      {
        endpoint = e;
        break;
      }
    }
    if (endpoint.empty())
    {
      return Azure::Nullable<StorageUrlParts>();
    }
    for (const std::string& service : {"blob", "dfs", "file", "queue", "table", "web"})
    {
      const auto suffix = "." + service + endpoint;
      if (hostname.length() > suffix.length()
          && hostname.compare(hostname.length() - suffix.length(), suffix.length(), suffix) == 0)
      {
        std::string accountName
            = StripAccountSuffix(hostname.substr(0, hostname.length() - suffix.length()));
        if (!accountName.empty())
        {
          StorageUrlParts ret;
          ret.Service = service;
          ret.AccountName = std::move(accountName);
          const auto& path = url.GetPath();
          if (!path.empty())
          {
            const auto containerNameEnd = path.find('/', 1);
            if (containerNameEnd == std::string::npos)
            {
              ret.ContainerName = path;
            }
            else
            {
              ret.ContainerName = path.substr(0, containerNameEnd);
            }
            ret.ContainerUrl = [url, &ret]() {
              auto containerUrl = url;
              containerUrl.SetPath(ret.ContainerName.Value());
              return containerUrl.GetAbsoluteUrl();
            }();
          }

          return Azure::Nullable<StorageUrlParts>(std::move(ret));
        }
        break;
      }
    }
    return Azure::Nullable<StorageUrlParts>();
  }
}}} // namespace Azure::Storage::_internal
