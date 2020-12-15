// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <utility> // for std::move

#include "azure/core/strings.hpp"

/*
Need two things:
  Polling loop that does the wait for the customer
  Enabling the scenario where the user can just wait
*/

namespace Azure { namespace Core {

  class LongRunningOperationState {
    std::string m_value;

  public:
    // Provide `explicit` conversion from string or types convertible to string:
    explicit LongRunningOperationState(const std::string& value) : m_value(value) {}
    explicit LongRunningOperationState(std::string&& value) : m_value(std::move(value)) {}
    explicit LongRunningOperationState(const char* value) : m_value(value) {}

    // Provide an equality comparison. If the service treats the enumeration case insensitively,
    // use LocaleInvariantCaseInsensitiveEqual to prevent differing locale settings from affecting
    // the SDK's behavior:
    bool operator==(const LongRunningOperationState& other) const noexcept
    {
      return Strings::LocaleInvariantCaseInsensitiveEqual(m_value, other.m_value);
    }

    bool operator!=(const LongRunningOperationState& other) const noexcept
    {
      return !(*this == other);
    }

    // Provide a "Get" accessor
    const std::string& Get() const noexcept { return m_value; }

    // Provide your example values as static const members
    const static LongRunningOperationState NotStarted;
    const static LongRunningOperationState InProgress;
    const static LongRunningOperationState SuccessfullyCompleted;
    const static LongRunningOperationState Failed;
    const static LongRunningOperationState UserCancelled;
  };

}} // namespace Azure::Core
