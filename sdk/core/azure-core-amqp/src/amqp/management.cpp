// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/management.hpp"

#include "azure/core/amqp/connection.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "azure/core/amqp/session.hpp"
#include "private/connection_impl.hpp"
#include "private/management_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/internal/unique_handle.hpp>

#include <azure_uamqp_c/amqp_management.h>

#include <iostream>
#include <memory>
#include <string>
#include <tuple>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

void Azure::Core::_internal::UniqueHandleHelper<AMQP_MANAGEMENT_INSTANCE_TAG>::FreeAmqpManagement(
    AMQP_MANAGEMENT_HANDLE value)
{
  amqp_management_destroy(value);
}

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  /**
   * @brief Open the management instance.
   *
   * @returns A tuple consisting of the status code for the open and the description of the
   * status.
   */
  ManagementOpenStatus ManagementClient::Open(Context const& context)
  {
    return m_impl->Open(context);
  }

  /**
   * @brief Close the management instance.
   */
  void ManagementClient::Close() { m_impl->Close(); }

  ManagementOperationResult ManagementClient::ExecuteOperation(
      std::string const& operationToPerform,
      std::string const& typeOfOperation,
      std::string const& locales,
      Models::AmqpMessage messageToSend,
      Context const& context)
  {
    return m_impl->ExecuteOperation(
        operationToPerform, typeOfOperation, locales, messageToSend, context);
  }
}}}} // namespace Azure::Core::Amqp::_internal

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  ManagementClientImpl::ManagementClientImpl(
      std::shared_ptr<SessionImpl> session,
      std::string const& managementEntityPath,
      Azure::Core::Amqp::_internal::ManagementClientOptions const& options,
      Azure::Core::Amqp::_internal::ManagementClientEvents* managementEvents)
      : m_options{options}, m_session{session}, m_eventHandler{managementEvents},
        m_managementEntityPath{managementEntityPath}
  {
  }
  ManagementClientImpl::~ManagementClientImpl() noexcept { m_eventHandler = nullptr; }

  void ManagementClientImpl::CreateManagementClient()
  {

    m_management.reset(amqp_management_create(*m_session, m_options.ManagementNodeName.c_str()));
    if (!m_management)
    {
      throw std::runtime_error("Could not create management object.");
    }
    if (m_options.EnableTrace)
    {
      amqp_management_set_trace(m_management.get(), m_options.EnableTrace);
    }
    if (!m_options.ExpectedStatusCodeKeyName.empty())
    {
      if (amqp_management_set_override_status_code_key_name(
              m_management.get(), m_options.ExpectedStatusCodeKeyName.c_str()))
      {
        throw std::runtime_error("Could not set override status code key name.");
      }
    }
    if (!m_options.ExpectedStatusDescriptionKeyName.empty())
    {
      if (amqp_management_set_override_status_description_key_name(
              m_management.get(), m_options.ExpectedStatusDescriptionKeyName.c_str()))
      {
        throw std::runtime_error("Could not set override description key name.");
      }
    }
  }

  _internal::ManagementOpenStatus ManagementClientImpl::Open(Context const& context)
  {
    /** Authentication needs to happen *before* the management object is created. */
    m_session->AuthenticateIfNeeded(
        m_managementEntityPath + "/" + m_options.ManagementNodeName, context);

    CreateManagementClient();

    if (amqp_management_open_async(
            m_management.get(),
            ManagementClientImpl::OnOpenCompleteFn,
            this,
            ManagementClientImpl::OnManagementErrorFn,
            this))
    {
      throw std::runtime_error("Could not open management object.");
    }
    auto result = m_openCompleteQueue.WaitForPolledResult(context, *m_session->GetConnection());
    if (result)
    {
      switch (std::get<0>(*result))
      {
        case AMQP_MANAGEMENT_OPEN_OK:
          return _internal::ManagementOpenStatus::Ok;
        case AMQP_MANAGEMENT_OPEN_ERROR: // LCOV_EXCL_LINE
          return _internal::ManagementOpenStatus::Error; // LCOV_EXCL_LINE
        case AMQP_MANAGEMENT_OPEN_CANCELLED: // LCOV_EXCL_LINE
          return _internal::ManagementOpenStatus::Cancelled; // LCOV_EXCL_LINE
        default: // LCOV_EXCL_LINE
          throw std::runtime_error("Unknown management open result."); // LCOV_EXCL_LINE
      }
    }
    throw Azure::Core::OperationCancelledException("Management Open operation was cancelled.");
  }

  _internal::ManagementOperationResult ManagementClientImpl::ExecuteOperation(
      std::string const& operationToPerform,
      std::string const& typeOfOperation,
      std::string const& locales,
      Models::AmqpMessage messageToSend,
      Context const& context)
  {
    auto token = m_session->GetConnection()->GetSecurityToken(m_managementNodeName, context);
    if (!token.empty())
    {
      //      messageToSend.ApplicationProperties["security_token"] = Models::AmqpValue{token};
    }
    if (!amqp_management_execute_operation_async(
            m_management.get(),
            operationToPerform.c_str(),
            typeOfOperation.c_str(),
            (locales.empty() ? nullptr : locales.c_str()),
            Models::_internal::AmqpMessageFactory::ToUamqp(messageToSend).get(),
            ManagementClientImpl::OnExecuteOperationCompleteFn,
            this))
    {
      throw std::runtime_error("Could not execute operation."); // LCOV_EXCL_LINE
    }

    auto result = m_messageQueue.WaitForPolledResult(context, *m_session->GetConnection());
    if (result)
    {
      _internal::ManagementOperationResult rv;
      rv.Status = std::get<0>(*result);
      rv.StatusCode = std::get<1>(*result);
      rv.Description = std::get<2>(*result);
      rv.Message = std::get<3>(*result);
      return rv;
    }
    else
    {
      throw Azure::Core::OperationCancelledException("Management operation cancelled.");
    }
  }

  void ManagementClientImpl::Close() { amqp_management_close(m_management.get()); }

  void ManagementClientImpl::OnOpenCompleteFn(void* context, AMQP_MANAGEMENT_OPEN_RESULT openResult)
  {
    ManagementClientImpl* management = static_cast<ManagementClientImpl*>(context);
    if (management->m_options.EnableTrace)
    {
      Log::Write(
          Logger::Level::Informational, "OnManagementOpenComplete: " + std::to_string(openResult));
    }
    management->m_openCompleteQueue.CompleteOperation(openResult);
  }

  void ManagementClientImpl::OnManagementErrorFn(void* context)
  {
    ManagementClientImpl* management = static_cast<ManagementClientImpl*>(context);
    if (management->m_options.EnableTrace)
    {
      Log::Write(Logger::Level::Error, "Error processing management operation.");
    }
    if (management->m_eventHandler)
    {
      management->m_eventHandler->OnError();
    }
    management->m_messageQueue.CompleteOperation(
        _internal::ManagementOperationStatus::Error,
        0,
        "Error processing management operation.",
        Models::AmqpMessage());
  }

  void ManagementClientImpl::OnExecuteOperationCompleteFn(
      void* context,
      AMQP_MANAGEMENT_EXECUTE_OPERATION_RESULT result,
      std::uint32_t statusCode,
      const char* statusDescription,
      MESSAGE_HANDLE message)
  {
    ManagementClientImpl* management = static_cast<ManagementClientImpl*>(context);
    switch (result)
    {
      case AMQP_MANAGEMENT_EXECUTE_OPERATION_OK:
        management->m_messageQueue.CompleteOperation(
            _internal::ManagementOperationStatus::Ok,
            statusCode,
            (statusDescription ? statusDescription : std::string()),
            Models::_internal::AmqpMessageFactory::FromUamqp(message));
        break;
      case AMQP_MANAGEMENT_EXECUTE_OPERATION_ERROR:
        management->m_messageQueue.CompleteOperation(
            _internal::ManagementOperationStatus::Error,
            statusCode,
            (statusDescription ? statusDescription : std::string()),
            Models::_internal::AmqpMessageFactory::FromUamqp(message));
        break;
      case AMQP_MANAGEMENT_EXECUTE_OPERATION_FAILED_BAD_STATUS:
        management->m_messageQueue.CompleteOperation(
            _internal::ManagementOperationStatus::FailedBadStatus,
            statusCode,
            (statusDescription ? statusDescription : std::string()),
            Models::_internal::AmqpMessageFactory::FromUamqp(message));
        break;
      case AMQP_MANAGEMENT_EXECUTE_OPERATION_INSTANCE_CLOSED:
        management->m_messageQueue.CompleteOperation(
            _internal::ManagementOperationStatus::InstanceClosed,
            statusCode,
            (statusDescription ? statusDescription : std::string()),
            Models::_internal::AmqpMessageFactory::FromUamqp(message));
        break;
      default: // LCOV_EXCL_LINE
        throw std::runtime_error("Unknown management status."); // LCOV_EXCL_LINE
    }
  }

}}}} // namespace Azure::Core::Amqp::_detail
