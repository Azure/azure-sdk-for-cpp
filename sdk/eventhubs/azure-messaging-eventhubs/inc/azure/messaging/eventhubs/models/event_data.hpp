// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include <azure/core/amqp.hpp>
namespace Azure { namespace Messaging { namespace EventHubs { namespace Models {

  /** @brief The type of the body of an EventData Message.
   *
   */
  struct EventDataBody final
  {
    /** @brief Value is encoded / decoded as the amqp - value section in the body.
     *
     * The type of Value can be any of the AMQP simple types, as listed in the comment for
     * AmqpMessage, as well as slices or maps of AMQP simple types.
     */
    Azure::Core::Amqp::Models::AmqpValue Value;

    /** @brief  Sequence is encoded/decoded as one or more amqp-sequence sections in the body.
     *
     * The values of the slices are are restricted to AMQP simple types, as listed in the comment
     * for AmqpMessage.
     */
    Azure::Core::Amqp::Models::AmqpList Sequence;

    /** @brief Data is encoded decoded as multiple data sections in the body.
     */
    Azure::Core::Amqp::Models::AmqpBinaryData Data;
  };

  /** @brief Represents an event sent to the Azure Event Hubs service.
   */
  struct EventData
  {
    /** @brief The body of the event data.
     */
    EventDataBody Body;

    /** Represents the MIME ContentType of the event data. */
    Azure::Nullable<std::string> ContentType;

    /** @brief The correlation identifier.
     *
     * Allows an application to specify a context for the event data, such as the type of the event
     * data or the entity that produced the event data.
     */
    Azure::Nullable<Azure::Core::Amqp::Models::AmqpValue> CorrelationId;

    /** @brief The message identifier.
     *
     * The identifier is an application-defined value that uniquely identifies the message
     * and its payload. The identifier is a free-form string and can reflect a GUID or an
     * identifier derived from the application context.
     */
    Azure::Nullable<Azure::Core::Amqp::Models::AmqpValue> MessageId;

    /** @brief The set of free-form event properties.
     *
     * The properties are for application-specific use.
     */
    std::map<std::string, Azure::Core::Amqp::Models::AmqpValue> Properties;

    EventData() = default;
    virtual ~EventData() = default;
  };

  /** @brief Represents an event received from the Azure Event Hubs service.
   *
   * Events Received from the EventHubs service have additional information associated with them,
   * specifically the date and time that the event was enqueued, the offset of the event data within
   * the partition, and the partition key for sending a message to a partition.
   */
  class ReceivedEventData final : public EventData {
  public:
    /** @brief The date and time that the event was enqueued.
     *
     * The value is expressed in UTC.
     */
    Azure::Nullable<Azure::DateTime> EnqueuedTime;

    /** @brief The offset of the event data within the partition.
     *
     * The offset is a marker or identifier for an event within the Event Hubs stream.
     * The identifier is unique within a partition of the Event Hubs stream.
     */
    Azure::Nullable<std::uint64_t> Offset;

    /** @brief The partition key for sending a message to a partition.
     *
     * The partition key is used to determine the partition that the message is sent to.
     * Messages with the same partition key are sent to the same partition.
     * Messages without a partition key are sent to a random partition.
     */
    Azure::Nullable<std::string> PartitionKey;

    /** @brief The sequence number of the event data.
     *
     * The sequence number is a unique identifier for the event within its partition.
     */
    Azure::Nullable<std::int64_t> SequenceNumber;

    /** @brief The set of system properties populated by the Event Hubs service.
     *
     * The properties are for read-only use by the application.
     */
    std::map<std::string, Azure::Core::Amqp::Models::AmqpValue> SystemProperties;

    /** @brief Construct a ReceivedEventData from an AMQP Message.
     *
     * This constructor is used internally during the receive operation.
     */
    ReceivedEventData(Azure::Core::Amqp::Models::AmqpMessage const& message);

    /** @brief Get the raw AMQP message.
     *
     * Returns the underlying AMQP message that was received from the Event Hubs service.
     */
    Azure::Core::Amqp::Models::AmqpMessage const& RawAmqpMessage() const { return m_message; }

  private:
    Azure::Core::Amqp::Models::AmqpMessage const m_message;
  };
}}}} // namespace Azure::Messaging::EventHubs::Models
