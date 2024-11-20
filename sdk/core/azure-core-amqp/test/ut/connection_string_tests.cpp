// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/connection_string_credential.hpp"

#include <azure/core/platform.hpp>

#include <utility>

#include <gtest/gtest.h>

class ConnectionStringTest : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

// EventHubs connection strings look like:
// Endpoint=sb://{NAMESPACE}.servicebus.windows.net/{EVENT_HUB_NAME};EntityPath={EVENT_HUB_NAME};SharedAccessKeyName={ACCESS_KEY_NAME};SharedAccessKey={ACCESS_KEY}

TEST_F(ConnectionStringTest, SaslPlainConnectionGood)
{
  {
    std::string connectionString
        = "Endpoint=sb://{NAMESPACE}.servicebus.windows.net/"
          "{EVENT_HUB_NAME};EntityPath={EVENT_HUB_NAME};SharedAccessKeyName={ACCESS_KEY_NAME};"
          "SharedAccessKey={ACCESS_KEY}";
    Azure::Core::Amqp::_internal::ConnectionStringParser credential(connectionString);
    EXPECT_EQ("sb://{NAMESPACE}.servicebus.windows.net/{EVENT_HUB_NAME}", credential.GetEndpoint());
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
    EXPECT_EQ("sb://{NAMESPACE}.servicebus.windows.net/{EVENT_HUB_NAME}", credential.GetEndpoint());
    EXPECT_EQ("{EVENT_HUB_NAME}", credential.GetEntityPath());
    EXPECT_EQ("{ACCESS_KEY_NAME}=", credential.GetSharedAccessKeyName());
    EXPECT_EQ("{ACCESS_KEY}=", credential.GetSharedAccessKey());
    {
#if !defined(AZURE_PLATFORM_MAC)
      auto xport = credential.GetTransport();
#endif // !defined(AZURE_PLATFORM_MAC)

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
