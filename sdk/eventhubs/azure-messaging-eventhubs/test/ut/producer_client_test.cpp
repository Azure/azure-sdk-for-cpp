// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words

#include "eventhubs_test_base.hpp"

#include <azure/core/amqp/internal/connection_string_credential.hpp>
#include <azure/core/context.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/core/uuid.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <atomic>
#include <chrono>
#include <numeric>
#include <thread>

#include <gtest/gtest.h>

// cspell: ignore edboptions

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {

  class ProducerClientTest : public EventHubsTestBaseParameterized {
  protected:
    std::string GetEventHubName()
    {
      if (GetParam() == AuthType::Emulator)
      {
        return "eh1";
      }
      return GetEnv("EVENTHUB_NAME");
    }
  };

  TEST_P(ProducerClientTest, SimpleProducerClient)
  {
    std::string const connStringNoEntityPath = GetEnv("EVENTHUBS_HOST");
    std::string eventHubName{GetEnv("EVENTHUB_NAME")};

    Azure::Messaging::EventHubs::ProducerClient client{
        connStringNoEntityPath, eventHubName, GetTestCredential()};
    EXPECT_EQ(eventHubName, client.GetEventHubName());
  }

  TEST_P(ProducerClientTest, SendMessage_LIVEONLY_)
  {
    Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
    producerOptions.Name = "sender-link";
    producerOptions.ApplicationID = "some";

    auto client{CreateProducerClient("", producerOptions)};

    auto message2{std::make_shared<Azure::Core::Amqp::Models::AmqpMessage>()};
    Azure::Messaging::EventHubs::Models::EventData message1;
    message2->SetBody(Azure::Core::Amqp::Models::AmqpValue("Hello7"));

    message1.Body = {'H', 'e', 'l', 'l', 'o', '2'};

    Azure::Messaging::EventHubs::Models::EventData message3;
    message3.Body = {'H', 'e', 'l', 'l', 'o', '3'};

    Azure::Messaging::EventHubs::EventDataBatchOptions edboptions;
    edboptions.MaxBytes = (std::numeric_limits<uint16_t>::max)();
    edboptions.PartitionId = "1";
    Azure::Messaging::EventHubs::EventDataBatch eventBatch{client->CreateBatch(edboptions)};

    Azure::Messaging::EventHubs::EventDataBatchOptions edboptions2;
    edboptions2.MaxBytes = (std::numeric_limits<uint16_t>::max)();
    ;
    edboptions2.PartitionId = "2";
    Azure::Messaging::EventHubs::EventDataBatch eventBatch2{client->CreateBatch(edboptions2)};

    EXPECT_TRUE(eventBatch.TryAdd(message1));
    EXPECT_TRUE(eventBatch.TryAdd(message2));

    EXPECT_TRUE(eventBatch2.TryAdd(message3));
    EXPECT_TRUE(eventBatch2.TryAdd(message2));

    for (int i = 0; i < 5; i++)
    {
      EXPECT_NO_THROW(client->Send(eventBatch));
    }
  }

  TEST_P(ProducerClientTest, EventHubRawMessageSend_LIVEONLY_)
  {
    Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
    producerOptions.Name = "sender-link";
    producerOptions.ApplicationID = "some";

    auto client{CreateProducerClient("", producerOptions)};

    client->Send(Azure::Messaging::EventHubs::Models::EventData{"This is a test message"});

    // Send using the implicit EventData constructor.
    client->Send(std::string{"String test message"});

    // Send using a vector of implicit EventData constructor with a binary buffer.
    client->Send({{12, 13, 14, 15}, {16, 17, 18, 19}});
  }

  TEST_P(ProducerClientTest, GetEventHubProperties_LIVEONLY_)
  {
    Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
    producerOptions.Name = "sender-link";
    producerOptions.ApplicationID = "some";

    auto client{CreateProducerClient("", producerOptions)};

    auto result = client->GetEventHubProperties();
    EXPECT_EQ(result.Name, GetEventHubName());
    EXPECT_TRUE(result.PartitionIds.size() > 0);
    client->Close();
  }

  TEST_P(ProducerClientTest, GetPartitionProperties_LIVEONLY_)
  {
    Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
    producerOptions.Name = "sender-link";
    producerOptions.ApplicationID = "some";

    auto client{CreateProducerClient("", producerOptions)};

    ASSERT_NO_THROW([&]() {
      auto result = client->GetPartitionProperties("0");
      EXPECT_EQ(result.Name, GetEventHubName());
      EXPECT_EQ(result.PartitionId, "0");
    }());
  }

  TEST_P(ProducerClientTest, GetEventHubProperties_Multithreaded_LIVEONLY_)
  {
    Azure::Messaging::EventHubs::ProducerClientOptions options;
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();

    options.Name = testing::UnitTest::GetInstance()->current_test_case()->name();
    auto client(CreateProducerClient("", options));

    std::string eventHubName{GetEventHubName()};

    std::vector<std::thread> threads;
    std::vector<size_t> iterationsPerThread;
    for (int i = 0; i < 20; i++)
    {
      threads.emplace_back([&client, eventHubName, &iterationsPerThread]() {
        size_t iterations = 0;
        std::chrono::system_clock::duration timeout = std::chrono::seconds(3);
        std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
        while ((std::chrono::system_clock::now() - start) <= timeout)
        {
          Azure::Messaging::EventHubs::Models::EventHubProperties result;
          ASSERT_NO_THROW(result = client->GetEventHubProperties());
          EXPECT_EQ(result.Name, eventHubName);
          EXPECT_TRUE(result.PartitionIds.size() > 0);
          std::this_thread::yield();
          iterations++;
        }
        iterationsPerThread.push_back(iterations);
      });
    }
    GTEST_LOG_(INFO) << "Waiting for threads to finish.";
    for (auto& t : threads)
    {
      if (t.joinable())
      {
        t.join();
      }
    }
    GTEST_LOG_(INFO) << "Threads finished.";
    for (const auto i : iterationsPerThread)
    {
      GTEST_LOG_(INFO) << "Thread iterations: " << i;
    }
  }

  constexpr size_t threadsPerPartition = 20;

  TEST_P(ProducerClientTest, GetPartitionProperties_Multithreaded_LIVEONLY_)
  {
    Azure::Messaging::EventHubs::ProducerClientOptions options;
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();

    options.Name = testing::UnitTest::GetInstance()->current_test_case()->name();
    auto client{CreateProducerClient()};

    auto ehProperties = client->GetEventHubProperties();
    std::mutex iterationLock;
    std::vector<std::thread> threads;
    std::map<std::thread::id, size_t> iterationsPerThread;
    for (const auto& partition : ehProperties.PartitionIds)
    {
      threads.emplace_back(
          std::thread([&client, partition, ehProperties, &iterationsPerThread, &iterationLock]() {
            GTEST_LOG_(INFO) << "Thread started for partition: " << partition << ".\n";
            GTEST_LOG_(INFO) << "Start " << threadsPerPartition
                             << " threads to retrieve properties.";
            for (size_t i = 0; i < threadsPerPartition; i++)
            {
              std::vector<std::thread> partitionThreads;
              partitionThreads.emplace_back(
                  [&client, &partition, ehProperties, &iterationsPerThread, &iterationLock]() {
                    size_t iterations = 0;
                    auto threadId = std::this_thread::get_id();
                    std::chrono::system_clock::duration timeout = std::chrono::seconds(3);
                    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

                    while ((std::chrono::system_clock::now() - start) <= timeout)
                    {
                      Azure::Messaging::EventHubs::Models::EventHubPartitionProperties result;
                      ASSERT_NO_THROW(result = client->GetPartitionProperties(partition));
                      EXPECT_EQ(result.Name, ehProperties.Name);
                      EXPECT_EQ(result.PartitionId, partition);
                      // Attempt to avoid service throttling.
                      std::this_thread::sleep_for(std::chrono::milliseconds(100));
                      iterations++;
                    }
                    {
                      std::unique_lock<std::mutex> lock(iterationLock);
                      iterationsPerThread.emplace(threadId, iterations);
                    }
                  });
              for (auto& t : partitionThreads)
              {
                if (t.joinable())
                {
                  t.join();
                }
              }
            }
            GTEST_LOG_(INFO) << "Threads finished for partition: " << partition << ".\n";
          }));
    }
    GTEST_LOG_(INFO) << "Waiting for threads to finish.";
    for (auto& t : threads)
    {
      if (t.joinable())
      {
        t.join();
      }
    }
    GTEST_LOG_(INFO) << "Threads finished.";
    GTEST_LOG_(INFO) << iterationsPerThread.size() << " threads finished.";
    for (const auto i : iterationsPerThread)
    {
      GTEST_LOG_(INFO) << "Thread iterations: " << i.second;
    }
  }

  // Send a batch that has a partition key and make sure that every event in the batch landed on one
  // partition. The Event Hubs service routes on the partition key annotation in the message
  // annotations of the batch envelope, so this test fails if the annotation is missing or if it is
  // in the delivery annotations.
  TEST_P(ProducerClientTest, SendBatchWithPartitionKey_LIVEONLY_)
  {
    constexpr uint32_t eventCount = 20;

    auto client{CreateProducerClient()};

    auto const partitionIds = client->GetEventHubProperties().PartitionIds;
    ASSERT_GT(partitionIds.size(), 1ul) << "This test needs more than one partition.";

    std::string const partitionKey{"ws5-" + Azure::Core::Uuid::CreateUuid().ToString()};

    std::map<std::string, int64_t> sequenceNumberBeforeSend;
    for (auto const& partitionId : partitionIds)
    {
      sequenceNumberBeforeSend[partitionId]
          = client->GetPartitionProperties(partitionId).LastEnqueuedSequenceNumber;
      // Attempt to avoid service throttling.
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    {
      Azure::Messaging::EventHubs::EventDataBatchOptions batchOptions;
      batchOptions.MaxBytes = (std::numeric_limits<uint16_t>::max)();
      batchOptions.PartitionKey = partitionKey;
      Azure::Messaging::EventHubs::EventDataBatch eventBatch{client->CreateBatch(batchOptions)};
      for (uint32_t i = 0; i < eventCount; i++)
      {
        // The body starts with the unique marker of this run. The test finds its own events by
        // that marker, so the check on the partition key below stays independent.
        EXPECT_TRUE(eventBatch.TryAdd(Azure::Messaging::EventHubs::Models::EventData{
            partitionKey + " message " + std::to_string(i)}));
      }
      // Stop here if the send fails, because the partition counts below would then report a
      // misleading failure.
      ASSERT_NO_THROW(client->Send(eventBatch));
    }

    // Give the service time to report the new events.
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Read every partition from the sequence number that it had before the send, and keep only the
    // events of this batch. The unique marker in the body identifies them. A count of new events
    // cannot do this, because another producer can write to the same Event Hub while this test
    // runs and can add events to any partition.
    auto consumer{CreateConsumerClient()};
    std::vector<std::string> partitionsWithBatch;
    std::vector<std::shared_ptr<const Azure::Messaging::EventHubs::Models::ReceivedEventData>>
        batchEvents;

    for (auto const& partitionId : partitionIds)
    {
      Azure::Messaging::EventHubs::PartitionClientOptions partitionOptions;
      partitionOptions.StartPosition.SequenceNumber = sequenceNumberBeforeSend[partitionId];
      auto receiver = consumer->CreatePartitionClient(partitionId, partitionOptions);

      // ReceiveEvents returns as soon as the receiver queue is empty, so one call can return fewer
      // events than the batch holds. Read until this partition holds the whole batch, or until the
      // partition stays quiet, or until the time runs out.
      std::vector<std::shared_ptr<const Azure::Messaging::EventHubs::Models::ReceivedEventData>>
          eventsOnPartition;
      auto const deadline = std::chrono::system_clock::now() + std::chrono::seconds(60);
      int quietReads = 0;
      while (eventsOnPartition.size() < eventCount && quietReads < 6
             && std::chrono::system_clock::now() < deadline)
      {
        auto batchOfEvents = receiver.ReceiveEvents(eventCount);
        if (batchOfEvents.empty())
        {
          quietReads++;
          std::this_thread::sleep_for(std::chrono::milliseconds(500));
          continue;
        }
        quietReads = 0;
        for (auto const& receivedEvent : batchOfEvents)
        {
          std::string const body(receivedEvent->Body.begin(), receivedEvent->Body.end());
          if (body.rfind(partitionKey, 0) == 0)
          {
            eventsOnPartition.push_back(receivedEvent);
          }
        }
      }

      GTEST_LOG_(INFO) << "Partition " << partitionId << " holds " << eventsOnPartition.size()
                       << " events of this batch.";
      if (!eventsOnPartition.empty())
      {
        partitionsWithBatch.push_back(partitionId);
        batchEvents.insert(batchEvents.end(), eventsOnPartition.begin(), eventsOnPartition.end());
      }
    }

    // Before the fix the service spread the batch over every partition, so more than one partition
    // holds events of the batch and this assertion fails.
    ASSERT_EQ(1ul, partitionsWithBatch.size())
        << "A batch with a partition key must land on exactly one partition.";
    ASSERT_EQ(static_cast<size_t>(eventCount), batchEvents.size())
        << "The partition must hold every event of the batch.";

    // The marker in the body found these events, so this check on the partition key does not
    // depend on the way the test found them.
    for (auto const& receivedEvent : batchEvents)
    {
      ASSERT_TRUE(receivedEvent->PartitionKey);
      EXPECT_EQ(partitionKey, receivedEvent->PartitionKey.Value());
    }
  }

#if ENABLE_UAMQP
  namespace {
    // Counts the tokens that the test asked for, and can report a shorter life
    // for each token than the real life.
    //
    // A real Microsoft Entra token lives for 60 to 90 minutes, so a test that
    // waits for a true expiry would run for hours. A shorter reported life makes
    // the connection refresh the token in seconds. The token itself is a real
    // token and stays valid, so the service still accepts it.
    class CountingTokenCredential final : public Azure::Core::Credentials::TokenCredential {
    public:
      CountingTokenCredential(
          std::shared_ptr<const Azure::Core::Credentials::TokenCredential> innerCredential,
          Azure::Nullable<std::chrono::seconds> reportedLifetime = {})
          // Report the name of the credential that this one wraps. The
          // connection asks for that name to decide whether it puts a shared
          // access signature token or a JWT, so a new name here would break a
          // connection string credential.
          : Azure::Core::Credentials::TokenCredential(innerCredential->GetCredentialName()),
            m_innerCredential(std::move(innerCredential)), m_reportedLifetime(reportedLifetime)
      {
      }

      int GetTokenCount() const { return m_tokenCount.load(); }

    private:
      std::shared_ptr<const Azure::Core::Credentials::TokenCredential> m_innerCredential;
      Azure::Nullable<std::chrono::seconds> m_reportedLifetime;
      mutable std::atomic<int> m_tokenCount{0};

      Azure::Core::Credentials::AccessToken GetToken(
          Azure::Core::Credentials::TokenRequestContext const& requestContext,
          Azure::Core::Context const& context) const override
      {
        auto token = m_innerCredential->GetToken(requestContext, context);
        if (m_reportedLifetime.HasValue())
        {
          token.ExpiresOn = std::chrono::system_clock::now() + m_reportedLifetime.Value();
        }
        m_tokenCount++;
        return token;
      }
    };

    // The connection replaces a token one buffer, seven minutes, before the
    // token expires. A token that reports seven minutes and thirty seconds of
    // life is due in about thirty seconds. That gap is long enough to see the
    // refresh and then to check that the refreshed token is the one in use.
    constexpr std::chrono::seconds SlowRefreshLifetime{450};

    // A token that reports less life than the buffer is due as soon as it
    // arrives, so the refresh thread replaces it every twenty seconds, which is
    // the smallest interval between two refresh passes. This drives the tests
    // that need several refreshes in a short run.
    constexpr std::chrono::seconds FastRefreshLifetime{80};

    Azure::Messaging::EventHubs::Models::EventData MakeTestEvent(std::uint8_t tag)
    {
      Azure::Messaging::EventHubs::Models::EventData event;
      event.Body = {'T', tag};
      return event;
    }

    // Return the credential that the refresh tests wrap and count.
    //
    // When the environment gives a connection string, build a shared access
    // signature credential from it. That path needs no Microsoft Entra sign in,
    // so a developer can run these tests from a workstation. Otherwise use the
    // credential of the test harness, which is what the pipeline uses.
    std::shared_ptr<const Azure::Core::Credentials::TokenCredential> MakeInnerCredential(
        std::shared_ptr<const Azure::Core::Credentials::TokenCredential> harnessCredential)
    {
      auto const connectionString
          = Azure::Core::_internal::Environment::GetVariable("EVENTHUBS_CONNECTION_STRING");
      if (!connectionString.empty())
      {
        return std::make_shared<
            Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential>(
            connectionString);
      }
      return harnessCredential;
    }

    // Wait until the credential issued at least `count` tokens, or until the
    // timeout. Returns the count that was reached.
    int WaitForTokenCount(
        std::shared_ptr<CountingTokenCredential> const& credential,
        int count,
        std::chrono::seconds timeout)
    {
      auto const deadline = std::chrono::steady_clock::now() + timeout;
      while (credential->GetTokenCount() < count && std::chrono::steady_clock::now() < deadline)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
      }
      return credential->GetTokenCount();
    }
  } // namespace

  // L1. The connection must replace the CBS token before that token expires,
  // and the replacement must reach the service.
  TEST_P(ProducerClientTest, TokenRefreshBeforeExpiry_LIVEONLY_)
  {
    auto credential = std::make_shared<CountingTokenCredential>(
        MakeInnerCredential(GetTestCredential()), SlowRefreshLifetime);

    Azure::Messaging::EventHubs::ProducerClientOptions options;
    options.Name = "sender-link";
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();

    Azure::Messaging::EventHubs::ProducerClient client{
        GetEnv("EVENTHUBS_HOST"), GetEventHubName(), credential, options};

    ASSERT_NO_THROW(client.Send(MakeTestEvent('1')));
    auto const countAfterFirstSend = credential->GetTokenCount();
    ASSERT_GE(countAfterFirstSend, 1);

    // The test makes no client call here, so only the refresh thread can raise
    // the count.
    auto const countAfterRefresh
        = WaitForTokenCount(credential, countAfterFirstSend + 1, std::chrono::seconds(120));
    ASSERT_GT(countAfterRefresh, countAfterFirstSend)
        << "The connection did not replace the CBS token before the token expired.";

    // Make sure the replacement reached the service. A refresh that failed makes
    // the connection drop the cached token, and then this send has to ask the
    // credential for another token. So a send that does not raise the count
    // proves that the refreshed token is in the cache and that the service
    // accepted it.
    ASSERT_NO_THROW(client.Send(MakeTestEvent('2')));
    EXPECT_EQ(credential->GetTokenCount(), countAfterRefresh)
        << "The send asked for a new token, so the refresh did not reach the service.";
    client.Close();
  }

  // L2. The client must keep sending past two token lifetimes, while the
  // refresh thread replaces the token repeatedly. This is the continuity gate
  // from issue #7254, on a compressed clock. L1 is the test that proves a
  // refresh reaches the service; this one proves that a long run keeps working.
  TEST_P(ProducerClientTest, TokenRefreshContinuityPastTwoLifetimes_LIVEONLY_)
  {
    auto credential = std::make_shared<CountingTokenCredential>(
        MakeInnerCredential(GetTestCredential()), FastRefreshLifetime);

    Azure::Messaging::EventHubs::ProducerClientOptions options;
    options.Name = "sender-link";
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();

    Azure::Messaging::EventHubs::ProducerClient client{
        GetEnv("EVENTHUBS_HOST"), GetEventHubName(), credential, options};

    auto const runFor = FastRefreshLifetime * 2 + std::chrono::seconds(10);
    auto const start = std::chrono::steady_clock::now();
    int sendCount = 0;
    while (std::chrono::steady_clock::now() - start < runFor)
    {
      ASSERT_NO_THROW(client.Send(MakeTestEvent('3')))
          << "A send failed after " << sendCount << " sends.";
      sendCount++;
      std::this_thread::sleep_for(std::chrono::seconds(20));
    }

    EXPECT_GT(sendCount, 0);
    EXPECT_GE(credential->GetTokenCount(), 3)
        << "The connection did not refresh the token while the client was running.";
    client.Close();
  }

  // L3. Shutting the client down must stop the refresh thread, and must not
  // hang.
  TEST_P(ProducerClientTest, TokenRefreshStopsOnClose_LIVEONLY_)
  {
    auto credential = std::make_shared<CountingTokenCredential>(
        MakeInnerCredential(GetTestCredential()), FastRefreshLifetime);

    {
      Azure::Messaging::EventHubs::ProducerClientOptions options;
      options.Name = "sender-link";
      options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();

      Azure::Messaging::EventHubs::ProducerClient client{
          GetEnv("EVENTHUBS_HOST"), GetEventHubName(), credential, options};

      ASSERT_NO_THROW(client.Send(MakeTestEvent('4')));
      EXPECT_GE(WaitForTokenCount(credential, 2, std::chrono::seconds(90)), 2);

      auto const closeStart = std::chrono::steady_clock::now();
      client.Close();
      EXPECT_LT(std::chrono::steady_clock::now() - closeStart, std::chrono::seconds(70))
          << "Close did not return promptly while the refresh thread was running.";
    }

    // The client is gone, so the refresh thread is stopped. The count must not
    // move after this point.
    auto const countAfterShutdown = credential->GetTokenCount();
    std::this_thread::sleep_for(std::chrono::seconds(30));
    EXPECT_EQ(credential->GetTokenCount(), countAfterShutdown)
        << "The refresh thread kept running after the client was destroyed.";
  }

  // L4. A token with a normal life must be fetched once. This is the control
  // case. It fails if the new expiry test makes the cache authenticate again on
  // every call.
  TEST_P(ProducerClientTest, NormalTokenIsNotRefetched_LIVEONLY_)
  {
    // No reported lifetime, so the real expiry of the token applies.
    auto credential
        = std::make_shared<CountingTokenCredential>(MakeInnerCredential(GetTestCredential()));

    Azure::Messaging::EventHubs::ProducerClientOptions options;
    options.Name = "sender-link";
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();

    Azure::Messaging::EventHubs::ProducerClient client{
        GetEnv("EVENTHUBS_HOST"), GetEventHubName(), credential, options};

    ASSERT_NO_THROW(client.Send(MakeTestEvent('5')));
    auto const countAfterFirstSend = credential->GetTokenCount();

    std::this_thread::sleep_for(std::chrono::seconds(10));
    ASSERT_NO_THROW(client.Send(MakeTestEvent('6')));

    EXPECT_EQ(credential->GetTokenCount(), countAfterFirstSend)
        << "The connection asked for a new token for an audience that already had a good one.";
    client.Close();
  }

  // L5. Calls from many threads must keep working while the refresh thread
  // replaces the token. The refresh thread and the calling threads take the same
  // lock, so this is the test for a deadlock.
  TEST_P(ProducerClientTest, TokenRefreshUnderConcurrentCalls_LIVEONLY_)
  {
    auto credential = std::make_shared<CountingTokenCredential>(
        MakeInnerCredential(GetTestCredential()), FastRefreshLifetime);

    Azure::Messaging::EventHubs::ProducerClientOptions options;
    options.Name = "sender-link";
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();

    Azure::Messaging::EventHubs::ProducerClient client{
        GetEnv("EVENTHUBS_HOST"), GetEventHubName(), credential, options};

    std::string const eventHubName{GetEventHubName()};
    std::atomic<bool> failed{false};
    std::vector<std::thread> threads;
    for (int i = 0; i < 8; i++)
    {
      threads.emplace_back([&client, &eventHubName, &failed]() {
        auto const start = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start < std::chrono::seconds(60))
        {
          try
          {
            auto result = client.GetEventHubProperties();
            if (result.Name != eventHubName)
            {
              failed = true;
            }
          }
          catch (std::exception const&)
          {
            failed = true;
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
      });
    }
    for (auto& t : threads)
    {
      if (t.joinable())
      {
        t.join();
      }
    }

    EXPECT_FALSE(failed.load()) << "A call failed while the refresh thread replaced the token.";
    EXPECT_GE(credential->GetTokenCount(), 2) << "The refresh thread did not run during the test.";
    client.Close();
  }
#endif // ENABLE_UAMQP

  namespace {
    static std::string GetSuffix(const testing::TestParamInfo<AuthType>& info)
    {
      std::string stringValue = "";
      switch (info.param)
      {
        case AuthType::Key:
          stringValue = "Key_LIVEONLY_";
          break;
        case AuthType::Emulator:
          stringValue = "Emulator";
          break;
      }
      return stringValue;
    }
  } // namespace

#if ENABLE_UAMQP
  INSTANTIATE_TEST_SUITE_P(
      EventHubs,
      ProducerClientTest,
      ::testing::Values(AuthType::Key),
      GetSuffix);
#else
  INSTANTIATE_TEST_SUITE_P(
      EventHubs,
      ProducerClientTest,
      ::testing::Values(AuthType::Key),
      GetSuffix);
#endif
}}}} // namespace Azure::Messaging::EventHubs::Test
