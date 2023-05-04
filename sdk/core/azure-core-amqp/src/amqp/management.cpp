// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/management.hpp"
#include "azure/core/amqp/connection.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "private/connection_impl.hpp"
#include "private/management_impl.hpp"
#include "azure/core/amqp/session.hpp"
#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/internal/unique_handle.hpp>
#include <azure_uamqp_c/amqp_management.h>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>

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
   * @param managementNodeName - the name of the message source and target.
   * @param options - additional options for the Management object.
   * @param managementEvents - events associated with the management object.
   */
  Management::Management(
      Session const& session,
      std::string const& managementNodeName,
      ManagementOptions const& options,
      ManagementEvents* managementEvents)
      : m_impl{std::make_shared<_detail::ManagementImpl>(
          session.GetImpl(),
          managementNodeName,
          options,
          managementEvents)}
  {
  }

  Management::operator bool() const { return static_cast<bool>(m_impl); }

  /**
   * @brief Open the management instance.
   *
   * @returns A tuple consisting of the status code for the open and the description of the
   * status.
   */
  ManagementOpenResult Management::Open(Azure::Core::Context const& context)
  {
    return m_impl->Open(context);
  }

  /**
   * @brief Close the management instance.
   */
  void Management::Close() { m_impl->Close(); }

  std::tuple<_internal::ManagementOperationResult, std::uint32_t, std::string, Models::AmqpMessage>
  Management::ExecuteOperation(
      std::string const& operationToPerform,
      std::string const& typeOfOperation,
      std::string const& locales,
      Azure::Core::Amqp::Models::AmqpMessage const& messageToSend,
      Azure::Core::Context context)
  {
    return m_impl->ExecuteOperation(
        operationToPerform, typeOfOperation, locales, messageToSend, context);
  }
}}}} // namespace Azure::Core::Amqp::_internal

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  ManagementImpl::ManagementImpl(
      std::shared_ptr<SessionImpl> session,
      std::string const& managementNodeName,
      Azure::Core::Amqp::_internal::ManagementOptions const& options,
      Azure::Core::Amqp::_internal::ManagementEvents* managementEvents)
      : m_management{amqp_management_create(*session, managementNodeName.c_str())},
        m_options{options}, m_session{session}, m_eventHandler{managementEvents}
  {
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

  ManagementImpl::operator bool() const { return static_cast<bool>(m_management); }

  _internal::ManagementOpenResult ManagementImpl::Open(Azure::Core::Context const& context)
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
    auto result
        = m_openCompleteQueue.WaitForPolledResult(context, *m_session->GetConnectionToPoll());
    if (result)
    {
      switch (std::get<0>(*result))
      {
        case AMQP_MANAGEMENT_OPEN_OK:
          return _internal::ManagementOpenResult::Ok;
        case AMQP_MANAGEMENT_OPEN_ERROR: // LCOV_EXCL_LINE
          return _internal::ManagementOpenResult::Error; // LCOV_EXCL_LINE
        case AMQP_MANAGEMENT_OPEN_CANCELLED: // LCOV_EXCL_LINE
          return _internal::ManagementOpenResult::Cancelled; // LCOV_EXCL_LINE
        default: // LCOV_EXCL_LINE
          throw std::runtime_error("Unknown management open result."); // LCOV_EXCL_LINE
      }
    }
    throw Azure::Core::OperationCancelledException("Management Open operation was cancelled.");
  }

  std::tuple<_internal::ManagementOperationResult, std::uint32_t, std::string, Models::AmqpMessage>
  ManagementImpl::ExecuteOperation(
      std::string const& operationToPerform,
      std::string const& typeOfOperation,
      std::string const& locales,
      Azure::Core::Amqp::Models::AmqpMessage const& messageToSend,
      Azure::Core::Context context)
  {
    if (!amqp_management_execute_operation_async(
            m_management.get(),
            operationToPerform.c_str(),
            typeOfOperation.c_str(),
            locales.c_str(),
            Models::_internal::AmqpMessageFactory::ToUamqp(messageToSend).get(),
            ManagementImpl::OnExecuteOperationCompleteFn,
            this))
    {
      throw std::runtime_error("Could not execute operation."); // LCOV_EXCL_LINE
    }

    auto result = m_messageQueue.WaitForPolledResult(context, *m_session->GetConnectionToPoll());
    if (result)
    {
      return std::move(*result);
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
    management->m_openCompleteQueue.CompleteOperation(openResult);
  }

  void ManagementImpl::OnManagementErrorFn(void* context)
  {
    Azure::Core::Diagnostics::_internal::Log::Write(
        Azure::Core::Diagnostics::Logger::Level::Error, "Error processing management operation.");
    ManagementImpl* management = static_cast<ManagementImpl*>(context);
    if (management->m_eventHandler)
    {
      management->m_eventHandler->OnError();
    }
    management->m_messageQueue.CompleteOperation(
        _internal::ManagementOperationResult::Error,
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
            _internal::ManagementOperationResult::Ok,
            statusCode,
            (statusDescription ? statusDescription : std::string()),
            Models::_internal::AmqpMessageFactory::FromUamqp(message));
        break;
      case AMQP_MANAGEMENT_EXECUTE_OPERATION_ERROR:
        management->m_messageQueue.CompleteOperation(
            _internal::ManagementOperationResult::Error,
            statusCode,
            (statusDescription ? statusDescription : std::string()),
            Models::_internal::AmqpMessageFactory::FromUamqp(message));
        break;
      case AMQP_MANAGEMENT_EXECUTE_OPERATION_FAILED_BAD_STATUS:
        management->m_messageQueue.CompleteOperation(
            _internal::ManagementOperationResult::FailedBadStatus,
            statusCode,
            (statusDescription ? statusDescription : std::string()),
            Models::_internal::AmqpMessageFactory::FromUamqp(message));
        break;
      case AMQP_MANAGEMENT_EXECUTE_OPERATION_INSTANCE_CLOSED:
        management->m_messageQueue.CompleteOperation(
            _internal::ManagementOperationResult::InstanceClosed,
            statusCode,
            (statusDescription ? statusDescription : std::string()),
            Models::_internal::AmqpMessageFactory::FromUamqp(message));
        break;
      default: // LCOV_EXCL_LINE
        throw std::runtime_error("Unknown management status."); // LCOV_EXCL_LINE
    }
  }

}}}} // namespace Azure::Core::Amqp::_detail
