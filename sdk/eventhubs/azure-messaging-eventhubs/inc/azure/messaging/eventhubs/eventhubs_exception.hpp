// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/amqp/internal/models/amqp_error.hpp>

#include <stdexcept>
#include <string>
namespace Azure { namespace Messaging { namespace EventHubs { namespace _detail {
  class EventHubsExceptionFactory;
}}}} // namespace Azure::Messaging::EventHubs::_detail

namespace Azure { namespace Messaging { namespace EventHubs {

  /**
   * @brief An exception thrown when an EventHubs service operation fails.
   */
  class EventHubsException final : public std::runtime_error {
  public:
    /**
     * @brief Constructs a #EventHubsException with a message.
     *
     * @param what An explanatory string.
     */
    explicit EventHubsException(const std::string& what)
        : std::runtime_error(what), ErrorDescription{what}
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

    /**
     * @brief Indicates whether the error is transient in nature.
     *
     * If this field is set to true, then retrying the operation may succeed at a later time.
     *
     */
    bool IsTransient{};

    friend _detail::EventHubsExceptionFactory;
  };
}}} // namespace Azure::Messaging::EventHubs


