// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/common/global_state.hpp"
#include "azure/core/amqp/internal/connection.hpp"
#include "azure/core/amqp/internal/management.hpp"
#include "azure/core/amqp/internal/models/messaging_values.hpp"
#include "azure/core/amqp/internal/session.hpp"
#include "azure/core/platform.hpp"

#include <azure/core/url.hpp>

#if ENABLE_RUST_AMQP
#define USE_NATIVE_BROKER
#elif ENABLE_UAMQP
#undef USE_NATIVE_BROKER
#endif

#if !defined(USE_NATIVE_BROKER)
#include "mock_amqp_server.hpp"
#else
#include <azure/core/internal/environment.hpp>
#endif

#include <gtest/gtest.h>

// cspell: ignore abcdabcd

namespace Azure { namespace Core { namespace Amqp { namespace Tests {
  using namespace Azure::Core::Amqp::_internal;

  class TestManagement : public testing::Test {
  protected:
    void SetUp() override
    {
#if defined(USE_NATIVE_BROKER)
      auto testBrokerUrl = Azure::Core::_internal::Environment::GetVariable("TEST_BROKER_ADDRESS");
      if (testBrokerUrl.empty())
      {
        GTEST_FATAL_FAILURE_("Could not find required environment variable TEST_BROKER_ADDRESS");
      }
      Azure::Core::Url brokerUrl(testBrokerUrl);
      m_brokerEndpoint = brokerUrl;
#else
      m_brokerEndpoint = Azure::Core::Url("amqp://localhost:" + std::to_string(GetPort()));
#endif
    }
    void TearDown() override
    { // When the test is torn down, the global state MUST be idle. If it is not,
      // something leaked.
      Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
    }

    std::string GetBrokerEndpoint() { return m_brokerEndpoint.GetAbsoluteUrl(); }

    std::uint16_t GetPort() { return m_brokerEndpoint.GetPort(); }

    auto CreateAmqpConnection(
        std::string const& containerId
        = testing::UnitTest::GetInstance()->current_test_info()->name(),
        bool enableTracing = false,
        Azure::Core::Context const& context = {})
    {
      ConnectionOptions options;
      options.ContainerId = containerId;
      options.EnableTrace = enableTracing;
      options.Port = GetPort();

      auto connection = Connection("localhost", nullptr, options);
#if ENABLE_RUST_AMQP
      connection.Open(context);
#endif
      return connection;
      (void)context;
    }
    auto CreateAmqpSession(Connection const& connection, Context const& context = {})
    {
      auto session = connection.CreateSession();
#if ENABLE_RUST_AMQP
      session.Begin(context);
#endif
      return session;
      (void)context;
    }

    void CloseAmqpConnection(Connection& connection, Azure::Core::Context const& context = {})
    {
#if ENABLE_RUST_AMQP
      connection.Close(context);
#endif
      (void)connection;
      (void)context;
    }
    void EndAmqpSession(Session& session, Azure::Core::Context const& context = {})
    {
#if ENABLE_RUST_AMQP
      session.End(context);
#endif
      (void)session;
      (void)context;
    }

    void StartServerListening()
    {
#if !defined(USE_NATIVE_BROKER)
      m_mockServer.StartListening();
#endif
    }

    void StopServerListening()
    {
#if !defined(USE_NATIVE_BROKER)
      m_mockServer.StopListening();
#endif
    }

#if !defined(USE_NATIVE_BROKER)
  protected:
    MessageTests::AmqpServerMock m_mockServer;
#endif
  private:
    Azure::Core::Url m_brokerEndpoint{};
  };

  using namespace Azure::Core::Amqp::Models;

#if !defined(AZ_PLATFORM_MAC) || defined(ENABLE_RUST_AMQP)
  TEST_F(TestManagement, BasicTests)
  {
    {
      auto connection{CreateAmqpConnection()};
      auto session{CreateAmqpSession(connection)};

      ManagementClient management(session.CreateManagementClient("Test", {}));
      EndAmqpSession(session);
      CloseAmqpConnection(connection);
    }
  }

#if !defined(ENABLE_RUST_AMQP)
  // With Rust AMQP, this failure is triggere when the connection is opened, so we can't test it
  // in the context of the management APIs.
  TEST_F(TestManagement, ManagementOpenCloseNoListener)
  {
    ConnectionOptions options;
    options.Port = 5151;
    Connection connection("localhost", nullptr, options);

#if ENABLE_RUST_AMQP
    EXPECT_NO_THROW(connection.Open({}));
#endif

    Session session{connection.CreateSession({})};
#if ENABLE_RUST_AMQP
    EXPECT_NO_THROW(session.Begin({}));
#endif
    ManagementClient management(session.CreateManagementClient("Test", {}));

    // EXPECT_ANY_THROW(management.Open());
    auto openResult = management.Open();
    EXPECT_EQ(openResult, ManagementOpenStatus::Error);

    management.Close();
    EndAmqpSession(session);
    CloseAmqpConnection(connection);
  }
#endif

#if !defined(USE_NATIVE_BROKER)
  namespace {
    class ManagementServiceEndpoint final : public MessageTests::MockServiceEndpoint {
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
      ManagementServiceEndpoint(MessageTests::MockServiceEndpointOptions const& options)
          : MockServiceEndpoint("$management", options)
      {
      }

      virtual ~ManagementServiceEndpoint() = default;

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
        return MockServiceEndpoint::OnMessageReceived(receiver, incomingMessage);
      }
      void MessageReceived(std::string const&, std::shared_ptr<AmqpMessage> const& incomingMessage)
          override
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
          auto& requestCorrelationId = incomingMessage->Properties.CorrelationId;
          if (incomingMessage->Properties.CorrelationId.IsNull())
          {
            requestCorrelationId = incomingMessage->Properties.MessageId;
          }
          responseMessage.Properties.CorrelationId = requestCorrelationId;

          // Block until the send is completed. Note: Do *not* use the listener context to
          // ensure that the send is completed.
          auto sendResult(GetMessageSender().Send(responseMessage));
          if (std::get<0>(sendResult) != MessageSendStatus::Ok)
          {
            GTEST_LOG_(INFO) << "Failed to send response message. This may be expected: "
                             << std::get<1>(sendResult);
          }
        }
      }

      AmqpValue m_expectedStatusCode{200};
      AmqpValue m_expectedStatusDescription{"Successful"};
      std::string m_expectedStatusCodeName = "statusCode";
      std::string m_expectedStatusDescriptionName = "statusDescription";
    };
  } // namespace
#endif

  TEST_F(TestManagement, ManagementOpenClose)
  {
#if !defined(USE_NATIVE_BROKER)
    MessageTests::MockServiceEndpointOptions managementEndpointOptions;
    managementEndpointOptions.EnableTrace = true;
    auto endpoint = std::make_shared<ManagementServiceEndpoint>(managementEndpointOptions);
    m_mockServer.AddServiceEndpoint(endpoint);

    auto connection{CreateAmqpConnection()};
    auto session{CreateAmqpSession(connection)};

    ManagementClientOptions options;
    options.EnableTrace = 1;
    ManagementClient management(session.CreateManagementClient("Test", options));

    StartServerListening();

    auto openResult = management.Open();
    EXPECT_EQ(openResult, ManagementOpenStatus::Ok);

    management.Close();

    StopServerListening();
    EndAmqpSession(session);
    CloseAmqpConnection(connection);
#else
#endif
  }

  TEST_F(TestManagement, ManagementOpenCloseAuthenticated)
  {
#if !defined(USE_NATIVE_BROKER)
    MessageTests::MockServiceEndpointOptions managementEndpointOptions;
    managementEndpointOptions.EnableTrace = true;
    auto endpoint = std::make_shared<ManagementServiceEndpoint>(managementEndpointOptions);
    m_mockServer.AddServiceEndpoint(endpoint);

    auto sasCredential = std::make_shared<ServiceBusSasConnectionStringCredential>(
        "Endpoint=amqp://localhost:" + std::to_string(GetPort())
        + "/;SharedAccessKeyName=MyTestKey;SharedAccessKey=abcdabcd;EntityPath=testLocation");

    auto connection{CreateAmqpConnection()};

    auto session{CreateAmqpSession(connection)};
    ManagementClientOptions options;
    options.EnableTrace = 1;
    ManagementClient management(session.CreateManagementClient("Test", options));

    StartServerListening();

    auto openResult = management.Open();
    EXPECT_EQ(openResult, ManagementOpenStatus::Ok);

    management.Close();

    StopServerListening();
    EndAmqpSession(session);
    CloseAmqpConnection(connection);
#else
#endif
  }

  TEST_F(TestManagement, ManagementOpenCloseAuthenticatedFail)
  {
#if !defined(USE_NATIVE_BROKER)
    MessageTests::MockServiceEndpointOptions managementEndpointOptions;
    managementEndpointOptions.EnableTrace = true;
    auto endpoint = std::make_shared<ManagementServiceEndpoint>(managementEndpointOptions);
    m_mockServer.AddServiceEndpoint(endpoint);

    auto sasCredential = std::make_shared<ServiceBusSasConnectionStringCredential>(
        "Endpoint=amqp://localhost:" + std::to_string(GetPort())
        + "/;SharedAccessKeyName=MyTestKey;SharedAccessKey=abcdabcd;EntityPath=testLocation");

    auto connection(CreateAmqpConnection());
    auto session(CreateAmqpSession(connection));
    ManagementClientOptions options;
    options.EnableTrace = 1;
    ManagementClient management(session.CreateManagementClient("Test", options));

    // Force an authentication error.
    m_mockServer.ForceCbsError(true);

    StartServerListening();

    try
    {
      ManagementOpenStatus openResult{ManagementOpenStatus::Error};
      EXPECT_THROW(
          openResult = management.Open(), Azure::Core::Credentials::AuthenticationException);
      EXPECT_EQ(openResult, ManagementOpenStatus::Error);

      management.Close();
    }
    catch (std::exception const& e)
    {
      GTEST_LOG_(INFO) << "Caught exception: " << e.what();
    }
    StopServerListening();
    EndAmqpSession(session);
    CloseAmqpConnection(connection);
#else
#endif
  }

  TEST_F(TestManagement, ManagementOpenCloseError)
  {
#if ENABLE_UAMQP
    MessageTests::AmqpServerMock mockServer;
    MessageTests::MockServiceEndpointOptions managementEndpointOptions;
    managementEndpointOptions.EnableTrace = true;
    auto endpoint = std::make_shared<ManagementServiceEndpoint>(managementEndpointOptions);
    mockServer.AddServiceEndpoint(endpoint);

    ConnectionOptions connectionOptions;
    connectionOptions.Port = GetPort();
    Connection connection("localhost", nullptr, connectionOptions);

    Session session{connection.CreateSession({})};
    ManagementClientOptions options;
    options.EnableTrace = 1;
    ManagementClient management(session.CreateManagementClient("Test", options));

    mockServer.StartListening();
    Azure::Core::Context context;
    context.Cancel();
    EXPECT_EQ(management.Open(context), ManagementOpenStatus::Cancelled);

    EXPECT_THROW(management.Close(), std::runtime_error);

    mockServer.StopListening();
#else
#endif
  }
#endif // !defined(AZ_PLATFORM_MAC)
#if !defined(AZ_PLATFORM_MAC) || defined(ENABLE_RUST_AMQP)
#if ENABLE_UAMQP
  namespace {

    class NullResponseManagementServiceEndpoint final : public MessageTests::MockServiceEndpoint {
    public:
      NullResponseManagementServiceEndpoint(MessageTests::MockServiceEndpointOptions const& options)
          : MockServiceEndpoint("$management", options)
      {
      }

    private:
      void MessageReceived(
          std::string const& linkName,
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message) override
      {
        GTEST_LOG_(INFO)
            << "NullResponseManagementServiceEndpoint::MessageReceived received on link "
            << linkName << ": " << *message;
      }
    };
  } // namespace
#endif

  TEST_F(TestManagement, ManagementRequestResponse)
  {
#if ENABLE_UAMQP
    MessageTests::AmqpServerMock mockServer;
    MessageTests::MockServiceEndpointOptions managementEndpointOptions;
    managementEndpointOptions.EnableTrace = true;
    auto endpoint
        = std::make_shared<NullResponseManagementServiceEndpoint>(managementEndpointOptions);
    mockServer.AddServiceEndpoint(endpoint);

    ConnectionOptions connectionOptions;
    connectionOptions.Port = GetPort();

    auto sasCredential = std::make_shared<ServiceBusSasConnectionStringCredential>(
        "Endpoint=amqp://localhost:" + std::to_string(GetPort())
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
    ManagementOperationResult result;
    EXPECT_ANY_THROW(
        result = management.ExecuteOperation(
            "Test",
            "Test",
            "Test",
            messageToSend,
            context.WithDeadline(std::chrono::system_clock::now() + std::chrono::seconds(2))));

    management.Close();

    mockServer.StopListening();
#else
#endif
  }
  TEST_F(TestManagement, ManagementRequestResponseSimple)
  {
#if ENABLE_UAMQP
    auto managementEndpoint
        = std::make_shared<ManagementServiceEndpoint>(MessageTests::MockServiceEndpointOptions{});
    m_mockServer.AddServiceEndpoint(managementEndpoint);

    ConnectionOptions connectionOptions;
    connectionOptions.EnableTrace = true;
    connectionOptions.Port = GetPort();

    Connection connection{CreateAmqpConnection()};
    Session session{CreateAmqpSession(connection)};
    ManagementClientOptions options;
    options.EnableTrace = true;
    ManagementClient management(session.CreateManagementClient("Test", options));

    StartServerListening();

    auto openResult = management.Open();
    ASSERT_EQ(openResult, ManagementOpenStatus::Ok);

    AmqpMessage messageToSend;
    messageToSend.SetBody(AmqpValue("Test"));

    auto response = management.ExecuteOperation("Test", "Test", "Test", messageToSend);
    EXPECT_EQ(response.Status, ManagementOperationStatus::Ok);
    EXPECT_EQ(response.StatusCode, 200);
    EXPECT_EQ(response.Error.Description, "Successful");
    management.Close();

    StopServerListening();

    EndAmqpSession(session);
    CloseAmqpConnection(connection);
#else
#endif
  }

  TEST_F(TestManagement, ManagementRequestResponseExpect500)
  {
#if ENABLE_UAMQP

    auto managementEndpoint
        = std::make_shared<ManagementServiceEndpoint>(MessageTests::MockServiceEndpointOptions{});
    m_mockServer.AddServiceEndpoint(managementEndpoint);

    ConnectionOptions connectionOptions;
    connectionOptions.EnableTrace = true;
    connectionOptions.Port = GetPort();

    auto connection{CreateAmqpConnection()};

    auto session{CreateAmqpSession(connection)};

    ManagementClientOptions options;
    options.EnableTrace = true;
    ManagementClient management(session.CreateManagementClient("Test", options));

    managementEndpoint->SetStatusCode(500);
    managementEndpoint->SetStatusDescription("Bad Things Happened.");

    StartServerListening();

    auto openResult = management.Open();
    ASSERT_EQ(openResult, ManagementOpenStatus::Ok);

    AmqpMessage messageToSend;
    messageToSend.SetBody(AmqpValue("Test"));

    auto response = management.ExecuteOperation("Test", "Test", "Test", messageToSend);
    EXPECT_EQ(response.Status, ManagementOperationStatus::FailedBadStatus);
    EXPECT_EQ(response.StatusCode, 500);
    EXPECT_EQ(response.Error.Description, "Bad Things Happened.");
    management.Close();

    StopServerListening();
#else
#endif
  }
  TEST_F(TestManagement, ManagementRequestResponseBogusStatusCode)
  {
#if ENABLE_UAMQP
    // Send a response with a bogus status code type.
    MessageTests::AmqpServerMock mockServer;
    auto managementEndpoint
        = std::make_shared<ManagementServiceEndpoint>(MessageTests::MockServiceEndpointOptions{});
    mockServer.AddServiceEndpoint(managementEndpoint);

    ConnectionOptions connectionOptions;
    connectionOptions.EnableTrace = true;
    connectionOptions.Port = GetPort();
    Connection connection("localhost", nullptr, connectionOptions);
    Session session{connection.CreateSession({})};
    ManagementClientOptions options;
    options.EnableTrace = true;
    ManagementClient management(session.CreateManagementClient("Test", options));

    // Set the response status code to something other than an int - that will cause the
    // response to be rejected by the management client.
    managementEndpoint->SetStatusCode(500u);
    managementEndpoint->SetStatusDescription("Bad Things Happened.");
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
#else
#endif
  }
  TEST_F(TestManagement, ManagementRequestResponseBogusStatusName)
  {
#if ENABLE_UAMQP
    // Send a response to the request with a bogus status code name.
    MessageTests::AmqpServerMock mockServer;
    auto managementEndpoint
        = std::make_shared<ManagementServiceEndpoint>(MessageTests::MockServiceEndpointOptions{});
    mockServer.AddServiceEndpoint(managementEndpoint);

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
    connectionOptions.Port = GetPort();

    Connection connection("localhost", nullptr, connectionOptions);
    Session session{connection.CreateSession({})};
    ManagementClientOptions options;
    options.EnableTrace = true;

    ManagementClient management(session.CreateManagementClient("Test", options, &managementEvents));

    // Set the response status code to something other than an int - that will cause the
    // response to be rejected by the management client.
    managementEndpoint->SetStatusCode(500);
    managementEndpoint->SetStatusCodeName("status-code");
    managementEndpoint->SetStatusDescription("Bad Things Happened.");
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
#else
#endif
  }
  TEST_F(TestManagement, ManagementRequestResponseBogusStatusName2)
  {
#if ENABLE_UAMQP
    // Send a response to the request with a bogus status code name.
    MessageTests::AmqpServerMock mockServer;
    auto managementEndpoint
        = std::make_shared<ManagementServiceEndpoint>(MessageTests::MockServiceEndpointOptions{});
    mockServer.AddServiceEndpoint(managementEndpoint);

    ConnectionOptions connectionOptions;
    connectionOptions.EnableTrace = true;
    connectionOptions.Port = GetPort();
    Connection connection("localhost", nullptr, connectionOptions);
    Session session{connection.CreateSession({})};
    ManagementClientOptions options;
    options.EnableTrace = true;
    options.ExpectedStatusCodeKeyName = "status-code";

    ManagementClient management(session.CreateManagementClient("Test", options));

    // Set the response status code to something other than an int - that will cause the
    // response to be rejected by the management client.
    managementEndpoint->SetStatusCode(235);
    managementEndpoint->SetStatusCodeName("status-code");
    managementEndpoint->SetStatusDescription("Bad Things Happened..");
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
#else
#endif
  }

  TEST_F(TestManagement, ManagementRequestResponseUnknownOperationName)
  {
#if ENABLE_UAMQP
    // Send a management request with an unknown operation name.
    MessageTests::AmqpServerMock mockServer;
    MessageTests::MockServiceEndpointOptions managementEndpointOptions;
    auto managementEndpoint
        = std::make_shared<ManagementServiceEndpoint>(managementEndpointOptions);
    mockServer.AddServiceEndpoint(managementEndpoint);

    mockServer.StartListening();

    ConnectionOptions connectionOptions;
    connectionOptions.EnableTrace = true;
    connectionOptions.Port = GetPort();
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
        Azure::Core::Context{}.WithDeadline(
            std::chrono::system_clock::now() + std::chrono::seconds(10)));
    EXPECT_EQ(response.Status, ManagementOperationStatus::Error);
    EXPECT_EQ(response.StatusCode, 500);
    EXPECT_EQ(response.Error.Description, "Unknown Request operation");
    management.Close();

    mockServer.StopListening();
#else
#endif
  }
#endif // !defined(AZ_PLATFORM_MAC)
}}}} // namespace Azure::Core::Amqp::Tests
