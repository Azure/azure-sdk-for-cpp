// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/connection_string_credential.hpp"

#include <azure/core/platform.hpp>

#include <stdexcept>
#include <string>
#include <utility>

#include <gtest/gtest.h>

namespace Azure { namespace Core { namespace Amqp { namespace Tests {
  class ConnectionStringTest : public testing::Test {
  protected:
    void SetUp() override {}
    void TearDown() override {}
  };

  // EventHubs connection strings look like:
  // Endpoint=sb://{NAMESPACE}.servicebus.windows.net/{EVENT_HUB_NAME};EntityPath={EVENT_HUB_NAME};SharedAccessKeyName={ACCESS_KEY_NAME};SharedAccessKey={ACCESS_KEY}

  // cspell: disable
  constexpr const char* ConnectionStringWithLowercaseEntityPath
      = "Endpoint=sb://test.servicebus.windows.net/;SharedAccessKeyName=Key;"
        "SharedAccessKey=Value;EntityPath=myhub";
  // cspell: enable

  TEST_F(ConnectionStringTest, SaslPlainConnectionGood)
  {
    {
      std::string connectionString
          = "Endpoint=sb://{NAMESPACE}.servicebus.windows.net/"
            "{EVENT_HUB_NAME};EntityPath={EVENT_HUB_NAME};SharedAccessKeyName={ACCESS_KEY_NAME};"
            "SharedAccessKey={ACCESS_KEY}";
      Azure::Core::Amqp::_internal::ConnectionStringParser credential(connectionString);
      EXPECT_EQ(
          "sb://{NAMESPACE}.servicebus.windows.net/{EVENT_HUB_NAME}", credential.GetEndpoint());
      EXPECT_EQ("{EVENT_HUB_NAME}", credential.GetEntityPath());
      EXPECT_EQ("{ACCESS_KEY_NAME}", credential.GetSharedAccessKeyName());
      EXPECT_EQ("{ACCESS_KEY}", credential.GetSharedAccessKey());
    }
  }

  TEST_F(ConnectionStringTest, ServiceBusSasConnectionGood)
  {
    {
      std::string connectionString
          = "Endpoint=sb://{NAMESPACE}.servicebus.windows.net/"
            "{EVENT_HUB_NAME};EntityPath={EVENT_HUB_NAME};SharedAccessKeyName={ACCESS_KEY_NAME}=;"
            "SharedAccessKey={ACCESS_KEY}=";
      Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential credential(
          connectionString);
      EXPECT_EQ(
          "sb://{NAMESPACE}.servicebus.windows.net/{EVENT_HUB_NAME}", credential.GetEndpoint());
      EXPECT_EQ("{EVENT_HUB_NAME}", credential.GetEntityPath());
      EXPECT_EQ("{ACCESS_KEY_NAME}=", credential.GetSharedAccessKeyName());
      EXPECT_EQ("{ACCESS_KEY}=", credential.GetSharedAccessKey());
      {
#if !defined(AZ_PLATFORM_MAC)
#if ENABLE_UAMQP
        auto xport = credential.GetTransport();
        (void)xport;
#endif
#endif // !defined(AZ_PLATFORM_MAC)

        // Generate a SAS token which expires in 60 seconds.
        Azure::Core::Credentials::TokenRequestContext trc;
        auto token = credential.GetToken(trc, {});
      }
    }
    EXPECT_NO_THROW([]() {
      Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential zz(
          "Endpoint=Bar;SharedAccessKeyName=Eek;SharedAccessKey=Bar", "entityPath");
    }());
    EXPECT_NO_THROW([]() {
      Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential zz(
          "Endpoint=Bar;SharedAccessKeyName=Eek;SharedAccessKey=Foo;EntityPath=otherPath",
          "otherPath");
    }());
  }

  TEST_F(ConnectionStringTest, ServiceBusSasEntityPathCaseInsensitive)
  {
    Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential credential(
        ConnectionStringWithLowercaseEntityPath, "MyHub");
    EXPECT_EQ("myhub", credential.GetEntityPath());
  }

  // An empty argument is not a mismatch, so the guard must survive.
  TEST_F(ConnectionStringTest, ServiceBusSasEntityPathArgumentEmpty)
  {
    Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential credential(
        ConnectionStringWithLowercaseEntityPath, "");
    EXPECT_EQ("myhub", credential.GetEntityPath());
  }

  TEST_F(ConnectionStringTest, ServiceBusSasEntityPathAbsentFromConnectionString)
  {
    Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential credential(
        "Endpoint=Bar;SharedAccessKeyName=Eek;SharedAccessKey=Bar", "entityPath");
    EXPECT_EQ("", credential.GetEntityPath());
  }

  // The two bytes are a Latin-1 case pair, so a locale-dependent fold would match them.
  TEST_F(ConnectionStringTest, ServiceBusSasEntityPathNonAsciiMismatch)
  {
    std::string const connectionString
        = std::string(ConnectionStringWithLowercaseEntityPath) + static_cast<char>(0xC9);
    std::string const entityPath = std::string("myhub") + static_cast<char>(0xE9);
    EXPECT_THROW(
        {
          Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential credential(
              connectionString, entityPath);
        },
        std::invalid_argument);
  }

  // The uAMQP backend builds its tokens with SASToken_Create and URL_EncodeString from
  // azure-c-shared-utility, and the Rust backend builds them in Rust. Both must produce
  // the same bytes, so this test runs unchanged on both backends.
  //
  // Two details of the expected value are worth knowing. The fields appear in the order
  // sr, sig, se, skn, which is the order SASToken_Create uses and not the order the
  // Microsoft documentation shows. The percent-encoded hex digits are lowercase.
  //
  // The shared access key is fake.
  TEST_F(ConnectionStringTest, GenerateSasTokenFixedVector)
  {
    // cspell: disable
    std::string const connectionString
        = "Endpoint=sb://fake.servicebus.windows.net/;SharedAccessKeyName=FakeKeyName;"
          "SharedAccessKey=ZmFrZWtleWZha2VrZXlmYWtla2V5ZmFrZWtleQ==;EntityPath=eventhub1";

    Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential credential(
        connectionString);

    // A fixed expiration time. GetToken derives its own from the clock, so it cannot
    // produce a stable token to compare against.
    auto const expiresOn{std::chrono::system_clock::from_time_t(1735689600)};

    EXPECT_EQ(
        "SharedAccessSignature sr=sb%3a%2f%2ffake.servicebus.windows.net%2feventhub1"
        "&sig=166yHuTCCC7xv5eXWn%2fzAaC%2fRlB8GsmwNyKJpMehFp0%3d"
        "&se=1735689600&skn=FakeKeyName",
        credential.GenerateSasToken(expiresOn));
    // cspell: enable
  }

  TEST_F(ConnectionStringTest, ConnectionStringParserBad)
  {
    {
      EXPECT_ANY_THROW([]() { Azure::Core::Amqp::_internal::ConnectionStringParser xx(""); }());
      EXPECT_ANY_THROW([]() {
        Azure::Core::Amqp::_internal::ConnectionStringParser yy("Foo=Bar;Boo=Eek;Yoiks=Blang!");
      }());
      EXPECT_ANY_THROW([]() {
        Azure::Core::Amqp::_internal::ConnectionStringParser zz(
            "Endpoint=Bar;SharedAccessKeyName=Eek;SharedAccessKey");
      }());
      EXPECT_ANY_THROW([]() {
        Azure::Core::Amqp::_internal::ConnectionStringParser zz(
            "Endpoint=Bar;SharedAccessKeyName=Eek;SharedAccessKey");
      }());
    }
  }
  TEST_F(ConnectionStringTest, ServiceBusSasBad)
  {
    {
      EXPECT_ANY_THROW(
          []() { Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential xx(""); }());
      EXPECT_ANY_THROW([]() {
        Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential yy(
            "Foo=Bar;Boo=Eek;Yoiks=Blang!");
      }());
      EXPECT_ANY_THROW([]() {
        Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential zz(
            "Endpoint=Bar;SharedAccessKeyName=Eek;SharedAccessKey");
      }());
      EXPECT_ANY_THROW([]() {
        Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential zz(
            "Endpoint=Bar;SharedAccessKeyName=Eek;SharedAccessKey=Foo;EntityPath=otherPath",
            "entityPath");
      }());
    }
  }
}}}} // namespace Azure::Core::Amqp::Tests
