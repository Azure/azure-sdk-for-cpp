// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Internal HKEY holder.
 *
 */
#pragma once

#include "azure/core/platform.hpp"

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

namespace Azure { namespace Core { namespace _internal {

  /**
   * @brief HkeyHolder ensures native handle resource is released.
   */
  class HkeyHolder {
  private:
    HKEY m_value = nullptr;

  public:
    explicit HkeyHolder() noexcept : m_value(nullptr){};

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

}}} // namespace Azure::Core::_internal

#endif

#endif
