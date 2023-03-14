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
    Azure::Core::_internal::Amqp::SaslPlainConnectionStringCredential credential(connectionString);
    EXPECT_EQ("sb://{NAMESPACE}.servicebus.windows.net/{EVENT_HUB_NAME}", credential.GetEndpoint());
    EXPECT_EQ("{EVENT_HUB_NAME}", credential.GetEntityPath());
    EXPECT_EQ("{ACCESS_KEY_NAME}", credential.GetSharedAccessKeyName());
    EXPECT_EQ("{ACCESS_KEY}", credential.GetSharedAccessKey());
    {
      auto xport = credential.GetTransport();
    }
  }
  {
    std::string connectionString
        = "Endpoint=sb://{NAMESPACE}.servicebus.windows.net/"
          "{EVENT_HUB_NAME};EntityPath={EVENT_HUB_NAME};SharedAccessKeyName={ACCESS_KEY_NAME}=;"
          "SharedAccessKey={ACCESS_KEY}=";
    Azure::Core::_internal::Amqp::SaslPlainConnectionStringCredential credential(connectionString);
    EXPECT_EQ("sb://{NAMESPACE}.servicebus.windows.net/{EVENT_HUB_NAME}", credential.GetEndpoint());
    EXPECT_EQ("{EVENT_HUB_NAME}", credential.GetEntityPath());
    EXPECT_EQ("{ACCESS_KEY_NAME}=", credential.GetSharedAccessKeyName());
    EXPECT_EQ("{ACCESS_KEY}=", credential.GetSharedAccessKey());
    {
      auto xport = credential.GetTransport();
    }
  }
}

TEST_F(ConnectionStringTest, SaslPlainConnectionBad)
{
  {
    EXPECT_ANY_THROW([]() { Azure::Core::_internal::Amqp::SaslPlainConnectionStringCredential xx(""); }());
  }
}