// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "common/storage_uri_builder.hpp"

#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace Azure { namespace Storage {
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

    std::string AccountName;

  private:
    friend class SharedKeyPolicy;
    std::string GetAccountKey()
    {
      std::lock_guard<std::mutex> guard(m_mutex);
      return m_accountKey;
    }

    std::mutex m_mutex;
    std::string m_accountKey;
  };

  namespace Details {

    struct ConnectionStringParts
    {
      UriBuilder BlobServiceUri;
      UriBuilder FileServiceUri;
      UriBuilder QueueServiceUri;
      UriBuilder DataLakeServiceUri;
      std::shared_ptr<SharedKeyCredential> KeyCredential;
    };

    ConnectionStringParts ParseConnectionString(const std::string& connectionString);

  } // namespace Details

}} // namespace Azure::Storage
