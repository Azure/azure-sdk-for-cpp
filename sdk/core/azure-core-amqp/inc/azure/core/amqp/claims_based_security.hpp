// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "azure/core/amqp/session.hpp"
#include <azure/core/context.hpp>

namespace Azure { namespace Core { namespace _internal { namespace Amqp {
  namespace _detail {
    class CbsImpl;
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

  class ClaimBasedSecurity final {
  public:
    ClaimBasedSecurity(
        Azure::Core::_internal::Amqp::Session const& session,
        Azure::Core::_internal::Amqp::Connection const& connectionToPoll);
    virtual ~ClaimBasedSecurity() noexcept;

    ClaimBasedSecurity(ClaimBasedSecurity const&) = default;
    ClaimBasedSecurity& operator=(ClaimBasedSecurity const&) = default;
    ClaimBasedSecurity(ClaimBasedSecurity&&) noexcept = default;
    ClaimBasedSecurity& operator=(ClaimBasedSecurity&&) noexcept = default;

    CbsOpenResult Open(Azure::Core::Context = {});
    void Close();
    std::tuple<CbsOperationResult, uint32_t, std::string> PutToken(
        CbsTokenType type,
        std::string const& audience,
        std::string const& token,
        Azure::Core::Context = {});
    std::tuple<CbsOperationResult, uint32_t, std::string> DeleteToken(
        CbsTokenType type,
        std::string const& audience,
        std::string const& token,
        Azure::Core::Context = {});
    void SetTrace(bool traceEnabled);

  private:
    std::shared_ptr<_detail::CbsImpl> m_impl;
  };
}}}} // namespace Azure::Core::_internal::Amqp