# Eventhub Samples

This repository contains samples for the Azure Event Hubs service.

## Sample Requirements

These samples are written with the assumption that the following environment
variables have been set by the user:

* EVENTHUBS_CONNECTION_STRING - The service connection string for the eventhubs instance.
* EVENTHUB_NAME - Name of the eventhubs instance to communicate with.
* EVENTHUBS_HOST - Fully qualified domain name for the eventhubs instance.
* AZURE_TENANT_ID - The tenant ID for the user or service principal which has
been granted access to the eventhubs service instance.
* AZURE_CLIENT_ID - The client ID for the user or service principal which has been 
granted access to the eventhubs service instance.
* AZURE_CLIENT_SECRET - The client secret for the user or service principal
  which has been granted access to the eventhubs service instance.

The tests also assume that the currently logged on user is authorized to call
into the Event Hubs service instance because they use [Azure::Core::Credentials::TokenCredential](https://azuresdkdocs.blob.core.windows.net/$web/cpp/azure-core/1.3.1/class_azure_1_1_core_1_1_credentials_1_1_token_credential.html) for authorization.


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

