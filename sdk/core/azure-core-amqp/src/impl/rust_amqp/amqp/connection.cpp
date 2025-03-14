// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words amqpconnection amqpconnectionoptions amqpconnectionoptionsbuilder

#include "azure/core/amqp/internal/connection.hpp"

#include "../../../models/private/value_impl.hpp"
#include "azure/core/amqp/internal/common/global_state.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"
#include "private/claims_based_security_impl.hpp"
#include "private/connection_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/uuid.hpp>

using namespace Azure::Core::Amqp::RustInterop::_detail;

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
      std::shared_ptr<const Credentials::TokenCredential> credential,
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
    UniqueAmqpConnectionOptions connectionOptions{amqpconnectionoptions_create()};
    Common::_detail::InvokeAmqpApi(
        amqpconnectionoptions_set_max_frame_size, connectionOptions, m_options.MaxFrameSize);

    Common::_detail::InvokeAmqpApi(
        amqpconnectionoptions_set_channel_max, connectionOptions, m_options.MaxChannelCount);

    Common::_detail::InvokeAmqpApi(
        amqpconnectionoptions_set_idle_timeout,
        connectionOptions,
        static_cast<std::uint32_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(m_options.IdleTimeout).count()));

    if (!m_options.IncomingLocales.empty())
    {
      std::vector<const char*> locales;
      for (auto& locale : m_options.IncomingLocales)
      {
        locales.push_back(locale.c_str());
      }

      Common::_detail::InvokeAmqpApi(
          amqpconnectionoptions_set_incoming_locales,
          connectionOptions,
          locales.data(),
          locales.size());
    }
    if (!m_options.OutgoingLocales.empty())
    {
      std::vector<const char*> locales;
      for (auto const& locale : m_options.OutgoingLocales)
      {
        locales.push_back(locale.c_str());
      }

      Common::_detail::InvokeAmqpApi(
          amqpconnectionoptions_set_outgoing_locales,
          connectionOptions,
          locales.data(),
          locales.size());
    }
    if (!m_options.OfferedCapabilities.empty())
    {
      std::vector<char*> capabilities;
      for (auto& capability : m_options.OfferedCapabilities)
      {
        capabilities.push_back(const_cast<char*>(capability.c_str()));
      }

      Common::_detail::InvokeAmqpApi(
          amqpconnectionoptions_set_offered_capabilities,
          connectionOptions,
          capabilities.data(),
          capabilities.size());
    }
    if (!m_options.DesiredCapabilities.empty())
    {
      std::vector<char*> capabilities;
      for (auto& capability : m_options.DesiredCapabilities)
      {
        capabilities.push_back(const_cast<char*>(capability.c_str()));
      }

      Common::_detail::InvokeAmqpApi(
          amqpconnectionoptions_set_desired_capabilities,
          connectionOptions,
          capabilities.data(),
          capabilities.size());
    }

    if (!m_options.Properties.empty())
    {
      Common::_detail::InvokeAmqpApi(
          amqpconnectionoptions_set_properties,
          connectionOptions,
          Models::_detail::AmqpValueFactory::ToImplementation(m_options.Properties.AsAmqpValue()));
    }

    if (m_options.BufferSize.HasValue())
    {
      Common::_detail::InvokeAmqpApi(
          amqpconnectionoptions_set_buffer_size, connectionOptions, m_options.BufferSize.Value());
    }
    m_connectionOptions = std::move(connectionOptions);
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
    m_connectionOpened = true;
  }
  void ConnectionImpl::Close(Azure::Core::Context const& context)
  {
    Azure::Core::Amqp::Common::_detail::CallContext callContext{
        Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()
            ->GetRuntimeContext(),
        context};
    Log::Stream(Logger::Level::Verbose)
        << "ConnectionImpl::Close: " << this << " ID: " << m_containerId;
    if (m_connection)
    {
      if (!IsOpen())
      {
        throw std::runtime_error("Cannot close an unopened connection.");
      }
      if (amqpconnection_close(callContext.GetCallContext(), m_connection.get()))
      {
        throw std::runtime_error("Could not close connection: " + callContext.GetError());
      }
      m_connectionOpened = false;
      m_connection.reset();
    }
    else
    {
      Log::Stream(Logger::Level::Informational) << "Closing an already closed connection.";
    }
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
