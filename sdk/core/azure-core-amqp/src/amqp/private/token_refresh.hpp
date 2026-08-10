// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/datetime.hpp>

#include <chrono>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  // The .NET client refreshes a CBS token seven minutes before the token
  // expires. See AmqpConnectionScope.cs in Azure.Messaging.EventHubs. Use the
  // same buffer here.
  constexpr std::chrono::minutes TokenRefreshBuffer{7};

  // The smallest time between two refresh passes. A service that issues tokens
  // with a lifetime shorter than the buffer makes every token due immediately.
  // This interval stops the refresh thread from spinning in that case.
  constexpr std::chrono::seconds MinimumTokenRefreshInterval{20};

  // The refresh thread does the early refresh. The cache only has to stop
  // itself from giving a caller a token that is about to die, so its margin is
  // small. A larger margin here would make the cache authenticate again on
  // every call for a token with a short lifetime.
  constexpr std::chrono::seconds MinimumTokenLifetimeToUse{30};

  // How long the refresh thread sleeps when no token is close to its expiry.
  constexpr std::chrono::minutes IdleTokenRefreshPoll{1};

  // The deadline for one refresh, which covers the credential call and the CBS
  // operation.
  constexpr std::chrono::seconds TokenRefreshOperationTimeout{60};

  // These functions compare in the Azure::DateTime domain on purpose. The cast
  // from Azure::DateTime to std::chrono::system_clock::time_point is explicit
  // and it throws when the value is outside the range of the system clock. A
  // credential can return any value in ExpiresOn, and a default constructed
  // ExpiresOn is year 1, so a cast here could throw on a caller thread or on
  // the refresh thread. The conversion in the other direction does not throw.

  // Return true while the cached token has enough life left to give to a
  // caller.
  inline bool IsCachedTokenUsable(
      Azure::Core::Credentials::AccessToken const& token,
      std::chrono::system_clock::time_point now)
  {
    return token.ExpiresOn > Azure::DateTime(now + MinimumTokenLifetimeToUse);
  }

  // Return true when the refresh thread must replace this token by the given
  // time. The refresh is due one buffer before the token expires, so the test
  // adds the buffer to the time instead of subtracting it from the expiry.
  // Adding avoids an underflow for a token that reports a very early expiry.
  inline bool IsTokenRefreshDue(
      Azure::Core::Credentials::AccessToken const& token,
      std::chrono::system_clock::time_point now)
  {
    return token.ExpiresOn <= Azure::DateTime(now + TokenRefreshBuffer);
  }

}}}} // namespace Azure::Core::Amqp::_detail
