// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "amqp_header.hpp"
#include "amqp_properties.hpp"
#include "amqp_value.hpp"

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
    Header GetHeader() const;

    void SetFooter(Value const& header);
    Value GetFooter() const;

    void SetDeliveryAnnotations(Value const& annotations);
    Value const GetDeliveryAnnotations() const;

    void SetFormat(uint32_t messageFormat);
    uint32_t GetFormat() const;

    void SetMessageAnnotations(Value const& annotations);
    Value const GetMessageAnnotations() const;

    void SetProperties(Properties const& properties);
    Properties const GetProperties() const;

    void SetApplicationProperties(Value const& value);
    Value const GetApplicationProperties() const;

    MessageBodyType GetBodyType() const;

    void AddBodyAmqpSequence(Value const& value);
    size_t GetBodyAmqpSequenceCount() const;
    Value const GetBodyAmqpSequence(uint32_t index) const;

    void AddBodyAmqpData(BinaryData binaryData);

    BinaryData GetBodyAmqpData(size_t index) const;
    size_t GetBodyAmqpDataCount() const;

    void SetBodyAmqpValue(Value value);
    Value GetBodyAmqpValue() const;

    friend std::ostream& operator<<(std::ostream&, Message const&);
  };
}}}} // namespace Azure::Core::Amqp::Models
