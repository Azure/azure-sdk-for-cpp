// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "azure/core/http/http.hpp"

namespace Azure { namespace Storage {

  struct AccountSasBuilder;
  namespace Blobs {
    struct BlobSasBuilder;
  }
  namespace Files { namespace Shares {
    struct ShareSasBuilder;
  }} // namespace Files::Shares

  struct SharedKeyCredential
  {
    explicit SharedKeyCredential(std::string accountName, std::string accountKey)
        : AccountName(std::move(accountName)), m_accountKey(std::move(accountKey))
    {
    }

    void SetAccountKey(std::string accountKey)
    {
      std::lock_guard<std::mutex> guard(m_mutex);
      m_accountKey = std::move(accountKey);
    }

    const std::string AccountName;

  private:
    friend class SharedKeyPolicy;
    friend struct Blobs::BlobSasBuilder;
    friend struct Files::Shares::ShareSasBuilder;
    friend struct AccountSasBuilder;
    std::string GetAccountKey() const
    {
      std::lock_guard<std::mutex> guard(m_mutex);
      return m_accountKey;
    }

    mutable std::mutex m_mutex;
    std::string m_accountKey;
  };

  namespace Details {

    struct ConnectionStringParts
    {
      Azure::Core::Http::Url BlobServiceUri;
      Azure::Core::Http::Url FileServiceUri;
      Azure::Core::Http::Url QueueServiceUri;
      Azure::Core::Http::Url DataLakeServiceUri;
      std::shared_ptr<SharedKeyCredential> KeyCredential;
    };

    ConnectionStringParts ParseConnectionString(const std::string& connectionString);

  } // namespace Details

}} // namespace Azure::Storage
