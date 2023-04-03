// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <gtest/gtest.h>
#include <utility>

#include "azure/core/amqp/connection_string_credential.hpp"

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
    Azure::Core::Amqp::_internal::SaslPlainConnectionStringCredential credential(connectionString);
    EXPECT_EQ("sb://{NAMESPACE}.servicebus.windows.net/{EVENT_HUB_NAME}", credential.GetEndpoint());
    EXPECT_EQ("{EVENT_HUB_NAME}", credential.GetEntityPath());
    EXPECT_EQ("{ACCESS_KEY_NAME}", credential.GetSharedAccessKeyName());
    EXPECT_EQ("{ACCESS_KEY}", credential.GetSharedAccessKey());
    {
      auto xport = credential.GetTransport();
      EXPECT_EQ(
          credential.GetCredentialType(), Azure::Core::Amqp::_internal::CredentialType::SaslPlain);
    }
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
      auto xport = credential.GetTransport();
      EXPECT_EQ(
          credential.GetCredentialType(),
          Azure::Core::Amqp::_internal::CredentialType::ServiceBusSas);

      // Generate a SAS token which expires in 60 seconds.
      auto token = credential.GenerateSasToken(
          std::chrono::system_clock::now() + std::chrono::seconds(60));
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

TEST_F(ConnectionStringTest, SaslPlainConnectionBad)
{
  {
    EXPECT_ANY_THROW(
        []() { Azure::Core::Amqp::_internal::SaslPlainConnectionStringCredential xx(""); }());
    EXPECT_ANY_THROW([]() {
      Azure::Core::Amqp::_internal::SaslPlainConnectionStringCredential yy(
          "Foo=Bar;Boo=Eek;Yoiks=Blang!");
    }());
    EXPECT_ANY_THROW([]() {
      Azure::Core::Amqp::_internal::SaslPlainConnectionStringCredential zz(
          "Endpoint=Bar;SharedAccessKeyName=Eek;SharedAccessKey");
    }());
    EXPECT_ANY_THROW([]() {
      Azure::Core::Amqp::_internal::SaslPlainConnectionStringCredential zz(
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
