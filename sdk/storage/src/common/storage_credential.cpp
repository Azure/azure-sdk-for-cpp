// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <algorithm>

#include "common/storage_credential.hpp"

namespace Azure { namespace Storage {

  std::map<std::string, std::string> ParseConnectionString(const std::string& connectionString)
  {
    std::map<std::string, std::string> result;

    std::string::const_iterator cur = connectionString.begin();

    while (cur != connectionString.end())
    {
      auto key_begin = cur;
      auto key_end = std::find(cur, connectionString.end(), '=');
      std::string key = std::string(key_begin, key_end);
      cur = key_end;
      if (cur != connectionString.end())
      {
        ++cur;
      }

      auto value_begin = cur;
      auto value_end = std::find(cur, connectionString.end(), ';');
      std::string value = std::string(value_begin, value_end);
      cur = value_end;
      if (cur != connectionString.end())
      {
        ++cur;
      }

      if (!key.empty() || !value.empty())
      {
        result[std::move(key)] = std::move(value);
      }
    }

    return result;
  }

}} // namespace Azure::Storage
