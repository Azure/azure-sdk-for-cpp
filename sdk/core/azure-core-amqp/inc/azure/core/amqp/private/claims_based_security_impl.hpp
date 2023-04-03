// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once
#include "azure/core/amqp/claims_based_security.hpp"
#include <azure_uamqp_c/cbs.h>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  class ClaimsBasedSecurityImpl {

  public:
    ClaimsBasedSecurityImpl(
        Azure::Core::Amqp::_internal::Session const& session,
        Azure::Core::Amqp::_internal::Connection const& connectionToPoll);
    virtual ~ClaimsBasedSecurityImpl() noexcept;

    // Disable copy and move because the underlying m_cbs takes a reference to this object.
    ClaimsBasedSecurityImpl(ClaimsBasedSecurityImpl const&) = delete;
    ClaimsBasedSecurityImpl& operator=(ClaimsBasedSecurityImpl const&) = delete;
    ClaimsBasedSecurityImpl(ClaimsBasedSecurityImpl&&) noexcept = delete;
    ClaimsBasedSecurityImpl& operator=(ClaimsBasedSecurityImpl&&) noexcept = delete;

    CbsOpenResult Open(Azure::Core::Context context);
    void Close();
    std::tuple<CbsOperationResult, uint32_t, std::string> PutToken(
        CbsTokenType type,
        std::string const& audience,
        std::string const& token,
        Azure::Core::Context context);
    void SetTrace(bool traceEnabled);

  private:
    CBS_HANDLE m_cbs;

    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<CbsOpenResult> m_openResultQueue;
    Azure::Core::Amqp::Common::_internal::
        AsyncOperationQueue<CbsOperationResult, uint32_t, std::string>
            m_operationResultQueue;
    static void OnCbsOpenCompleteFn(void* context, CBS_OPEN_COMPLETE_RESULT openResult);
    static void OnCbsErrorFn(void* context);
    static void OnCbsOperationCompleteFn(
        void* context,
        CBS_OPERATION_RESULT operationResult,
        uint32_t statusCode,
        const char* statusDescription);

    _internal::Connection const& m_connectionToPoll;
  };
}}}} // namespace Azure::Core::Amqp::_detail
