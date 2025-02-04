// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "batch_stress_tests.hpp"

#include "scope_guard.hpp"
#include "shared_functions.hpp"

#include <azure/identity/default_azure_credential.hpp>
#include <azure/identity/environment_credential.hpp>

using namespace Azure::Messaging::EventHubs;

namespace trace_sdk = opentelemetry::sdk::trace;
namespace trace = opentelemetry::trace;
namespace logs_sdk = opentelemetry::sdk::logs;
namespace logs = opentelemetry::logs;

namespace {

} // namespace
BatchStressTest::BatchStressTest() {}

namespace argagg { namespace convert {

  // Convert a string to a std::chrono::system_clock::duration
  template <> std::chrono::system_clock::duration arg(const char* s)
  {
    std::string str(s);
    std::string number;
    std::string unit;
    for (auto c : str)
    {
      if (std::isdigit(c))
      {
        number.push_back(c);
      }
      else
      {
        unit.push_back(c);
      }
    }
    auto value = std::stoll(number);
    if (unit == "s")
    {
      return std::chrono::seconds(value);
    }
    else if (unit == "m")
    {
      return std::chrono::minutes(value);
    }
    else if (unit == "h")
    {
      return std::chrono::hours(value);
    }
    else if (unit == "ms")
    {
      return std::chrono::milliseconds(value);
    }
    else if (unit == "us")
    {
      return std::chrono::microseconds(value);
    }
    else
    {
      throw std::invalid_argument("Invalid duration unit: " + unit);
    }
  }
}} // namespace argagg::convert

const std::string& BatchStressTest::GetStressScenarioName() { return m_scenarioName; }

std::vector<EventHubsScenarioOptions> BatchScenarioOptions{
    {"NumberToSend", {"-c", "--send"}, "Number of events to send", 1},
    {"BatchSize",
     {"-r", "--receive"},
     "Size to request each time we call ReceiveEvents(). Higher batch sizes will require higher "
     "amounts of memory for this test",
     1},
    {"BatchDuration", {"-t", "--timeout"}, "Time to wait for each batch (ie: 1m, 30s, etc...)", 1},
    {"Prefetch",
     {"-f", "--prefetch"},
     "Number of events to set for the prefetch. Negative numbers disable prefetch altogether. 0 "
     "uses the default for the package",
     1},
    {"Rounds",
     {"-n", "--rounds"},
     "Number of rounds to run with these parameters. -1 means MAX_INT",
     1},

    {"PaddingBytes",
     {"-P", "--padding"},
     "Extra number of bytes to add onto each message body.",
     1},
    {"PartitionId",
     {"-p", "--partition"},
     "Partition ID to send events to and receive events from",
     1},
    {"MaxTimeouts",
     {"-m", "--maxtimeouts"},
     "Number of consecutive receive timeouts allowed before quitting",
     0},
    {"UseSasCredential",
     {"-S", "--useSasCredential"},
     "Use a SAS credential for authentication",
     0},
    {"SleepAfter", {"--sleepAfter"}, "Time to sleep after test completes", 1},
};

// Default option values.

// Note that the DefaultNumberToSend value is artificially reduced to 100 until the
// MessageSender::Open code fully supports Open.
constexpr const std::uint32_t DefaultNumberToSend = 1000000;
constexpr const std::uint32_t DefaultBatchSize = 1000;
constexpr const std::uint32_t DefaultPrefetch = 0;
constexpr const auto DefaultDuration = std::chrono::seconds(60);
constexpr const std::uint32_t DefaultRounds = 100;
constexpr const std::uint32_t DefaultPaddingBytes = 1024;
const std::string DefaultPartitionId{"0"}; // constexpr std::string is a c++17 feature.
constexpr const std::uint32_t DefaultMaxTimeouts = 10;

const std::vector<EventHubsScenarioOptions>& BatchStressTest::GetScenarioOptions()
{
  return BatchScenarioOptions;
}

void BatchStressTest::Initialize(argagg::parser_results const& parserResults)
{
  m_numberToSend = parserResults["NumberToSend"].as<uint32_t>(DefaultNumberToSend);
  m_batchSize = parserResults["BatchSize"].as<uint32_t>(DefaultBatchSize);
  m_batchDuration
      = parserResults["BatchDuration"].as<std::chrono::system_clock::duration>(DefaultDuration);
  m_prefetchCount = parserResults["Prefetch"].as<uint32_t>(DefaultPrefetch);
  m_rounds = parserResults["Rounds"].as<uint32_t>(DefaultRounds);
  m_paddingBytes = parserResults["PaddingBytes"].as<uint32_t>(DefaultPaddingBytes);
  m_partitionId = parserResults["PartitionId"].as<std::string>(DefaultPartitionId);
  m_maxTimeouts = parserResults["MaxTimeouts"].as<uint32_t>(DefaultMaxTimeouts);
  m_verbose = parserResults["verbose"].as<bool>(false);
  if (m_rounds == 0xffffffff)
  {
    m_rounds = (std::numeric_limits<uint32_t>::max)();
  }

  auto span{CreateStressSpan("Initialize")};
  span.first->SetAttribute("NumberToSend", m_numberToSend);
  span.first->SetAttribute("BatchSize", m_batchSize);
  span.first->SetAttribute("BatchDuration", m_batchDuration.count());
  span.first->SetAttribute("Prefetch", m_prefetchCount);
  span.first->SetAttribute("Rounds", m_rounds);
  span.first->SetAttribute("PaddingBytes", m_paddingBytes);
  span.first->SetAttribute("PartitionId", m_partitionId);
  span.first->SetAttribute("MaxTimeouts", m_maxTimeouts);
  span.first->SetAttribute("Verbose", m_verbose);

  if (parserResults.has_option("SleepAfter"))
  {
    m_sleepAfterFunction
        = GetSleepAfterFunction(parserResults["SleepAfter"].as<std::chrono::system_clock::duration>(
            std::chrono::seconds(0)));
  }

  m_eventHubName = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_NAME");

  if (m_eventHubName.empty())
  {
    GetLogger()->Fatal("Could not find required environment variable EVENTHUB_NAME");
    std::cerr << "Missing required environment variable EVENTHUB_NAME" << std::endl;
    throw std::runtime_error("Missing environment variable, aborting.");
  }

  m_eventHubNamespace = Azure::Core::_internal::Environment::GetVariable("EVENTHUBS_HOST");
  if (m_eventHubNamespace.empty())
  {
    GetLogger()->Fatal("Could not find required environment variable EVENTHUBS_HOST");
    std::cerr << "Missing required environment variable EVENTHUBS_HOST" << std::endl;
    throw std::runtime_error("Missing environment variable, aborting.");
  }

  m_eventHubHost = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_HOST");
  if (m_eventHubHost.empty())
  {
    std::cerr << "Missing required environment variable EVENTHUB_HOST" << std::endl;
    GetLogger()->Fatal("Could not find required environment variable EVENTHUB_HOST");
    throw std::runtime_error("Missing environment variable, aborting.");
  }
  m_checkpointStoreConnectionString
      = Azure::Core::_internal::Environment::GetVariable("CHECKPOINT_STORE_CONNECTION_STRING");
}

void BatchStressTest::Run()
{
  auto scopeGuard{sg::make_scope_guard([&]() {
    if (m_sleepAfterFunction)
    {
      m_sleepAfterFunction({});
    }
  })};
  std::cout << "Run " << std::endl;
  auto sendOutput = SendMessages();
  std::cout << "Starting receive tests for partition " << m_partitionId << std::endl;
  std::cout << "  Start position: " << sendOutput.first << std::endl
            << "  End position: " << sendOutput.second.LastEnqueuedSequenceNumber << std::endl;

  ReceiveMessages(sendOutput.first);
}
void BatchStressTest::Cleanup() {}

std::pair<
    Azure::Messaging::EventHubs::Models::StartPosition,
    Azure::Messaging::EventHubs::Models::EventHubPartitionProperties>
BatchStressTest::SendMessages()
{
  std::unique_ptr<Azure::Messaging::EventHubs::ProducerClient> producerClient;
  producerClient = std::make_unique<Azure::Messaging::EventHubs::ProducerClient>(
      m_eventHubNamespace,
      m_eventHubName,
      //        std::make_shared<Azure::Identity::EnvironmentCredential>());
      std::make_shared<Azure::Identity::DefaultAzureCredential>());
  Azure::Core::Context context;
  auto scopeGuard{
      sg::make_scope_guard([&context, &producerClient]() { producerClient->Close(context); })};

  try
  {
    EventSenderOptions senderOptions;
    senderOptions.PartitionId = m_partitionId;
    senderOptions.MessageLimit = m_numberToSend;
    senderOptions.NumberOfExtraBytes = m_paddingBytes;
    auto sendEventsResult
        = EventSender::SendEventsToPartition(producerClient, senderOptions, context);
    producerClient->Close(context);

    return std::make_pair(sendEventsResult.first, sendEventsResult.second);
  }
  catch (std::exception const& ex)
  {
    GetTracer()->GetCurrentSpan()->AddEvent(
        "Exception received", {{trace::SemanticConventions::kExceptionMessage, ex.what()}});
    std::cerr << "Exception " << ex.what();
    throw;
  }
}
void BatchStressTest::ReceiveMessages(
    Azure::Messaging::EventHubs::Models::StartPosition const& startPosition)
{
  auto span{CreateStressSpan("ReceiveMessages")};

  try
  {
    Azure::Core::Context context;
    ConsumerClientOptions clientOptions;
    clientOptions.ApplicationID = "StressConsumerClient";

    std::unique_ptr<ConsumerClient> consumerClient;
    consumerClient = std::make_unique<ConsumerClient>(
        m_eventHubNamespace,
        m_eventHubName,
        std::make_shared<Azure::Identity::EnvironmentCredential>());

    {
      auto getPartitionPropertiesSpan{
          CreateStressSpan("ReceiveMessages::GetPartitionProperties to warm up connection")};
      auto consumerProperties = consumerClient->GetEventHubProperties(context);
    }

    for (auto round = 0u; round < m_rounds; round += 1)
    {
      std::cout << "Round " << round << std::endl;
      auto consumeForTesterSpan{CreateStressSpan("ConsumeForBatchTester")};
      consumeForTesterSpan.first->SetAttribute("Round", round);
      ConsumeForBatchTester(round, *consumerClient, startPosition, context);
    }

    consumerClient->Close(context);
  }
  catch (std::exception const& ex)
  {
    GetTracer()->GetCurrentSpan()->AddEvent(
        "Exception received", {{trace::SemanticConventions::kExceptionMessage, ex.what()}});
    std::cerr << "Exception " << ex.what();
    throw;
  }
}

void BatchStressTest::ConsumeForBatchTester(
    uint32_t round,
    ConsumerClient& client,
    Models::StartPosition const& startPosition,
    Azure::Core::Context const& context) const
{
  std::unique_ptr<PartitionClient> partitionClient;
  {
    auto span{CreateStressSpan("ConsumeForBatchTester::CreatePartitionClient")};
    PartitionClientOptions partitionOptions;
    partitionOptions.StartPosition = startPosition;
    partitionOptions.Prefetch = m_prefetchCount;
    partitionClient = std::make_unique<PartitionClient>(
        client.CreatePartitionClient(m_partitionId, partitionOptions));
    std::cout << "[r: " << round << "/" << m_rounds << "p: " << m_partitionId
              << "] Starting to receive messages from partition" << std::endl;
  }
  size_t total = 0;
  uint32_t numCancels = 0;
  constexpr const uint32_t cancelLimit = 5;

  std::cout << "Receiving events from partition " << m_partitionId << " for round " << round
            << "starting at " << startPosition << " with a timeout of "
            << std::chrono::duration_cast<std::chrono::seconds>(m_batchDuration).count()
            << " seconds" << std::endl;

  do
  {
    auto duration = std::chrono::system_clock::now() + m_batchDuration;

    auto receiveContext = context.WithDeadline(duration);

    try
    {
      auto span{CreateStressSpan("ConsumeForBatchTester::ReceiveEvents")};
      auto events = partitionClient->ReceiveEvents(m_batchSize, receiveContext);
      total += events.size();
      if (total >= m_numberToSend)
      {
        break;
      }
    }
    catch (Azure::Messaging::EventHubs::EventHubsException& ex)
    {
      std::cerr << "Exception thrown while receiving messages." << ex.what() << std::endl;
      if (!ex.IsTransient)
      {
        std::cerr << "Error is not transient, aborting test." << std::endl;
        throw;
      }
    }
    catch (Azure::Core::OperationCancelledException&)
    {
      numCancels += 1;
      if (numCancels > cancelLimit)
      {
        std::cerr << "cancellation errors were received " << numCancels
                  << " times in a row. Stoping test." << std::endl;
        throw std::runtime_error("Too many cancellations received in a row, aborting test.");
      }
      else
      {
        std::cout << "received " << total << "/" << m_numberToSend << "then received error";
      }
    }
  } while (true);
}
