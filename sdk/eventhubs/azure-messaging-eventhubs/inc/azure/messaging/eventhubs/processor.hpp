// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

#include "checkpoint_store.hpp"
#include "consumer_client.hpp"
#include "models/processor_load_balancer_models.hpp"
#include "models/processor_models.hpp"
#include "processor_partition_client.hpp"

#include <azure/core/amqp/internal/common/async_operation_queue.hpp>
#include <azure/core/context.hpp>

#include <chrono>
#include <thread>

#ifdef _azure_TESTING_BUILD_AMQP
namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {
  class ProcessorTest_LoadBalancing_Test;
}}}} // namespace Azure::Messaging::EventHubs::Test
#endif
namespace Azure { namespace Messaging { namespace EventHubs {
  /**@brief ProcessorOptions are the options for the CreateProcessor
   * function.
   */
  struct ProcessorOptions final
  {
    /**@brief LoadBalancingStrategy dictates how concurrent Processor instances distribute
     * ownership of partitions between them.
     * The default strategy is ProcessorStrategyBalanced.
     */
    Models::ProcessorStrategy LoadBalancingStrategy{
        Models::ProcessorStrategy::ProcessorStrategyBalanced};

    /**@brief UpdateInterval controls how often attempt to claim partitions.
     * The default value is 10 seconds.
     */
    Azure::DateTime::duration UpdateInterval{std::chrono::seconds(10)};

    /**@brief PartitionExpirationDuration is the amount of time before a partition is
     * considered unowned. The default value is 60 seconds.
     */
    Azure::DateTime::duration PartitionExpirationDuration{std::chrono::seconds(60)};

    /**@brief StartPositions are the default start positions (configurable per
     * partition, or with an overall default value) if a checkpoint is not found
     * in the CheckpointStore. The default position is Latest.
     */
    Models::StartPositions StartPositions;

    /**@brief Prefetch represents the size of the internal prefetch buffer for
     * each ProcessorPartitionClient created by this Processor. When
     * set, this client will attempt to always maintain an internal
     * cache of events of this size, asynchronously, increasing the odds
     * that ReceiveEvents() will use a locally stored cache of events,
     * rather than having to wait for events to arrive from the network.
     *
     * Defaults to 300 events.
     * Disabled if Prefetch < 0.
     */
    int32_t Prefetch{300};

    /** @brief Specifies the maximum number of partitions to process.
     *
     * By default, the processor will process all available partitions. If a client desires limiting
     * the number of partitions to a restricted set, set the MaximumNumberOfPartitions to the number
     * of partitions to process.
     */
    int32_t MaximumNumberOfPartitions{0};
  };

  /**@brief Processor uses a [ConsumerClient] and [CheckpointStore] to provide automatic
   * load balancing between multiple Processor instances, even in separate
   *processes or on separate machines.
   */

  namespace _detail {
    class ProcessorLoadBalancer;
  }

  /** @brief Processor uses a ConsumerClient and CheckpointStore to provide automatic load balancing
   * between multiple Processor instances, even in separate processes or on separate machines.
   */
  class Processor final {
#ifdef _azure_TESTING_BUILD_AMQP
    friend class Test::ProcessorTest_LoadBalancing_Test;
#endif

  public:
    /** @brief Construct a new Processor object.
     *
     * @param consumerClient A [ConsumerClient] that is used to receive events from the Event Hub.
     * @param checkpointStore A [CheckpointStore] that is used to load and update checkpoints.
     * @param options Optional configuration for the processor.
     */
    Processor(
        std::shared_ptr<ConsumerClient> consumerClient,
        std::shared_ptr<CheckpointStore> checkpointStore,
        ProcessorOptions const& options = {});

    ~Processor();

    /** Construct a Processor from another Processor. */
    Processor(Processor const& other) = delete;

    /** Assign a Processor to another Processor. */
    Processor& operator=(Processor const& other) = delete;

    /** Move to the next partition client
     *
     * @param context The context to control whether this function is canceled or not.
     *
     * @return A shared pointer to the next partition client.
     *
     * NextPartitionClient will retrieve the next ProcessorPartitionClient if one is acquired or
     * will block until a new one arrives, or the processor is stopped.
     */
    std::shared_ptr<ProcessorPartitionClient> NextPartitionClient(
        Azure::Core::Context const& context = {});

    /** @brief Executes the processor.
     *
     * @param context The context to control the request lifetime.
     *
     * @remark This function will block until Stop() is called. It is intended for customers who
     * would prefer to manage the call to Run from their own threads.
     *
     */
    void Run(Core::Context const& context);

    /** @brief Starts the processor.
     *
     * @param context The context to control the request lifetime of the processor. Cancelling this
     * context will stop the processor from running.
     *
     * @remark This function starts the processor running in a new thread.
     */
    void Start(Azure::Core::Context const& context = {});

    /** @brief Stops a running processor.
     *
     * @remark This function stops the processor. If the Start method has been called, it will wait
     * for the thread to complete.
     */
    void Stop();

    /** @brief Closes the processor and cancels any current operations.
     *
     */
    void Close(Core::Context const& context = {})
    {
      if (m_isRunning)
      {
        throw std::runtime_error("cannot close a processor that is running");
      }

      // Drain the partition clients queue.
      for (;;)
      {
        auto client = m_nextPartitionClients.TryRemove();
        if (client)
        {
          client->Close();
        }
        else
        {
          break;
        }
      }
      (void)context;
    }

  private:
    /** Representation of Go channel construct.
     *
     * A channel represents a size limited queue, where items can be inserted and removed. If there
     * are no items in the queue, the caller will block until an item is inserted into the queue.
     *
     * @tparam T The type of the items in the queue.
     *
     */
    template <class T> class Channel {
    public:
      Channel() : m_maximumDepth{0} {}

      ~Channel()
      {
        Azure::Core::Diagnostics::_internal::Log::Stream(
            Azure::Core::Diagnostics::Logger::Level::Verbose)
            << "~Channel. Currently depth is " << m_channelDepth << " and maximum depth is "
            << m_maximumDepth;
        Azure::Core::Diagnostics::_internal::Log::Stream(
            Azure::Core::Diagnostics::Logger::Level::Verbose)
            << "Clear channel queue.";
        while (m_channelDepth > 0)
        {
          auto value = m_channelQueue.TryWaitForResult();
          if (value)
          {
            m_channelDepth -= 1;
          }
        }
      }

      // Insert an item into the channel, returning true if successful, false if the channel is
      // full.
      bool Insert(T item)
      {
        std::lock_guard<std::mutex> lock{m_channelLock};
        if ((m_maximumDepth != 0) && (m_channelDepth >= m_maximumDepth))
        {
          return false;
        }
        m_channelQueue.CompleteOperation(item);
        m_channelDepth += 1;
        return true;
      }

      // Remove an item from the channel.
      T Remove(Azure::Core::Context const& context)
      {
        auto value = m_channelQueue.WaitForResult(context);
        if (!value)
        {
          throw Azure::Core::OperationCancelledException("Operation was cancelled.");
        }
        std::lock_guard<std::mutex> lock{m_channelLock};
        m_channelDepth -= 1;
        return std::get<0>(*value);
      }

      /** Try to remove an item from the channel, returning an item from the channel or a default
       * constructed object.
       *
       * @return An item from the channel or a default constructed object.
       *
       * @remark The T type should have semantics that can distinguish between a default constructed
       * item and a valid item. So for example, if T is a pointer, then a nullptr should be returned
       * if the channel is empty. But if T is a value such as a string, it will not be possible to
       * distinguish between an empty string inserted into the channel and an empty channel.
       */
      T TryRemove()
      {
        std::lock_guard<std::mutex> lock{m_channelLock};
        auto value = m_channelQueue.TryWaitForResult();
        if (value)
        {
          m_channelDepth -= 1;
          return std::get<0>(*value);
        }
        return T{};
      }

      void SetMaximumDepth(size_t maximumDepth)
      {
        std::lock_guard<std::mutex> lock{m_channelLock};
        m_maximumDepth = maximumDepth;
      }

    private:
      std::mutex m_channelLock;
      size_t m_channelDepth{};
      size_t m_maximumDepth{};
      Core::Amqp::Common::_internal::AsyncOperationQueue<T> m_channelQueue;
    };

    Azure::DateTime::duration m_ownershipUpdateInterval;
    Models::StartPositions m_defaultStartPositions;
    int32_t m_maximumNumberOfPartitions;
    std::shared_ptr<CheckpointStore> m_checkpointStore;
    std::shared_ptr<ConsumerClient> m_consumerClient;
    int32_t m_prefetch;
    Channel<std::shared_ptr<ProcessorPartitionClient>> m_nextPartitionClients;
    Models::ConsumerClientDetails m_consumerClientDetails;
    std::shared_ptr<_detail::ProcessorLoadBalancer> m_loadBalancer;
    int64_t m_processorOwnerLevel{0};
    bool m_isRunning{false};
    std::thread m_processorThread;

    typedef std::map<std::string, std::shared_ptr<ProcessorPartitionClient>> ConsumersType;

    /** @brief Dispatches events to the appropriate partition clients.
     *
     * @param eventHubProperties The properties of the Event Hub.
     * @param consumers The map of partition id to partition client.
     * @param context The context to control the request lifetime.
     */
    void Dispatch(
        Models::EventHubProperties const& eventHubProperties,
        std::shared_ptr<ConsumersType> consumers,
        Core::Context const& context);

    void AddPartitionClient(
        Models::Ownership const& ownership,
        std::map<std::string, Models::Checkpoint>& checkpoints,
        std::weak_ptr<ConsumersType> consumers);

    void RunInternal(Core::Context const& context, bool manualRun);

    Models::StartPosition GetStartPosition(
        Models::Ownership const& ownership,
        std::map<std::string, Models::Checkpoint> const& checkpoints)
    {
      Models::StartPosition startPosition = m_defaultStartPositions.Default;

      if (checkpoints.find(ownership.PartitionId) != checkpoints.end())
      {
        Models::Checkpoint checkpoint = checkpoints.at(ownership.PartitionId);

        if (checkpoint.Offset.HasValue())
        {
          startPosition.Offset = checkpoint.Offset;
        }
        else if (checkpoint.SequenceNumber.HasValue())
        {
          startPosition.SequenceNumber = checkpoint.SequenceNumber;
        }
        else
        {
          throw std::runtime_error(
              "invalid checkpoint" + ownership.PartitionId + "no offset or sequence number");
        }
      }
      else if (
          m_defaultStartPositions.PerPartition.find(ownership.PartitionId)
          != m_defaultStartPositions.PerPartition.end())
      {
        startPosition = m_defaultStartPositions.PerPartition.at(ownership.PartitionId);
      }
      return startPosition;
    }

    std::map<std::string, Models::Checkpoint> GetCheckpointsMap(Core::Context const& context);
  };
}}} // namespace Azure::Messaging::EventHubs
