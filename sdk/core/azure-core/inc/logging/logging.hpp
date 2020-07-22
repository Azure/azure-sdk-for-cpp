// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure.hpp"

#include <functional>
#include <string>
#include <vector>

namespace Azure { namespace Core { namespace Logging {
  class LogClassification;

  typedef std::function<void(LogClassification classification, std::string const& message) noexcept>
      LogListener;

  void SetLogListener(LogListener logListener);
  void ResetLogListener();

  void SetLogClassifications(std::vector<LogClassification> const& logClassifications);
  void ResetLogClassifications();

  namespace Details {
    enum class Facility : uint16_t
    {
      Core = 1,
      Storage = 100,
    };

    template <Facility> class LogClassifications;

    class LogClassificationsCompare;
  } // namespace Details

  class LogClassification {
    template <Details::Facility> friend class Details::LogClassifications;
    friend class Details::LogClassificationsCompare;

    int32_t m_value;

    constexpr explicit LogClassification(Details::Facility facility, int16_t number)
        : m_value((static_cast<int32_t>(number) << 16) | static_cast<int32_t>(facility))
    {
    }
  };

  namespace Details {
    template <Facility F> class LogClassifications {
    protected:
      constexpr static auto Classification(int16_t number) { return LogClassification(F, number); }
    };

    class LogClassificationsCompare {
    public:
      bool operator()(LogClassification const& lhs, LogClassification const& rhs)
      {
        return lhs.m_value < rhs.m_value;
      }
    };
  } // namespace Details
}}} // namespace Azure::Core::Logging
