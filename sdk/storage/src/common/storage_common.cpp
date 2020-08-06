// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "common/storage_common.hpp"

#include <random>

namespace Azure { namespace Storage {

  std::string CreateUniqueLeaseId()
  {
    // TODO: return UUID provided by Azure Core once they provide one.

    static thread_local std::mt19937_64 random_generator(std::random_device{}());

    auto getRandomChar = []() {
      const char charset[] = "0123456789abcdef";
      std::uniform_int_distribution<size_t> distribution(0, sizeof(charset) - 2);
      return charset[distribution(random_generator)];
    };

    std::string result;
    result.reserve(37);
    for (int i : {8, 4, 4, 4, 12})
    {
      for (int j = 0; j < i; ++j)
        result += getRandomChar();
      result += '-';
    }
    result.pop_back();

    return result;
  }

}} // namespace Azure::Storage
