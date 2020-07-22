// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure.hpp"

#include <algorithm>
#include <functional>
#include <set>
#include <string>

namespace Azure { namespace Core { namespace Logging {
  class LogClassification;

  typedef std::function<
      void(LogClassification const& classification, std::string const& message) noexcept>
      LogListener;

  void SetLogListener(LogListener logListener);
  void ResetLogListener();

  void SetLogClassifications(std::set<LogClassification> logClassifications);
  void ResetLogClassifications();

  namespace Details {
    enum class Facility : uint16_t
    {
      Core = 1,
      Storage = 100,
    };

    template <Facility> class LogClassifications;
  } // namespace Details

  class LogClassification {
    template <Details::Facility> friend class Details::LogClassifications;

    int32_t m_value;

    constexpr explicit LogClassification(Details::Facility facility, int16_t number)
        : m_value((static_cast<int32_t>(number) << 16) | static_cast<int32_t>(facility))
    {
    }

  public:
    constexpr bool operator<(LogClassification const& other) const
    {
      return m_value < other.m_value;
    }

    constexpr bool operator==(LogClassification const& other) const
    {
      return m_value == other.m_value;
    }

    constexpr bool operator!=(LogClassification const& other) const
    {
      return m_value != other.m_value;
    }
  };

  namespace Details {
    template <Facility F> class LogClassifications {
    protected:
      constexpr static auto Classification(int16_t number) { return LogClassification(F, number); }
    };
  } // namespace Details
}}} // namespace Azure::Core::Logging
