// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "mock_amqp_server.hpp"

#include <azure/core/amqp/internal/connection.hpp>
#include <azure/core/amqp/internal/management.hpp>
#include <azure/core/amqp/internal/models/messaging_values.hpp>
#include <azure/core/amqp/internal/session.hpp>
#include <azure/core/platform.hpp>

#include <gtest/gtest.h>

// cspell: ignore abcdabcd

namespace Azure { namespace Core { namespace Amqp { namespace Tests {

  class TestManagement : public testing::Test {
  protected:
    void SetUp() override {}
    void TearDown() override {}
  };

  using namespace Azure::Core::Amqp::Models;
  using namespace Azure::Core::Amqp::_internal;

#if !defined(AZ_PLATFORM_MAC)
  TEST_F(TestManagement, BasicTests)
  {
    {
      ConnectionOptions options;
      options.Port = 5151;
      Connection connection("localhost", nullptr, options);

      Session session{connection.CreateSession({})};
      ManagementClient management(session.CreateManagementClient("Test", {}));
    }
  }
  TEST_F(TestManagement, ManagementOpenCloseNoListener)
  {
    ConnectionOptions options;
    options.Port = 5151;
    Connection connection("localhost", nullptr, options);

    Session session{connection.CreateSession({})};
    ManagementClient management(session.CreateManagementClient("Test", {}));

    // EXPECT_ANY_THROW(management.Open());
    auto openResult = management.Open();
    EXPECT_EQ(openResult, ManagementOpenStatus::Error);
  }

  TEST_F(TestManagement, ManagementOpenClose)
  {
    MessageTests::AmqpServerMock mockServer;

    ConnectionOptions connectionOptions;
    connectionOptions.Port = mockServer.GetPort();
    Connection connection("localhost", nullptr, connectionOptions);

    Session session{connection.CreateSession({})};
    ManagementClientOptions options;
    options.EnableTrace = 1;
    ManagementClient management(session.CreateManagementClient("Test", options));

    mockServer.StartListening();

    auto openResult = management.Open();
    EXPECT_EQ(openResult, ManagementOpenStatus::Ok);

    management.Close();

    mockServer.StopListening();
  }

  TEST_F(TestManagement, ManagementOpenCloseAuthenticated)
  {
    MessageTests::AmqpServerMock mockServer;

    auto sasCredential = std::make_shared<ServiceBusSasConnectionStringCredential>(
        "Endpoint=amqp://localhost:" + std::to_string(mockServer.GetPort())
        + "/;SharedAccessKeyName=MyTestKey;SharedAccessKey=abcdabcd;EntityPath=testLocation");

    ConnectionOptions connectionOptions;
    connectionOptions.Port = mockServer.GetPort();
    Connection connection("localhost", sasCredential, connectionOptions);

    Session session{connection.CreateSession({})};
    ManagementClientOptions options;
    options.EnableTrace = 1;
    ManagementClient management(session.CreateManagementClient("Test", options));

    mockServer.StartListening();

    auto openResult = management.Open();
    EXPECT_EQ(openResult, ManagementOpenStatus::Ok);

    management.Close();

    mockServer.StopListening();
  }

  TEST_F(TestManagement, ManagementOpenCloseError)
  {
    MessageTests::AmqpServerMock mockServer;

    ConnectionOptions connectionOptions;
    connectionOptions.Port = mockServer.GetPort();
    Connection connection("localhost", nullptr, connectionOptions);

    Session session{connection.CreateSession({})};
    ManagementClientOptions options;
    options.EnableTrace = 1;
    ManagementClient management(session.CreateManagementClient("Test", options));

    mockServer.StartListening();
    Azure::Core::Context context;
    context.Cancel();
    EXPECT_EQ(management.Open(context), ManagementOpenStatus::Cancelled);

    management.Close();

    mockServer.StopListening();
  }
#endif // !defined(AZ_PLATFORM_MAC)
#if !defined(AZ_PLATFORM_MAC)

  namespace {
    class ManagementReceiver : public MessageTests::AmqpServerMock {
    public:
      void SetStatusCode(AmqpValue expectedStatusCode)
      {
        m_expectedStatusCode = expectedStatusCode;
      }
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
      AmqpValue OnMessageReceived(
          MessageReceiver const& receiver,
          std::shared_ptr<AmqpMessage> const& incomingMessage) override
      {
        // We can only listen on the management or cbs nodes.
        if (receiver.GetSourceName() != "$management" && receiver.GetSourceName() != "$cbs")
        {
          GTEST_LOG_(INFO) << "Rejecting message because it is for an unexpected node name.";
          auto rv = Azure::Core::Amqp::Models::_internal::Messaging::DeliveryRejected(
              "test:Rejected", "Unknown message source.", {});
          GTEST_LOG_(INFO) << "RV=" << rv;
          return rv;
        }
        // If this is coming on the management node, we only support the Test operation.
        if (receiver.GetSourceName() == "$management"
            && incomingMessage->ApplicationProperties.at("operation") != "Test")
        {
          GTEST_LOG_(INFO) << "Rejecting message because it is for an unknown operation.";
          auto rv = Azure::Core::Amqp::Models::_internal::Messaging::DeliveryRejected(
              "amqp:status:rejected", "Unknown Request operation", {});
          GTEST_LOG_(INFO) << "RV=" << rv;
          return rv;
        }
        return AmqpServerMock::OnMessageReceived(receiver, incomingMessage);
      }
      void MessageReceived(
          std::string const&,
          AmqpServerMock::MessageLinkComponents const& linkComponents,
          std::shared_ptr<AmqpMessage> const& incomingMessage) const override
      {
        if (incomingMessage->ApplicationProperties.at("operation") == "Test")
        {
          AmqpMessage responseMessage;
          responseMessage.ApplicationProperties[m_expectedStatusCodeName] = m_expectedStatusCode;
          responseMessage.ApplicationProperties[m_expectedStatusDescriptionName]
              = m_expectedStatusDescription;
          responseMessage.SetBody("This is a response body");

          // Management specification section 3.2: The correlation-id of the response message
          // MUST be the correlation-id from the request message (if present), else the
          // message-id from the request message.
          auto requestCorrelationId = incomingMessage->Properties.CorrelationId;
          if (!incomingMessage->Properties.CorrelationId.HasValue())
          {
            requestCorrelationId = incomingMessage->Properties.MessageId.Value();
          }
          responseMessage.Properties.CorrelationId = requestCorrelationId;

          // Block until the send is completed. Note: Do *not* use the listener context to ensure
          // that the send is completed.
          linkComponents.LinkSender->Send(responseMessage);
        }
      }

      AmqpValue m_expectedStatusCode{200};
      AmqpValue m_expectedStatusDescription{"Successful"};
      std::string m_expectedStatusCodeName = "statusCode";
      std::string m_expectedStatusDescriptionName = "statusDescription";
    };
  } // namespace

  TEST_F(TestManagement, ManagementRequestResponse)
  {
    {
      MessageTests::AmqpServerMock mockServer;

      ConnectionOptions connectionOptions;
      connectionOptions.Port = mockServer.GetPort();

      auto sasCredential = std::make_shared<ServiceBusSasConnectionStringCredential>(
          "Endpoint=amqp://localhost:" + std::to_string(mockServer.GetPort())
          + "/;SharedAccessKeyName=MyTestKey;SharedAccessKey=abcdabcd;EntityPath=testLocation");
      Connection connection("localhost", sasCredential, connectionOptions);
      Session session{connection.CreateSession({})};
      ManagementClientOptions options;
      options.EnableTrace = 1;
      ManagementClient management(session.CreateManagementClient("Test", {}));

      mockServer.StartListening();

      auto openResult = management.Open();
      ASSERT_EQ(openResult, ManagementOpenStatus::Ok);

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
  }
  TEST_F(TestManagement, ManagementRequestResponseSimple)
  {
    {
      ManagementReceiver mockServer;

      ConnectionOptions connectionOptions;
      connectionOptions.EnableTrace = true;
      connectionOptions.Port = mockServer.GetPort();

      Connection connection("localhost", nullptr, connectionOptions);
      Session session{connection.CreateSession({})};
      ManagementClientOptions options;
      options.EnableTrace = true;
      ManagementClient management(session.CreateManagementClient("Test", options));

      mockServer.StartListening();

      auto openResult = management.Open();
      ASSERT_EQ(openResult, ManagementOpenStatus::Ok);

      AmqpMessage messageToSend;
      messageToSend.SetBody(AmqpValue("Test"));

      auto response = management.ExecuteOperation("Test", "Test", "Test", messageToSend);
      EXPECT_EQ(response.Status, ManagementOperationStatus::Ok);
      EXPECT_EQ(response.StatusCode, 200);
      EXPECT_EQ(response.Error.Description, "Successful");
      management.Close();

      mockServer.StopListening();
    }
  }
  TEST_F(TestManagement, ManagementRequestResponseExpect500)
  {

    {
      ManagementReceiver mockServer;
      ConnectionOptions connectionOptions;
      connectionOptions.EnableTrace = true;
      connectionOptions.Port = mockServer.GetPort();

      Connection connection("localhost", nullptr, connectionOptions);
      Session session{connection.CreateSession({})};

      ManagementClientOptions options;
      options.EnableTrace = true;
      ManagementClient management(session.CreateManagementClient("Test", options));

      mockServer.SetStatusCode(500);
      mockServer.SetStatusDescription("Bad Things Happened.");
      mockServer.StartListening();

      auto openResult = management.Open();
      ASSERT_EQ(openResult, ManagementOpenStatus::Ok);

      AmqpMessage messageToSend;
      messageToSend.SetBody(AmqpValue("Test"));

      auto response = management.ExecuteOperation("Test", "Test", "Test", messageToSend);
      EXPECT_EQ(response.Status, ManagementOperationStatus::FailedBadStatus);
      EXPECT_EQ(response.StatusCode, 500);
      EXPECT_EQ(response.Error.Description, "Bad Things Happened.");
      management.Close();

      mockServer.StopListening();
    }
  }
  TEST_F(TestManagement, ManagementRequestResponseBogusStatusCode)
  {
    // Send a response with a bogus status code type.
    {
      ManagementReceiver mockServer;

      ConnectionOptions connectionOptions;
      connectionOptions.EnableTrace = true;
      connectionOptions.Port = mockServer.GetPort();
      Connection connection("localhost", nullptr, connectionOptions);
      Session session{connection.CreateSession({})};
      ManagementClientOptions options;
      options.EnableTrace = true;
      ManagementClient management(session.CreateManagementClient("Test", options));

      // Set the response status code to something other than an int - that will cause the response
      // to be rejected by the management client.
      mockServer.SetStatusCode(500u);
      mockServer.SetStatusDescription("Bad Things Happened.");
      mockServer.StartListening();

      auto openResult = management.Open();
      ASSERT_EQ(openResult, ManagementOpenStatus::Ok);

      AmqpMessage messageToSend;
      messageToSend.SetBody(AmqpValue("Test"));

      auto response = management.ExecuteOperation("Test", "Type", "Locales", messageToSend);
      EXPECT_EQ(response.Status, ManagementOperationStatus::Error);
      EXPECT_EQ(response.StatusCode, 500);
      EXPECT_EQ(
          response.Error.Description,
          "Message Delivery Rejected: Received message statusCode value is not an int.");
      management.Close();

      mockServer.StopListening();
    }
  }
  TEST_F(TestManagement, ManagementRequestResponseBogusStatusName)
  {
    // Send a response to the request with a bogus status code name.
    {
      ManagementReceiver mockServer;

      struct ManagementEventsHandler : public ManagementClientEvents
      {
        void OnError(Azure::Core::Amqp::Models::_internal::AmqpError const&) override
        {
          Error = true;
        }
        bool Error{false};
      };
      ManagementEventsHandler managementEvents;

      ConnectionOptions connectionOptions;
      connectionOptions.EnableTrace = true;
      connectionOptions.Port = mockServer.GetPort();

      Connection connection("localhost", nullptr, connectionOptions);
      Session session{connection.CreateSession({})};
      ManagementClientOptions options;
      options.EnableTrace = true;

      ManagementClient management(
          session.CreateManagementClient("Test", options, &managementEvents));

      // Set the response status code to something other than an int - that will cause the response
      // to be rejected by the management client.
      mockServer.SetStatusCode(500);
      mockServer.SetStatusCodeName("status-code");
      mockServer.SetStatusDescription("Bad Things Happened.");
      mockServer.StartListening();

      auto openResult = management.Open();
      ASSERT_EQ(openResult, ManagementOpenStatus::Ok);

      AmqpMessage messageToSend;
      messageToSend.SetBody(AmqpValue("Test"));

      auto response = management.ExecuteOperation("Test", "Type", "Locales", messageToSend);
      EXPECT_EQ(response.Status, ManagementOperationStatus::Error);
      EXPECT_EQ(response.StatusCode, 500);
      EXPECT_EQ(
          response.Error.Description,
          "Message Delivery Rejected: Received message does not have a statusCode status code "
          "key.");
      EXPECT_TRUE(managementEvents.Error);
      management.Close();

      mockServer.StopListening();
    }
  }
  TEST_F(TestManagement, ManagementRequestResponseBogusStatusName2)
  {
    // Send a response to the request with a bogus status code name.
    {
      ManagementReceiver mockServer;

      ConnectionOptions connectionOptions;
      connectionOptions.EnableTrace = true;
      connectionOptions.Port = mockServer.GetPort();
      Connection connection("localhost", nullptr, connectionOptions);
      Session session{connection.CreateSession({})};
      ManagementClientOptions options;
      options.EnableTrace = true;
      options.ExpectedStatusCodeKeyName = "status-code";

      ManagementClient management(session.CreateManagementClient("Test", options));

      // Set the response status code to something other than an int - that will cause the response
      // to be rejected by the management client.
      mockServer.SetStatusCode(235);
      mockServer.SetStatusCodeName("status-code");
      mockServer.SetStatusDescription("Bad Things Happened..");
      mockServer.StartListening();

      auto openResult = management.Open();
      ASSERT_EQ(openResult, ManagementOpenStatus::Ok);

      AmqpMessage messageToSend;
      messageToSend.SetBody(AmqpValue("Test"));

      auto response = management.ExecuteOperation("Test", "Type", "Locales", messageToSend);
      EXPECT_EQ(response.Status, ManagementOperationStatus::Ok);
      EXPECT_EQ(response.StatusCode, 235);
      EXPECT_EQ(response.Error.Description, "Bad Things Happened..");
      management.Close();

      mockServer.StopListening();
    }
  }

  TEST_F(TestManagement, ManagementRequestResponseUnknownOperationName)
  {
    // Send a management request with an unknown operation name.
    {
      ManagementReceiver mockServer;
      mockServer.StartListening();

      ConnectionOptions connectionOptions;
      connectionOptions.EnableTrace = true;
      connectionOptions.Port = mockServer.GetPort();
      Connection connection("localhost", nullptr, connectionOptions);

      Session session{connection.CreateSession({})};

      ManagementClientOptions options;
      options.EnableTrace = true;
      ManagementClient management(session.CreateManagementClient("Test", options));

      auto openResult = management.Open();
      ASSERT_EQ(openResult, ManagementOpenStatus::Ok);

      AmqpMessage messageToSend;
      messageToSend.SetBody(AmqpValue("Test"));

      auto response = management.ExecuteOperation(
          "Unknown Operation",
          "Type",
          "Locales",
          messageToSend,
          Azure::Core::Context::ApplicationContext.WithDeadline(
              std::chrono::system_clock::now() + std::chrono::seconds(10)));
      EXPECT_EQ(response.Status, ManagementOperationStatus::Error);
      EXPECT_EQ(response.StatusCode, 500);
      EXPECT_EQ(response.Error.Description, "Unknown Request operation");
      management.Close();

      mockServer.StopListening();
    }
  }
#endif // !defined(AZ_PLATFORM_MAC)
}}}} // namespace Azure::Core::Amqp::Tests
