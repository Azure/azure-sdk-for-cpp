// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/session.hpp"

#include <azure/core/context.hpp>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  class ClaimsBasedSecurityImpl;

  enum class CbsOperationResult
  {
    Invalid,
    Ok,
    Error,
    Failed,
    InstanceClosed,
    Cancelled,
  };
  std::ostream& operator<<(std::ostream& os, CbsOperationResult operationResult);

  enum class CbsOpenResult
  {
    Invalid,
    Ok,
    Error,
    Cancelled,
  };
  std::ostream& operator<<(std::ostream& os, CbsOpenResult operationResult);

  enum class CbsTokenType
  {
    Invalid,
    Sas,
    Jwt,
  };

#if defined(_azure_TESTING_BUILD)
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
     *
     */
    ClaimsBasedSecurity(Azure::Core::Amqp::_internal::Session const& session);
    ~ClaimsBasedSecurity() noexcept;

    ClaimsBasedSecurity(ClaimsBasedSecurity const&) = default;
    ClaimsBasedSecurity& operator=(ClaimsBasedSecurity const&) = default;
    ClaimsBasedSecurity(ClaimsBasedSecurity&&) noexcept = default;
    ClaimsBasedSecurity& operator=(ClaimsBasedSecurity&&) noexcept = default;

    CbsOpenResult Open(Context const& context = {});
    void Close(Context const& context = {});
    std::tuple<CbsOperationResult, uint32_t, std::string> PutToken(
        CbsTokenType type,
        std::string const& audience,
        std::string const& token,
        Context const& context = {});

  private:
    std::shared_ptr<ClaimsBasedSecurityImpl> m_impl;
  };
#endif // _azure_TESTING_BUILD
}}}} // namespace Azure::Core::Amqp::_detail
