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
   * @brief Create a new Management object instance.
   *
   * @param session - the session on which to create the instance.
   * @param managementEntityPath - the name of the AMQP instance.
   * @param options - additional options for the Management object.
   * @param managementEvents - events associated with the management object.
   */
  Management::Management(
      Session const& session,
      std::string const& managementEntityPath,
      ManagementOptions const& options,
      ManagementEvents* managementEvents)
      : m_impl{std::make_shared<_detail::ManagementImpl>(
          _detail::SessionFactory::GetImpl(session),
          managementEntityPath,
          options,
          managementEvents)}
  {
  }

  /**
   * @brief Open the management instance.
   *
   * @returns A tuple consisting of the status code for the open and the description of the
   * status.
   */
  ManagementOpenStatus Management::Open(Context const& context) { return m_impl->Open(context); }

  /**
   * @brief Close the management instance.
   */
  void Management::Close() { m_impl->Close(); }

  ManagementOperationResult Management::ExecuteOperation(
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
  ManagementImpl::ManagementImpl(
      std::shared_ptr<SessionImpl> session,
      std::string const& managementEntityPath,
      Azure::Core::Amqp::_internal::ManagementOptions const& options,
      Azure::Core::Amqp::_internal::ManagementEvents* managementEvents)
      : m_options{options}, m_session{session}, m_eventHandler{managementEvents}
  {
    /** Authentication needs to happen *before* the management object is created. */
    m_session->AuthenticateIfNeeded(managementEntityPath + "/" + m_options.ManagementNodeName, {});

    m_management.reset(amqp_management_create(*session, m_options.ManagementNodeName.c_str()));
    if (options.EnableTrace)
    {
      amqp_management_set_trace(m_management.get(), options.EnableTrace);
    }
    if (!options.ExpectedStatusCodeKeyName.empty())
    {
      amqp_management_set_override_status_code_key_name(
          m_management.get(), options.ExpectedStatusCodeKeyName.c_str());
    }
    if (!options.ExpectedStatusDescriptionKeyName.empty())
    {
      amqp_management_set_override_status_description_key_name(
          m_management.get(), options.ExpectedStatusDescriptionKeyName.c_str());
    }
  }
  ManagementImpl::~ManagementImpl() noexcept { m_eventHandler = nullptr; }

  _internal::ManagementOpenStatus ManagementImpl::Open(Context const& context)
  {
    if (amqp_management_open_async(
            m_management.get(),
            ManagementImpl::OnOpenCompleteFn,
            this,
            ManagementImpl::OnManagementErrorFn,
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

  _internal::ManagementOperationResult ManagementImpl::ExecuteOperation(
      std::string const& operationToPerform,
      std::string const& typeOfOperation,
      std::string const& locales,
      Models::AmqpMessage messageToSend,
      Context const& context)
  {
    auto token = m_session->GetSecurityToken(m_managementNodeName);
    if (!token.empty())
    {
      messageToSend.ApplicationProperties["security_token"] = Models::AmqpValue{token};
    }
    if (!amqp_management_execute_operation_async(
            m_management.get(),
            operationToPerform.c_str(),
            typeOfOperation.c_str(),
            (locales.empty() ? nullptr : locales.c_str()),
            Models::_internal::AmqpMessageFactory::ToUamqp(messageToSend).get(),
            ManagementImpl::OnExecuteOperationCompleteFn,
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

  void ManagementImpl::Close() { amqp_management_close(m_management.get()); }

  void ManagementImpl::OnOpenCompleteFn(void* context, AMQP_MANAGEMENT_OPEN_RESULT openResult)
  {
    ManagementImpl* management = static_cast<ManagementImpl*>(context);
    if (management->m_options.EnableTrace)
    {
      Log::Write(
          Logger::Level::Informational, "OnManagementOpenComplete: " + std::to_string(openResult));
    }
    management->m_openCompleteQueue.CompleteOperation(openResult);
  }

  void ManagementImpl::OnManagementErrorFn(void* context)
  {
    ManagementImpl* management = static_cast<ManagementImpl*>(context);
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

  void ManagementImpl::OnExecuteOperationCompleteFn(
      void* context,
      AMQP_MANAGEMENT_EXECUTE_OPERATION_RESULT result,
      std::uint32_t statusCode,
      const char* statusDescription,
      MESSAGE_HANDLE message)
  {
    ManagementImpl* management = static_cast<ManagementImpl*>(context);
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
