// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "azure/core/amqp/session.hpp"
#include <azure/core/context.hpp>

namespace Azure { namespace Core { namespace _internal { namespace Amqp {
  namespace _detail {
    class ClaimsBasedSecurityImpl;
  }

  enum class CbsOperationResult
  {
    Invalid,
    Ok,
    Error,
    Failed,
    InstanceClosed
  };
  enum class CbsOpenResult
  {
    Invalid,
    Ok,
    Error,
    Cancelled,
  };

  enum class CbsTokenType
  {
    Invalid,
    Sas,
    Jwt
  };

  /** @brief Implementation of AMQP 1.0 Claims-based Security (CBS) protocol.
   *
   * This class allows AMQP clients to implement the CBS protocol for authentication and
   * authorization: https://docs.oasis-open.org/amqp/amqp-cbs/v1.0/csd01/amqp-cbs-v1.0-csd01.html
   *
   * The ServiceBus and EventHubs services use this protocol to authenticate and authorize clients.
   * See [ServiceBus Claims-based
   * authorization](https://learn.microsoft.com/en-us/azure/service-bus-messaging/service-bus-amqp-protocol-guide#claims-based-authorization)
   * for more information about how the CBS protocol is implemented.
   */
  class ClaimsBasedSecurity final {
  public:
    /** @brief Construct a new instance of a ClaimsBasedSecurity client.
     *
     * @param session - Session on which to authenticate the client.
     * @param connectionToPoll - connection on which the session was opened. Used primarily to pump
     * AMQP messages.
     *
     * @remarks Note that this method takes a reference to the session and connectionToPoll objects
     * so it is critical that the lifetime of the ClaimsBasedSecurity object be scoped shorter than
     * the lifetime of the session and connectionToPoll object.
     */
    ClaimsBasedSecurity(
        Azure::Core::_internal::Amqp::Session const& session,
        Azure::Core::_internal::Amqp::Connection const& connectionToPoll);
    ~ClaimsBasedSecurity() noexcept;

    ClaimsBasedSecurity(ClaimsBasedSecurity const&) = default;
    ClaimsBasedSecurity& operator=(ClaimsBasedSecurity const&) = default;
    ClaimsBasedSecurity(ClaimsBasedSecurity&&) noexcept = default;
    ClaimsBasedSecurity& operator=(ClaimsBasedSecurity&&) noexcept = default;

    CbsOpenResult Open(Azure::Core::Context = {});
    void Close();
    std::tuple<CbsOperationResult, uint32_t, std::string> PutToken(
        CbsTokenType type,
        std::string const& audience,
        std::string const& token,
        Azure::Core::Context = {});
    void SetTrace(bool traceEnabled);

  private:
    std::shared_ptr<_detail::ClaimsBasedSecurityImpl> m_impl;
  };
}}}} // namespace Azure::Core::_internal::Amqp