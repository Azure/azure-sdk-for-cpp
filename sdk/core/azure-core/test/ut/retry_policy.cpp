// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "http/retry_policy_private.hpp"

#include <gtest/gtest.h>

using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_detail;

TEST(RetryPolicy, Exponential)
{
  using namespace std::chrono_literals;

  RetryOptions const options{3, 1s, 2min, {}};

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::ShouldRetryOnTransportFailure(options, 1, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 1s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::ShouldRetryOnTransportFailure(options, 2, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 2s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::ShouldRetryOnTransportFailure(options, 3, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 4s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::ShouldRetryOnTransportFailure(options, 4, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, false);
  }
}

TEST(RetryPolicy, LessThan2Retries)
{
  using namespace std::chrono_literals;

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::ShouldRetryOnTransportFailure({1, 1s, 2min, {}}, 1, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 1s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::ShouldRetryOnTransportFailure({0, 1s, 2min, {}}, 1, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, false);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::ShouldRetryOnTransportFailure({-1, 1s, 2min, {}}, 1, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, false);
  }
}

TEST(RetryPolicy, NotExceedingMaxRetryDelay)
{
  using namespace std::chrono_literals;

  RetryOptions const options{7, 1s, 20s, {}};

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::ShouldRetryOnTransportFailure(options, 1, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 1s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::ShouldRetryOnTransportFailure(options, 2, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 2s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::ShouldRetryOnTransportFailure(options, 3, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 4s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::ShouldRetryOnTransportFailure(options, 4, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 8s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::ShouldRetryOnTransportFailure(options, 5, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 16s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::ShouldRetryOnTransportFailure(options, 6, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 20s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::ShouldRetryOnTransportFailure(options, 7, retryAfter, 1.0);

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
        = RetryLogic::ShouldRetryOnTransportFailure(options, 31, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 1073741824s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::ShouldRetryOnTransportFailure(options, 32, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 2147483647s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::ShouldRetryOnTransportFailure(options, 33, retryAfter, 1.0);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 2147483647s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::ShouldRetryOnTransportFailure(options, 34, retryAfter, 1.0);

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
    bool const shouldRetry = RetryLogic::ShouldRetryOnTransportFailure(options, 1, retryAfter, 0.8);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 8s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::ShouldRetryOnTransportFailure(options, 1, retryAfter, 1.3);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 13s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::ShouldRetryOnTransportFailure(options, 2, retryAfter, 0.8);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 16s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::ShouldRetryOnTransportFailure(options, 2, retryAfter, 1.3);

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
        = RetryLogic::ShouldRetryOnTransportFailure({3, 1ms, 2min, {}}, 1, retryAfter, 0.8);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 0ms);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::ShouldRetryOnTransportFailure({3, 2ms, 2min, {}}, 1, retryAfter, 0.8);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 1ms);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::ShouldRetryOnTransportFailure({3, 10s, 21s, {}}, 2, retryAfter, 1.3);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 21s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry
        = RetryLogic::ShouldRetryOnTransportFailure({3, 10s, 21s, {}}, 3, retryAfter, 1.3);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 21s);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::ShouldRetryOnTransportFailure(
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
    bool const shouldRetry = RetryLogic::ShouldRetryOnResponse(
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
    bool const shouldRetry = RetryLogic::ShouldRetryOnResponse(
        RawResponse(1, 1, HttpStatusCode::RequestTimeout, ""),
        {3, 654s, 3h, {HttpStatusCode::Ok}},
        1,
        retryAfter,
        1.0);

    EXPECT_EQ(shouldRetry, false);
  }

  {
    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::ShouldRetryOnResponse(
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
    bool const shouldRetry = RetryLogic::ShouldRetryOnResponse(
        response, {3, 1s, 2min, {HttpStatusCode::RequestTimeout}}, 1, retryAfter, 1.3);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 1234ms);
  }

  {
    RawResponse response(1, 1, HttpStatusCode::RequestTimeout, "");
    response.SetHeader("X-mS-ReTrY-aFtEr-MS", "5678");

    std::chrono::milliseconds retryAfter{};
    bool const shouldRetry = RetryLogic::ShouldRetryOnResponse(
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
    bool const shouldRetry = RetryLogic::ShouldRetryOnResponse(
        response, {3, 1s, 2min, {HttpStatusCode::RequestTimeout}}, 1, retryAfter, 1.1);

    EXPECT_EQ(shouldRetry, true);
    EXPECT_EQ(retryAfter, 90s);
  }
}
