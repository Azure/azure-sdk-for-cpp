// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words amqpconnection amqpconnectionoptions amqpconnectionoptionsbuilder

#include "azure/core/amqp/internal/connection.hpp"

#include "../../../models/private/value_impl.hpp"
#include "azure/core/amqp/internal/common/global_state.hpp"
#include "azure/core/amqp/internal/network/socket_transport.hpp"
#include "azure/core/amqp/internal/network/tls_transport.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"
#include "private/claims_based_security_impl.hpp"
#include "private/connection_impl.hpp"
#include "private/session_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/uuid.hpp>

using namespace Azure::Core::Amqp::_detail::RustInterop;

#include <memory>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  void UniqueHandleHelper<AmqpConnectionImplementation>::FreeAmqpConnection(
      AmqpConnectionImplementation* value)
  {
    amqpconnection_destroy(value);
  }
  void UniqueHandleHelper<AmqpConnectionOptionsImplementation>::FreeAmqpConnectionOptions(
      AmqpConnectionOptionsImplementation* value)
  {
    amqpconnectionoptions_destroy(value);
  }

  void
  UniqueHandleHelper<AmqpConnectionOptionsBuilderImplementation>::FreeAmqpConnectionOptionsBuilder(
      AmqpConnectionOptionsBuilderImplementation* value)
  {
    amqpconnectionoptionsbuilder_destroy(value);
  }
}}}} // namespace Azure::Core::Amqp::_detail

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace Amqp { namespace _internal {
}}}} // namespace Azure::Core::Amqp::_internal

namespace {
void EnsureGlobalStateInitialized()
{
  // Force the global instance to exist. This is required to ensure that uAMQP and
  // azure-c-shared-utility is
  auto globalInstance
      = Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance();
  (void)globalInstance;
}
} // namespace

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  // Create a connection with a request URI and options.
  ConnectionImpl::ConnectionImpl(
      std::string const& hostName,
      std::shared_ptr<Credentials::TokenCredential> credential,
      _internal::ConnectionOptions const& options)
      : m_connection{amqpconnection_create()}, m_options{options}, m_credential{credential}
  {
    EnsureGlobalStateInitialized();

    std::string connectionUrl;
    uint16_t port = options.Port;
    if (port == _internal::AmqpPort)
    {
      connectionUrl = "amqp://";
    }
    else if (port == _internal::AmqpTlsPort)
    {
      connectionUrl = "amqps://";
    }
    else
    {
      Log::Write(
          Logger::Level::Informational, "Unknown port specified, assuming non-TLS connection.");
      connectionUrl = "amqp://";
    }
    connectionUrl += hostName + ":" + std::to_string(port);
    m_hostUrl = Azure::Core::Url(connectionUrl);
  }

  ConnectionImpl::~ConnectionImpl()
  {
    std::unique_lock<LockType> lock(m_amqpMutex);
    if (m_openCount.load() != 0)
    {
      AZURE_ASSERT_MSG(m_openCount.load() == 0, "Connection is being destroyed while polling.");
      Azure::Core::_internal::AzureNoReturnPath("Connection is being destroyed while polling.");
    }
    if (m_connectionOpened)
    {
      AZURE_ASSERT_MSG(!m_connectionOpened, "Connection being destroyed while open.");
      Azure::Core::_internal::AzureNoReturnPath("Connection is being destroyed while open.");
    }
    m_isClosing = true;
    lock.unlock();
  }

  void ConnectionImpl::FinishConstruction()
  {
    std::string containerId{m_options.ContainerId};
    if (containerId.empty())
    {
      containerId = Azure::Core::Uuid::CreateUuid().ToString();
    }
    m_containerId = containerId;
    // Transfer the configuration options to the connection options builder.
    UniqueAmqpConnectionOptionsBuilder builder{amqpconnectionoptionsbuilder_create()};
    if (amqpconnectionoptionsbuilder_set_max_frame_size(builder.get(), m_options.MaxFrameSize))
    {
      throw std::runtime_error("Failed to set max frame size.");
    }
    if (amqpconnectionoptionsbuilder_set_channel_max(builder.get(), m_options.MaxChannelCount))
    {
      throw std::runtime_error("Failed to set max channel count.");
    }
    if (amqpconnectionoptionsbuilder_set_idle_timeout(
            builder.get(),
            static_cast<std::uint32_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(m_options.IdleTimeout)
                    .count())))
    {
      throw std::runtime_error("Failed to set idle timeout.");
    }
    if (!m_options.IncomingLocales.empty())
    {
      std::vector<const char*> locales;
      for (auto& locale : m_options.IncomingLocales)
      {
        locales.push_back(locale.c_str());
      }

      if (amqpconnectionoptionsbuilder_set_incoming_locales(
              builder.get(), locales.data(), locales.size()))
      {
        throw std::runtime_error("Failed to set incoming locales.");
      }
    }
    if (!m_options.OutgoingLocales.empty())
    {
      std::vector<const char*> locales;
      for (auto const& locale : m_options.OutgoingLocales)
      {
        locales.push_back(locale.c_str());
      }

      if (amqpconnectionoptionsbuilder_set_outgoing_locales(
              builder.get(), locales.data(), locales.size()))
      {
        throw std::runtime_error("Failed to set incoming locales.");
      }
    }
    if (!m_options.OfferedCapabilities.empty())
    {
      std::vector<char*> capabilities;
      for (auto& capability : m_options.OfferedCapabilities)
      {
        capabilities.push_back(const_cast<char*>(capability.c_str()));
      }

      if (amqpconnectionoptionsbuilder_set_offered_capabilities(
              builder.get(), capabilities.data(), capabilities.size()))
      {
        throw std::runtime_error("Failed to set incoming locales.");
      }
    }
    if (!m_options.DesiredCapabilities.empty())
    {
      std::vector<char*> capabilities;
      for (auto& capability : m_options.DesiredCapabilities)
      {
        capabilities.push_back(const_cast<char*>(capability.c_str()));
      }

      if (amqpconnectionoptionsbuilder_set_desired_capabilities(
              builder.get(), capabilities.data(), capabilities.size()))
      {
        throw std::runtime_error("Failed to set incoming locales.");
      }
    }

    if (!m_options.Properties.empty())
    {
      if (amqpconnectionoptionsbuilder_set_properties(
              builder.get(),
              Models::_detail::AmqpValueFactory::ToImplementation(
                  m_options.Properties.AsAmqpValue())))
      {
        throw std::runtime_error("Failed to set connection properties.");
      }
    }

    if (amqpconnectionoptionsbuilder_set_buffer_size(builder.get(), m_options.BufferSize))
    {
      throw std::runtime_error("Failed to set buffer size.");
    }

    m_connectionOptions.reset(amqpconnectionoptionsbuilder_build(builder.get()));
  }

  void ConnectionImpl::Open(Azure::Core::Context const& context)
  {
    Azure::Core::Amqp::Common::_detail::CallContext callContext{
        Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()
            ->GetRuntimeContext(),
        context};
    if (m_options.EnableTrace)
    {
      Log::Stream(Logger::Level::Verbose)
          << "ConnectionImpl::Open: " << this << " ID: " << m_containerId;
    }
    if (amqpconnection_open(
            callContext.GetCallContext(),
            m_connection.get(),
            m_hostUrl.GetAbsoluteUrl().c_str(),
            m_containerId.c_str(),
            m_connectionOptions.get()))
    {
      throw std::runtime_error("Could not open connection: " + callContext.GetError());
    }
  }
  void ConnectionImpl::Close(Azure::Core::Context const& context)
  {
    Azure::Core::Amqp::Common::_detail::CallContext callContext{
        Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()
            ->GetRuntimeContext(),
        context};
    Log::Stream(Logger::Level::Verbose)
        << "ConnectionImpl::Close: " << this << " ID: " << m_containerId;
    if (!m_connection)
    {
      throw std::logic_error("Connection not opened.");
    }
    if (amqpconnection_close(callContext.GetCallContext(), m_connection.get()))
    {
      throw std::runtime_error("Could not close connection: " + callContext.GetError());
    }
    m_connectionOpened = false;
  }

  void ConnectionImpl::Close(
      const std::string& condition,
      const std::string& description,
      Models::AmqpValue info,
      Azure::Core::Context const& context)
  {
    Azure::Core::Amqp::Common::_detail::CallContext callContext{
        Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()
            ->GetRuntimeContext(),
        context};
    Log::Stream(Logger::Level::Verbose)
        << "ConnectionImpl::Close: " << this << " ID: " << m_containerId;
    if (!m_connection)
    {
      throw std::logic_error("Connection not opened.");
    }
    if (amqpconnection_close_with_error(
            callContext.GetCallContext(),
            m_connection.get(),
            condition.c_str(),
            description.c_str(),
            Models::_detail::AmqpValueFactory::ToImplementation(info)))
    {
      throw std::runtime_error("Could not close connection: " + callContext.GetError());
    }
    m_connectionOpened = false;
  }

  uint32_t ConnectionImpl::GetMaxFrameSize() const
  {
    uint32_t maxSize = {};
    maxSize = amqpconnectionoptions_get_max_frame_size(m_connectionOptions.get());
    return maxSize;
  }

  uint16_t ConnectionImpl::GetMaxChannel() const
  {
    uint16_t maxChannel = {};
    maxChannel = amqpconnectionoptions_get_channel_max(m_connectionOptions.get());
    return maxChannel;
  }

  std::chrono::milliseconds ConnectionImpl::GetIdleTimeout() const
  {
    return std::chrono::milliseconds(
        amqpconnectionoptions_get_idle_timeout(m_connectionOptions.get()));
  }

  Models::AmqpMap ConnectionImpl::GetProperties() const
  {
    auto value = amqpconnectionoptions_get_properties(m_connectionOptions.get());
    return Models::_detail::AmqpValueFactory::FromImplementation(
               Models::_detail::UniqueAmqpValueHandle{value})
        .AsMap();
  }

}}}} // namespace Azure::Core::Amqp::_detail
