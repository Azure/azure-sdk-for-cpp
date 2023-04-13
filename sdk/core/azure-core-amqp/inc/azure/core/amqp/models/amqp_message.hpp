// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "amqp_header.hpp"
#include "amqp_properties.hpp"
#include "amqp_value.hpp"
#include <azure/core/internal/unique_handle.hpp>
#include <azure/core/nullable.hpp>
#include <map>
#include <vector>

struct MESSAGE_INSTANCE_TAG;
namespace Azure { namespace Core { namespace _internal {

  template <> struct UniqueHandleHelper<MESSAGE_INSTANCE_TAG>
  {
    static void FreeAmqpMessage(MESSAGE_INSTANCE_TAG* obj);

    using type = Azure::Core::_internal::BasicUniqueHandle<MESSAGE_INSTANCE_TAG, FreeAmqpMessage>;
  };
}}} // namespace Azure::Core::_internal

namespace Azure { namespace Core { namespace Amqp { namespace Models {

  /**
   * @brief The type of the body of an AMQP Message.
   *
   */
  enum class MessageBodyType
  {
    Invalid,
    None,
    Data,
    Sequence,
    Value,
  };

  class Message {
  public:
    /** @brief Construct a new AMQP Message object. */
    Message() = default;

    /** @brief Destroy an instance of an AMQP Message object. */
    ~Message() = default;

    /** @brief Construct a new AMQP message object from an existing object. */
    Message(Message const&) = default;

    /** @brief Copy an AMQP message object to another object. @returns A reference to this.*/
    Message& operator=(Message const&) = default;

    /** @brief Create a new AMQP Message from an existing message moving the contents. */
    Message(Message&&) noexcept = default;

    /** @brief Move an AMQP message object to another object. @returns A reference to this.*/
    Message& operator=(Message&&) noexcept = default;

    /** @brief The header for the message.
     *
     * For more information, see [AMQP Message
     * Format](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#section-message-format).
     */
    MessageHeader Header;

    /** @brief Specifies the 'format' of the message.
     *
     * For more information, see [AMQP Transfer
     * performative](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-transfer)
     */
    Azure::Nullable<uint32_t> MessageFormat;

    /** @brief Delivery Annotations for the message.
     *
     * For more information, see [AMQP Delivery
     * Annotations](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-delivery-annotations).
     */
    AmqpMap DeliveryAnnotations;

    /** @brief Message Annotations for the message.
     *
     * For more information, see [AMQP Message
     * Annotations](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-message-annotations).
     */
    AmqpMap MessageAnnotations;

    /** @brief Immutable Properties for the message.
     *
     * For more information, see [AMQP
     * Properties](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-properties).
     */
    MessageProperties Properties;

    /** @brief Application Properties for the message.
     *
     * For more information, see [AMQP Application
     * Properties](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-application-properties).
     */
    std::map<std::string, AmqpValue> ApplicationProperties;

    /** @brief Footer for the message.
     *
     * For more information, see [AMQP
     * Footer](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-footer).
     */
    AmqpMap Footer;

    /** Specifies the type of the body.
     *
     * An AMQP Message Body can be one of the following formats:
     *
     * - One or more binary data sections
     * - One or more sequence sections.
     * - A single AMQP Value.
     *
     */
    MessageBodyType BodyType{MessageBodyType::None};

    /** @brief Set the body of the message.
     *
     * An AMQP Message Body can be one of the following formats:
     *
     * - One or more binary data sections
     * - One or more sequence sections.
     * - A single AMQP Value.
     *
     * This method sets the body of the message to a sequence of sections. See [Amqp
     * Sequence](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-sequence)
     * for more information.
     *
     * @param bodySequence - the list of AMQP values which make up the body of the message.
     *
     */
    void SetBody(AmqpList const& bodySequence);

    /** @brief Set the body of the message.
     *
     * An AMQP Message Body can be one of the following formats:
     *
     * - One or more binary data sections
     * - One or more sequence sections.
     * - A single AMQP Value.
     *
     * This method sets the body of the message to a sequence data values. See [Amqp
     * Data](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-data)
     * for more information.
     *
     * @param bodyBinarySequence - a sequence of binary data which which makes up the body of
     * the message.
     *
     */
    void SetBody(std::vector<AmqpBinaryData> const& bodyBinarySequence);

    /** @brief Set the body of the message.
     *
     * An AMQP Message Body can be one of the following formats:
     *
     * - One or more binary data sections
     * - One or more sequence sections.
     * - A single AMQP Value.
     *
     * This method sets the body of the message to a sequence data values. See [Amqp
     * Data](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-data)
     * for more information.
     *
     * @param bodyBinary - a single value binary data.
     *
     * @remarks This is a convenience method to make it simpler to set a single binary value for
     * the message body.
     *
     */
    void SetBody(AmqpBinaryData const& bodyBinary);

    /** @brief Set the body of the message.
     *
     * An AMQP Message Body can be one of the following formats:
     *
     * - One or more binary data sections
     * - One or more sequence sections.
     * - A single AMQP Value.
     *
     * This method sets the body of the message to a single AMQP value. See [Amqp
     * Value](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-amqp-value)
     * for more information.
     *
     * @param bodyValue - a single value binary data.
     *
     */
    void SetBody(AmqpValue const& bodyValue);

    /** @brief Returns an Amqp Sequence message body.
     *
     * @remarks This API will fail if BodyType is not MessageBodyType::Sequence.
     */
    AmqpList GetBodyAsAmqpList() const;

    /** @brief Returns an Amqp Value message body.
     *
     * @remarks This API will fail if BodyType is not MessageBodyType::Value.
     */
    AmqpValue GetBodyAsAmqpValue() const;

    /** @brief Returns an Amqp Binary message body.
     *
     * @remarks This API will fail if BodyType is not MessageBodyType::Data.
     */
    std::vector<AmqpBinaryData> GetBodyAsBinary() const;

    // uAMQP interop functions.
    Message(MESSAGE_INSTANCE_TAG* message);
    operator MESSAGE_INSTANCE_TAG*() const;

    friend std::ostream& operator<<(std::ostream&, Message const&);

  private:
    std::vector<AmqpBinaryData> m_binaryDataBody;
    AmqpList m_amqpSequenceBody;
    AmqpValue m_amqpValueBody;
  };
}}}} // namespace Azure::Core::Amqp::Models
