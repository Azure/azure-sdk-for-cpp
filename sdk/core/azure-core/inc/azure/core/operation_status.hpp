// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <utility> // for std::move

#include "azure/core/strings.hpp"

namespace Azure { namespace Core {

  class OperationStatus {
    std::string m_value;

  public:
    // Provide `explicit` conversion from string or types convertible to string:
    explicit OperationStatus(const std::string& value) : m_value(value) {}
    explicit OperationStatus(std::string&& value) : m_value(std::move(value)) {}
    explicit OperationStatus(const char* value) : m_value(value) {}

    // Provide an equality comparison. If the service treats the enumeration case insensitively,
    // use LocaleInvariantCaseInsensitiveEqual to prevent differing locale settings from affecting
    // the SDK's behavior:
    bool operator==(const OperationStatus& other) const noexcept
    {
      return Strings::LocaleInvariantCaseInsensitiveEqual(m_value, other.m_value);
    }

    bool operator!=(const OperationStatus& other) const noexcept
    {
      return !(*this == other);
    }

    // Provide a "Get" accessor
    const std::string& Get() const noexcept { return m_value; }

    // Provide your example values as static const members
    const static OperationStatus NotStarted;
    const static OperationStatus Running;
    const static OperationStatus Succeeded;
    const static OperationStatus Cancelled;
    const static OperationStatus Failed;
  };

}} // namespace Azure::Core
