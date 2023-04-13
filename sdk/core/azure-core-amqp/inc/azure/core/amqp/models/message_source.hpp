// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "amqp_header.hpp"
#include "amqp_properties.hpp"
#include "amqp_value.hpp"

struct SOURCE_INSTANCE_TAG;
namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {

  class MessageSource {
  public:
    MessageSource();
    ~MessageSource();

    MessageSource(MessageSource const&);
    MessageSource& operator=(MessageSource const&);
    MessageSource(MessageSource&&) noexcept;
    MessageSource& operator=(MessageSource&&) noexcept;

    MessageSource(SOURCE_INSTANCE_TAG* message);
    // Create a described source from an AMQP Value - used in the OnLinkAttached.
    MessageSource(Azure::Core::Amqp::Models::AmqpValue const& value);

    MessageSource(std::string const& value);
    MessageSource(char const* value);

    operator SOURCE_INSTANCE_TAG*() const { return m_source; }
    operator const Azure::Core::Amqp::Models::AmqpValue() const;

    Azure::Core::Amqp::Models::AmqpValue GetAddress() const;
    void SetAddress(Azure::Core::Amqp::Models::AmqpValue const& address);

    Azure::Core::Amqp::Models::TerminusDurability GetTerminusDurability() const;
    void SetTerminusDurability(Azure::Core::Amqp::Models::TerminusDurability terminusDurability);

    Azure::Core::Amqp::Models::TerminusExpiryPolicy GetExpiryPolicy() const;
    void SetExpiryPolicy(Azure::Core::Amqp::Models::TerminusExpiryPolicy expiryPolicy);

    std::chrono::system_clock::time_point GetTimeout() const;
    void SetTimeout(std::chrono::system_clock::time_point const& timeout);

    bool GetDynamic() const;
    void SetDynamic(bool dynamic);

    Azure::Core::Amqp::Models::AmqpMap GetDynamicNodeProperties() const;
    void SetDynamicNodeProperties(Azure::Core::Amqp::Models::AmqpMap const& dynamicNodeProperties);

    std::string GetDistributionMode() const;
    void SetDistributionMode(std::string const& distributionMode);

    Azure::Core::Amqp::Models::AmqpMap GetFilter() const;
    void SetFilter(Azure::Core::Amqp::Models::AmqpMap const& filter);

    Azure::Core::Amqp::Models::AmqpValue GetDefaultOutcome() const;
    void SetDefaultOutcome(Azure::Core::Amqp::Models::AmqpValue const& defaultOutcome);

    Azure::Core::Amqp::Models::AmqpArray GetOutcomes() const;
    void SetOutcomes(Azure::Core::Amqp::Models::AmqpValue const& outcomes);
    void SetOutcomes(Azure::Core::Amqp::Models::AmqpArray const& outcomes);

    Azure::Core::Amqp::Models::AmqpArray GetCapabilities() const;
    void SetCapabilities(Azure::Core::Amqp::Models::AmqpValue const& capabilities);
    void SetCapabilities(Azure::Core::Amqp::Models::AmqpArray const& capabilities);

    friend std::ostream& operator<<(std::ostream&, MessageSource const&);

  private:
    SOURCE_INSTANCE_TAG* m_source;
  };
}}}}} // namespace Azure::Core::Amqp::Models::_internal
