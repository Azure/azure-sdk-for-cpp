// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure.hpp"

#include <functional>
#include <initializer_list>
#include <set>
#include <string>
#include <utility>

namespace Azure { namespace Core { namespace Logging {
  class LogClassification;
  class LogClassifications;

  typedef std::function<void(LogClassification const& classification, std::string const& message)>
      LogListener;

  void SetLogListener(LogListener logListener);
  void SetLogClassifications(LogClassifications logClassifications);

  namespace Details {
    enum class Facility : uint16_t
    {
      Core = 1,
      Storage = 100,
    };

    template <Facility> class LogClassificationProvider;

    class LogClassificationsPrivate;
  } // namespace Details

  class LogClassifications {
    friend class Details::LogClassificationsPrivate;

    std::set<LogClassification> m_classifications;
    bool m_all;

    explicit LogClassifications(bool all) : m_all(all) {}

  public:
    LogClassifications(std::initializer_list<LogClassification> list)
        : m_classifications(list), m_all(false)
    {
    }

    explicit LogClassifications(std::set<LogClassification> set)
        : m_classifications(std::move(set)), m_all(false)
    {
    }
  };

  class LogClassification {
    template <Details::Facility> friend class Details::LogClassificationProvider;
    friend struct std::less<LogClassification>;

    int32_t m_value;

    constexpr explicit LogClassification(Details::Facility facility, int16_t number)
        : m_value((static_cast<int32_t>(number) << 16) | static_cast<int32_t>(facility))
    {
    }

    constexpr bool operator<(LogClassification const& other) const
    {
      return m_value < other.m_value;
    }

  public:
    constexpr bool operator==(LogClassification const& other) const
    {
      return m_value == other.m_value;
    }

    constexpr bool operator!=(LogClassification const& other) const
    {
      return m_value != other.m_value;
    }

    static LogClassifications const All;
    static LogClassifications const None;
  };

  namespace Details {
    template <Facility F> class LogClassificationProvider {
    protected:
      constexpr static auto Classification(int16_t number) { return LogClassification(F, number); }
    };
  } // namespace Details
}}} // namespace Azure::Core::Logging
