// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "amqp_header.hpp"
#include "amqp_properties.hpp"
#include "amqp_value.hpp"

struct SOURCE_INSTANCE_TAG;

template <> struct Azure::Core::_internal::UniqueHandleHelper<SOURCE_INSTANCE_TAG>
{
  static void FreeMessageSource(SOURCE_INSTANCE_TAG* obj);

  using type = Azure::Core::_internal::BasicUniqueHandle<SOURCE_INSTANCE_TAG, FreeMessageSource>;
};

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {

  class MessageSource final {
  public:
    MessageSource();
    ~MessageSource() = default;

    MessageSource(MessageSource const&);
    MessageSource& operator=(MessageSource const&);
    MessageSource(MessageSource&&) noexcept;
    MessageSource& operator=(MessageSource&&) noexcept;

    MessageSource(SOURCE_INSTANCE_TAG* message);
    // Create a described source from an AMQP Value - used in the OnLinkAttached.
    MessageSource(Azure::Core::Amqp::Models::AmqpValue const& value);

    MessageSource(std::string const& value);
    MessageSource(char const* value);

    operator SOURCE_INSTANCE_TAG*() const { return m_source.get(); }
    operator const Azure::Core::Amqp::Models::AmqpValue() const;

    Azure::Core::Amqp::Models::AmqpValue GetAddress() const;
    void SetAddress(Azure::Core::Amqp::Models::AmqpValue const& address);

    TerminusDurability GetTerminusDurability() const;
    void SetTerminusDurability(TerminusDurability terminusDurability);

    TerminusExpiryPolicy GetExpiryPolicy() const;
    void SetExpiryPolicy(TerminusExpiryPolicy expiryPolicy);

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

  private:
    Azure::Core::_internal::UniqueHandle<SOURCE_INSTANCE_TAG> m_source;
  };
  std::ostream& operator<<(std::ostream&, MessageSource const&);
}}}}} // namespace Azure::Core::Amqp::Models::_internal
