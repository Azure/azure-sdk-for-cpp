// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <gtest/gtest.h>

#include "mock_amqp_server.hpp"
#include <azure/core/amqp/connection.hpp>
#include <azure/core/amqp/management.hpp>
#include <azure/core/amqp/models/messaging_values.hpp>
#include <azure/core/amqp/session.hpp>

class TestManagement : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

using namespace Azure::Core::Amqp::Models;
using namespace Azure::Core::Amqp::_internal;

TEST_F(TestManagement, BasicTests)
{
  {
    Management management;
  }

  {
    Connection connection("amqps://localhost:5151", {});
    Session session(connection);
    Management management(session, "Test", {});
  }
}

TEST_F(TestManagement, ManagementOpenClose)
{
  {
    Connection connection("amqps://localhost:5151", {});
    Session session(connection);
    Management management(session, "Test", {});

    EXPECT_ANY_THROW(management.Open());
    //    auto openResult = management.Open();
    //    EXPECT_EQ(openResult, ManagementOpenResult::Error);
  }

  {
    MessageTests::AmqpServerMock mockServer;

    Connection connection("amqp://localhost:" + std::to_string(mockServer.GetPort()), {});
    Session session(connection);
    ManagementOptions options;
    options.EnableTrace = 1;
    Management management(session, "Test", {});

    mockServer.StartListening();

    auto openResult = management.Open();
    EXPECT_EQ(openResult, ManagementOpenResult::Ok);

    management.Close();

    mockServer.StopListening();
  }
}

TEST_F(TestManagement, ManagementRequestResponse)
{
  {
    MessageTests::AmqpServerMock mockServer;

    Connection connection("amqp://localhost:" + std::to_string(mockServer.GetPort()), {});
    Session session(connection);
    ManagementOptions options;
    options.EnableTrace = 1;
    Management management(session, "Test", {});

    mockServer.StartListening();

    auto openResult = management.Open();
    EXPECT_EQ(openResult, ManagementOpenResult::Ok);

    AmqpMessage messageToSend;
    messageToSend.SetBody(AmqpValue("Test"));
    // There's nobody to respond, so we expect this to time out.
    Azure::Core::Context context;
    EXPECT_ANY_THROW(management.ExecuteOperation(
        "Test",
        "Test",
        "Test",
        messageToSend,
        context.WithDeadline(std::chrono::system_clock::now() + std::chrono::seconds(2))));

    management.Close();

    mockServer.StopListening();
  }

  class ManagementReceiver : public MessageTests::AmqpServerMock {
  public:
    void SetStatusCode(AmqpValue expectedStatusCode) { m_expectedStatusCode = expectedStatusCode; }
    void SetStatusDescription(AmqpValue expectedStatusDescription)
    {
      m_expectedStatusDescription = expectedStatusDescription;
    }
    void SetStatusCodeName(std::string const& expectedStatusCodeName)
    {
      m_expectedStatusCodeName = expectedStatusCodeName;
    }
    void SetStatusDescriptionName(std::string const& expectedStatusDescriptionName)
    {
      m_expectedStatusDescriptionName = expectedStatusDescriptionName;
    }

  private:
    AmqpValue OnMessageReceived(MessageReceiver const& receiver, AmqpMessage const& incomingMessage) override
    {
      if (receiver.GetSourceName() != "Test" && receiver.GetSourceName() != "$cbs")
      {
        GTEST_LOG_(INFO) << "Rejecting message because it is for an unexpected node name.";
        return Azure::Core::Amqp::Models::_internal::Messaging::DeliveryRejected(
            "test:Rejected", "Unknown message source.");
      }
      if (receiver.GetSourceName() == "Test"
          && incomingMessage.ApplicationProperties.at("operation") != "Test")
      {
        GTEST_LOG_(INFO) << "Rejecting message because is for an unknown operation.";
        return Azure::Core::Amqp::Models::_internal::Messaging::DeliveryRejected(
            "amqp:status:rejected", "Unknown Request operation");
      }
      return AmqpServerMock::OnMessageReceived(receiver, incomingMessage);
    }
    void MessageReceived(
        std::string const&,
        AmqpServerMock::MessageLinkComponents const& linkComponents,
        AmqpMessage const& incomingMessage) const override
    {
      if (incomingMessage.ApplicationProperties.at("operation") == "Test")
      {
        AmqpMessage responseMessage;
        responseMessage.ApplicationProperties[m_expectedStatusCodeName] = m_expectedStatusCode;
        responseMessage.ApplicationProperties[m_expectedStatusDescriptionName]
            = m_expectedStatusDescription;
        responseMessage.SetBody("This is a response body");

        // Management specification section 3.2: The correlation-id of the response message
        // MUST be the correlation-id from the request message (if present), else the
        // message-id from the request message.
        auto requestCorrelationId = incomingMessage.Properties.CorrelationId;
        if (!incomingMessage.Properties.CorrelationId.HasValue())
        {
          requestCorrelationId = incomingMessage.Properties.MessageId.Value();
        }
        responseMessage.Properties.CorrelationId = requestCorrelationId;

        // Block until the send is completed. Note: Do *not* use the listener context to ensure that
        // the send is completed.
        linkComponents.LinkSender->Send(responseMessage);
      }
    }

    AmqpValue m_expectedStatusCode{200};
    AmqpValue m_expectedStatusDescription{"Successful"};
    std::string m_expectedStatusCodeName = "statusCode";
    std::string m_expectedStatusDescriptionName = "statusDescription";
  };
  {
    ManagementReceiver mockServer;

    Connection connection("amqp://localhost:" + std::to_string(mockServer.GetPort()), {});
    Session session(connection);
    ManagementOptions options;
    options.EnableTrace = true;
    Management management(session, "Test", options);

    mockServer.StartListening();

    auto openResult = management.Open();
    EXPECT_EQ(openResult, ManagementOpenResult::Ok);

    AmqpMessage messageToSend;
    messageToSend.SetBody(AmqpValue("Test"));

    auto response = management.ExecuteOperation("Test", "Test", "Test", messageToSend);
    EXPECT_EQ(std::get<0>(response), ManagementOperationResult::Ok);
    EXPECT_EQ(std::get<1>(response), 200);
    EXPECT_EQ(std::get<2>(response), "Successful");
    management.Close();

    mockServer.StopListening();
  }

  {
    ManagementReceiver mockServer;

    Connection connection("amqp://localhost:" + std::to_string(mockServer.GetPort()), {});
    Session session(connection);
    ManagementOptions options;
    options.EnableTrace = true;
    Management management(session, "Test", options);

    mockServer.SetStatusCode(500);
    mockServer.SetStatusDescription("Bad Things Happened.");
    mockServer.StartListening();

    auto openResult = management.Open();
    EXPECT_EQ(openResult, ManagementOpenResult::Ok);

    AmqpMessage messageToSend;
    messageToSend.SetBody(AmqpValue("Test"));

    auto response = management.ExecuteOperation("Test", "Test", "Test", messageToSend);
    EXPECT_EQ(std::get<0>(response), ManagementOperationResult::FailedBadStatus);
    EXPECT_EQ(std::get<1>(response), 500);
    EXPECT_EQ(std::get<2>(response), "Bad Things Happened.");
    management.Close();

    mockServer.StopListening();
  }

  // Send a response with a bogus status code type.
  {
    ManagementReceiver mockServer;

    Connection connection("amqp://localhost:" + std::to_string(mockServer.GetPort()), {});
    Session session(connection);
    ManagementOptions options;
    options.EnableTrace = true;
    Management management(session, "Test", options);

    // Set the response status code to something other than an int - that will cause the response to
    // be rejected by the management client.
    mockServer.SetStatusCode(500u);
    mockServer.SetStatusDescription("Bad Things Happened.");
    mockServer.StartListening();

    auto openResult = management.Open();
    EXPECT_EQ(openResult, ManagementOpenResult::Ok);

    AmqpMessage messageToSend;
    messageToSend.SetBody(AmqpValue("Test"));

    auto response = management.ExecuteOperation("Test", "Type", "Locales", messageToSend);
    EXPECT_EQ(std::get<0>(response), ManagementOperationResult::Error);
    EXPECT_EQ(std::get<1>(response), 0);
    EXPECT_EQ(std::get<2>(response), "Error processing management operation.");
    management.Close();

    mockServer.StopListening();
  }

  // Send a response to the request with a bogus status code name.
  {
    ManagementReceiver mockServer;

    Connection connection("amqp://localhost:" + std::to_string(mockServer.GetPort()), {});
    Session session(connection);
    ManagementOptions options;
    options.EnableTrace = true;

    Management management(session, "Test", options);

    // Set the response status code to something other than an int - that will cause the response to
    // be rejected by the management client.
    mockServer.SetStatusCode(500);
    mockServer.SetStatusCodeName("status-code");
    mockServer.SetStatusDescription("Bad Things Happened.");
    mockServer.StartListening();

    auto openResult = management.Open();
    EXPECT_EQ(openResult, ManagementOpenResult::Ok);

    AmqpMessage messageToSend;
    messageToSend.SetBody(AmqpValue("Test"));

    auto response = management.ExecuteOperation("Test", "Type", "Locales", messageToSend);
    EXPECT_EQ(std::get<0>(response), ManagementOperationResult::Error);
    EXPECT_EQ(std::get<1>(response), 0);
    EXPECT_EQ(std::get<2>(response), "Error processing management operation.");
    management.Close();

    mockServer.StopListening();
  }

    // Send a response to the request with a bogus status code name.
  {
    ManagementReceiver mockServer;

    Connection connection("amqp://localhost:" + std::to_string(mockServer.GetPort()), {});
    Session session(connection);
    ManagementOptions options;
    options.EnableTrace = true;
    options.ExpectedStatusCodeKeyName = "status-code";

    Management management(session, "Test", options);

    // Set the response status code to something other than an int - that will cause the response to
    // be rejected by the management client.
    mockServer.SetStatusCode(235);
    mockServer.SetStatusCodeName("status-code");
    mockServer.SetStatusDescription("Bad Things Happened..");
    mockServer.StartListening();

    auto openResult = management.Open();
    EXPECT_EQ(openResult, ManagementOpenResult::Ok);

    AmqpMessage messageToSend;
    messageToSend.SetBody(AmqpValue("Test"));

    auto response = management.ExecuteOperation("Test", "Type", "Locales", messageToSend);
    EXPECT_EQ(std::get<0>(response), ManagementOperationResult::Ok);
    EXPECT_EQ(std::get<1>(response), 235);
    EXPECT_EQ(std::get<2>(response), "Bad Things Happened..");
    management.Close();

    mockServer.StopListening();
  }

  // Send a management request with an invalid node name.
  {
    ManagementReceiver mockServer;

    Connection connection("amqp://localhost:" + std::to_string(mockServer.GetPort()), {});
    Session session(connection);
    ManagementOptions options;
    options.EnableTrace = true;
    Management management(session, "IncorrectNodeName", options);

    mockServer.StartListening();

    auto openResult = management.Open();
    EXPECT_EQ(openResult, ManagementOpenResult::Ok);

    AmqpMessage messageToSend;
    messageToSend.SetBody(AmqpValue("Test"));

    auto response = management.ExecuteOperation("Test", "Type", "Locales", messageToSend);
    EXPECT_EQ(std::get<0>(response), ManagementOperationResult::Error);
    EXPECT_EQ(std::get<1>(response), 0);
    EXPECT_EQ(std::get<2>(response), "");
    management.Close();

    mockServer.StopListening();
  }

    // Send a management request with an unknown operation name.
  {
    ManagementReceiver mockServer;

    Connection connection("amqp://localhost:" + std::to_string(mockServer.GetPort()), {});
    Session session(connection);
    ManagementOptions options;
    options.EnableTrace = true;
    Management management(session, "Test", options);

    mockServer.StartListening();

    auto openResult = management.Open();
    EXPECT_EQ(openResult, ManagementOpenResult::Ok);

    AmqpMessage messageToSend;
    messageToSend.SetBody(AmqpValue("Test"));

    auto response = management.ExecuteOperation("Unknown Operation", "Type", "Locales", messageToSend);
    EXPECT_EQ(std::get<0>(response), ManagementOperationResult::Error);
    EXPECT_EQ(std::get<1>(response), 0);
    EXPECT_EQ(std::get<2>(response), "");
    management.Close();

    mockServer.StopListening();
  }
}
