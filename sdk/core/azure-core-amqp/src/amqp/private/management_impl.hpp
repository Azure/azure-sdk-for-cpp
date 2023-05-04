// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "azure/core/amqp/connection.hpp"
#include "azure/core/amqp/management.hpp"
#include "azure/core/amqp/session.hpp"
#include "connection_impl.hpp"
#include "message_receiver_impl.hpp"
#include "session_impl.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure_uamqp_c/amqp_management.h>
#include <memory>
#include <vector>

template <> struct Azure::Core::_internal::UniqueHandleHelper<AMQP_MANAGEMENT_INSTANCE_TAG>
{
  static void FreeAmqpManagement(AMQP_MANAGEMENT_INSTANCE_TAG* obj);

  using type
      = Azure::Core::_internal::BasicUniqueHandle<AMQP_MANAGEMENT_INSTANCE_TAG, FreeAmqpManagement>;
};

using UniqueAmqpManagementHandle
    = Azure::Core::_internal::UniqueHandle<AMQP_MANAGEMENT_INSTANCE_TAG>;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  class ManagementImpl final : public ::std::enable_shared_from_this<ManagementImpl> {
  public:
    ManagementImpl(
        std::shared_ptr<SessionImpl> session,
        std::string const& managementNodeName,
        _internal::ManagementOptions const& options,
        _internal::ManagementEvents* managementEvents);

    ManagementImpl() = default;
    ~ManagementImpl() noexcept;
    operator bool() const;

    /**
     * @brief Open the management instance.
     *
     * @returns A tuple consisting of the status code for the open and the description of the
     * status.
     */
    _internal::ManagementOpenResult Open(Azure::Core::Context const& context = {});

    /**
     * @brief Close the management instance.
     */
    void Close();

    std::
        tuple<_internal::ManagementOperationResult, std::uint32_t, std::string, Models::AmqpMessage>
        ExecuteOperation(
            std::string const& operationToPerform,
            std::string const& typeOfOperation,
            std::string const& locales,
            Azure::Core::Amqp::Models::AmqpMessage const& messageToSend,
            Azure::Core::Context context);

  private:
    UniqueAmqpManagementHandle m_management{};
    _internal::ManagementOptions m_options;
    std::string m_source;
    std::shared_ptr<SessionImpl> m_session;
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<AMQP_MANAGEMENT_OPEN_RESULT>
        m_openCompleteQueue;

    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
        _internal::ManagementOperationResult,
        std::uint32_t,
        std::string,
        Models::AmqpMessage>
        m_messageQueue;

    _internal::ManagementEvents* m_eventHandler{};

    static void OnExecuteOperationCompleteFn(
        void* context,
        AMQP_MANAGEMENT_EXECUTE_OPERATION_RESULT executeResult,
        uint32_t statusCode,
        const char* statusDescription,
        MESSAGE_HANDLE messageHandle);
    static void OnManagementErrorFn(void* context);
    static void OnOpenCompleteFn(void* context, AMQP_MANAGEMENT_OPEN_RESULT openResult);
  };
}}}} // namespace Azure::Core::Amqp::_detail
