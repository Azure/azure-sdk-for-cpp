// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "amqp_header.hpp"
#include "amqp_properties.hpp"
#include "amqp_value.hpp"
#include <vector>

struct MESSAGE_INSTANCE_TAG;
namespace Azure { namespace Core { namespace Amqp { namespace Models {

  enum class MessageBodyType
  {
    Invalid,
    None,
    Data,
    Sequence,
    Value,
  };

  class Message {
    MESSAGE_INSTANCE_TAG* m_message;

  public:
    Message();
    ~Message();

    Message(Message const&);
    Message& operator=(Message const&);
    Message(Message&&) noexcept;
    Message& operator=(Message&&) noexcept;

    Message(MESSAGE_INSTANCE_TAG* message);

    operator MESSAGE_INSTANCE_TAG*() const { return m_message; }
    operator bool() const { return m_message != nullptr; }

    void SetHeader(Header const& header);
    Header const GetHeader() const;

    void SetFooter(AmqpValue const& header);
    AmqpValue const GetFooter() const;

    void SetDeliveryAnnotations(AmqpValue const& annotations);
    AmqpValue const GetDeliveryAnnotations() const;

    void SetFormat(uint32_t messageFormat);
    uint32_t GetFormat() const;

    void SetMessageAnnotations(AmqpValue const& annotations);
    AmqpValue const GetMessageAnnotations() const;

    void SetProperties(MessageProperties const& properties);
    MessageProperties const GetProperties() const;

    // message application-properties are maps with the key scoped to a string,
    // and whose values cannot be map, list and array types.
    void SetApplicationProperties(AmqpMap const& value);
    AmqpMap const GetApplicationProperties() const;

    MessageBodyType GetBodyType() const;

    void AddBodyAmqpSequence(AmqpValue const& value);
    std::size_t GetBodyAmqpSequenceCount() const;
    AmqpValue const GetBodyAmqpSequence(std::uint32_t index) const;

    void AddBodyData(std::vector<uint8_t> const&binaryData);
    std::vector<uint8_t> GetBodyData(std::uint32_t dataIndex) const;
    size_t GetBodyDataCount() const;

    void SetBodyAmqpValue(AmqpValue value);
    AmqpValue GetBodyAmqpValue() const;

    friend std::ostream& operator<<(std::ostream&, Message const&);
  };
}}}} // namespace Azure::Core::Amqp::Models
