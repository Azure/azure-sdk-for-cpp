// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include <azure/core/amqp.hpp>
namespace Azure { namespace Messaging { namespace EventHubs { namespace Models {

  /** @brief The type of the body of an AMQP Message.
   *
   */
  struct AmqpMessageBody
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
    std::vector<Azure::Core::Amqp::Models::AmqpBinaryData> Data;

    void SetMessageBody(Azure::Core::Amqp::Models::AmqpMessage& message)
    {
      if (!Data.empty())
      {
        message.SetBody(Data);
      }
      else if (!Sequence.empty())
      {
        message.SetBody(Sequence);
      }
      else
      {
        message.SetBody(Value);
      }
    };
  };

  struct AmqpAnnotatedMessage : private Azure::Core::Amqp::Models::AmqpMessage
  {
    using Azure::Core::Amqp::Models::AmqpMessage::ApplicationProperties;
    AmqpMessageBody Body;
    using Azure::Core::Amqp::Models::AmqpMessage::DeliveryAnnotations;
    using Azure::Core::Amqp::Models::AmqpMessage::Footer;
    using Azure::Core::Amqp::Models::AmqpMessage::Header;
    using Azure::Core::Amqp::Models::AmqpMessage::MessageAnnotations;
    using Azure::Core::Amqp::Models::AmqpMessage::Properties;

    Azure::Core::Amqp::Models::AmqpMessage ToAMQPMessage()
    {
      Azure::Core::Amqp::Models::AmqpMessage message;

      message.Footer = std::move(Footer);
      message.DeliveryAnnotations = std::move(DeliveryAnnotations);
      message.MessageAnnotations = std::move(MessageAnnotations);
      message.ApplicationProperties = std::move(ApplicationProperties);
      message.Header = std::move(Header);
      // message ID is set by the message not by the user
      // so we need to move it back
      Properties.MessageId = std::move(message.Properties.MessageId);
      message.Properties = std::move(Properties);

      Body.SetMessageBody(message);

      return message;
    };
  };

  struct EventData : private Azure::Core::Amqp::Models::AmqpMessage,
                     private Azure::Core::Amqp::Models::MessageProperties
  {
    using Azure::Core::Amqp::Models::AmqpMessage::ApplicationProperties;
    AmqpMessageBody Body;
    using Azure::Core::Amqp::Models::MessageProperties::ContentType;
    using Azure::Core::Amqp::Models::MessageProperties::CorrelationId;
    // we need this for when we receive messages from the service
    using Azure::Core::Amqp::Models::MessageProperties::MessageId;

    Azure::Core::Amqp::Models::AmqpMessage ToAMQPMessage()
    {
      Azure::Core::Amqp::Models::AmqpMessage message;

      Body.SetMessageBody(message);

      message.ApplicationProperties = std::move(ApplicationProperties);
      message.Properties.ContentType = std::move(ContentType);
      message.Properties.CorrelationId = std::move(CorrelationId);

      return message;
    };
  };
}}}} // namespace Azure::Messaging::EventHubs::Models