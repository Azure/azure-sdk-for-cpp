// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief This header defines the types and functions your application uses to be notified of Azure
 * SDK client library log messages.
 */

#pragma once

#include <functional>
#include <initializer_list>
#include <set>
#include <string>
#include <utility>

namespace Azure { namespace Core { namespace Logging {
  class LogClassification;
  class LogClassifications;

  /**
   * @brief Defines the signature of the callback function that application developers must write in
   * order to receive Azure SDK log messages.
   *
   * @param classification The log message classification.
   * @param classification The log message.
   */
  typedef std::function<void(LogClassification const& classification, std::string const& message)>
      LogListener;

  /**
   * @brief Set the function that will be invoked to report an SDK log message.
   *
   * @param logListener A #LogListener function that will be invoked when the SDK reports a log
   * message matching one of the log classifications passed to #SetLogClassifications(). If null, no
   * function will be invoked.
   */
  void SetLogListener(LogListener logListener);

  /**
   * @brief Allows the application to specify which log classification types it is interested in
   * receiving.
   *
   * @param logClassifications Log classification values.
   */
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

  /**
   * @brief Represents a set of log classifications.
   */
  class LogClassifications {
    friend class Details::LogClassificationsPrivate;

    std::set<LogClassification> m_classifications;
    bool m_all;

    explicit LogClassifications(bool all) : m_all(all) {}

  public:
    /**
     * @brief Initialize the list of log classifications with `std::initializer_list`.
     * @param list An initializer list.
     */
    LogClassifications(std::initializer_list<LogClassification> list)
        : m_classifications(list), m_all(false)
    {
    }

    /**
     * @brief Initialize the list of log classifications with `std::set`.
     * @param set A set of classifications.
     */
    explicit LogClassifications(std::set<LogClassification> set)
        : m_classifications(std::move(set)), m_all(false)
    {
    }
  };

  /**
   * @brief Represents a log classification.
   */
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
    /**
     * @brief Compare log classification to another one.
     * @param other Another log classification to compare to.
     * @return `true` if this log classification equals to \p other, `false` otherwise.
     */
    constexpr bool operator==(LogClassification const& other) const
    {
      return m_value == other.m_value;
    }

    /**
     * @brief Compare log classification to another one.
     * @param other Another log classification to compare to.
     * @return `true` if this log classification does not equal to \p other, `false` otherwise.
     */
    constexpr bool operator!=(LogClassification const& other) const
    {
      return m_value != other.m_value;
    }

    /**
     * @brief Represents a list of all classifications.
     */
    static LogClassifications const All;

    /**
     * @brief Represents an empty list of classifications.
     */
    static LogClassifications const None;
  };

  namespace Details {
    template <Facility F> class LogClassificationProvider {
    protected:
      constexpr static auto Classification(int16_t number) { return LogClassification(F, number); }
    };
  } // namespace Details
}}} // namespace Azure::Core::Logging
