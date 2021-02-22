// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Internal HKEY holder.
 *
 */
#pragma once

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#include <windows.h>

namespace Azure { namespace Core { namespace Internal {

  /**
   * @brief HKEYHolder ensures native handle resource is released.
  */
  class HKEYHolder {
  private:
    HKEY m_value = nullptr;

  public:
    explicit HKEYHolder() noexcept : m_value(nullptr){};

    ~HKEYHolder() noexcept
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

}}} // namespace Azure::Core::Internal

#endif
