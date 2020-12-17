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
  class OperationStatus {
    std::string m_value;

  public:
    /**
     * @brief Construct a #OperationStatus with \p value.
     *
     * @param value A non-absent value to initialize with.
     */
    explicit OperationStatus(const std::string& value) : m_value(value) {}
    /**
     * @brief Construct a #OperationStatus with \p value.
     *
     * @param value A non-absent value to initialize with.
     */
    explicit OperationStatus(std::string&& value) : m_value(std::move(value)) {}
    /**
     * @brief Construct a #OperationStatus with \p value.
     *
     * @param value A non-absent value to initialize with.
     */
    explicit OperationStatus(const char* value) : m_value(value) {}

    /**
     * @brief Compare two #OperationStatus objects
     *
     * @param other A #OperationStatus to compare with.
     *
     * @return `true` if the states have the same string representation. `false` otherwise.
     */
    bool operator==(const OperationStatus& other) const noexcept
    {
      return Strings::LocaleInvariantCaseInsensitiveEqual(m_value, other.m_value);
    }

    /**
     * @brief Compare two #OperationStatus objects
     *
     * @param other A #OperationStatus to compare with.
     *
     * @return `false` if the states have the same string representation. `true` otherwise.
     */
    bool operator!=(const OperationStatus& other) const noexcept { return !(*this == other); }

    /**
     * @brief The std::string representation of the value
     */
    const std::string& Get() const noexcept { return m_value; }

    static const OperationStatus NotStarted;
    static const OperationStatus Running;
    static const OperationStatus Succeeded;
    static const OperationStatus Cancelled;
    static const OperationStatus Failed;
  };

}} // namespace Azure::Core
