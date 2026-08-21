<!-- cspell:words azeventhubs  -->
# Azure Event Hubs Client Package for C++

[Azure Event Hubs](https://azure.microsoft.com/services/event-hubs/) is a big data streaming platform and event ingestion service from Microsoft. For more information about Event Hubs see: [link](https://learn.microsoft.com/azure/event-hubs/event-hubs-about).

Use the client library `github.com/Azure/azure-sdk-for-cpp/sdk/eventhubs` in your application to:

- Send events to an event hub.
- Consume events from an event hub.

Key links:
- [Source code][source]
- [API Reference Documentation][cppdoc]
- [Product documentation](https://azure.microsoft.com/services/event-hubs/)
- [Samples][cppdoc_examples]

## Getting started

### Install the package

Install the Azure Event Hubs client package for C++ with `vcpkg`:

```bash
vcpkg install azure-messaging-eventhubs-cpp
```

### Prerequisites

- A C++ Compiler with C++14 support
- An [Azure subscription](https://azure.microsoft.com/free/)
- An [Event Hub namespace](https://learn.microsoft.com/azure/event-hubs/).
- An Event Hub. You can create an event hub in your Event Hubs Namespace using the [Azure Portal](https://learn.microsoft.com/azure/event-hubs/event-hubs-create), or the [Azure CLI](https://learn.microsoft.com/azure/event-hubs/event-hubs-quickstart-cli).

#### Create a namespace using the Azure CLI

Login to the CLI:
```pwsh
az login
```

Create a resource group:
```pwsh
az group create --name <your group name> --location <your location> --subscription <your subscription>
```

This should output something like:
```json
{
  "id": "/subscriptions/<your subscription ID>/resourceGroups/<your group name>",
  "location": "<your location>",
  "managedBy": null,
  "name": "<yourgroup name>",
  "properties": {
    "provisioningState": "Succeeded"
  },
  "tags": null,
  "type": "Microsoft.Resources/resourceGroups"
}
```

Create an EventHubs namespace:
```pwsh
 az eventhubs namespace create --resource-group <your group name> --name <your namespace name> --sku Standard  --subscription <your subscription>
 ```

This should output something like:

```json
{
  "createdAt": "2023-08-10T18:41:54.19Z",
  "disableLocalAuth": false,
  "id": "/subscriptions/<your subscription ID>/resourceGroups/<your group name>/providers/Microsoft.EventHub/namespaces/<your namespace>",
  "isAutoInflateEnabled": false,
  "kafkaEnabled": true,
  "location": "West US",
  "maximumThroughputUnits": 0,
  "metricId": "REDACTED",
  "minimumTlsVersion": "1.2",
  "name": "<your namespace name>",
  "provisioningState": "Succeeded",
  "publicNetworkAccess": "Enabled",
  "resourceGroup": "<your resource group>",
  "serviceBusEndpoint": "https://<your namespace name>.servicebus.windows.net:443/",
  "sku": {
    "capacity": 1,
    "name": "Standard",
    "tier": "Standard"
  },
  "status": "Active",
  "tags": {},
  "type": "Microsoft.EventHub/Namespaces",
  "updatedAt": "2023-08-10T18:42:41.343Z",
  "zoneRedundant": false
}
```

Create an EventHub:

```pwsh
az eventhubs eventhub create --resource-group <your resource group> --namespace-name <your namespace name> --name <your eventhub name>
```

That should output something like:

```json
{
  "createdAt": "2023-08-10T21:02:07.62Z",
  "id": "/subscriptions/<your subscription>/resourceGroups/<your group name>/providers/Microsoft.EventHub/namespaces/<your namespace name>/eventhubs/<your eventhub name>",
  "location": "westus",
  "messageRetentionInDays": 7,
  "name": "<your eventhub name>",
  "partitionCount": 4,
  "partitionIds": [
    "0",
    "1",
    "2",
    "3"
  ],
  "resourceGroup": "<your group name>",
  "retentionDescription": {
    "cleanupPolicy": "Delete",
    "retentionTimeInHours": 168
  },
  "status": "Active",
  "type": "Microsoft.EventHub/namespaces/eventhubs",
  "updatedAt": "2023-08-10T21:02:16.29Z"
}
```


### Authenticate the client

Event Hub clients are created using a credential from the [Azure Identity package][azure_identity_pkg], like [DefaultAzureCredential][default_azure_credential].
Alternatively, you can create a client using a connection string.

#### Using Microsoft Entra ID

```cpp
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

std::string fullyQualifiedNamespace = "<namespace>.servicebus.windows.net";
std::string eventHubName = "<event_hub_name>";

auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();

Azure::Messaging::EventHubs::ProducerClient producer(
    fullyQualifiedNamespace, eventHubName, credential);
Azure::Messaging::EventHubs::ConsumerClient consumer(
    fullyQualifiedNamespace, eventHubName, credential);
```

 - ConsumerClient: [link][consumer_client]
 - ProducerClient: [link][producer_client]

Samples: [create_consumer_aad.cpp](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/eventhubs/azure-messaging-eventhubs/samples/basic-operations/create_consumer_aad.cpp) and [create_producer_aad.cpp](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/eventhubs/azure-messaging-eventhubs/samples/basic-operations/create_producer_aad.cpp).

#### Using a connection string

Use a token credential for production applications when possible. A connection string is useful for local development, for the Event Hubs emulator, or when a shared access key is required.

A namespace connection string does not contain an `EntityPath`. Pass the Event Hub name separately:

```text
Endpoint=sb://<namespace>.servicebus.windows.net/;SharedAccessKeyName=<key-name>;SharedAccessKey=<key>
```

An Event Hub connection string contains an `EntityPath`. The Event Hub argument can be empty, or it must match that value. The match ignores ASCII letter case:

```text
Endpoint=sb://<namespace>.servicebus.windows.net/;SharedAccessKeyName=<key-name>;SharedAccessKey=<key>;EntityPath=<event-hub-name>
```

An Event Hub argument that differs by more than ASCII letter case throws `std::invalid_argument`. When the two names differ only by case, the client uses the connection string spelling. The comparison folds ASCII A-Z only. It is not the culture-aware `InvariantCultureIgnoreCase` comparison that .NET uses.

Samples: [create_consumer.cpp](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/eventhubs/azure-messaging-eventhubs/samples/basic-operations/create_consumer.cpp) and [create_producer.cpp](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/eventhubs/azure-messaging-eventhubs/samples/basic-operations/create_producer.cpp).

# Key concepts

An Event Hub [**namespace**](https://learn.microsoft.com/azure/event-hubs/event-hubs-features#namespace) can have multiple event hubs.
Each event hub, in turn, contains [**partitions**](https://learn.microsoft.com/azure/event-hubs/event-hubs-features#partitions) which 
store events.

<!-- NOTE: Fix dead links -->
Events are published to an event hub using an [event publisher](https://learn.microsoft.com/azure/event-hubs/event-hubs-features#event-publishers). In this package, the event publisher is the [ProducerClient](https://azure.github.io/azure-sdk-for-cpp/storage.html)

Events can be consumed from an event hub using an [event consumer](https://learn.microsoft.com/azure/event-hubs/event-hubs-features#event-consumers). In this package there are two types for consuming events: 
- The basic event consumer is the PartitionClient, in the [ConsumerClient][consumer_client]. This consumer is useful if you already known which partitions you want to receive from.
- A distributed event consumer, which uses Azure Blobs for checkpointing and coordination. This is implemented in the [Processor](https://azure.github.io/azure-sdk-for-cpp/storage.html). 
The Processor is useful when you want to have the partition assignment be dynamically chosen, and balanced with other Processor instances.

More information about Event Hubs features and terminology can be found here: [link](https://learn.microsoft.com/azure/event-hubs/event-hubs-features)


# Examples

Examples for various scenarios can be found on [azure.github.io](https://azure.github.io/azure-sdk-for-cpp/eventhubs.html) or in the samples directory in our GitHub repo for 
[EventHubs](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/eventhubs/azure-messaging-eventhubs/samples).

## Send events

The following example shows how to send events to an event hub:

```cpp
#include <azure/messaging/eventhubs.hpp>

#include <stdexcept>

// Your Event Hubs namespace connection string is available in the Azure portal.
std::string connectionString = "<connection_string>";
std::string eventHubName = "<event_hub_name>";

Azure::Messaging::EventHubs::ProducerClient client(connectionString, eventHubName);

Azure::Messaging::EventHubs::EventDataBatchOptions batchOptions;
batchOptions.PartitionId = "1";
Azure::Messaging::EventHubs::EventDataBatch eventBatch{client.CreateBatch(batchOptions)};

Azure::Messaging::EventHubs::Models::EventData message{"Hello Event Hubs"};

if (!eventBatch.TryAdd(message))
{
  throw std::runtime_error("Failed to add the event to the batch.");
}

client.Send(eventBatch);
```

## Receive events

The following example shows how to receive events from partition 1 on an event hub:

```cpp
#include <azure/messaging/eventhubs.hpp>


// Your Event Hubs namespace connection string is available in the Azure portal.
std::string connectionString = "<connection_string>";
std::string eventHubName = "<event_hub_name>";

Azure::Messaging::EventHubs::ConsumerClient client(connectionString, eventHubName);

Azure::Messaging::EventHubs::PartitionClient partitionClient
    = client.CreatePartitionClient("1");

auto events = partitionClient.ReceiveEvents(1);
```

## Distributed tracing

The `ProducerClient` and the `PartitionClient` create distributed tracing spans through the Azure Core tracing API. This package does not depend on opentelemetry-cpp. The application creates the OpenTelemetry tracer provider and links the `azure-core-tracing-opentelemetry` package.

To get the spans, set the `TracingProvider` field on the client options. The Event Hubs options structs declare this field at the top level:

```cpp
#include <azure/core/tracing/opentelemetry/opentelemetry.hpp>
#include <azure/messaging/eventhubs.hpp>

// Your Event Hubs namespace connection string is available in the Azure portal.
std::string connectionString = "<connection_string>";
std::string eventHubName = "<event_hub_name>";

// Use the opentelemetry-cpp tracer provider of the application.
opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> tracerProvider
    = opentelemetry::trace::Provider::GetTracerProvider();

std::shared_ptr<Azure::Core::Tracing::TracerProvider> provider
    = Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(tracerProvider);

Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
producerOptions.TracingProvider = provider;
Azure::Messaging::EventHubs::ProducerClient producer(
    connectionString, eventHubName, producerOptions);

Azure::Messaging::EventHubs::ConsumerClientOptions consumerOptions;
consumerOptions.TracingProvider = provider;
Azure::Messaging::EventHubs::ConsumerClient consumer(
    connectionString,
    eventHubName,
    Azure::Messaging::EventHubs::DefaultConsumerGroup,
    consumerOptions);
```

The clients create these operation spans:

| Span name | Span kind | Notes |
|---|---|---|
| `ProducerClient.Send` | Producer | One span for each `Send` call. The span covers all the retry attempts. The overloads that take events also create the batch inside the span. |
| `PartitionClient.ReceiveEvents` | Client | One span for each `ReceiveEvents` call. |

The clients also create child spans around calls into the AMQP transport:

| Span name | What its duration measures |
|---|---|
| `ProducerClient.AmqpLink.Open` | Sender-link attachment or reattachment. On uAMQP, an initial attachment can also include lazy connection and session establishment. A retry can create another span. |
| `ProducerClient.AmqpSend` | The synchronous AMQP send through the service disposition. The `az.eventhubs.retry.attempt` attribute identifies the attempt. |
| `PartitionClient.AmqpLink.Open` | Receiver-link attachment or reattachment. On uAMQP, an initial attachment also includes lazy connection and session establishment. |
| `PartitionClient.AmqpReceive` | Time blocked in the AMQP transport waiting for a message or transport error. More than one can occur during one `ReceiveEvents` call. |

The operation span duration is total SDK latency as observed by the caller. Subtracting the non-overlapping child-span durations from the operation duration gives the time spent in SDK work, retry delay, and locally queued message processing. The AMQP child duration is the client-observed service round-trip boundary; it does not isolate processing time inside the Event Hubs service. A receive can complete from AMQP prefetch, so `PartitionClient.AmqpReceive` can be shorter than a network round trip.

The spans have these attributes. The names follow the OpenTelemetry semantic conventions version 1.17.0, which is the schema of the `azure-core-tracing-opentelemetry` package:

| Attribute | Value |
|---|---|
| `az.namespace` | `Microsoft.EventHub` |
| `messaging.system` | `eventhubs` |
| `messaging.destination.name` | The Event Hub name on send spans. |
| `messaging.source.name` | The Event Hub name on receive spans. |
| `messaging.operation` | `publish` on a send span, `receive` on a receive span. |
| `messaging.batch.message_count` | The number of events in the operation. A receive span gets this attribute when the call is successful. |
| `net.peer.name` | The fully qualified namespace. |

AMQP child spans also have these Event Hubs diagnostic attributes:

| Attribute | Value |
|---|---|
| `az.eventhubs.client.id` | A unique ID for the producer or consumer. It includes the configured client name when one exists and a generated UUID. |
| `az.eventhubs.partition.id` | The partition ID, or `<gateway>` for the producer gateway link. |
| `az.eventhubs.amqp.component.type` | `link`. |
| `az.eventhubs.amqp.component.name` | The AMQP link name. |
| `az.eventhubs.amqp.component.id` | A unique ID composed from the client, partition, component generation, and type. |
| `az.eventhubs.amqp.component.generation` | Starts at 1 and increases when the component is recreated. |
| `az.eventhubs.retry.attempt` | The one-based retry or rebuild attempt, when applicable. |

The instrumentation scope is `azure-messaging-eventhubs-cpp` with the package version.

When the application does not set `TracingProvider`, the client creates no spans and records nothing. There is no global fallback provider.

For the OpenTelemetry provider setup, see [Distributed Tracing in the C++ SDK][distributed_tracing].

# Troubleshooting

## Logging

The EventHubs SDK client uses the [Azure SDK log message](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/core/azure-core#sdk-log-messages) functionality to enable diagnostics.

AMQP lifecycle records start with `Event Hubs AMQP lifecycle:` and contain queryable `key='value'` fields. `client.id`, `partition.id`, `component.type`, `component.name`, `component.id`, and `component.generation` identify the exact connection, session, or link. The `event` field records creation, attachment, failure, discard, close, and recreation. Failure records use the warning level; successful recreations use the informational level; initial creation and normal close records use the verbose level.

Failure records use the AMQP error-condition namespace to attribute `amqp:connection:*` and `amqp:session:*` failures to the connection or session. Other failures are attributed to the link operation where the client observed them.

The producer gives every rebuilt connection, session, and link the same component generation. A receiver reattachment increases the link generation while keeping its owning connection and session. Low-level uAMQP connection and link failure records include the same connection container ID or link name, so they can be correlated with the Event Hubs lifecycle records.

Azure Core does not currently expose a provider-neutral metrics API to service libraries. Applications can derive failure and recreation counters from lifecycle records, and latency histograms from the operation and AMQP child span durations.


## Contributing
For details on contributing to this repository, see the [contributing guide][azure_sdk_for_cpp_contributing].

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

### Additional Helpful Links for Contributors  
Many people all over the world have helped make this project better.  You'll want to check out:

* [What are some good first issues for new contributors to the repo?](https://github.com/azure/azure-sdk-for-cpp/issues?q=is%3Aopen+is%3Aissue+label%3A%22up+for+grabs%22)
* [How to build and test your change][azure_sdk_for_cpp_contributing_developer_guide]
* [How you can make a change happen!][azure_sdk_for_cpp_contributing_pull_requests]
* Frequently Asked Questions (FAQ) and Conceptual Topics in the detailed [Azure SDK for C++ wiki](https://github.com/azure/azure-sdk-for-cpp/wiki).

<!-- ### Community-->
### Reporting security issues and security bugs

Security issues and bugs should be reported privately, via email, to the Microsoft Security Response Center (MSRC) <secure@microsoft.com>. You should receive a response within 24 hours. If for some reason you do not, please follow up via email to ensure we received your original message. Further information, including the MSRC PGP key, can be found in the [Security TechCenter](https://www.microsoft.com/msrc/faqs-report-an-issue).

### License

Azure SDK for C++ is licensed under the [MIT](https://github.com/Azure/azure-sdk-for-cpp/blob/main/LICENSE.txt) license.

<!-- LINKS -->
[azure_sdk_for_cpp_contributing]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[azure_sdk_for_cpp_contributing_developer_guide]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#developer-guide
[azure_sdk_for_cpp_contributing_pull_requests]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#pull-requests

[consumer_client]: https://azuresdkdocs.z19.web.core.windows.net/cpp/azure-messaging-eventhubs/latest/class_azure_1_1_messaging_1_1_event_hubs_1_1_consumer_client.html
[producer_client]: https://azuresdkdocs.z19.web.core.windows.net/cpp/azure-messaging-eventhubs/latest/class_azure_1_1_messaging_1_1_event_hubs_1_1_producer_client.html

[source]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/eventhubs
[distributed_tracing]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/doc/DistributedTracing.md
[azure_identity_pkg]: https://azuresdkdocs.z19.web.core.windows.net/cpp/azure-identity/latest/index.html
[default_azure_credential]: https://azuresdkdocs.z19.web.core.windows.net/cpp/azure-identity/latest/index.html#defaultazurecredential

[cppdoc]: https://azuresdkdocs.z19.web.core.windows.net/cpp/azure-messaging-eventhubs/latest/index.html
[cppdoc_examples]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/eventhubs/azure-messaging-eventhubs/samples
