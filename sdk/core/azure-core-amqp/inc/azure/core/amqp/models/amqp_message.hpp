// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

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

    using type = BasicUniqueHandle<MESSAGE_INSTANCE_TAG, FreeAmqpMessage>;
  };
}}} // namespace Azure::Core::_internal

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {
  class AmqpMessageFactory;
}}}}} // namespace Azure::Core::Amqp::Models::_internal

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
  using UniqueMessageHandle = Azure::Core::_internal::UniqueHandle<MESSAGE_INSTANCE_TAG>;

  constexpr int AmqpMessageFormatValue = 0; // Specifies the message format for an AMQP message.

  /** @brief An AmqpMessage object represents a received AMQP message.
   * @remark An AMQP message is comprised of a header, properties, application properties, and
   * body. The body of the message can be one of the following types:
   * - A single AMQP Value.
   * - One or more binary data sections.
   * - One or more sequence sections.
   *
   * For more information, see [AMQP Message
   * Format](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#section-message-format).
   *
   */
  class AmqpMessage final {
  public:
    /** @brief Construct a new AMQP Message object. */
    AmqpMessage() = default;

    /** @brief Destroy an instance of an AMQP Message object. */
    ~AmqpMessage() = default;

    /** @brief Create a new AMQP Message from an existing message moving the contents. */
    AmqpMessage(AmqpMessage&&) = default;

    /** @brief Construct a new AMQP message object from an existing object. */
    AmqpMessage(AmqpMessage const&) = default;

    /** @brief Copy an AMQP message object to another object.
     * @returns A reference to this.
     */
    AmqpMessage& operator=(AmqpMessage const&) = default;

    /** @brief Move an AMQP message object to another object.
     * @returns A reference to this.
     */
    AmqpMessage& operator=(AmqpMessage&&) = default;

    /** @brief Compare two AmqpMessages for equality. */
    bool operator==(AmqpMessage const& that) const noexcept;

    /** @brief Compare two AmqpMessage values. */
    bool operator!=(AmqpMessage const& that) const noexcept { return !(*this == that); }

    /** @brief Construct an empty AMQP Message. */
    AmqpMessage(std::nullptr_t) : m_hasValue{false} {}

    /** @brief Returns True if the AMQP message has a value, false otherwise.
     *
     * @returns true if the AMQP message has a value, false otherwise.
     */
    operator bool() const noexcept { return m_hasValue; }

    /** @brief The header for the message.
     *
     * For more information, see [AMQP Message
     * Format](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#section-message-format).
     */
    MessageHeader Header;

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

    /** @brief Sets the body of the message to a list of sequence sections.
     *
     * An AMQP Message Body can be one of the following formats:
     *
     * - One or more binary data sections
     * - One or more sequence sections.
     * - A single AMQP Value.
     *
     * This method appends the bodySequence value to the sequence of sections. See [Amqp
     * Sequence](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-sequence)
     * for more information.
     *
     * @param bodySequence - the list of AMQP values which make up the body of the message.
     *
     */
    void SetBody(std::vector<AmqpList> const& bodySequence);

    /** @brief Appends a list to the body of the message.
     *
     * An AMQP Message Body can be one of the following formats:
     *
     * - One or more binary data sections
     * - One or more sequence sections.
     * - A single AMQP Value.
     *
     * This method appends the bodySequence value to the sequence of sections. See [Amqp
     * Sequence](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-sequence)
     * for more information.
     *
     * @param bodySequence - the list of AMQP values which make up the body of the message.
     *
     * @remarks This is a convenience method to make it simpler to append a single binary value to
     * the message body.
     *
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

    /** @brief Appends a binary value to the body of the message.
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
     * @remarks This is a convenience method to make it simpler to append a single binary value to
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

    /** @brief Returns a list of Amqp Sequence values.
     *
     * @remarks This API will fail if BodyType is not MessageBodyType::Sequence.
     */
    std::vector<AmqpList> GetBodyAsAmqpList() const;

    /** @brief Returns an Amqp Value message body.
     *
     * @remarks This API will fail if BodyType is not MessageBodyType::Value.
     */
    AmqpValue GetBodyAsAmqpValue() const;

    /** @brief Returns an Amqp Binary message body.
     *
     * @remarks This API will fail if BodyType is not MessageBodyType::Binary.
     */
    std::vector<AmqpBinaryData> GetBodyAsBinary() const;

    /** @brief Serialize the message into a buffer.
     *
     * @remarks This API will fail if BodyType is not set.
     */
    static std::vector<uint8_t> Serialize(AmqpMessage const& message);

    /** @brief Deserialize the message from a buffer.
     *
     * @remarks This API will fail if BodyType is not set.
     */
    static AmqpMessage Deserialize(std::uint8_t const* buffer, size_t size);

    friend class _internal::AmqpMessageFactory;

  private:
    std::vector<AmqpBinaryData> m_binaryDataBody;
    std::vector<AmqpList> m_amqpSequenceBody;
    AmqpValue m_amqpValueBody;
    bool m_hasValue{true}; // By default, an AmqpMessage has a value.
  };
  std::ostream& operator<<(std::ostream&, AmqpMessage const&);
}}}} // namespace Azure::Core::Amqp::Models

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {
  /**
   * @brief uAMQP interoperability functions to convert a MessageProperties to a uAMQP
   * PROPERTIES_HANDLE and back.
   *
   * @remarks This class should not be used directly. It is used by the uAMQP interoperability
   * layer.
   */
  class AmqpMessageFactory final {
  public:
    static AmqpMessage FromUamqp(UniqueMessageHandle const& properties);
    static AmqpMessage FromUamqp(MESSAGE_INSTANCE_TAG* properties);
    static UniqueMessageHandle ToUamqp(AmqpMessage const& properties);
  };
}}}}} // namespace Azure::Core::Amqp::Models::_internal
