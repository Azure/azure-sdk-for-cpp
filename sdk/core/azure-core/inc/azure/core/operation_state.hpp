// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Valid states for long-running Operations.  Services can extend upon the default set of
 * values.
 */

#pragma once

#include <string>
#include <utility> // for std::move

#include "azure/core/strings.hpp"

namespace Azure { namespace Core {

  /**
   * @brief Long-running operation states.
   */
  class OperationState {
    std::string m_value;

  public:
    /**
     * @brief Construct a #OperationState with \p value.
     *
     * @param value A non-absent value to initialize with.
     */
    explicit OperationState(const std::string& value) : m_value(value) {}
    /**
     * @brief Construct a #OperationState with \p value.
     *
     * @param value A non-absent value to initialize with.
     */
    explicit OperationState(std::string&& value) : m_value(std::move(value)) {}
    /**
     * @brief Construct a #OperationState with \p value.
     *
     * @param value A non-absent value to initialize with.
     */
    explicit OperationState(const char* value) : m_value(value) {}

    /**
     * @brief Compare two #OperationState objects
     *
     * @param other A #OperationState to compare with.
     *
     * @return `true` if the states have the same string representation. `false` otherwise.
     */
    bool operator==(const OperationState& other) const noexcept
    {
      return Strings::LocaleInvariantCaseInsensitiveEqual(m_value, other.m_value);
    }

    /**
     * @brief Compare two #OperationState objects
     *
     * @param other A #OperationState to compare with.
     *
     * @return `false` if the states have the same string representation. `true` otherwise.
     */
    bool operator!=(const OperationState& other) const noexcept { return !(*this == other); }

    /**
     * @brief The std::string representation of the value
     */
    const std::string& Get() const noexcept { return m_value; }

    static const OperationState NotStarted;
    static const OperationState Running;
    static const OperationState Succeeded;
    static const OperationState Cancelled;
    static const OperationState Failed;
  };

}} // namespace Azure::Core
