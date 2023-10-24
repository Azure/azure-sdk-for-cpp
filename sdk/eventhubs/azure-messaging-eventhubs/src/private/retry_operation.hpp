// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include <azure/core/http/policies/policy.hpp>
#include <azure/messaging/eventhubs/eventhubs_exception.hpp>

#include <chrono>
#include <functional>

#if defined(azure_TESTING_BUILD_AMQP)
// Define the class used from tests to validate retry enabled
namespace Azure { namespace Messaging { namespace EventHubs { namespace _internal { namespace Test {
  class RetryOperationTest_ShouldRetryTrue1_Test;
  class RetryOperationTest_ShouldRetryTrue2_Test;
  class RetryOperationTest_ShouldRetryFalse1_Test;
  class RetryOperationTest_ShouldRetryFalse2_Test;
}}}}} // namespace Azure::Messaging::EventHubs::_internal::Test
#endif
namespace Azure { namespace Messaging { namespace EventHubs { namespace _detail {
  class RetryOperation {
#if defined(azure_TESTING_BUILD_AMQP)
    // make tests classes friends to validate set Retry
    friend class Azure::Messaging::EventHubs::_internal::Test::
        RetryOperationTest_ShouldRetryTrue1_Test;
    friend class Azure::Messaging::EventHubs::_internal::Test::
        RetryOperationTest_ShouldRetryTrue2_Test;
    friend class Azure::Messaging::EventHubs::_internal::Test::
        RetryOperationTest_ShouldRetryFalse1_Test;
    friend class Azure::Messaging::EventHubs::_internal::Test::
        RetryOperationTest_ShouldRetryFalse2_Test;
#endif
  protected:
    Azure::Core::Http::Policies::RetryOptions m_retryOptions;

    /**
     * @brief Calculate the exponential delay needed for this retry.
     *
     * @param attempt Which attempt is this?
     * @param jitterFactor Test hook removing the randomness from the delay algorithm.
     *
     * @returns Number of milliseconds to delay.
     *
     * @remarks This function calculates the exponential backoff needed for each retry, including a
     * jitter factor.
     */
    std::chrono::milliseconds CalculateExponentialDelay(int32_t attempt, double jitterFactor);

    bool WasLastAttempt(int32_t attempt) { return attempt >= m_retryOptions.MaxRetries; }

    bool ShouldRetry(
        bool response,
        int32_t attempt,
        std::chrono::milliseconds& retryAfter,
        double jitterFactor = -1);

    bool ShouldRetry(
        Azure::Messaging::EventHubs::EventHubsException const& exception,
        int32_t attempt,
        std::chrono::milliseconds& retryAfter,
        double jitterFactor = -1);

  public:
    explicit RetryOperation(Azure::Core::Http::Policies::RetryOptions& retryOptions)
        : m_retryOptions(std::move(retryOptions))
    {
    }

    RetryOperation(RetryOperation const& other) : m_retryOptions(other.m_retryOptions) {}

    RetryOperation& operator=(RetryOperation const& other)
    {
      if (this != &other)
      {
        m_retryOptions = other.m_retryOptions;
      }
      return *this;
    }

    bool Execute(std::function<bool()> operation);
  };
}}}} // namespace Azure::Messaging::EventHubs::_detail
