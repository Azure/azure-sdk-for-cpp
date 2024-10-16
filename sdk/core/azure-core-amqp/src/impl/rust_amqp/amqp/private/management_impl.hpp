// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/management.hpp"
#include "rust_amqp_wrapper.h"
#include "session_impl.hpp"

#include <azure/core/credentials/credentials.hpp>

#include <memory>
#include <mutex>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  template <> struct UniqueHandleHelper<RustInterop::RustAmqpManagement>
  {
    static void FreeManagement(RustInterop::RustAmqpManagement* value);

    using type
        = Core::_internal::BasicUniqueHandle<RustInterop::RustAmqpManagement, FreeManagement>;
  };

  using UniqueAmqpManagement = UniqueHandle<RustInterop::RustAmqpManagement>;

  class ManagementClientFactory final {
  public:
    static Azure::Core::Amqp::_internal::ManagementClient CreateFromInternal(
        std::shared_ptr<ManagementClientImpl> clientImpl)
    {
      return Azure::Core::Amqp::_internal::ManagementClient(clientImpl);
    }

    static std::shared_ptr<ManagementClientImpl> GetImpl(
        Azure::Core::Amqp::_internal::ManagementClient const& client)
    {
      return client.m_impl;
    }
  };

  class ManagementClientImpl final : public ::std::enable_shared_from_this<ManagementClientImpl> {
  public:
    ManagementClientImpl(
        std::shared_ptr<SessionImpl> session,
        std::string const& managementEntityName,
        _internal::ManagementClientOptions const& options);

    ~ManagementClientImpl() noexcept;

    /**
     * @brief Open the management instance.
     *
     * @returns A tuple consisting of the status code for the open and the description of the
     * status.
     */
    _internal::ManagementOpenStatus Open(Context const& context = {});

    /**
     * @brief Close the management instance.
     */
    void Close(Context const& context);

    _internal::ManagementOperationResult ExecuteOperation(
        std::string const& operationToPerform,
        std::string const& typeOfOperation,
        std::string const& locales,
        Models::AmqpMessage messageToSend,
        Context const& context);

  private:
    bool m_isOpen{false};
    UniqueAmqpManagement m_management;
    _internal::ManagementClientOptions m_options;
    std::shared_ptr<_detail::SessionImpl> m_session;
    std::string m_managementEntityPath;

    Azure::Core::Credentials::AccessToken m_accessToken;
  };
}}}} // namespace Azure::Core::Amqp::_detail
