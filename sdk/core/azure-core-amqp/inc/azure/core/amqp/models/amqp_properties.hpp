// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "amqp_value.hpp"

#include <azure/core/nullable.hpp>

#include <chrono>
#include <vector>

namespace Azure { namespace Core { namespace Amqp { namespace Models {

  /** @brief Represents the immutable properties of an AMQP message.
   *
   * The properties section is part of the bare message used for a range of features including
   * reliable delivery, routing and security.
   *
   * @see
   * https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-properties
   */
  struct MessageProperties final
  {
    /** @brief Default constructor.
     *
     * Creates an empty MessageProperties object.
     */
    MessageProperties() = default;

    /** @brief Destructor.
     *
     * Destroys the MessageProperties object.
     */
    ~MessageProperties() = default;

    /** @brief Move Constructor. */
    MessageProperties(MessageProperties&& other) = default;

    /** @brief Move Assignment operator. */
    MessageProperties& operator=(MessageProperties&& other) = default;

    /** @brief Copy Constructor. */
    MessageProperties(MessageProperties const& other) = default;

    /** @brief Copy Assignment operator. */
    MessageProperties& operator=(MessageProperties const& other) = default;

    /** @brief The message-id, if set, uniquely identifies a message within the message system.
     * The message producer is usually responsible for setting the message-id in such a way that
     * it is assured to be globally unique. A broker MAY discard a message as a duplicate if the
     * value of the message-id matches that of a previously received message sent to the same
     * node.
     *
     * @remarks The message producer is usually responsible for setting the message-id in such a
     * way that it is assured to be globally unique.
     */
    AmqpValue MessageId;

    /** @brief User ID.
     *
     * The identity of the user responsible for producing the message. The client sets this value,
     * and it MAY be authenticated by intermediaries.
     */
    Nullable<std::vector<uint8_t>> UserId;

    /** @brief The to field identifies the node that is the intended destination of the message.
     *
     * @remarks A message with no to field is interpreted as being addressed to the implied
     * "anonymous" node.
     */
    AmqpValue To;

    /** @brief The subject of the message
     *
     * A common field for summary information about the message content and purpose.
     */
    Nullable<std::string> Subject;

    /** @brief This is a client-specific id that can be used to mark or identify messages between
     * clients.
     */
    AmqpValue CorrelationId;

    /** @brief The address of the node to send replies to.
     */
    AmqpValue ReplyTo;

    /** @brief The content-type field describes the payload type.
     *
     * The RFC-2046
     * [RFC2046](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-overview-v1.0-os.html#anchor-RFC2046)
     * MIME type for the message's application-data section (body). As per RFC-2046 [RFC2046] this
     * can contain a charset parameter defining the character encoding used: e.g., 'text/plain;
     * charset="utf-8"'.
     *
     *  For clarity, as per section 7.2.1 of RFC-2616
     * [RFC2046](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-overview-v1.0-os.html#anchor-RFC2046),
     * where the content type is unknown the content-type SHOULD NOT be set. This allows the
     * recipient the opportunity to determine the actual type. Where the section is known to be
     * truly opaque binary data, the content-type SHOULD be set to application/octet-stream.
     *
     * When using an application-data section with a section code other than data, content-type
     * SHOULD NOT be set
     */
    Nullable<std::string> ContentType;

    // cspell: ignore IANAHTTPPARAMS
    /** @brief MIME Content Encoding
     *
     * The content-encoding property is used as a modifier to the content-type. When present, its
     * value indicates what additional content encodings have been applied to the
     * application-data, and thus what decoding mechanisms need to be applied in order to obtain
     * the media-type referenced by the content-type header field.
     *
     * Content-encoding is primarily used to allow a document to be compressed without losing the
     * identity of its underlying content type.
     *
     * Content-encodings are to be interpreted as per section 3.5 of RFC 2616 [RFC2616]. Valid
     * content-encodings are registered at IANA [IANAHTTPPARAMS].
     *
     * The content-encoding MUST NOT be set when the application-data section is other than data.
     * The binary representation of all other application-data section types is defined completely
     * in terms of the AMQP type system.
     *
     * Implementations MUST NOT use the identity encoding. Instead, implementations SHOULD NOT set
     * this property. Implementations SHOULD NOT use the compress encoding, except as to remain
     * compatible with messages originally sent with other protocols, e.g. HTTP or SMTP.
     *
     * Implementations SHOULD NOT specify multiple content-encoding values except as to be
     * compatible with messages originally sent with other protocols, e.g. HTTP or SMTP.
     *
     */
    Nullable<std::string> ContentEncoding;

    /** @brief The absolute expiry time of the message.
     *
     * An absolute time when this message is considered to be expired.
     */
    Nullable<std::chrono::system_clock::time_point> AbsoluteExpiryTime;

    /** @brief the time when this message was created. */
    Nullable<std::chrono::system_clock::time_point> CreationTime;

    /** @brief The group this message belongs to.
     *
     * Identifies the group the message belongs to.
     */
    Nullable<std::string> GroupId;

    /** @brief The sequence-number of this message within its group .
     *
     * The relative position of this message within its group.
     */
    Nullable<uint32_t> GroupSequence;

    /** @brief The group the reply message belongs to.
     *
     * This is a client-specific id that is used so that client can send replies to this message
     * to a specific group
     */
    Nullable<std::string> ReplyToGroupId;

    /** @brief Compare two message properties objects.
     *
     * @param that The other message properties object to compare with.
     */
    bool operator==(MessageProperties const& that) const noexcept;

    /** @brief Should this MessageProperties object be serialized
     *
     * @returns true if the object should be serialized, false otherwise.
     */
    bool ShouldSerialize() const noexcept;

    /** @brief Serialize a MessageProperties object into a vector of bytes.
     *
     * @param properties The MessageProperties object to serialize.
     * @returns A vector of bytes representing the serialized MessageProperties object.
     */
    static std::vector<uint8_t> Serialize(MessageProperties const& properties);

    /** @brief Deserialize a MessageProperties object from a vector of bytes.
     *
     * @param data The vector of bytes representing the serialized MessageProperties object.
     * @param size The size of the vector of bytes.
     * @returns The deserialized MessageProperties object.
     */
    static MessageProperties Deserialize(uint8_t const* data, size_t size);
  };

  /** @brief Stream a MessageProperties object to a std::ostream.
   *
   * @param stream The target stream.
   * @param properties The MessageProperties object to serialize.
   * @returns The input ostream object.
   */
  std::ostream& operator<<(std::ostream& stream, MessageProperties const& properties);
}}}} // namespace Azure::Core::Amqp::Models
