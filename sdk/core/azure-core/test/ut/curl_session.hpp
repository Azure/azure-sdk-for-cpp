// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief The base class for testing a curl session.
 *
 * @remark The curl connection mock is defined here.
 *
 */

#ifdef _MSC_VER
#pragma warning(push)
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) // !_MSC_VER
#pragma GCC diagnostic push
#elif defined(__clang__) // !_MSC_VER !__clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif // _MSC_VER

#include <azure/core/http/curl/curl.hpp>
#include <curl/curl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <string>

#include <http/curl/curl_connection_pool_private.hpp>

namespace Azure { namespace Core { namespace Test {

  class CurlSession : public ::testing::Test {
  };

  /**
   * @brief mock the network connection
   *
   */
  class MockCurlNetworkConnection : public Azure::Core::Http::CurlNetworkConnection {
  public:
    MOCK_METHOD(std::string const&, GetConnectionKey, (), (const, override));
    MOCK_METHOD(void, updateLastUsageTime, (), (override));
    MOCK_METHOD(bool, isExpired, (), (override));
    MOCK_METHOD(
        int64_t,
        ReadFromSocket,
        (Context const& context, uint8_t* buffer, int64_t bufferSize),
        (override));
    MOCK_METHOD(
        CURLcode,
        SendBuffer,
        (Context const& context, uint8_t const* buffer, size_t bufferSize),
        (override));

    /* This is a way to test we are calling the destructor
     *  Adding an extra mock method that is called from the destructor
     */
    MOCK_METHOD(void, DestructObj, ());
    virtual ~MockCurlNetworkConnection() { DestructObj(); }
  };

}}} // namespace Azure::Core::Test

#ifdef _MSC_VER
#pragma warning(pop)
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) // !_MSC_VER
#pragma GCC diagnostic pop
#elif defined(__clang__) // !_MSC_VER !__clang__
#pragma clang diagnostic pop // NOLINT(clang-diagnostic-unknown-pragmas)
#endif // _MSC_VER
