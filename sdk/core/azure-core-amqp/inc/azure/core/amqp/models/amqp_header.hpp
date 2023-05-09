// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/internal/unique_handle.hpp>
#include <azure/core/nullable.hpp>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <vector>

struct HEADER_INSTANCE_TAG;

template <> struct Azure::Core::_internal::UniqueHandleHelper<HEADER_INSTANCE_TAG>
{
  static void FreeAmqpHeader(HEADER_INSTANCE_TAG* obj);

  using type = Azure::Core::_internal::BasicUniqueHandle<HEADER_INSTANCE_TAG, FreeAmqpHeader>;
};

namespace Azure { namespace Core { namespace Amqp { namespace Models {

  using UniqueMessageHeaderHandle = Azure::Core::_internal::UniqueHandle<HEADER_INSTANCE_TAG>;

  struct MessageHeader
  {

    MessageHeader() = default;
    ~MessageHeader() = default;

    bool operator==(MessageHeader const&) const noexcept;

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

    bool ShouldSerialize() const noexcept;
    static size_t GetSerializedSize(MessageHeader const& messageHeader);
    static std::vector<uint8_t> Serialize(MessageHeader const& messageHeader);
    static MessageHeader Deserialize(std::uint8_t const* data, size_t size);
  };
  std::ostream& operator<<(std::ostream&, MessageHeader const&);

}}}} // namespace Azure::Core::Amqp::Models

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {
  /**
   * @brief uAMQP interoperability functions to convert a MessageHeader to a uAMQP HEADER_HANDLE
   * and back.
   *
   * @remarks This class should not be used directly. It is used by the uAMQP interoperability
   * layer.
   */
  struct MessageHeaderFactory
  {
    static MessageHeader FromUamqp(UniqueMessageHeaderHandle const& properties);
    static UniqueMessageHeaderHandle ToUamqp(MessageHeader const& properties);
  };

}}}}} // namespace Azure::Core::Amqp::Models::_internal
