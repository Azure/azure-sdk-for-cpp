# Eventhub Samples

This repository contains samples for the Azure Event Hubs service.

## Sample Requirements

These samples are written with the assumption that the following environment
variables have been set by the user:

* EVENTHUB_CONNECTION_STRING - The Event Hubs namespace or Event Hub connection string.
* EVENTHUB_NAME - The Event Hub name. This is optional when the connection string contains `EntityPath`.
* EVENTHUBS_HOST - Fully qualified namespace used by the Microsoft Entra ID samples.

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

The value of the `primaryConnectionString` property should be used as the `EVENTHUB_CONNECTION_STRING` environment variable.


## Samples

| Sample | Description |
|--------|-------------|
| basic-operations/create_producer.cpp | Create a `ProducerClient` using a connection string. |
| basic-operations/create_consumer.cpp | Create a `ConsumerClient` using a connection string. |
| basic-operations/create_producer_aad.cpp | Create a `ProducerClient` using a Microsoft Entra ID credential. |
| basic-operations/create_consumer_aad.cpp | Create a `ConsumerClient` using a Microsoft Entra ID credential. |
| | |
| produce-events/produce_events.cpp | Send events using a connection string. |
| produce-events/produce_events_aad.cpp | Send events using a Microsoft Entra ID credential. |
| consume-events/consume_events.cpp | Receive events using a connection string. |
| consume-events/consume_events_aad.cpp | Receive events using a Microsoft Entra ID credential. |
