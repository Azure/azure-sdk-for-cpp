// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Internal utility functions for strings.
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

  class HKeyHolder {
  private:
    HKEY m_value;

  public:
    HKeyHolder() : m_value(NULL) {}

    ~HKeyHolder()
    {
      if (m_value != NULL)
      {
        ::RegCloseKey(m_value);
      }
    }

    void operator=(HKEY p)
    {
      if (p != NULL)
      {
        m_value = p;
      }
    }

    operator HKEY()
    {
      return m_value;
    }

    operator HKEY*()
    {
      return &m_value;
    }

    HKEY* operator&()
    {
      return &m_value;
    }
  };

}}} // namespace Azure::Core::Internal

#endif
