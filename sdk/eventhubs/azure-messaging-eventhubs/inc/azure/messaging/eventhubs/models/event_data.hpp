// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

#include <azure/core/amqp/models/amqp_message.hpp>
#include <azure/core/amqp/models/amqp_value.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/nullable.hpp>

#include <initializer_list>
#include <iostream>
#include <map>
#include <vector>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Models {

  /** @brief Represents an event sent to the Azure Event Hubs service.
   */
  class EventData {
  public:
    /** @brief The body of the event data.
     */
    std::vector<uint8_t> Body;

    /** Represents the MIME ContentType of the event data. */
    Azure::Nullable<std::string> ContentType;

    /** @brief The correlation identifier.
     *
     * Allows an application to specify a context for the event data, such as the type of the event
     * data or the entity that produced the event data.
     */
    Azure::Core::Amqp::Models::AmqpValue CorrelationId;

    /** @brief The message identifier.
     *
     * The identifier is an application-defined value that uniquely identifies the message
     * and its payload. The identifier is a free-form string and can reflect a GUID or an
     * identifier derived from the application context.
     */
    Azure::Core::Amqp::Models::AmqpValue MessageId;

    /** @brief The set of free-form event properties.
     *
     * The properties are for application-specific use.
     */
    std::map<std::string, Azure::Core::Amqp::Models::AmqpValue> Properties;

    /** @brief Construct a default EventData object. */
    EventData() : m_message{nullptr} {};

    /** @brief Construct a new EventData object from an AMQP message.
     *
     * @param message - AMQP message to construct the EventData from.
     */
    EventData(std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage const> const& message);

    /** @brief Construct a new EventData object from an initializer list of bytes
     *
     * @param body - Body for the newly created EventData.
     */
    EventData(std::initializer_list<uint8_t> const& body) : Body(body), m_message{nullptr} {}

    /** @brief Construct a new EventData object from a vector of bytes.
     *
     * @param body - Body for the newly created EventData.
     */
    EventData(std::vector<uint8_t> const& body) : Body(body), m_message{nullptr} {}

    /** @brief Construct a new EventData object from a string.
     *
     * @param body - Body for the newly created EventData.
     */
    EventData(std::string const& body) : Body(body.begin(), body.end()), m_message{nullptr} {}

    virtual ~EventData() = default;

    /** Copy an EventData to another.
     */
    EventData(EventData const&) = default;

    /** Assign an EventData to another.
     */
    EventData& operator=(EventData const&) = default;

    /** Create an EventData moving from another.
     */
    EventData(EventData&&) = default;

    /** Move an EventData to another.
     */
    EventData& operator=(EventData&&) = default;

    /** @brief Get the AMQP message associated with this EventData.
     *
     * Returns an underlying AMQP message corresponding to this EventData object.
     *
     * @note When this method is called on an EventData object, the returned message is
     * constructed from the fields of the EventData object and does NOT reflect the value received
     * from the service.
     *
     */
    virtual std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage const> GetRawAmqpMessage() const;

  protected:
    /** The incoming AMQP message, if one was received. */
    std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage const> m_message;
  };
  std::ostream& operator<<(std::ostream&, EventData const&);

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
    Azure::Nullable<std::string> Offset;

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
    ReceivedEventData(std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage const> const& message);

    // Destructor
    ~ReceivedEventData() override = default;

    /** @brief Copy an ReceivedEventData to another.
     */
    ReceivedEventData(ReceivedEventData const&) = default;

    /** @brief Assign an ReceivedEventData to another.
     * /
     */
    ReceivedEventData& operator=(ReceivedEventData const&) = default;

    /** @brief Create an ReceivedEventData moving from another.
     */
    ReceivedEventData(ReceivedEventData&&) = default;

    /** @brief Move an ReceivedEventData to another.
     */
    ReceivedEventData& operator=(ReceivedEventData&&) = default;

    /** @brief Get the raw AMQP message.
     *
     * Returns the underlying AMQP message that was received from the Event Hubs service.
     */
    std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage const> GetRawAmqpMessage() const override
    {
      return m_message;
    }
  };
  std::ostream& operator<<(std::ostream&, ReceivedEventData const&);
}}}} // namespace Azure::Messaging::EventHubs::Models
