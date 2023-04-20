#include "azure/messaging/eventhubs/producer_client.hpp"
#include <azure/core/amqp.hpp>

Azure::Messaging::EventHubs::ProducerClient::ProducerClient(
    std::string connectionString,
    std::string eventHub,
    ProducerClientOptions options)
    : m_retryOptions(options.RetryOptions), m_credentials{connectionString, "", nullptr, eventHub}
{

  auto credentials
      = std::make_shared<Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential>(
          m_credentials.ConnectionString);
  m_credentials.EventHub
      = (credentials->GetEntityPath().empty() ? m_credentials.EventHub
                                              : credentials->GetEntityPath());
  std::string targetUrl = "amqps://" + credentials->GetHostName() + "/" + m_credentials.EventHub;

  Azure::Core::Amqp::_internal::ConnectionOptions connectOptions;
  connectOptions.ContainerId = options.ApplicationID;
  connectOptions.EnableTrace = options.SenderOptions.EnableTrace;
  connectOptions.HostName = credentials->GetHostName();

   Azure::Core::Amqp::_internal::Connection connection(targetUrl, connectOptions, nullptr);
  Azure::Core::Amqp::_internal::Session session(connection, nullptr);
  session.SetIncomingWindow(std::numeric_limits<int32_t>::max());
  session.SetOutgoingWindow(std::numeric_limits<uint16_t>::max());

  m_sender = Azure::Core::Amqp::_internal::MessageSender(
      session,
      credentials,
      targetUrl,
      options.SenderOptions,
      nullptr);

  m_sender.Open();
}

Azure::Messaging::EventHubs::ProducerClient::ProducerClient(
    std::string fullyQualifiedNamespace,
    std::string eventHub,
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
    ProducerClientOptions options)
    : m_credentials{"", fullyQualifiedNamespace, credential, eventHub}
{
  Azure::Core::Amqp::_internal::ConnectionOptions connectOptions;
  connectOptions.ContainerId = options.ApplicationID;
  connectOptions.EnableTrace = true;
  connectOptions.HostName = m_credentials.FullyQualifiedNamespace;

  std::string targetUrl
      = "amqps://" + m_credentials.FullyQualifiedNamespace + "/" + m_credentials.EventHub;

  Azure::Core::Amqp::_internal::Connection connection(targetUrl, connectOptions, nullptr);
  Azure::Core::Amqp::_internal::Session session(connection, nullptr);
  session.SetIncomingWindow(std::numeric_limits<int32_t>::max());
  session.SetOutgoingWindow(std::numeric_limits<uint16_t>::max());

  m_sender = Azure::Core::Amqp::_internal::MessageSender(
      session, credential, targetUrl, options.SenderOptions, nullptr);
  m_sender.Open();
}

std::vector<std::tuple<
    Azure::Core::Amqp::_internal::MessageSendResult,
    Azure::Core::Amqp::Models::AmqpValue>>
Azure::Messaging::EventHubs::ProducerClient::SendEventDataBatch(
    EventDataBatch& eventDataBatch,
    Azure::Core::Context ctx)
{
  std::vector<std::tuple<
      Azure::Core::Amqp::_internal::MessageSendResult,
      Azure::Core::Amqp::Models::AmqpValue>>
      returnValue;

  auto messages = eventDataBatch.GetMessages();
  for (int i = 0; i < messages.size(); i++)
  {
    auto result = m_sender.Send(messages[0]);
    returnValue.emplace_back(result);
  }
  return returnValue;
}