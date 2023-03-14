#pragma once

#include "azure/core/amqp/cancellable.hpp"
#include "azure/core/amqp/common/async_operation_queue.hpp"
#include "azure/core/amqp/session.hpp"

struct CBS_INSTANCE_TAG;
enum CBS_OPERATION_RESULT_TAG : int;
enum CBS_OPEN_COMPLETE_RESULT_TAG : int;

namespace Azure { namespace Core { namespace _internal { namespace Amqp {

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
    CBS_INSTANCE_TAG* m_cbs;

    Azure::Core::_internal::Amqp::Common::AsyncOperationQueue<CbsOpenResult> m_openResultQueue;
    Azure::Core::_internal::Amqp::Common::
        AsyncOperationQueue<CbsOperationResult, uint32_t, std::string>
            m_operationResultQueue;
    static void OnCbsOpenCompleteFn(void* context, CBS_OPEN_COMPLETE_RESULT_TAG openResult);
    static void OnCbsErrorFn(void* context);
    static void OnCbsOperationCompleteFn(
        void* context,
        CBS_OPERATION_RESULT_TAG operationResult,
        uint32_t statusCode,
        const char* statusDescription);

    Connection const& m_connectionToPoll;
  };
}}}} // namespace Azure::Core::_internal::Amqp