// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <map>
#include <mutex>
#include <string>

namespace Azure { namespace Storage {

  struct TokenCredential
  {
    explicit TokenCredential(std::string token) : Token(std::move(token)) {}

    void SetToken(std::string token)
    {
      std::lock_guard<std::mutex> guard(Mutex);
      Token = std::move(token);
    }

  private:
    std::string GetToken()
    {
      std::lock_guard<std::mutex> guard(Mutex);
      return Token;
    }
    std::mutex Mutex;
    std::string Token;
  };

  struct SharedKeyCredential
  {
    explicit SharedKeyCredential(std::string accountName, std::string accountKey)
        : AccountName(std::move(accountName)), AccountKey(std::move(accountKey))
    {
    }

    void SetAccountKey(std::string accountKey)
    {
      std::lock_guard<std::mutex> guard(Mutex);
      AccountKey = std::move(accountKey);
    }

    std::string AccountName;

  private:
    friend class SharedKeyPolicy;
    std::string GetAccountKey()
    {
      std::lock_guard<std::mutex> guard(Mutex);
      return AccountKey;
    }

    std::mutex Mutex;
    std::string AccountKey;
  };

  std::map<std::string, std::string> ParseConnectionString(const std::string& connectionString);

}} // namespace Azure::Storage
