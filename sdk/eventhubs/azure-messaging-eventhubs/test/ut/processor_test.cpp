// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "./test_checkpoint_store.hpp"
#include "eventhubs_test_base.hpp"

#include <azure/core/amqp/common/global_state.hpp>
#include <azure/core/context.hpp>
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
      processorOptions.UpdateInterval = std::chrono::milliseconds(1500);
      processorOptions.Prefetch = 1500; // Set the initial link credits to 1500.

      Processor processor{consumerClient, checkpointStore, processorOptions};

      // Warm up the consumer client - establish connection to the server, etc.
      auto eventHubProperties = consumerClient->GetEventHubProperties(context);

      ProducerClientOptions producerOptions;
      producerOptions.Name = "Producer for LoadBalancerTest";
      ProducerClient producerClient{connectionString, eventHubName, producerOptions};

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
                // We've processed events for the client, close it so it gets recycled into the
                // queue.
                partitionClient->Close();
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
        const int32_t batchSize = 100;
        EXPECT_EQ(0, expectedEventsCount % batchSize)
            << "Keep the math simple - even # of messages for each batch";

        std::thread produceEvents([partitionClient,
                                   &producerClient,
                                   &producerContext,
                                   expectedEventsCount,
                                   batchSize]() {
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

        std::vector<Models::ReceivedEventData> allEvents;
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
              produceEvents.join();
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
#if 0 
	t.Logf("goroutine started for partition %s", partitionClient.PartitionID())

	const expectedEventsCount = 1000
	const batchSize = 1000
	require.Zero(t, expectedEventsCount%batchSize, "keep the math simple - even # of messages for each batch")

	// start producing events. We'll give the consumer client a moment, just to ensure
	// it's actually started up the link.
	go func() {
		time.Sleep(10 * time.Second)

		ctr := 0

		for i := 0; i < expectedEventsCount/batchSize; i++ {
			pid := partitionClient.PartitionID()
			batch, err := producerClient.NewEventDataBatch(context.Background(), &azeventhubs.EventDataBatchOptions{
				PartitionID: &pid,
			})
			require.NoError(t, err)

			for j := 0; j < batchSize; j++ {
				err := batch.AddEventData(&azeventhubs.EventData{
					Body: []byte(fmt.Sprintf("[%s:%d] Message", partitionClient.PartitionID(), ctr)),
				}, nil)
				require.NoError(t, err)
				ctr++
			}

			err = producerClient.SendEventDataBatch(context.Background(), batch, nil)
			require.NoError(t, err)
		}
	}()

	var allEvents []*azeventhubs.ReceivedEventData

	for {
		receiveCtx, receiveCtxCancel := context.WithTimeout(context.TODO(), 3*time.Second)
		events, err := partitionClient.ReceiveEvents(receiveCtx, 100, nil)
		receiveCtxCancel()

		if err != nil && !errors.Is(err, context.DeadlineExceeded) {
			if eventHubError := (*azeventhubs.Error)(nil); errors.As(err, &eventHubError) && eventHubError.Code == exported.ErrorCodeOwnershipLost {
				fmt.Printf("Partition %s was stolen\n", partitionClient.PartitionID())
			}

			return err
		}

		if len(events) != 0 {
			t.Logf("Processing %d event(s) for partition %s", len(events), partitionClient.PartitionID())

			allEvents = append(allEvents, events...)

			// Update the checkpoint with the last event received. If the processor is restarted
			// it will resume from this point in the partition.

			t.Logf("Updating checkpoint for partition %s", partitionClient.PartitionID())

			if err := partitionClient.UpdateCheckpoint(context.TODO(), events[len(events)-1], nil); err != nil {
				return err
			}

			if len(allEvents) == expectedEventsCount {
				t.Logf("! All events acquired for partition %s, ending...", partitionClient.PartitionID())
				return nil
			}
		}
	}
#endif
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
    GTEST_LOG_(INFO) << "Sleep for 2 seconds to allow the processor to stabilize.";
    std::this_thread::sleep_for(std::chrono::seconds(2));

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
