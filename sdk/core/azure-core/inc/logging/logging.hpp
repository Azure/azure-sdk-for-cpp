// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure.hpp"

#include <functional>
#include <set>
#include <string>

namespace Azure { namespace Core { namespace Logging {
  class LogClassification {
    int32_t m_value;

  protected:
    enum class Facility : uint16_t
    {
      Core = 1,
      Storage = 100,
    };

    explicit LogClassification(Facility facility, int16_t number)
        : m_value((static_cast<int>(number) << 16) | static_cast<int>(facility))
    {
    }
  };

  typedef std::function<void(LogClassification classification, std::string const& message) noexcept>
      LogListener;

  void SetLogListener(LogListener logListener);
  void ResetLogListener();

  void SetLogClassifications(std::set<LogClassification> logClassifications);
  void ResetLogClassifications();
}}} // namespace Azure::Core::Logging
