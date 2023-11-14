// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "./test_checkpoint_store.hpp"
#include "eventhubs_test_base.hpp"

#include <azure/core/amqp/internal/common/global_state.hpp>
#include <azure/core/context.hpp>
#include <azure/core/platform.hpp>
#include <azure/core/test/test_base.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs/consumer_client.hpp>
#include <azure/messaging/eventhubs/processor.hpp>
#include <azure/messaging/eventhubs/producer_client.hpp>

#include <set>

#include <gtest/gtest.h>

// cspell: words azeventhubs

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {
  namespace {

    std::string GetRandomName(const char* baseName = "checkpoint")
    {
      std::string name = baseName;
      name.append(Azure::Core::Uuid::CreateUuid().ToString());
      return name;
    }

    class scope_guard {
      std::function<void()> m_callback;

    public:
      scope_guard(std::function<void()>&& callback) : m_callback(std::move(callback)) {}
      ~scope_guard() noexcept { m_callback(); }
    };

    // C++ implementation of Go "WaitGroup" construct.
    class WaitGroup {
    public:
      void Wait()
      {
        std::unique_lock<std::mutex> lock(m_stateLock);

        if (m_waitables == 0)
        {
          return;
        }
        m_waitComplete.wait(lock, [this]() -> bool { return m_waitables == 0; });
      }
      void AddWaiter(int32_t count = 1)
      {
        std::unique_lock<std::mutex> lock(m_stateLock);
        m_waitables += count;
      }
      void CompleteWaiter()
      {
        std::unique_lock<std::mutex> lock(m_stateLock);
        m_waitables -= 1;
        if (m_waitables == 0)
        {
          m_waitComplete.notify_all();
        }
      }

    private:
      int32_t m_waitables{};
      std::mutex m_stateLock;
      std::condition_variable m_waitComplete;
    };
  } // namespace

  class ProcessorTest : public EventHubsTestBase {
  protected:
    static void SetUpTestSuite() { EventHubsTestBase::SetUpTestSuite(); }
    static void TearDownTestSuite() { EventHubsTestBase::TearDownTestSuite(); }

    void TearDown() override
    {
      EventHubsTestBase::TearDown();
      // When the test is torn down, the global state MUST be idle. If it is not, something leaked.
      Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
    }

    void TestWithLoadBalancer(Models::ProcessorStrategy processorStrategy)
    {
      Azure::Core::Context context = Azure::Core::Context::ApplicationContext.WithDeadline(
          Azure::DateTime::clock::now() + std::chrono::minutes(5));

      std::string eventHubName{GetEnv("EVENTHUB_NAME")};
      std::string consumerGroup = GetEnv("EVENTHUB_CONSUMER_GROUP");

      std::string const connectionString = GetEnv("EVENTHUB_CONNECTION_STRING");
      Azure::Messaging::EventHubs::ConsumerClientOptions consumerClientOptions;
      consumerClientOptions.ApplicationID
          = testing::UnitTest::GetInstance()->current_test_info()->name();
      consumerClientOptions.Name = testing::UnitTest::GetInstance()->current_test_info()->name();

      std::string containerName{GetRandomName("proctest")};

      // Create the checkpoint store
      std::shared_ptr<CheckpointStore> checkpointStore{std::make_shared<TestCheckpointStore>()};
      GTEST_LOG_(INFO) << "Checkpoint store created";

      std::shared_ptr<ConsumerClient> consumerClient{std::make_shared<ConsumerClient>(
          connectionString, eventHubName, consumerGroup, consumerClientOptions)};
      GTEST_LOG_(INFO) << "Consumer Client created";

      ProcessorOptions processorOptions;
      processorOptions.LoadBalancingStrategy = processorStrategy;
      processorOptions.UpdateInterval = std::chrono::milliseconds(1000);

      Processor processor{consumerClient, checkpointStore, processorOptions};

      // Warm up the consumer client - establish connection to the server, etc.
      auto eventHubProperties = consumerClient->GetEventHubProperties(context);

      ProducerClientOptions producerOptions;
      producerOptions.Name = "Producer for LoadBalancerTest";
      ProducerClient producerClient{connectionString, eventHubName, producerOptions};

#if PROCESSOR_ON_TEST_THREAD
      std::thread processEventsThread([&]() {
        std::set<std::string> partitionsAcquired;
        std::vector<std::thread> processEventsThreads;
        // When we exit the process thread, cancel the context to unblock the processor.
        scope_guard onExit([&context] { context.Cancel(); });

        WaitGroup waitGroup;
        for (auto const& partitionId : eventHubProperties.PartitionIds)
        {
          std::shared_ptr<ProcessorPartitionClient> partitionClient
              = processor.NextPartitionClient(context);
          waitGroup.AddWaiter();
          ASSERT_EQ(partitionsAcquired.find(partitionId), partitionsAcquired.end())
              << "No previous client for " << partitionClient->PartitionId();
          processEventsThreads.push_back(
              std::thread([&waitGroup, &producerClient, partitionClient, &context, this] {
                scope_guard onExit([&] { waitGroup.CompleteWaiter(); });
                ProcessEventsForLoadBalancerTest(producerClient, partitionClient, context);
              }));
        }
        // Block until all the events have been processed.
        waitGroup.Wait();

        // And wait until all the threads have completed.
        for (auto& thread : processEventsThreads)
        {
          if (thread.joinable())
          {
            thread.join();
          }
        }
        // Stop the processor, we're done with the test.
        processor.Stop();
      });

      processor.Run(context);

      processEventsThread.join();
#else
      // Run the processor on a background thread and the test on the foreground.
      processor.Start(context);

#if ASYNC_PROCESS_EVENTS
      std::set<std::string> partitionsAcquired;
      std::vector<std::thread> processEventsThreads;
      // When we exit the process thread, cancel the context to unblock the processor.
      //      scope_guard onExit([&context] { context.Cancel(); });

      WaitGroup waitGroup;
      for (auto const& partitionId : eventHubProperties.PartitionIds)
      {
        std::shared_ptr<ProcessorPartitionClient> partitionClient
            = processor.NextPartitionClient(context);
        waitGroup.AddWaiter();
        ASSERT_EQ(partitionsAcquired.find(partitionId), partitionsAcquired.end())
            << "No previous client for " << partitionClient->PartitionId();
        processEventsThreads.push_back(
            std::thread([&waitGroup, &producerClient, partitionClient, &context, this] {
              scope_guard onExit([&] { waitGroup.CompleteWaiter(); });
              ProcessEventsForLoadBalancerTest(producerClient, partitionClient, context);
            }));
      }
      // Block until all the events have been processed.
      waitGroup.Wait();

      // And wait until all the threads have completed.
      for (auto& thread : processEventsThreads)
      {
        if (thread.joinable())
        {
          thread.join();
        }
      }
#else
      std::set<std::string> partitionsAcquired;
      for (auto const& partitionId : eventHubProperties.PartitionIds)
      {
        std::shared_ptr<ProcessorPartitionClient> partitionClient
            = processor.NextPartitionClient(context);
        ASSERT_EQ(partitionsAcquired.find(partitionId), partitionsAcquired.end())
            << "No previous client for " << partitionClient->PartitionId();

        ProcessEventsForLoadBalancerTest(producerClient, partitionClient, context);
      }
#endif
      // Stop the processor, we're done with the test.
      processor.Stop();
#endif
    }

    void TestPartitionAcquisition(Models::ProcessorStrategy processorStrategy)
    {
      Azure::Core::Context context = Azure::Core::Context::ApplicationContext.WithDeadline(
          Azure::DateTime::clock::now() + std::chrono::minutes(5));

      std::string eventHubName{GetEnv("EVENTHUB_NAME")};
      std::string consumerGroup = GetEnv("EVENTHUB_CONSUMER_GROUP");

      std::string const connString = GetEnv("EVENTHUB_CONNECTION_STRING");
      Azure::Messaging::EventHubs::ConsumerClientOptions consumerClientOptions;
      consumerClientOptions.ApplicationID
          = testing::UnitTest::GetInstance()->current_test_info()->name();
      consumerClientOptions.Name = testing::UnitTest::GetInstance()->current_test_info()->name();

      std::string containerName{GetRandomName("proctest")};

      // Create the checkpoint store
      std::shared_ptr<CheckpointStore> checkpointStore{std::make_shared<TestCheckpointStore>()};
      GTEST_LOG_(INFO) << "Checkpoint store created";

      std::shared_ptr<ConsumerClient> consumerClient{std::make_shared<ConsumerClient>(
          connString, eventHubName, consumerGroup, consumerClientOptions)};
      GTEST_LOG_(INFO) << "Consumer Client created";

      ProcessorOptions processorOptions;
      processorOptions.LoadBalancingStrategy = processorStrategy;
      processorOptions.UpdateInterval = std::chrono::milliseconds(1);

      Processor processor{consumerClient, checkpointStore, processorOptions};

      Azure::Core::Context runContext;

      processor.Start(runContext);
      scope_guard onExit{[&processor]() { processor.Stop(); }};

      auto eventhubProperties{consumerClient->GetEventHubProperties(runContext)};
      std::set<std::string> partitionsAcquired;
      for (auto& partitionId : eventhubProperties.PartitionIds)
      {
        GTEST_LOG_(INFO) << "Waiting for next partition client. Might be " << partitionId;
        auto partitionClient = processor.NextPartitionClient(runContext);
        EXPECT_EQ(partitionsAcquired.find(partitionClient->PartitionId()), partitionsAcquired.end())
            << "No previous client for partition " << partitionClient->PartitionId();
        partitionsAcquired.emplace(partitionClient->PartitionId());
      }
    }

    void ProcessEventsForLoadBalancerTest(
        ProducerClient& producerClient,
        std::shared_ptr<ProcessorPartitionClient> partitionClient,
        Azure::Core::Context const& context)
    {
      Azure::Core::Context producerContext{context};
      try
      {
        // initialize any resources needed to process the partition
        // This is the equivalent to PartitionOpen
        GTEST_LOG_(INFO) << "Started processing partition " << partitionClient->PartitionId();
        const int32_t expectedEventsCount = 1000;
        const int32_t batchSize = 1000;
        EXPECT_EQ(0, expectedEventsCount % batchSize)
            << "Keep the math simple - even # of messages for each batch";
#if ASYNC_GENERATE_EVENTS
        std::thread produceEvents([&, partitionClient]() {
          // Wait for 10 seconds for all of the consumer clients to be spun up.
          GTEST_LOG_(INFO)
              << "Produce Events thread: Wait for 10 seconds for processor to create receivers.";
          std::this_thread::sleep_for(std::chrono::seconds(10));
          try
          {
            GTEST_LOG_(INFO) << "Generate " << std::dec << expectedEventsCount << " events in "
                             << std::dec << (expectedEventsCount / batchSize) << " batch messages.";
            for (int i = 0; i < expectedEventsCount / batchSize; i++)
            {
              producerContext.ThrowIfCancelled();
              EventDataBatchOptions batchOptions;
              batchOptions.PartitionId = partitionClient->PartitionId();
              auto batch = producerClient.CreateBatch(batchOptions, producerContext);
              for (int j = 0; j < batchSize; j++)
              {
                std::stringstream ss;
                ss << "[" << partitionClient->PartitionId() << ":[" << i << ":" << j
                   << "]] Message";
                batch.TryAddMessage(Models::EventData{ss.str()});
              }
              GTEST_LOG_(INFO) << "Send batch " << i << ", targeting partition "
                               << partitionClient->PartitionId();
              producerClient.Send(batch, producerContext);
            }
          }
          catch (std::runtime_error& ex)
          {
            GTEST_LOG_(FATAL) << "Exception thrown sending messages" << ex.what();
          }
        });
#else
        try
        {
          GTEST_LOG_(INFO) << "Generate " << std::dec << expectedEventsCount << " events in "
                           << std::dec << (expectedEventsCount / batchSize)
                           << " batch messages on partition " << partitionClient->PartitionId();
          for (int i = 0; i < expectedEventsCount / batchSize; i++)
          {
            producerContext.ThrowIfCancelled();
            EventDataBatchOptions batchOptions;
            batchOptions.PartitionId = partitionClient->PartitionId();
            auto batch = producerClient.CreateBatch(batchOptions, producerContext);
            for (int j = 0; j < batchSize; j++)
            {
              std::stringstream ss;
              ss << "[" << partitionClient->PartitionId() << ":[" << i << ":" << j << "]] Message";
              batch.TryAddMessage(Models::EventData{ss.str()});
            }
            GTEST_LOG_(INFO) << "Send batch " << i << ", targeting partition "
                             << partitionClient->PartitionId();
            producerClient.Send(batch, producerContext);
          }
        }
        catch (std::runtime_error& ex)
        {
          GTEST_LOG_(FATAL) << "Exception thrown sending messages" << ex.what();
        }
#endif
        std::vector<std::shared_ptr<const Models::ReceivedEventData>> allEvents;
        while (!context.IsCancelled())
        {
          auto receiveContext
              = context.WithDeadline(Azure::DateTime::clock::now() + std::chrono::seconds(50));
          GTEST_LOG_(INFO) << "Receive up to 100 events with a 50 second timeout on partition "
                           << partitionClient->PartitionId();
          auto events = partitionClient->ReceiveEvents(100, receiveContext);
          if (events.size() != 0)
          {
            GTEST_LOG_(INFO) << "Processing " << events.size() << " events for partition "
                             << partitionClient->PartitionId();
            allEvents.insert(allEvents.end(), events.begin(), events.end());
            GTEST_LOG_(INFO) << "Updating checkpoint for partition "
                             << partitionClient->PartitionId();
            partitionClient->UpdateCheckpoint(events.back(), context);
            if (allEvents.size() == expectedEventsCount)
            {
              GTEST_LOG_(INFO) << "Received all expected events; returning.";
#if ASYNC_GENERATE_EVENTS
              produceEvents.join();
#endif
              return;
            }
          }
        }
      }
      catch (std::runtime_error const& ex)
      {
        GTEST_LOG_(FATAL) << "Exception thrown receiving messages." << ex.what();
        producerContext.Cancel();
      }
    }
  };

  TEST_F(ProcessorTest, BasicTest)
  {
    std::shared_ptr<Azure::Messaging::EventHubs::CheckpointStore> checkpointStore{
        std::make_shared<Azure::Messaging::EventHubs::Test::TestCheckpointStore>()};

    std::string eventHubName{GetEnv("EVENTHUB_NAME")};
    std::string consumerGroup = GetEnv("EVENTHUB_CONSUMER_GROUP");

    std::string const connStringNoEntityPath = GetEnv("EVENTHUB_CONNECTION_STRING");
    Azure::Messaging::EventHubs::ConsumerClientOptions consumerClientOptions;
    consumerClientOptions.ApplicationID
        = testing::UnitTest::GetInstance()->current_test_info()->name();
    consumerClientOptions.Name = testing::UnitTest::GetInstance()->current_test_info()->name();

    ProcessorOptions processorOptions;
    processorOptions.LoadBalancingStrategy = Models::ProcessorStrategy::ProcessorStrategyBalanced;
    processorOptions.UpdateInterval = std::chrono::seconds(2);

    Processor processor(
        std::make_shared<ConsumerClient>(
            connStringNoEntityPath, eventHubName, consumerGroup, consumerClientOptions),
        checkpointStore,
        processorOptions);
  }

  TEST_F(ProcessorTest, StartStop_LIVEONLY_)
  {
    std::shared_ptr<Azure::Messaging::EventHubs::CheckpointStore> checkpointStore{
        std::make_shared<Azure::Messaging::EventHubs::Test::TestCheckpointStore>()};

    std::string eventHubName{GetEnv("EVENTHUB_NAME")};
    std::string consumerGroup = GetEnv("EVENTHUB_CONSUMER_GROUP");

    std::string const connStringNoEntityPath = GetEnv("EVENTHUB_CONNECTION_STRING");
    Azure::Messaging::EventHubs::ConsumerClientOptions consumerClientOptions;
    consumerClientOptions.ApplicationID
        = testing::UnitTest::GetInstance()->current_test_info()->name();
    consumerClientOptions.Name = testing::UnitTest::GetInstance()->current_test_info()->name();

    ProcessorOptions processorOptions;
    processorOptions.LoadBalancingStrategy = Models::ProcessorStrategy::ProcessorStrategyBalanced;
    processorOptions.UpdateInterval = std::chrono::seconds(2);

    Processor processor(
        std::make_shared<ConsumerClient>(
            connStringNoEntityPath, eventHubName, consumerGroup, consumerClientOptions),
        checkpointStore,
        processorOptions);

    processor.Start();

    processor.Stop();
    processor.Close();
  }

  TEST_F(ProcessorTest, JustStop_LIVEONLY_)
  {
    std::shared_ptr<Azure::Messaging::EventHubs::CheckpointStore> checkpointStore{
        std::make_shared<Azure::Messaging::EventHubs::Test::TestCheckpointStore>()};

    std::string eventHubName{GetEnv("EVENTHUB_NAME")};
    std::string consumerGroup = GetEnv("EVENTHUB_CONSUMER_GROUP");

    std::string const connStringNoEntityPath = GetEnv("EVENTHUB_CONNECTION_STRING");
    Azure::Messaging::EventHubs::ConsumerClientOptions consumerClientOptions;
    consumerClientOptions.ApplicationID
        = testing::UnitTest::GetInstance()->current_test_info()->name();
    consumerClientOptions.Name = testing::UnitTest::GetInstance()->current_test_info()->name();

    ProcessorOptions processorOptions;
    processorOptions.LoadBalancingStrategy = Models::ProcessorStrategy::ProcessorStrategyBalanced;
    processorOptions.UpdateInterval = std::chrono::seconds(2);

    Processor processor(
        std::make_shared<ConsumerClient>(
            connStringNoEntityPath, eventHubName, consumerGroup, consumerClientOptions),
        checkpointStore,
        processorOptions);

    processor.Stop();
    processor.Close();
  }

  TEST_F(ProcessorTest, LoadBalancing_LIVEONLY_)
  {
    std::string const testName = GetRandomName();
    std::shared_ptr<Azure::Messaging::EventHubs::CheckpointStore> checkpointStore{
        std::make_shared<Azure::Messaging::EventHubs::Test::TestCheckpointStore>()};

    std::string eventHubName{GetEnv("EVENTHUB_NAME")};
    std::string consumerGroup = GetEnv("EVENTHUB_CONSUMER_GROUP");

    std::string const connStringNoEntityPath = GetEnv("EVENTHUB_CONNECTION_STRING");
    Azure::Messaging::EventHubs::ConsumerClientOptions consumerClientOptions;
    consumerClientOptions.ApplicationID
        = testing::UnitTest::GetInstance()->current_test_info()->name();
    consumerClientOptions.Name = testing::UnitTest::GetInstance()->current_test_info()->name();

    ProcessorOptions processorOptions;
    processorOptions.LoadBalancingStrategy = Models::ProcessorStrategy::ProcessorStrategyBalanced;
    processorOptions.UpdateInterval = std::chrono::seconds(2);

    Processor processor(
        std::make_shared<ConsumerClient>(
            connStringNoEntityPath, eventHubName, consumerGroup, consumerClientOptions),
        checkpointStore,
        processorOptions);

    Azure::Core::Context context;
    std::thread workerThread([&processor, &context]() { processor.Run(context); });
    GTEST_LOG_(INFO) << "Sleep for 5 seconds to allow the processor to stabilize.";
    std::this_thread::sleep_for(std::chrono::seconds(5));

    context.Cancel();

    // Now wait for the worker thread to finish.
    workerThread.join();

    processor.Close();
  }

  TEST_F(ProcessorTest, LoadBalancing_Cancel_LIVEONLY_)
  {
    std::string const testName = GetRandomName();
    std::shared_ptr<Azure::Messaging::EventHubs::CheckpointStore> checkpointStore{
        std::make_shared<Azure::Messaging::EventHubs::Test::TestCheckpointStore>()};

    std::string eventHubName{GetEnv("EVENTHUB_NAME")};
    std::string consumerGroup = GetEnv("EVENTHUB_CONSUMER_GROUP");

    std::string const connStringNoEntityPath = GetEnv("EVENTHUB_CONNECTION_STRING");
    Azure::Messaging::EventHubs::ConsumerClientOptions consumerClientOptions;
    consumerClientOptions.ApplicationID
        = testing::UnitTest::GetInstance()->current_test_info()->name();
    consumerClientOptions.Name = testing::UnitTest::GetInstance()->current_test_info()->name();

    ProcessorOptions processorOptions;
    processorOptions.LoadBalancingStrategy = Models::ProcessorStrategy::ProcessorStrategyBalanced;
    processorOptions.UpdateInterval = std::chrono::seconds(2);

    Processor processor(
        std::make_shared<ConsumerClient>(
            connStringNoEntityPath, eventHubName, consumerGroup, consumerClientOptions),
        checkpointStore,
        processorOptions);

    Azure::Core::Context runContext;
    std::thread workerThread([runContext, &processor]() { processor.Run(runContext); });
    GTEST_LOG_(INFO) << "Sleep for 10 seconds to allow the processor to stabilize.";
    std::this_thread::sleep_for(std::chrono::seconds(10));

    runContext.Cancel();

    // Now wait for the worker thread to finish.
    workerThread.join();

    processor.Close();
  }

  TEST_F(ProcessorTest, Processor_ClientUniquePartitionClients_LIVEONLY_)
  {
    std::string const testName = GetRandomName();

    std::string eventHubName{GetEnv("EVENTHUB_NAME")};
    std::string consumerGroup = GetEnv("EVENTHUB_CONSUMER_GROUP");

    std::string const connStringNoEntityPath = GetEnv("EVENTHUB_CONNECTION_STRING");
    Azure::Messaging::EventHubs::ConsumerClientOptions consumerClientOptions;
    consumerClientOptions.ApplicationID
        = testing::UnitTest::GetInstance()->current_test_info()->name();
    consumerClientOptions.Name = testing::UnitTest::GetInstance()->current_test_info()->name();

    auto consumerClient = std::make_shared<ConsumerClient>(
        connStringNoEntityPath, eventHubName, consumerGroup, consumerClientOptions);

    auto eventhubInfo = consumerClient->GetEventHubProperties();

    std::shared_ptr<Azure::Messaging::EventHubs::CheckpointStore> checkpointStore{
        std::make_shared<Azure::Messaging::EventHubs::Test::TestCheckpointStore>()};

    ProcessorOptions processorOptions;
    processorOptions.LoadBalancingStrategy = Models::ProcessorStrategy::ProcessorStrategyBalanced;
    processorOptions.UpdateInterval = std::chrono::seconds(2);

    Processor processor(consumerClient, checkpointStore, processorOptions);

    // Start the processor running.
    processor.Start();

    std::set<std::string> clientIds;
    std::map<std::string, std::shared_ptr<ProcessorPartitionClient>> partitionClients;

    for (size_t i = 0; i < eventhubInfo.PartitionIds.size(); i += 1)
    {
      auto partitionClient = processor.NextPartitionClient();
      GTEST_LOG_(INFO) << "Received partition client for partition "
                       << partitionClient->PartitionId();
      ASSERT_EQ(clientIds.find(partitionClient->PartitionId()), clientIds.end())
          << "Received duplicate partition client for partition " << partitionClient->PartitionId();
      clientIds.emplace(partitionClient->PartitionId());
      partitionClients.emplace(partitionClient->PartitionId(), partitionClient);
    }

    // Attempts to retrieve a partition client should throw because there are no clients available.
    auto context = Azure::Core::Context::ApplicationContext.WithDeadline(
        Azure::DateTime::clock::now() + std::chrono::milliseconds(50));
    EXPECT_ANY_THROW(processor.NextPartitionClient(context));

    while (!partitionClients.empty())
    {
      auto partitionClientIterator = partitionClients.begin();
      auto partitionClient = partitionClientIterator->second;
      if (partitionClientIterator != partitionClients.end())
      {
        partitionClients.erase(partitionClientIterator);
      }
      partitionClient->Close();
    }

    processor.Stop();
  }

  TEST_F(ProcessorTest, Processor_Balanced_LIVEONLY_)
  {
    TestWithLoadBalancer(Models::ProcessorStrategy::ProcessorStrategyBalanced);
  }
  TEST_F(ProcessorTest, Processor_Greedy_LIVEONLY_)
  {
    TestWithLoadBalancer(Models::ProcessorStrategy::ProcessorStrategyGreedy);
  }

  // The processor balanced_acquisitiononly tests are multi-threaded and until the half-closed
  // message sender/message receiver bug is fixed, they cannot be run.
#if 0
  TEST_F(ProcessorTest, Processor_Balanced_AcquisitionOnly_LIVEONLY_)
  {
    TestPartitionAcquisition(Models::ProcessorStrategy::ProcessorStrategyBalanced);
  }
  TEST_F(ProcessorTest, Processor_Greedy_AcquisitionOnly_LIVEONLY_)
  {
    TestPartitionAcquisition(Models::ProcessorStrategy::ProcessorStrategyGreedy);
  }
#endif

}}}} // namespace Azure::Messaging::EventHubs::Test
