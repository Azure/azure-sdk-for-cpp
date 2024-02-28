<!-- cspell:words azeventhubs  -->
# Azure Event Hubs Client Package for C++

[Azure Event Hubs](https://azure.microsoft.com/services/event-hubs/) is a big data streaming platform and event ingestion service from Microsoft. For more information about Event Hubs see: [link](https://docs.microsoft.com/azure/event-hubs/event-hubs-about).

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
- An [Event Hub namespace](https://docs.microsoft.com/azure/event-hubs/).
- An Event Hub. You can create an event hub in your Event Hubs Namespace using the [Azure Portal](https://docs.microsoft.com/azure/event-hubs/event-hubs-create), or the [Azure CLI](https://docs.microsoft.com/azure/event-hubs/event-hubs-quickstart-cli).

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

<!-- NOTE: Fix dead Links -->
#### Using a service principal
 - ConsumerClient: [link][consumer_client]
 - ProducerClient: [link][producer_client]

<!-- NOTE: Fix dead links -->
#### Using a connection string
 - ConsumerClient: [link](https://azure.github.io/azure-sdk-for-cpp/storage.html)
 - ProducerClient: [link](https://azure.github.io/azure-sdk-for-cpp/storage.html)

# Key concepts

An Event Hub [**namespace**](https://docs.microsoft.com/azure/event-hubs/event-hubs-features#namespace) can have multiple event hubs.
Each event hub, in turn, contains [**partitions**](https://docs.microsoft.com/azure/event-hubs/event-hubs-features#partitions) which 
store events.

<!-- NOTE: Fix dead links -->
Events are published to an event hub using an [event publisher](https://docs.microsoft.com/azure/event-hubs/event-hubs-features#event-publishers). In this package, the event publisher is the [ProducerClient](https://azure.github.io/azure-sdk-for-cpp/storage.html)

Events can be consumed from an event hub using an [event consumer](https://docs.microsoft.com/azure/event-hubs/event-hubs-features#event-consumers). In this package there are two types for consuming events: 
- The basic event consumer is the PartitionClient, in the [ConsumerClient][consumer_client]. This consumer is useful if you already known which partitions you want to receive from.
- A distributed event consumer, which uses Azure Blobs for checkpointing and coordination. This is implemented in the [Processor](https://azure.github.io/azure-sdk-for-cpp/storage.html). 
The Processor is useful when you want to have the partition assignment be dynamically chosen, and balanced with other Processor instances.

More information about Event Hubs features and terminology can be found here: [link](https://docs.microsoft.com/azure/event-hubs/event-hubs-features)


# Examples

Examples for various scenarios can be found on [azure.github.io](https://azure.github.io/azure-sdk-for-cpp/eventhubs.html) or in the samples directory in our GitHub repo for 
[EventHubs](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/eventhubs/azure-messaging-eventhubs/samples).

## Send events

The following example shows how to send events to an event hub:

```cpp
#include <azure/messaging/eventhubs.hpp>

// Your Event Hubs namespace connection string is available in the Azure portal.
std::string connectionString = "<connection_string>";
std::string eventHubName = "<event_hub_name>";

Azure::Messaging::EventHubs::EventDataBatchOptions batchOptions;
batchOptions.PartitionId = "1";
Azure::Messaging::EventHubs::EventDataBatch eventBatch(batchOptions);

Azure::Messaging::EventHubs::Models::EventData message;
message.Body.Data = {'H', 'e', 'l', 'l', 'o', '2'};

eventBatch.AddMessage(message);

Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
producerOptions.Name = "sender-link";
producerOptions.ApplicationID = "some";

auto client = Azure::Messaging::EventHubs::ProducerClient(
      connectionString, eventHubName, producerOptions);
auto result = client.SendEventDataBatch(eventBatch);
```

## Receive events

The following example shows how to receive events from partition 1 on an event hub:

```cpp
#include <azure/messaging/eventhubs.hpp>


// Your Event Hubs namespace connection string is available in the Azure portal.
std::string connectionString = "<connection_string>";
std::string eventHubName = "<event_hub_name>";

auto client = Azure::Messaging::EventHubs::ConsumerClient(
	connectionString, eventHubName);

Azure::Messaging::EventHubs::PartitionClient partitionClient
        = client.CreatePartitionClient("1");

auto events = partitionClient.ReceiveEvents(1);
```


# Troubleshooting

## Logging

The EventHubs SDK client uses the [Azure SDK log message](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/core/azure-core#sdk-log-messages) functionality to 
enable diagnostics.


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

[consumer_client]: https://azuresdkdocs.blob.core.windows.net/$web/cpp/azure-messaging-eventhubs/latest/class_azure_1_1_messaging_1_1_event_hubs_1_1_consumer_client.html
[producer_client]: https://azuresdkdocs.blob.core.windows.net/$web/cpp/azure-messaging-eventhubs/1.0.0-beta.1/class_azure_1_1_messaging_1_1_event_hubs_1_1_producer_client.html

[source]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/eventhubs
[azure_identity_pkg]: https://azuresdkdocs.blob.core.windows.net/$web/cpp/azure-identity/latest/index.html
[default_azure_credential]: https://azuresdkdocs.blob.core.windows.net/$web/cpp/azure-identity/latest/index.html#defaultazurecredential

[cppdoc]: https://azuresdkdocs.blob.core.windows.net/$web/cpp/azure-messaging-eventhubs/latest/index.html
[cppdoc_examples]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/eventhubs/azure-messaging-eventhubs/samples

![Impressions](https://azure-sdk-impressions.azurewebsites.net/api/impressions/azure-sdk-for-cpp%2Fsdk%2Feventhubs%2FREADME.png)
