# Eventhub Samples

This repository contains samples for the Azure Event Hubs service.

## Sample Requirements

These samples are written with the assumption that the following environment
variables have been set by the user:

* EVENTHUBS_CONNECTION_STRING - The service connection string for the eventhubs instance.
* EVENTHUB_NAME - Name of the eventhubs instance to communicate with.
* EVENTHUBS_HOST - Fully qualified domain name for the eventhubs instance.

The tests also assume that the currently logged on user is authorized to call
into the Event Hubs service instance because they use [Azure::Core::Credentials::TokenCredential](https://azuresdkdocs.z19.web.core.windows.net/cpp/azure-core/latest/class_azure_1_1_core_1_1_credentials_1_1_token_credential.html) for authorization.

### Setting Environment Variables

For the samples which use a connection string, the connection string can be retrieved using the Azure CLI with the following:

```pwsh
az eventhubs namespace authorization-rule keys list --resource-group <your resource group> --namespace-name <your namespace name> --name RootManageSharedAccessKey
```

```json
{
  "keyName": "RootManageSharedAccessKey",
  "primaryConnectionString": "Endpoint=sb://REDACTED.servicebus.windows.net/;SharedAccessKeyName=RootManageSharedAccessKey;SharedAccessKey=REDACTED",
  "primaryKey": "REDACTED",
  "secondaryConnectionString": "Endpoint=sb://REDACTED.servicebus.windows.net/;SharedAccessKeyName=RootManageSharedAccessKey;SharedAccessKey=REDACTED",
  "secondaryKey": "REDACTED"
}
```

The value of the `primaryConnectionString` property should be used as the `EVENTHUBS_CONNECTION_STRING` environment variable.


## Samples

| Sample | Description |
|--------|-------------|
| basic-operations/create_producer.cpp | This sample demonstrates how to create an `EventHubProducerClient` using a connection string. |
| basic-operation/create_consumer.cpp | This sample demonstrates how to create an `EventHubConsumerClient` using a connection string. |
| basic-operations/create_producer-aad.cpp | This sample demonstrates how to create an `EventHubProducerClient` using an Azure Active Directory account. |
| basic-operation/create_consumer-aad.cpp | This sample demonstrates how to create an `EventHubConsumerClient` using an Azure Active Directory account. |
| | |
| produce-events/produce_events.cpp | This sample demonstrates how to send events to an Event Hub using the `EventHubProducerClient`. |
| produce-events/produce_events_aad.cpp | This sample demonstrates how to send events to an Event Hub using the `EventHubProducerClient` using an Azure Active Directory account. |
| consume-events/consume_events.cpp | This sample demonstrates how to receive events from an Event Hub using the `EventHubConsumerClient`. |
| consume-events/consume_events_aad.cpp | This sample demonstrates how to receive events from an Event Hub using the `EventHubConsumerClient` using an Azure Active Directory account. |

