// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief makes sure azure/core.hpp can be included.
 *
 * @remark This file will catch any issue while trying to use/include the core.hpp header
 *
 */

#ifdef _MSC_VER
#pragma warning(push)
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) // !_MSC_VER
#pragma GCC diagnostic push
#elif defined(__clang__) // !_MSC_VER !__clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#endif // _MSC_VER

#include <azure/core.hpp>
#include <gtest/gtest.h>

#include <vector>

class DllExportTest final {
  AZ_CORE_DLLEXPORT static const bool DllExportHIncluded;
};

TEST(SimplifiedHeader, core)
{
  EXPECT_NO_THROW(Azure::Core::CaseInsensitiveMap imap);
  EXPECT_NO_THROW(Azure::Core::CaseInsensitiveSet iset); // cspell:disable-line
  EXPECT_NO_THROW(Azure::Core::Context c);
  EXPECT_NO_THROW(Azure::DateTime(2020, 11, 03, 15, 30, 44));
  EXPECT_NO_THROW(Azure::ETag e);
  EXPECT_NO_THROW(Azure::Core::Convert::Base64Decode("foo="));
  EXPECT_NO_THROW(Azure::Core::Cryptography::Md5Hash m);
  EXPECT_NO_THROW(Azure::Core::Http::RawResponse r(
      1, 1, Azure::Core::Http::HttpStatusCode::Accepted, "phrase"));
  EXPECT_NO_THROW({
    Azure::Core::Diagnostics::Logger::Level ll;
    (void)ll;
  });
  EXPECT_NO_THROW(Azure::MatchConditions mc);
  EXPECT_NO_THROW(Azure::ModifiedConditions mc);
  EXPECT_NO_THROW(Azure::Nullable<int> n);
  EXPECT_NO_THROW(Azure::Core::Uuid::CreateUuid());
  EXPECT_NO_THROW(Azure::Core::RequestFailedException("foo"));
  EXPECT_NO_THROW(Azure::Core::OperationStatus("foo"));
  EXPECT_NO_THROW(Azure::Core::Http::RawResponse(0, 0, Azure::Core::Http::HttpStatusCode::Ok, ""));
  EXPECT_NO_THROW(Azure::Core::Credentials::TokenCredentialOptions());

  {
    std::vector<uint8_t> buffer(10);
    EXPECT_NO_THROW(Azure::Core::IO::MemoryBodyStream mb(buffer));
  }
  EXPECT_NO_THROW(Azure::Core::Http::Policies::_internal::TelemetryPolicy tp("", ""));
  EXPECT_NO_THROW(Azure::Core::Websockets::WebsocketClient(Azure::Core::Url("ws://foo")));
}

#ifdef _MSC_VER
#pragma warning(pop)
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) // !_MSC_VER
#pragma GCC diagnostic pop
#elif defined(__clang__) // !_MSC_VER !__clang__
#pragma clang diagnostic pop // NOLINT(clang-diagnostic-unknown-pragmas)
#endif // _MSC_VER
