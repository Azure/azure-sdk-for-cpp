// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/diagnostics/logger.hpp"
#include "azure/core/http/policies/policy.hpp"
#include "azure/core/internal/http/pipeline.hpp"

#include <gtest/gtest.h>

#include <functional>

using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;

namespace {
class TestTransportPolicy final : public HttpPolicy {
private:
  std::function<std::unique_ptr<RawResponse>()> m_send;

public:
  TestTransportPolicy(std::function<std::unique_ptr<RawResponse>()> send) : m_send(send) {}

  std::unique_ptr<Azure::Core::Http::RawResponse> Send(
      Request&,
      NextHttpPolicy,
      Azure::Core::Context const&) const override
  {
    return m_send();
  }

  std::unique_ptr<HttpPolicy> Clone() const override
  {
    return std::make_unique<TestTransportPolicy>(*this);
  }
};

class RetryPolicyTest final : public RetryPolicy {
private:
  std::function<bool(RetryOptions const&, int32_t, std::chrono::milliseconds&, double)>
      m_shouldRetryOnTransportFailure;

  std::function<
      bool(RawResponse const&, RetryOptions const&, int32_t, std::chrono::milliseconds&, double)>
      m_shouldRetryOnResponse;

public:
  bool BaseShouldRetryOnTransportFailure(
      RetryOptions const& retryOptions,
      int32_t attempt,
      std::chrono::milliseconds& retryAfter,
      double jitterFactor) const
  {
    return RetryPolicy::ShouldRetryOnTransportFailure(
        retryOptions, attempt, retryAfter, jitterFactor);
  }

  bool BaseShouldRetryOnResponse(
      RawResponse const& response,
      RetryOptions const& retryOptions,
      int32_t attempt,
      std::chrono::milliseconds& retryAfter,
      double jitterFactor) const
  {
    return RetryPolicy::ShouldRetryOnResponse(
        response, retryOptions, attempt, retryAfter, jitterFactor);
  }

  RetryPolicyTest(
      RetryOptions const& retryOptions,
      decltype(m_shouldRetryOnTransportFailure) shouldRetryOnTransportFailure,
      decltype(m_shouldRetryOnResponse) shouldRetryOnResponse)
      : RetryPolicy(retryOptions),
        m_shouldRetryOnTransportFailure(
            shouldRetryOnTransportFailure != nullptr //
                ? shouldRetryOnTransportFailure
                : static_cast<decltype(m_shouldRetryOnTransportFailure)>( //
                    [this](auto options, auto attempt, auto retryAfter, auto jitter) {
                      retryAfter = std::chrono::milliseconds(0);
                      auto ignore = decltype(retryAfter)();
                      return this->BaseShouldRetryOnTransportFailure(
                          options, attempt, ignore, jitter);
                    })),
        m_shouldRetryOnResponse(
            shouldRetryOnResponse != nullptr //
                ? shouldRetryOnResponse
                : static_cast<decltype(m_shouldRetryOnResponse)>( //
                    [this](
                        RawResponse const& response,
                        auto options,
                        auto attempt,
                        auto retryAfter,
                        auto jitter) {
                      retryAfter = std::chrono::milliseconds(0);
                      auto ignore = decltype(retryAfter)();
                      return this->BaseShouldRetryOnResponse(
                          response, options, attempt, ignore, jitter);
                    }))
  {
  }

  std::unique_ptr<HttpPolicy> Clone() const override
  {
    return std::make_unique<RetryPolicyTest>(*this);
  }

protected:
  bool ShouldRetryOnTransportFailure(
      RetryOptions const& retryOptions,
      int32_t attempt,
      std::chrono::milliseconds& retryAfter,
      double jitterFactor) const override
  {
    return m_shouldRetryOnTransportFailure(retryOptions, attempt, retryAfter, jitterFactor);
  }

  bool ShouldRetryOnResponse(
      RawResponse const& response,
      RetryOptions const& retryOptions,
      int32_t attempt,
      std::chrono::milliseconds& retryAfter,
      double jitterFactor) const override
  {
    return m_shouldRetryOnResponse(response, retryOptions, attempt, retryAfter, jitterFactor);
  }
};
} // namespace

TEST(RetryPolicy, ShouldRetryOnResponse)
{
  using namespace std::chrono_literals;
  RetryOptions const retryOptions{5, 10s, 5min, {HttpStatusCode::Ok}};

  RawResponse const* responsePtrSent = nullptr;

  RawResponse const* responsePtrReceived = nullptr;
  RetryOptions retryOptionsReceived{0, 0ms, 0ms, {}};
  int32_t attemptReceived = -1234;
  double jitterReceived = -5678;

  int onTransportFailureInvoked = 0;
  int onResponseInvoked = 0;

  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<RetryPolicyTest>(
        retryOptions,
        [&](auto options, auto attempt, auto, auto jitter) {
          ++onTransportFailureInvoked;
          retryOptionsReceived = options;
          attemptReceived = attempt;
          jitterReceived = jitter;

          return false;
        },
        [&](RawResponse const& response, auto options, auto attempt, auto, auto jitter) {
          ++onResponseInvoked;
          responsePtrReceived = &response;
          retryOptionsReceived = options;
          attemptReceived = attempt;
          jitterReceived = jitter;

          return false;
        }));

    policies.emplace_back(std::make_unique<TestTransportPolicy>([&]() {
      auto response = std::make_unique<RawResponse>(1, 1, HttpStatusCode::Ok, "Test");

      responsePtrSent = response.get();

      return response;
    }));

    Azure::Core::Http::_internal::HttpPipeline pipeline(policies);

    Request request(HttpMethod::Get, Azure::Core::Url("https://www.microsoft.com"));
    pipeline.Send(request, Azure::Core::Context());
  }

  EXPECT_EQ(onTransportFailureInvoked, 0);
  EXPECT_EQ(onResponseInvoked, 1);

  EXPECT_NE(responsePtrSent, nullptr);
  EXPECT_EQ(responsePtrSent, responsePtrReceived);

  EXPECT_EQ(retryOptionsReceived.MaxRetries, retryOptions.MaxRetries);
  EXPECT_EQ(retryOptionsReceived.RetryDelay, retryOptions.RetryDelay);
  EXPECT_EQ(retryOptionsReceived.MaxRetryDelay, retryOptions.MaxRetryDelay);
  EXPECT_EQ(retryOptionsReceived.StatusCodes, retryOptions.StatusCodes);

  EXPECT_EQ(attemptReceived, 1);
  EXPECT_EQ(jitterReceived, -1);

  // 3 attempts
  responsePtrSent = nullptr;

  responsePtrReceived = nullptr;
  retryOptionsReceived = RetryOptions{0, 0ms, 0ms, {}};
  attemptReceived = -1234;
  jitterReceived = -5678;

  onTransportFailureInvoked = 0;
  onResponseInvoked = 0;

  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<RetryPolicyTest>(
        retryOptions,
        [&](auto options, auto attempt, auto, auto jitter) {
          ++onTransportFailureInvoked;
          retryOptionsReceived = options;
          attemptReceived = attempt;
          jitterReceived = jitter;

          return false;
        },
        [&](RawResponse const& response, auto options, auto attempt, auto retryAfter, auto jitter) {
          ++onResponseInvoked;
          responsePtrReceived = &response;
          retryOptionsReceived = options;
          attemptReceived = attempt;
          jitterReceived = jitter;

          retryAfter = 1ms;
          return onResponseInvoked < 3;
        }));

    policies.emplace_back(std::make_unique<TestTransportPolicy>([&]() {
      auto response = std::make_unique<RawResponse>(1, 1, HttpStatusCode::Ok, "Test");

      responsePtrSent = response.get();

      return response;
    }));

    Azure::Core::Http::_internal::HttpPipeline pipeline(policies);

    Request request(HttpMethod::Get, Azure::Core::Url("https://www.microsoft.com"));
    pipeline.Send(request, Azure::Core::Context());
  }

  EXPECT_EQ(onTransportFailureInvoked, 0);
  EXPECT_EQ(onResponseInvoked, 3);

  EXPECT_NE(responsePtrSent, nullptr);
  EXPECT_EQ(responsePtrSent, responsePtrReceived);

  EXPECT_EQ(retryOptionsReceived.MaxRetries, retryOptions.MaxRetries);
  EXPECT_EQ(retryOptionsReceived.RetryDelay, retryOptions.RetryDelay);
  EXPECT_EQ(retryOptionsReceived.MaxRetryDelay, retryOptions.MaxRetryDelay);
  EXPECT_EQ(retryOptionsReceived.StatusCodes, retryOptions.StatusCodes);

  EXPECT_EQ(attemptReceived, 3);
  EXPECT_EQ(jitterReceived, -1);
}

TEST(RetryPolicy, ShouldRetryOnTransportFailure)
{
  using namespace std::chrono_literals;
  RetryOptions const retryOptions{5, 10s, 5min, {HttpStatusCode::Ok}};

  RetryOptions retryOptionsReceived{0, 0ms, 0ms, {}};
  int32_t attemptReceived = -1234;
  double jitterReceived = -5678;

  int onTransportFailureInvoked = 0;
  int onResponseInvoked = 0;

  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<RetryPolicyTest>(
        retryOptions,
        [&](auto options, auto attempt, auto, auto jitter) {
          ++onTransportFailureInvoked;
          retryOptionsReceived = options;
          attemptReceived = attempt;
          jitterReceived = jitter;

          return false;
        },
        [&](auto, auto options, auto attempt, auto, auto jitter) {
          ++onResponseInvoked;
          retryOptionsReceived = options;
          attemptReceived = attempt;
          jitterReceived = jitter;

          return false;
        }));

    policies.emplace_back(std::make_unique<TestTransportPolicy>(
        []() -> std::unique_ptr<RawResponse> { throw TransportException("Test"); }));

    Azure::Core::Http::_internal::HttpPipeline pipeline(policies);

    Request request(HttpMethod::Get, Azure::Core::Url("https://www.microsoft.com"));
    EXPECT_THROW(pipeline.Send(request, Azure::Core::Context()), TransportException);
  }

  EXPECT_EQ(onTransportFailureInvoked, 1);
  EXPECT_EQ(onResponseInvoked, 0);

  EXPECT_EQ(retryOptionsReceived.MaxRetries, retryOptions.MaxRetries);
  EXPECT_EQ(retryOptionsReceived.RetryDelay, retryOptions.RetryDelay);
  EXPECT_EQ(retryOptionsReceived.MaxRetryDelay, retryOptions.MaxRetryDelay);
  EXPECT_EQ(retryOptionsReceived.StatusCodes, retryOptions.StatusCodes);

  EXPECT_EQ(attemptReceived, 1);
  EXPECT_EQ(jitterReceived, -1);

  // 3 attempts
  retryOptionsReceived = RetryOptions{0, 0ms, 0ms, {}};
  attemptReceived = -1234;
  jitterReceived = -5678;

  onTransportFailureInvoked = 0;
  onResponseInvoked = 0;

  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<RetryPolicyTest>(
        retryOptions,
        [&](auto options, auto attempt, auto, auto jitter) {
          ++onTransportFailureInvoked;
          retryOptionsReceived = options;
          attemptReceived = attempt;
          jitterReceived = jitter;

          return onTransportFailureInvoked < 3;
        },
        [&](auto, auto options, auto attempt, auto retryAfter, auto jitter) {
          ++onResponseInvoked;
          retryOptionsReceived = options;
          attemptReceived = attempt;
          jitterReceived = jitter;

          retryAfter = 1ms;
          return false;
        }));

    policies.emplace_back(std::make_unique<TestTransportPolicy>(
        []() -> std::unique_ptr<RawResponse> { throw TransportException("Test"); }));

    Azure::Core::Http::_internal::HttpPipeline pipeline(policies);

    Request request(HttpMethod::Get, Azure::Core::Url("https://www.microsoft.com"));
    EXPECT_THROW(pipeline.Send(request, Azure::Core::Context()), TransportException);
  }

  EXPECT_EQ(onTransportFailureInvoked, 3);
  EXPECT_EQ(onResponseInvoked, 0);

  EXPECT_EQ(retryOptionsReceived.MaxRetries, retryOptions.MaxRetries);
  EXPECT_EQ(retryOptionsReceived.RetryDelay, retryOptions.RetryDelay);
  EXPECT_EQ(retryOptionsReceived.MaxRetryDelay, retryOptions.MaxRetryDelay);
  EXPECT_EQ(retryOptionsReceived.StatusCodes, retryOptions.StatusCodes);

  EXPECT_EQ(attemptReceived, 3);
  EXPECT_EQ(jitterReceived, -1);
}

namespace {
class RetryLogic final : private RetryPolicy {
  RetryLogic() : RetryPolicy(RetryOptions()) {}
  ~RetryLogic() {}

  static RetryLogic const g_retryPolicy;

public:
  static bool TestShouldRetryOnTransportFailure(
      RetryOptions const& retryOptions,
      int32_t attempt,
      std::chrono::milliseconds& retryAfter,
      double jitterFactor)
  {
    return g_retryPolicy.ShouldRetryOnTransportFailure(
        retryOptions, attempt, retryAfter, jitterFactor);
  }

  static bool TestShouldRetryOnResponse(
      RawResponse const& response,
      RetryOptions const& retryOptions,
      int32_t attempt,
      std::chrono::milliseconds& retryAfter,
      double jitterFactor)
  {
    return g_retryPolicy.ShouldRetryOnResponse(
        response, retryOptions, attempt, retryAfter, jitterFactor);
  }
};

RetryLogic const RetryLogic::g_retryPolicy;
} // namespace

TEST(RetryPolicy, Exponential)
{
  using namespace std::chrono_literals;

  RetryOptions const options{3, 1s, 2min, {}};

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure(options, 1, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 1s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure(options, 2, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 2s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure(options, 3, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 4s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure(options, 4, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, false);
  }
}

TEST(RetryPolicy, LessThan2Retries)
{
  using namespace std::chrono_literals;

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure({1, 1s, 2min, {}}, 1, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 1s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure({0, 1s, 2min, {}}, 1, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, false);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure({-1, 1s, 2min, {}}, 1, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, false);
  }
}

TEST(RetryPolicy, NotExceedingMaxRetryDelay)
{
  using namespace std::chrono_literals;

  RetryOptions const options{7, 1s, 20s, {}};

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure(options, 1, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 1s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure(options, 2, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 2s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure(options, 3, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 4s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure(options, 4, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 8s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure(options, 5, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 16s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure(options, 6, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 20s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure(options, 7, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 20s);
  }
}

TEST(RetryPolicy, NotExceedingInt32Max)
{
  using namespace std::chrono_literals;

  RetryOptions const options{35, 1s, 9999999999999s, {}};

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure(options, 31, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 1073741824s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure(options, 32, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 2147483647s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure(options, 33, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 2147483647s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure(options, 34, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 2147483647s);
  }
}

TEST(RetryPolicy, Jitter)
{
  using namespace std::chrono_literals;

  RetryOptions const options{3, 10s, 20min, {}};

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure(options, 1, retryAfter, 0.8);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 8s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure(options, 1, retryAfter, 1.3);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 13s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure(options, 2, retryAfter, 0.8);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 16s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure(options, 2, retryAfter, 1.3);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 26s);
  }
}

TEST(RetryPolicy, JitterExtremes)
{
  using namespace std::chrono_literals;

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure({3, 1ms, 2min, {}}, 1, retryAfter, 0.8);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 0ms);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure({3, 2ms, 2min, {}}, 1, retryAfter, 0.8);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 1ms);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure({3, 10s, 21s, {}}, 2, retryAfter, 1.3);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 21s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::TestShouldRetryOnTransportFailure({3, 10s, 21s, {}}, 3, retryAfter, 1.3);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 21s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::TestShouldRetryOnTransportFailure(
        {35, 1s, 9999999999999s, {}}, 33, retryAfter, 1.3);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 2791728741100ms);
  }
}

TEST(RetryPolicy, HttpStatusCode)
{
  using namespace std::chrono_literals;

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::TestShouldRetryOnResponse(
        RawResponse(1, 1, HttpStatusCode::RequestTimeout, ""),
        {3, 3210s, 3h, {HttpStatusCode::RequestTimeout}},
        1,
        retryAfter,
        1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 3210s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::TestShouldRetryOnResponse(
        RawResponse(1, 1, HttpStatusCode::RequestTimeout, ""),
        {3, 654s, 3h, {HttpStatusCode::Ok}},
        1,
        retryAfter,
        1.0);

    EXPECT_EQ(shouldRetry, false);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::TestShouldRetryOnResponse(
        RawResponse(1, 1, HttpStatusCode::Ok, ""),
        {3, 987s, 3h, {HttpStatusCode::Ok}},
        1,
        retryAfter,
        1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 987s);
  }
}

TEST(RetryPolicy, RetryAfterMs)
{
  using namespace std::chrono_literals;

  {
    RawResponse response(1, 1, HttpStatusCode::RequestTimeout, "");
    response.SetHeader("rEtRy-aFtEr-mS", "1234");

    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::TestShouldRetryOnResponse(
        response, {3, 1s, 2min, {HttpStatusCode::RequestTimeout}}, 1, retryAfter, 1.3);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 1234ms);
  }

  {
    RawResponse response(1, 1, HttpStatusCode::RequestTimeout, "");
    response.SetHeader("X-mS-ReTrY-aFtEr-MS", "5678");

    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::TestShouldRetryOnResponse(
        response, {3, 1s, 2min, {HttpStatusCode::RequestTimeout}}, 1, retryAfter, 0.8);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 5678ms);
  }
}

TEST(RetryPolicy, RetryAfter)
{
  using namespace std::chrono_literals;

  {
    RawResponse response(1, 1, HttpStatusCode::RequestTimeout, "");
    response.SetHeader("rEtRy-aFtEr", "90");

    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::TestShouldRetryOnResponse(
        response, {3, 1s, 2min, {HttpStatusCode::RequestTimeout}}, 1, retryAfter, 1.1);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 90s);
  }
}

TEST(RetryPolicy, LogMessages)
{
  using Azure::Core::Diagnostics::Logger;

  struct Log
  {
    struct Entry
    {
      Logger::Level Level;
      std::string Message;
    };

    std::vector<Entry> Entries;

    Log()
    {
      Logger::SetLevel(Logger::Level::Informational);
      Logger::SetListener([&](auto lvl, auto msg) { Entries.emplace_back(Entry{lvl, msg}); });
    }

    ~Log()
    {
      Logger::SetListener(nullptr);
      Logger::SetLevel(Logger::Level::Warning);
    }

  } log;

  {
    using namespace std::chrono_literals;
    RetryOptions const retryOptions{5, 10s, 5min, {HttpStatusCode::InternalServerError}};

    auto requestNumber = 0;

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<RetryPolicyTest>(retryOptions, nullptr, nullptr));
    policies.emplace_back(std::make_unique<TestTransportPolicy>([&]() {
      ++requestNumber;

      if (requestNumber == 1)
      {
        throw TransportException("Cable Unplugged");
      }

      return std::make_unique<RawResponse>(
          1,
          1,
          requestNumber == 2 ? HttpStatusCode::InternalServerError
                             : HttpStatusCode::ServiceUnavailable,
          "Test");
    }));

    Azure::Core::Http::_internal::HttpPipeline pipeline(policies);

    Request request(HttpMethod::Get, Azure::Core::Url("https://www.microsoft.com"));
    pipeline.Send(request, Azure::Core::Context());
  }

  EXPECT_EQ(log.Entries.size(), 5);

  EXPECT_EQ(log.Entries[0].Level, Logger::Level::Warning);
  EXPECT_EQ(log.Entries[0].Message, "HTTP Transport error: Cable Unplugged");

  EXPECT_EQ(log.Entries[1].Level, Logger::Level::Informational);
  EXPECT_EQ(log.Entries[1].Message, "HTTP Retry attempt #1 will be made in 0ms.");

  EXPECT_EQ(log.Entries[2].Level, Logger::Level::Informational);
  EXPECT_EQ(log.Entries[2].Message, "HTTP status code 500 will be retried.");

  EXPECT_EQ(log.Entries[3].Level, Logger::Level::Informational);
  EXPECT_EQ(log.Entries[3].Message, "HTTP Retry attempt #2 will be made in 0ms.");

  EXPECT_EQ(log.Entries[4].Level, Logger::Level::Informational);
  EXPECT_EQ(log.Entries[4].Message, "HTTP status code 503 won't be retried.");
}
