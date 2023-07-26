// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/amqp/models/amqp_error.hpp>

#include <stdexcept>
#include <string>

namespace Azure { namespace Messaging { namespace EventHubs {

  enum class EventHubsStatusCode : int32_t
  {
    Invalid,
    Ok,
    Error,
    Timeout,
    Cancelled,
  };

  /**
   * @brief An exception thrown when an EventHubs service operation fails.
   */
  struct EventHubsException final : std::runtime_error
  {
    /**
     * @brief Constructs a #EventHubsException with a message.
     *
     * @param what An explanatory string.
     */
    explicit EventHubsException(const std::string& what)
        : std::runtime_error(what), ErrorDescription{what}
    {
    }

    /**
     * @brief Constructs a #EventHubsException with an error condition.
     *
     * @param error The AMQP Error indicating the error.
     */
    explicit EventHubsException(Azure::Core::Amqp::Models::_internal::AmqpError const& error)
        : std::runtime_error(error.Description), ErrorCondition(error.Condition.ToString()),
          ErrorDescription(error.Description)
    {
    }

    /**
     * @brief Constructs a #EventHubsException with a message, an error condition, and an HTTP
     * status code.
     *
     * This constructor is primarily intended for use by the EventHubs Properties events, which
     * report their status using HTTP status codes.
     *
     * @param error The AMQP error indicating the error.
     * @param statusCode The HTTP status code associated with the error.
     */
    explicit EventHubsException(
        Azure::Core::Amqp::Models::_internal::AmqpError const& error,
        std::uint32_t statusCode)
        : std::runtime_error(error.Description), ErrorCondition(error.Condition.ToString()),
          ErrorDescription(error.Description), StatusCode(statusCode)
    {
    }

    /** @brief A symbolic value indicating the error condition.
     *
     * @remarks For more information, see [AMQP
     * Section 2.8.14](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-error).
     *
     */

    std::string ErrorCondition{};

    /**
     * @brief A description of the error intended for the developer to understand what the error
     * refers to and how to fix it.
     *
     * @remarks For more information, see [AMQP
     * Section 2.8.15](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-error).
     * 	 */
    std::string ErrorDescription;

    /**
     * @brief The status code associated with the error, if any.
     *
     * If this field has a value, then it will typically be an HTTP status code with additional
     * information about the failure. This property is only filled in for the GetEventHubProperties
     * and GetEventHubPartitionProperties operations.
     *
     */
    Azure::Nullable<std::uint32_t> StatusCode{};
  };
}}} // namespace Azure::Messaging::EventHubs
