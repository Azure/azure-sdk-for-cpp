// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "queue_client_test.hpp"

#include <chrono>
#include <thread>

namespace Azure { namespace Storage { namespace Test {

  TEST_F(QueueClientTest, EnqueueMessage)
  {
    auto queueClient = Azure::Storage::Queues::QueueClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_testNameLowercase, m_options);
    queueClient.Create();

    const std::string message = "message content.";
    auto res = queueClient.EnqueueMessage(message).Value;
    EXPECT_FALSE(res.MessageId.empty());
    EXPECT_TRUE(IsValidTime(res.InsertedOn));
    EXPECT_TRUE(IsValidTime(res.ExpiresOn - std::chrono::hours(24 * 7)));
    EXPECT_FALSE(res.PopReceipt.empty());
    EXPECT_TRUE(IsValidTime(res.NextVisibleOn));

    auto peekedMessage = queueClient.PeekMessages().Value.Messages[0];

    EXPECT_EQ(peekedMessage.MessageText, message);
    EXPECT_EQ(peekedMessage.MessageId, res.MessageId);
    EXPECT_EQ(peekedMessage.InsertedOn, res.InsertedOn);
    EXPECT_EQ(peekedMessage.ExpiresOn, res.ExpiresOn);

    queueClient.Delete();
  }

  TEST_F(QueueClientTest, EnqueueMessageTTL)
  {
    auto queueClient = Azure::Storage::Queues::QueueClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_testNameLowercase, m_options);
    queueClient.Create();

    const std::string message = "message content.";
    Queues::EnqueueMessageOptions enqueueOptions;
    enqueueOptions.VisibilityTimeout = std::chrono::seconds(1);
    enqueueOptions.TimeToLive = std::chrono::seconds(2);
    auto res = queueClient.EnqueueMessage(message, enqueueOptions).Value;

    EXPECT_TRUE(queueClient.PeekMessages().Value.Messages.empty());
    TestSleep(std::chrono::milliseconds(1200));
    EXPECT_FALSE(queueClient.PeekMessages().Value.Messages.empty());
    TestSleep(std::chrono::milliseconds(1200));
    EXPECT_TRUE(queueClient.PeekMessages().Value.Messages.empty());

    enqueueOptions = Queues::EnqueueMessageOptions();
    enqueueOptions.TimeToLive = Queues::EnqueueMessageOptions::MessageNeverExpires;
    res = queueClient.EnqueueMessage(message, enqueueOptions).Value;

    const Azure::DateTime neverExpireDateTime = Azure::DateTime::Parse(
        "Fri, 31 Dec 9999 23:59:59 GMT", Azure::DateTime::DateFormat::Rfc1123);

    EXPECT_EQ(res.ExpiresOn, neverExpireDateTime);
    auto peekRes = queueClient.PeekMessages();
    ASSERT_FALSE(peekRes.Value.Messages.empty());
    EXPECT_EQ(peekRes.Value.Messages[0].ExpiresOn, neverExpireDateTime);
    auto receiveRes = queueClient.ReceiveMessages();
    ASSERT_FALSE(receiveRes.Value.Messages.empty());
    EXPECT_EQ(receiveRes.Value.Messages[0].ExpiresOn, neverExpireDateTime);

    queueClient.Delete();
  }

  TEST_F(QueueClientTest, ReceiveMessage)
  {
    auto queueClient = Azure::Storage::Queues::QueueClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_testNameLowercase, m_options);
    queueClient.Create();

    EXPECT_TRUE(queueClient.ReceiveMessages().Value.Messages.empty());

    const std::string message = "message content.";
    auto res = queueClient.EnqueueMessage(message).Value;

    Queues::ReceiveMessagesOptions receiveOptions;
    receiveOptions.VisibilityTimeout = std::chrono::seconds(1);
    auto receivedMessage = queueClient.ReceiveMessages(receiveOptions).Value.Messages[0];

    EXPECT_EQ(receivedMessage.MessageText, message);
    EXPECT_EQ(receivedMessage.MessageId, res.MessageId);
    EXPECT_EQ(receivedMessage.InsertedOn, res.InsertedOn);
    EXPECT_EQ(receivedMessage.ExpiresOn, res.ExpiresOn);
    EXPECT_FALSE(receivedMessage.PopReceipt.empty());
    EXPECT_TRUE(IsValidTime(receivedMessage.NextVisibleOn));
    EXPECT_EQ(receivedMessage.DequeueCount, 1);

    TestSleep(std::chrono::milliseconds(1200));
    receivedMessage = queueClient.ReceiveMessages().Value.Messages[0];
    EXPECT_EQ(receivedMessage.DequeueCount, 2);

    queueClient.Delete();
  }

  TEST_F(QueueClientTest, ReceiveMessages)
  {
    auto queueClient = Azure::Storage::Queues::QueueClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_testNameLowercase, m_options);
    queueClient.Create();

    Queues::ReceiveMessagesOptions receiveOptions;
    receiveOptions.MaxMessages = 1;
    auto receivedMessages = queueClient.ReceiveMessages(receiveOptions).Value.Messages;
    EXPECT_TRUE(receivedMessages.empty());
    receivedMessages = queueClient.ReceiveMessages().Value.Messages;
    EXPECT_TRUE(receivedMessages.empty());

    const std::string message1 = "message content.1";
    const std::string message2 = "message content.2";
    const std::string message3 = "message content.3";
    const std::string message4 = "message content.4";
    queueClient.EnqueueMessage(message1);
    queueClient.EnqueueMessage(message2);
    queueClient.EnqueueMessage(message3);
    queueClient.EnqueueMessage(message4);

    receiveOptions.MaxMessages = 1;
    receivedMessages = queueClient.ReceiveMessages(receiveOptions).Value.Messages;

    EXPECT_EQ(receivedMessages.size(), static_cast<size_t>(receiveOptions.MaxMessages.Value()));
    EXPECT_EQ(receivedMessages[0].MessageText, message1);

    receivedMessages = queueClient.ReceiveMessages().Value.Messages;
    EXPECT_EQ(receivedMessages.size(), size_t(1));
    EXPECT_EQ(receivedMessages[0].MessageText, message2);

    receiveOptions.MaxMessages = 10;
    receivedMessages = queueClient.ReceiveMessages(receiveOptions).Value.Messages;
    EXPECT_EQ(receivedMessages.size(), size_t(2));
    EXPECT_EQ(receivedMessages[0].MessageText, message3);
    EXPECT_EQ(receivedMessages[1].MessageText, message4);

    queueClient.Delete();
  }

  TEST_F(QueueClientTest, PeekMessage)
  {
    auto queueClient = Azure::Storage::Queues::QueueClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_testNameLowercase, m_options);
    queueClient.Create();

    EXPECT_TRUE(queueClient.PeekMessages().Value.Messages.empty());

    const std::string message = "message content.";
    auto res = queueClient.EnqueueMessage(message).Value;

    auto peekedMessage = queueClient.PeekMessages().Value.Messages[0];

    EXPECT_EQ(peekedMessage.MessageText, message);
    EXPECT_EQ(peekedMessage.MessageId, res.MessageId);
    EXPECT_EQ(peekedMessage.InsertedOn, res.InsertedOn);
    EXPECT_EQ(peekedMessage.ExpiresOn, res.ExpiresOn);
    EXPECT_EQ(peekedMessage.DequeueCount, 0);

    queueClient.Delete();
  }

  TEST_F(QueueClientTest, PeekMessages)
  {
    auto queueClient = Azure::Storage::Queues::QueueClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_testNameLowercase, m_options);
    queueClient.Create();

    Queues::PeekMessagesOptions peekOptions;
    peekOptions.MaxMessages = 1;
    auto peekedMessages = queueClient.PeekMessages(peekOptions).Value.Messages;
    EXPECT_TRUE(peekedMessages.empty());
    peekedMessages = queueClient.PeekMessages().Value.Messages;
    EXPECT_TRUE(peekedMessages.empty());

    const std::string message1 = "message content.1";
    const std::string message2 = "message content.2";
    const std::string message3 = "message content.3";
    const std::string message4 = "message content.4";
    queueClient.EnqueueMessage(message1);
    queueClient.EnqueueMessage(message2);
    queueClient.EnqueueMessage(message3);
    queueClient.EnqueueMessage(message4);

    peekOptions.MaxMessages = 1;
    peekedMessages = queueClient.PeekMessages(peekOptions).Value.Messages;

    EXPECT_EQ(peekedMessages.size(), static_cast<size_t>(peekOptions.MaxMessages.Value()));
    EXPECT_EQ(peekedMessages[0].MessageText, message1);

    peekedMessages = queueClient.PeekMessages().Value.Messages;
    EXPECT_EQ(peekedMessages.size(), size_t(1));
    EXPECT_EQ(peekedMessages[0].MessageText, message1);

    peekOptions.MaxMessages = 10;
    peekedMessages = queueClient.PeekMessages(peekOptions).Value.Messages;
    EXPECT_EQ(peekedMessages.size(), size_t(4));
    EXPECT_EQ(peekedMessages[0].MessageText, message1);
    EXPECT_EQ(peekedMessages[1].MessageText, message2);
    EXPECT_EQ(peekedMessages[2].MessageText, message3);
    EXPECT_EQ(peekedMessages[3].MessageText, message4);

    queueClient.Delete();
  }

  TEST_F(QueueClientTest, UpdateMessage)
  {
    auto queueClient = Azure::Storage::Queues::QueueClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_testNameLowercase, m_options);
    queueClient.Create();

    const std::string message = "message content.";
    const std::string updatedMessage = "MESSAGE CONTENT2";
    auto res = queueClient.EnqueueMessage(message).Value;

    auto updateRes
        = queueClient.UpdateMessage(res.MessageId, res.PopReceipt, std::chrono::seconds(0)).Value;
    EXPECT_FALSE(updateRes.PopReceipt.empty());
    EXPECT_TRUE(IsValidTime(updateRes.NextVisibleOn));

    auto peekedMessage = queueClient.PeekMessages().Value.Messages[0];
    EXPECT_EQ(peekedMessage.MessageText, message);

    Queues::UpdateMessageOptions updateOptions;
    updateOptions.MessageText = updatedMessage;
    queueClient.UpdateMessage(
        res.MessageId, updateRes.PopReceipt, std::chrono::seconds(1), updateOptions);
    EXPECT_TRUE(queueClient.PeekMessages().Value.Messages.empty());

    TestSleep(std::chrono::milliseconds(1200));
    peekedMessage = queueClient.PeekMessages().Value.Messages[0];
    EXPECT_EQ(peekedMessage.MessageText, updatedMessage);

    queueClient.Delete();
  }

  TEST_F(QueueClientTest, DeleteMessage)
  {
    auto queueClient = Azure::Storage::Queues::QueueClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_testNameLowercase, m_options);
    queueClient.Create();

    const std::string message = "message content.";
    auto res = queueClient.EnqueueMessage(message).Value;

    EXPECT_NO_THROW(queueClient.DeleteMessage(res.MessageId, res.PopReceipt));
    EXPECT_TRUE(queueClient.PeekMessages().Value.Messages.empty());

    queueClient.Delete();
  }

  TEST_F(QueueClientTest, ClearMessages)
  {
    auto queueClient = Azure::Storage::Queues::QueueClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_testNameLowercase, m_options);
    queueClient.Create();

    const std::string message = "message content.";
    queueClient.EnqueueMessage(message);

    EXPECT_NO_THROW(queueClient.ClearMessages());
    EXPECT_TRUE(queueClient.PeekMessages().Value.Messages.empty());

    queueClient.Delete();
  }

  TEST_F(QueueClientTest, MessageSpecialCharacters)
  {
    auto queueClient = Azure::Storage::Queues::QueueClient::CreateFromConnectionString(
        StandardStorageConnectionString(), m_testNameLowercase, m_options);
    queueClient.Create();

    const std::string message = "message content`~!@#$%^&*()-=_+[]{}\\|;':\",.<>/?";

    auto res = queueClient.EnqueueMessage(message).Value;

    auto peekedMessage = queueClient.PeekMessages().Value.Messages[0];

    EXPECT_EQ(peekedMessage.MessageText, message);

    queueClient.Delete();
  }

}}} // namespace Azure::Storage::Test