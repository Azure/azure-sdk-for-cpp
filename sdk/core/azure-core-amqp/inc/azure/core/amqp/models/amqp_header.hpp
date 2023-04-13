// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/internal/unique_handle.hpp>
#include <azure/core/nullable.hpp>
#include <chrono>
#include <cstddef>
#include <exception>
#include <stdexcept>

struct HEADER_INSTANCE_TAG;

namespace Azure { namespace Core { namespace _internal {

  template <> struct UniqueHandleHelper<HEADER_INSTANCE_TAG>
  {
    static void FreeAmqpHeader(HEADER_INSTANCE_TAG* obj);

    using type = Azure::Core::_internal::BasicUniqueHandle<HEADER_INSTANCE_TAG, FreeAmqpHeader>;
  };
}}} // namespace Azure::Core::_internal

namespace Azure { namespace Core { namespace Amqp { namespace Models {

  using UniqueMessageHeaderHandle = Azure::Core::_internal::UniqueHandle<HEADER_INSTANCE_TAG>;

  class MessageHeader {
  public:
    // uAMQP integration functions.
    MessageHeader(HEADER_INSTANCE_TAG* header);
    operator UniqueMessageHeaderHandle() const;

  public:
    MessageHeader();
    virtual ~MessageHeader();

    /** @brief True if the message is considered "durable"
     *
     * @remarks For more information, see [AMQP
     * Section 3.2.1](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-header).
     */
    bool Durable{false};

    /** @brief Priority of the message.
     *
     * @remarks For more information, see [AMQP
     * Section 3.2.1](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-header).
     */
    std::uint8_t Priority{4}; // Default priority is 4.

    /** @brief If present, defines the time to live for the message.
     *
     * @remarks For more information, see [AMQP
     * Section 3.2.1](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-header).
     */
    Azure::Nullable<std::chrono::milliseconds> TimeToLive;

    /** @brief If true, the message has not been acquired by any other link.
     *
     * @remarks For more information, see [AMQP
     * Section 3.2.1](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-header).
     */
    bool IsFirstAcquirer{false};

    /** @brief The number of unsuccessful previous attempts to deliver this message.
     *
     * @remarks For more information, see [AMQP
     * Section 3.2.1](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-header).
     */

    std::uint32_t DeliveryCount{0};

    friend std::ostream& operator<<(std::ostream&, MessageHeader const&);
  };
}}}} // namespace Azure::Core::Amqp::Models
