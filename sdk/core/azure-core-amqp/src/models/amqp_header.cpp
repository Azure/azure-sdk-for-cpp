// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/models/amqp_header.hpp"

#include <azure_uamqp_c/amqp_definitions_milliseconds.h>

#include <azure_uamqp_c/amqp_definitions_header.h>
#include <chrono>
#include <iostream>

namespace Azure { namespace Core { namespace Amqp { namespace Models {

  Header::Header() : m_header(header_create())
  {
    if (!m_header)
    {
      throw std::runtime_error("Could not create header.");  // LCOV_EXCL_LINE
    }
  }

  Header::~Header() { header_destroy(m_header); }

  bool Header::IsDurable() const
  {
    bool isDurable;
    if (header_get_durable(m_header, &isDurable))
    {
      throw std::runtime_error("Could not get durable state from header."); // LCOV_EXCL_LINE
    }
    return isDurable;
  }
  void Header::IsDurable(bool durable) { header_set_durable(m_header, durable); }

  uint8_t Header::Priority() const
  {
    uint8_t priority;
    if (header_get_priority(m_header, &priority))
    {
      throw std::runtime_error("Could not get priority from header."); // LCOV_EXCL_LINE
    }
    return priority;
  }
  void Header::SetPriority(uint8_t priority) { header_set_priority(m_header, priority); }

  std::chrono::milliseconds Header::GetTimeToLive() const
  {
    milliseconds ms;
    if (header_get_ttl(m_header, &ms))
    {
      throw std::runtime_error("Could not get header TTL.");
    }
    return std::chrono::milliseconds(ms);
  }
  void Header::SetTimeToLive(std::chrono::milliseconds timeToLive)
  {
    if (header_set_ttl(m_header, static_cast<milliseconds>(timeToLive.count())))
    {
      throw std::runtime_error("Could not set header TTL."); // LCOV_EXCL_LINE
    }
  }

  bool Header::IsFirstAcquirer() const
  {
    bool firstAcquirer;
    if (header_get_first_acquirer(m_header, &firstAcquirer))
    {
      throw std::runtime_error("Could not get first acquirer value."); // LCOV_EXCL_LINE
    }
    return firstAcquirer;
  }
  void Header::SetFirstAcquirer(bool value)
  {
    if (header_set_first_acquirer(m_header, value))
    {
      throw std::runtime_error("Could not set first acquirer value.");
    }
  }

  uint32_t Header::GetDeliveryCount() const
  {
    uint32_t count;
    if (header_get_delivery_count(m_header, &count))
    {
      throw std::runtime_error("Could not get delivery count."); // LCOV_EXCL_LINE
    }
    return count;
  }
  void Header::SetDeliveryCount(uint32_t value)
  {
    if (header_set_delivery_count(m_header, value))
    {
      throw std::runtime_error("Could not set delivery count."); // LCOV_EXCL_LINE
    }
  }

  std::ostream& operator<<(std::ostream& os, Header const& header)
  {
    os << "Header{";
    os << "durable=" << header.IsDurable();
    {
      uint8_t priority;
      if (!header_get_priority(header, &priority))
      {
        os << ", priority=" << header.Priority();
      }
    }
    {
      milliseconds ttl;
      if (!header_get_ttl(header, &ttl))
      {
        os << ", ttl=" << header.GetTimeToLive().count() << " milliseconds";
      }
    }
    os << ", firstAcquirer=" << header.IsFirstAcquirer();
    os << ", deliveryCount=" << header.GetDeliveryCount();
    os << "}";
    return os;
  }
}}}} // namespace Azure::Core::Amqp::Models
