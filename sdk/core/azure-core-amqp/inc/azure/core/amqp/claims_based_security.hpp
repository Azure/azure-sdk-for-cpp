// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#pragma once

#include "azure/core/amqp/cancellable.hpp"
#include "azure/core/amqp/common/async_operation_queue.hpp"
#include "azure/core/amqp/session.hpp"

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

  class Cbs {

  public:
    Cbs(Azure::Core::_internal::Amqp::Session const& session,
        Azure::Core::_internal::Amqp::Connection const& connectionToPoll);
    virtual ~Cbs() noexcept;

    // Disable copy and move because the underlying m_cbs takes a reference to this object.
    Cbs(Cbs const&) = delete;
    Cbs& operator=(Cbs const&) = delete;
    Cbs(Cbs&&) noexcept = delete;
    Cbs& operator=(Cbs&&) noexcept = delete;

    CbsOpenResult Open();
    void Close();
    std::tuple<CbsOperationResult, uint32_t, std::string> PutToken(
        CbsTokenType type,
        std::string const& audience,
        std::string const& token);
    std::tuple<CbsOperationResult, uint32_t, std::string> DeleteToken(
        CbsTokenType type,
        std::string const& audience,
        std::string const& token);
    void SetTrace(bool traceEnabled);

  private:
    std::shared_ptr<_detail::CbsImpl> m_impl;
  };
}}}} // namespace Azure::Core::_internal::Amqp