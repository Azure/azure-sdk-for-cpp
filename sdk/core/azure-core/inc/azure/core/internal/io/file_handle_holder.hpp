// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief An internal FILE* Holder.
 *
 */
#pragma once

#include <cstdio>

namespace Azure { namespace IO { namespace _internal {

  /**
   * @brief FileHandleHolder ensures native handle resource is released.
   */
  class FileHandleHolder {
  private:
    FILE* m_value = nullptr;

  public:
    explicit FileHandleHolder() noexcept : m_value(nullptr){};

    ~FileHandleHolder() noexcept
    {
      if (m_value != nullptr)
      {
        fclose(m_value);
        m_value = nullptr;
      }
    }

    void operator=(FILE* p) noexcept
    {
      if (p != nullptr)
      {
        m_value = p;
      }
    }

    FILE* GetValue() noexcept { return m_value; }
  };

}}} // namespace Azure::IO::_internal
