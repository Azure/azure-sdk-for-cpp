// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "amqp_value.hpp"
#include <chrono>
#include <vector>

struct PROPERTIES_INSTANCE_TAG;

namespace Azure { namespace Core { namespace Amqp { namespace Models {

  class MessageProperties {

  private:
    PROPERTIES_INSTANCE_TAG* m_properties{};

    // uAMQP interop functions.
  public:
    MessageProperties(PROPERTIES_INSTANCE_TAG* properties) : m_properties(properties) {}
    operator PROPERTIES_INSTANCE_TAG*() const { return m_properties; }

  public:
    MessageProperties();
    operator bool() const { return m_properties != nullptr; }

    ~MessageProperties();

    AmqpValue GetMessageId() const;
    void SetMessageId(AmqpValue const& messageId);

    AmqpValue GetCorrelationId() const;
    void SetCorrelationId(AmqpValue const& correlationId);

    std::vector<uint8_t> GetUserId() const;
    void SetUserId(std::vector<uint8_t> const& userId);

    AmqpValue GetTo() const;
    void SetTo(AmqpValue replyTo);

    std::string GetSubject() const;
    void SetSubject(std::string const& replyTo);

    AmqpValue GetReplyTo() const;
    void SetReplyTo(AmqpValue replyTo);

    std::string GetContentType() const;
    void SetContentType(std::string const& contentType);

    std::string GetContentEncoding() const;
    void SetContentEncoding(std::string const& contentEncoding);

    std::chrono::system_clock::time_point GetAbsoluteExpiryTime() const;
    void SetAbsoluteExpiryTime(std::chrono::system_clock::time_point const& absoluteExpiryTime);

    std::chrono::system_clock::time_point GetCreationTime() const;
    void SetCreationTime(std::chrono::system_clock::time_point const& creationTime);

    std::string GetGroupId() const;
    void SetGroupId(std::string const& groupId);

    uint32_t GetGroupSequence() const;
    void SetGroupSequence(uint32_t groupSequence);

    std::string GetReplyToGroupId() const;
    void SetReplyToGroupId(std::string const& replyToGroupId);

    friend std::ostream& operator<<(std::ostream&, MessageProperties const&);
  };

}}}} // namespace Azure::Core::Amqp::Models
