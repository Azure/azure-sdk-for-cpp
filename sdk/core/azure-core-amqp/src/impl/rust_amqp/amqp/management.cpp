// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/management.hpp"

#include "../../../models/private/value_impl.hpp"
#include "azure/core/amqp/internal/models/messaging_values.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"
#include "private/connection_impl.hpp"
#include "private/management_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <tuple>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  using namespace RustInterop;

  void UniqueHandleHelper<RustAmqpManagement>::FreeManagement(RustAmqpManagement* value)
  {
    amqpmanagement_destroy(value);
  }
  ManagementClientImpl::ManagementClientImpl(
      std::shared_ptr<SessionImpl> session,
      std::string const& managementEntityPath,
      Azure::Core::Amqp::_internal::ManagementClientOptions const& options)
      : m_options{options}, m_session{session}, m_managementEntityPath{managementEntityPath}
  {
  }
  ManagementClientImpl::~ManagementClientImpl() noexcept
  {
    if (m_isOpen)
    {
      AZURE_ASSERT_MSG(!m_isOpen, "Management being destroyed while open.");
      Azure::Core::_internal::AzureNoReturnPath("Management is being destroyed while open.");
    }
  }

  _internal::ManagementOpenStatus ManagementClientImpl::Open(Context const& context)
  {
    if (m_isOpen)
    {
      throw std::runtime_error("Management object is already open.");
    }

    try
    {
      m_accessToken = m_session->GetConnection()->AuthenticateAudience(
          m_session, m_managementEntityPath + "/" + m_options.ManagementNodeName, context);

      Common::_detail::CallContext callContext(
          Common::_detail::GlobalStateHolder::GlobalStateInstance()->GetRuntimeContext(), context);

      RustAccessToken rustToken;
      rustToken.secret = m_accessToken.Token.c_str();
      rustToken.expires_on = std::chrono::duration_cast<std::chrono::seconds>(
                                 m_accessToken.ExpiresOn.time_since_epoch())
                                 .count();

      m_management.reset(amqpmanagement_create(
          callContext.GetCallContext(),
          m_session->GetAmqpSession().get(),
          m_managementEntityPath.c_str(),
          &rustToken));
      if (!m_management)
      {
        throw std::runtime_error("Could not create management object: " + callContext.GetError());
      }

      if (amqpmanagement_attach(callContext.GetCallContext(), m_management.get()))
      {
        throw std::runtime_error("Could not attach management object: " + callContext.GetError());
      }
      m_isOpen = true;
      return _internal::ManagementOpenStatus::Ok;
    }
    catch (...)
    {
      Log::Stream(Logger::Level::Warning) << "Exception thrown during management open.";
      throw;
    }
  }

  _internal::ManagementOperationResult ManagementClientImpl::ExecuteOperation(
      std::string const&,
      std::string const& typeOfOperation,
      std::string const&,
      Models::AmqpMessage messageToSend,
      Context const& context)
  {
    if (!m_management)
    {
      Log::Stream(Logger::Level::Error)
          << "Execute Operation called when management is not initialized.";
      throw std::runtime_error("Management is not open!");
    }
    Common::_detail::CallContext callContext(
        Common::_detail::GlobalStateHolder::GlobalStateInstance()->GetRuntimeContext(), context);

    Azure::Core::Amqp::Models::AmqpMap propertiesMap;
    for (const auto& val : messageToSend.ApplicationProperties)
    {
      propertiesMap.emplace(Models::AmqpValue{val.first}, val.second);
    }

    Azure::Core::Amqp::Models::AmqpValue applicationProperties = propertiesMap.AsAmqpValue();

    Models::_detail::UniqueAmqpValueHandle value{amqpmanagement_call(
        callContext.GetCallContext(),
        m_management.get(),
        typeOfOperation.c_str(),
        Models::_detail::AmqpValueFactory::ToImplementation(applicationProperties))};

    auto responseMessage = std::make_shared<Azure::Core::Amqp::Models::AmqpMessage>();
    responseMessage->SetBody(Models::_detail::AmqpValueFactory::FromImplementation(value));
    _internal::ManagementOperationResult result;
    result.Message = responseMessage;
    result.Status = _internal::ManagementOperationStatus::Ok;
    return result;
  }

  void ManagementClientImpl::Close(Context const& context)
  {
    Log::Stream(Logger::Level::Verbose) << "ManagementClient::Close" << std::endl;
    if (!m_isOpen)
    {
      throw std::runtime_error("Management object is not open.");
    }

    Common::_detail::CallContext callContext(
        Common::_detail::GlobalStateHolder::GlobalStateInstance()->GetRuntimeContext(), context);

    if (amqpmanagement_detach_and_release(callContext.GetCallContext(), m_management.release()))
    {
      throw std::runtime_error("Could not close management client: " + callContext.GetError());
    }
    m_isOpen = false;
    Log::Stream(Logger::Level::Verbose) << "ManagementClient::Close completed." << std::endl;
  }

}}}} // namespace Azure::Core::Amqp::_detail
