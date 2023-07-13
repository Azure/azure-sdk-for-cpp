// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

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

namespace Azure { namespace Core { namespace _internal {
  template <> struct UniqueHandleHelper<HEADER_INSTANCE_TAG>
  {
    static void FreeAmqpHeader(HEADER_INSTANCE_TAG* obj);

    using type = BasicUniqueHandle<HEADER_INSTANCE_TAG, FreeAmqpHeader>;
  };
}}} // namespace Azure::Core::_internal

namespace Azure { namespace Core { namespace Amqp { namespace Models {

  using UniqueMessageHeaderHandle = Azure::Core::_internal::UniqueHandle<HEADER_INSTANCE_TAG>;

  /**
   * @brief The message header section carries standard delivery details about the transfer of a
   * message through the AMQP network.
   *
   * @remarks For more information, see [AMQP
   * Section 3.2.1](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-header).
   */
  struct MessageHeader final
  {

    MessageHeader() = default;
    ~MessageHeader() = default;

    /** @brief Compare two AMQP Message Headers for equality.
     *
     * @param that - the AMQP Message Header to compare to.
     * @returns true if the two headers are equal, false otherwise.
     */
    bool operator==(MessageHeader const& that) const noexcept;

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
    Nullable<std::chrono::milliseconds> TimeToLive;

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

    /** @brief Returns true if this MessageHeader should be serialized into an AMQP message.
     *
     * Message headers can be serialize if their values are different from the defined default
     * values as specified in the AMQP spec.
     */
    bool ShouldSerialize() const noexcept;

    /** @brief Returns the serialized size of this MessageHeader.
     *
     * @remarks This is used to calculate the AMQP message size.
     */
    static size_t GetSerializedSize(MessageHeader const& messageHeader);

    /** @brief Serializes this MessageHeader into a vector of bytes.
     *
     * @remarks This is used to serialize the AMQP message header.
     *
     * @returns A vector of bytes representing this MessageHeader.
     */
    static std::vector<uint8_t> Serialize(MessageHeader const& messageHeader);

    /** @brief Deserializes a vector of bytes into a MessageHeader.
     *
     * @remarks This is used to create an AMQP message header from a vector of bytes.
     */
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
