// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/session.hpp"

#include <azure/core/context.hpp>

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

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

  /** @brief Thrown when the $cbs management client could not be opened.
   *
   * The three failures need different handling and cannot be told apart from the message text:
   * `Error` reached the transport and may be treated as transient, while `Cancelled` is the
   * caller's own cancellation or deadline and `Invalid` is a state error - retrying either is
   * wrong.
   *
   * Derives from `std::runtime_error` with the same message, so existing handlers keep working.
   */
  class CbsOpenFailedException final : public std::runtime_error {
  public:
    CbsOpenFailedException(CbsOpenResult result, std::string const& what)
        : std::runtime_error(what), Result(result)
    {
    }

    /** @brief The result reported by the failed open. */
    CbsOpenResult Result;
  };

#if ENABLE_UAMQP
  /** @brief Identifies a failed uAMQP CBS put-token operation. */
  class CbsPutTokenFailedException final : public std::runtime_error {
  public:
    CbsPutTokenFailedException(std::exception_ptr original, std::string const& what)
        : std::runtime_error(what), m_original{std::move(original)}
    {
    }

    std::exception_ptr GetOriginal() const { return m_original; }
    [[noreturn]] void RethrowOriginal() const { std::rethrow_exception(m_original); }

  private:
    std::exception_ptr m_original;
  };
#endif

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
   * authorization. It sends a `put-token` request to the `$cbs` node and reads the `status-code`
   * and the `status-description` fields of the reply.
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
        Azure::DateTime const& tokenExpirationTime,
        Context const& context);

  private:
    std::shared_ptr<ClaimsBasedSecurityImpl> m_impl;
  };
#endif // _azure_TESTING_BUILD
}}}} // namespace Azure::Core::Amqp::_detail
