// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <exception>
#include <stdexcept>

struct HEADER_INSTANCE_TAG;
namespace Azure { namespace Core { namespace Amqp { namespace Models {

  class Header {
    HEADER_INSTANCE_TAG* m_header;

  public:
    // uAMQP integration functions.
    Header(HEADER_INSTANCE_TAG* header) : m_header(header) {}
    operator HEADER_INSTANCE_TAG*() const { return m_header; }

  public:
    Header();
    operator bool() const { return m_header != nullptr; }

    virtual ~Header();

    bool IsDurable() const;
    void IsDurable(bool durable);

    uint8_t Priority() const;
    void SetPriority(uint8_t priority);

    std::chrono::milliseconds GetTimeToLive() const;
    void SetTimeToLive(std::chrono::milliseconds timeToLive);

    bool IsFirstAcquirer() const;
    void SetFirstAcquirer(bool value);

    uint32_t GetDeliveryCount() const;
    void SetDeliveryCount(uint32_t value);

    bool operator==(Header const& that) const
    {
      return (this->IsDurable() == that.IsDurable())
          && (this->GetDeliveryCount() == that.GetDeliveryCount())
          && (this->GetTimeToLive() == that.GetTimeToLive())
          && (this->IsFirstAcquirer() == that.IsFirstAcquirer());
    }
    friend std::ostream& operator<<(std::ostream&, Header const&);
  };
}}}} // namespace Azure::Core::Amqp::Models
