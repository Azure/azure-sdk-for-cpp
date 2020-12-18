// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Valid states for long-running Operations. Services can extend upon the default set of
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
     * @brief Construct an #OperationStatus with \p value.
     *
     * @param value A non-absent value to initialize with.
     */
    explicit OperationStatus(const std::string& value) : m_value(value) {}
    /**
     * @brief Construct an #OperationStatus with \p value.
     *
     * @param value The value to initialize with.
     */
    explicit OperationStatus(std::string&& value) : m_value(std::move(value)) {}
    /**
     * @brief Construct an #OperationStatus with \p value.
     *
     * @param value A non-absent value to initialize with.
     */
    explicit OperationStatus(const char* value) : m_value(value) {}

    /**
     * @brief Compare two #OperationStatus objects for equality.
     *
     * @param other An #OperationStatus to compare with.
     *
     * @return `true` if the states have the same string representation. `false` otherwise.
     */
    bool operator==(const OperationStatus& other) const noexcept
    {
      return Strings::LocaleInvariantCaseInsensitiveEqual(m_value, other.m_value);
    }

    /**
     * @brief Compare two #OperationStatus objects for equality.
     *
     * @param other A #OperationStatus to compare with.
     *
     * @return `false` if the states have the same string representation. `true` otherwise.
     */
    bool operator!=(const OperationStatus& other) const noexcept { return !(*this == other); }

    /**
     * @brief The std::string representation of the operation status.
     */
    const std::string& Get() const noexcept { return m_value; }

    /**
     * @brief The #Operation is Not Started.
     */
    static const OperationStatus NotStarted;
    /**
     * @brief The #Operation is Running.
     */
    static const OperationStatus Running;
    /**
     * @brief The #Operation Succeeded.
     */
    static const OperationStatus Succeeded;
    /**
     * @brief The #Operation was Cancelled.
     */
    static const OperationStatus Cancelled;
    /**
     * @brief The #Operation Failed.
     */
    static const OperationStatus Failed;
  };

}} // namespace Azure::Core
