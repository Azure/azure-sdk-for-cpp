# _Azure Storage C++ Protocol Layer

> see https://aka.ms/autorest

## _Configuration

```yaml
package-name: azure-storage-queues
namespace: Azure::Storage::Queues
output-folder: generated
clear-output-folder: true
input-file: https://raw.githubusercontent.com/Azure/azure-rest-api-specs/main/specification/storage/data-plane/Microsoft.QueueStorage/preview/2018-03-28/queue.json
```

## _ModelFour Options

```yaml
modelerfour:
  naming:
    property: pascal
    parameter: pascal
```

## _Customizations for Track 2 Generator

See the [AutoRest samples](https://github.com/Azure/autorest/tree/master/Samples/3b-custom-transformations)
for more about how we're customizing things.

### _Fix Generator Warnings

```yaml
directive:
  - from: swagger-document
    where: $.info
    transform: >
      delete $["x-ms-code-generation-settings"];
  - from: swagger-document
    where: $.definitions
    transform: >
      delete $.DequeuedMessagesList.items.xml;
      delete $.PeekedMessagesList.items.xml;
      delete $.EnqueuedMessageList.items.xml;
      delete $.SignedIdentifiers.items.xml;
      delete $.StorageServiceProperties.properties.Cors.items.xml;
```

### _Delete Unused Query Parameters and Headers

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"].*.*.parameters
    transform: >
      $ = $.filter(p => !(p["$ref"] && (p["$ref"].endsWith("#/parameters/Timeout") || p["$ref"].endsWith("#/parameters/ClientRequestId"))));  
  - from: swagger-document
    where: $["x-ms-paths"].*.*.responses.*.headers
    transform: >
      for (const h in $) {
        if (["x-ms-client-request-id", "x-ms-request-id", "x-ms-version", "Date"].includes(h)) {
          delete $[h];
        }
      }
```

### _API Version

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.ApiVersion = {
        "type": "string",
        "x-ms-export": true,
        "x-namespace": "_detail",
        "x-ms-enum": {
          "name": "ApiVersion",
          "modelAsString": false
          },
        "enum": ["2018-03-28"],
        "description": "The version used for the operations to Azure storage services."
      };
```

### _Rename Operations

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]
    transform: >
      $["/{queueName}/messages"].get.operationId = "Queue_ReceiveMessages";
      $["/{queueName}/messages"].delete.operationId = "Queue_ClearMessages";
      $["/{queueName}/messages"].post.operationId = "Queue_EnqueueMessage";
      $["/{queueName}/messages?peekonly=true"].get.operationId = "Queue_PeekMessages";
      $["/{queueName}/messages/{messageid}"].put.operationId = "Queue_UpdateMessage";
      $["/{queueName}/messages/{messageid}"].delete.operationId = "Queue_DeleteMessage";
```

### _Define names for return types

```yaml
directive:
  - from: swagger-document
    where: $
    transform: >
      const operationReturnTypeNames = new Map(Object.entries({
        "Queue_ClearMessages": "ClearMessagesResult",
        "Queue_UpdateMessage": "UpdateMessageResult",
        "Queue_DeleteMessage": "DeleteMessageResult",
      }));
      for (const url in $["x-ms-paths"]) {
        for (const verb in $["x-ms-paths"][url]) {
          const operation = $["x-ms-paths"][url][verb];
          if (!operationReturnTypeNames.has(operation.operationId)) {
            continue;
          }
          const returnTypeName = operationReturnTypeNames.get(operation.operationId);
          const status_codes = Object.keys(operation.responses).filter(s => s !== "default");
          const emptySchemaDefinition = {
            "type": "object",
            "x-ms-client-name": returnTypeName,
            "x-ms-sealed": false,
            "properties": {
              "__placeHolder": {"type": "integer"}
            }
          };
          $.definitions[returnTypeName] = emptySchemaDefinition;
          status_codes.map(i => {
            operation.responses[i].schema = {"$ref": `#/definitions/${returnTypeName}`};
          });
        }
      }
```

### _Global Changes for Definitions, Types etc.

```yaml
directive:
  - from: swagger-document
    where: $.parameters
    transform: >
      $.NumOfMessages.format = "int64";
      $.MaxResults["x-ms-client-name"] = "MaxResults";
```

### _GetServiceProperties

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.QueueServiceProperties = $.StorageServiceProperties;
      delete $.StorageServiceProperties;
      $.QueueServiceProperties.xml = { "name": "StorageServiceProperties" };
      $.RetentionPolicy.properties["Enabled"]["x-ms-client-name"] = "IsEnabled";
      $.AnalyticsLogging = $.Logging;
      delete $.Logging;
      $.AnalyticsLogging.xml = { "name": "Logging" };
      $.QueueServiceProperties.properties["Logging"]["$ref"] = "#/definitions/AnalyticsLogging";
      $.Metrics["type"] = "object";
      $.Metrics.properties["Enabled"]["x-ms-client-name"] = "IsEnabled";
      $.Metrics.properties["IncludeAPIs"]["x-ms-client-name"] = "IncludeApis";
      delete $.Metrics.required;
      $.Metrics.properties["IncludeAPIs"]["x-nullable"] = true;
  - from: swagger-document
    where: $.parameters
    transform: >
      $.QueueServiceProperties = $.StorageServiceProperties;
      $.QueueServiceProperties.name = "QueueServiceProperties";
      $.QueueServiceProperties.schema["$ref"] = "#/definitions/QueueServiceProperties";
      delete $.StorageServiceProperties;
  - from: swagger-document
    where: $["x-ms-paths"]["/?restype=service&comp=properties"]
    transform: >
      $.put.parameters[0]["$ref"] = "#/parameters/QueueServiceProperties";
      $.get.responses["200"].schema["$ref"] = "#/definitions/QueueServiceProperties";
```

### _GetServiceStatistics 

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.ServiceStatistics = $.StorageServiceStats;
      delete $.StorageServiceStats;
      $.ServiceStatistics["xml"] = { "name": "StorageServiceStats" };
      $.GeoReplication.properties["LastSyncTime"]["x-ms-client-name"] = "LastSyncedOn";
      $.GeoReplication.required = ["Status"];
      $.GeoReplication.properties["Status"]["x-ms-enum"]["name"] = "GeoReplicationStatus";
      $.GeoReplication.description = "Geo-Replication information for the Secondary Storage Service";
  - from: swagger-document
    where: $["x-ms-paths"]["/?restype=service&comp=stats"]
    transform: >
      $.get.responses["200"].schema["$ref"] = "#/definitions/ServiceStatistics";
```

### _ListQueues

```yaml
directive:
  - from: swagger-document
    where: $.parameters
    transform: >
      $.ListQueuesInclude.items["x-ms-enum"]["name"] = "ListQueuesIncludeFlags";
  - from: swagger-document
    where: $.definitions
    transform: >
      $.Metadata = {"type": "object", "x-ms-format": "caseinsensitivemap", properties: {"__placeHolder" : {"type": "integer"}}, "description": "A set of name-value pairs associated with this queue."};
      $.ListQueuesSegmentResponse["x-ms-client-name"] = "ListQueuesResult";
      $.ListQueuesSegmentResponse["x-namespace"] = "_detail";
      $.ListQueuesSegmentResponse.properties["QueueItems"]["x-ms-client-name"] = "Items";
      $.ListQueuesSegmentResponse.properties["QueueItems"]["x-ms-xml"] = {"name": "Queues"};
      $.ListQueuesSegmentResponse.properties["NextMarker"]["x-ms-client-name"] = "ContinuationToken";
      $.ListQueuesSegmentResponse.required = ["ServiceEndpoint", "Prefix"];
      delete $.ListQueuesSegmentResponse.properties["Marker"];
      delete $.ListQueuesSegmentResponse.properties["MaxResults"];
```

### _CreateQueue

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.CreateQueueResult = {
        "type": "object",
        "properties": {
          "Created": {"type": "boolean", "x-ms-xml": {"name": ""}, "description": "Indicates if the queue was successfully created by this operation."}
        }
      };
  - from: swagger-document
    where: $["x-ms-paths"]["/{queueName}"].put.responses
    transform: >
      $["201"].schema = {"$ref": "#/definitions/CreateQueueResult"};
      $["204"].schema = {"$ref": "#/definitions/CreateQueueResult"};
```

### _DeleteQueue

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{queueName}"].delete.responses["204"]
    transform: >
      $.schema = {
        "type": "object",
        "x-ms-client-name": "DeleteQueueResult",
        "x-ms-sealed": false,
        "properties": {
          "Deleted": {"type": "boolean", "x-ms-client-default": true, "x-ms-xml": {"name": ""}, "description": "Indicates if the queue was successfully deleted by this operation."}
        }
      };
```


### _GetQueueProperties

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{queueName}?comp=metadata"].get.responses["200"].headers
    transform: >
      $["x-ms-meta"]["x-ms-format"] = "caseinsensitivemap";
      $["x-ms-approximate-messages-count"]["x-ms-client-name"] = "ApproximateMessageCount";
  - from: swagger-document
    where: $["x-ms-paths"]["/{queueName}?comp=metadata"].get.responses["200"]
    transform: >
      $.schema = {
        "type": "object",
        "x-ms-client-name": "QueueProperties",
        "x-ms-sealed": false,
        "properties": {"__placeHolder": {"type": "integer"}}
      };
```

### _GetQueueAccessPolicy

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      delete $.SignedIdentifier.properties["AccessPolicy"];
      $.SignedIdentifier.properties = {...$.SignedIdentifier.properties, ...$.AccessPolicy.properties};
      $.SignedIdentifier.properties["Start"]["x-ms-client-name"] = "StartsOn";
      $.SignedIdentifier.properties["Start"]["x-ms-xml"] = {"name": "AccessPolicy/Start"};
      $.SignedIdentifier.properties["Expiry"]["x-ms-client-name"] = "ExpiresOn";
      $.SignedIdentifier.properties["Expiry"]["x-ms-xml"] = {"name": "AccessPolicy/Expiry"};
      $.SignedIdentifier.properties["Permission"]["x-ms-client-name"] = "Permissions";
      $.SignedIdentifier.properties["Permission"]["x-ms-xml"] = {"name": "AccessPolicy/Permission"};
      $.SignedIdentifier.required = ["Id", "Permission"];
      delete $.AccessPolicy;
  - from: swagger-document
    where: $["x-ms-paths"]["/{queueName}?comp=acl"].get.responses["200"]
    transform: >
      $.schema = {
        "type": "object",
        "x-ms-client-name": "QueueAccessPolicy",
        "xml": {"name": "."},
        "x-ms-sealed": false,
        "properties": {
          "SignedIdentifiers": {"$ref": "#/definitions/SignedIdentifiers"}
        }
      }
```

### _ReceiveMessages

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.DequeuedMessageItem["x-ms-client-name"] = "QueueMessage";
      $.DequeuedMessageItem.properties["InsertionTime"]["x-ms-client-name"] = "InsertedOn";
      $.DequeuedMessageItem.properties["ExpirationTime"]["x-ms-client-name"] = "ExpiresOn";
      $.DequeuedMessageItem.properties["TimeNextVisible"]["x-ms-client-name"] = "NextVisibleOn";
  - from: swagger-document
    where: $["x-ms-paths"]["/{queueName}/messages"].get.responses["200"]
    transform: >
      $.schema = {
        "type": "object",
        "x-ms-client-name": "ReceivedMessages",
        "xml": {"name": "."},
        "x-ms-sealed": false,
        "properties": {
          "Messages": {"$ref": "#/definitions/DequeuedMessagesList", "x-ms-xml": {"name": "QueueMessagesList"}}
        }
      }
```

### _UpdateMessage

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.QueueMessage["x-ms-client-name"] = "QueueMessageInternal";
      $.QueueMessage.xml = {"name": "QueueMessage"};
      $.QueueMessage["x-namespace"] = "_detail";
```

### _EnqueueMessage

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.EnqueuedMessage["x-ms-client-name"] = "EnqueueMessageResult";
      $.EnqueuedMessage.properties["InsertionTime"]["x-ms-client-name"] = "InsertedOn";
      $.EnqueuedMessage.properties["ExpirationTime"]["x-ms-client-name"] = "ExpiresOn";
      $.EnqueuedMessage.properties["TimeNextVisible"]["x-ms-client-name"] = "NextVisibleOn";
      $.EnqueuedMessage.xml.name = "QueueMessagesList/QueueMessage";
  - from: swagger-document
    where: $["x-ms-paths"]["/{queueName}/messages"].post.responses["201"]
    transform: >
      $.schema["$ref"] = "#/definitions/EnqueuedMessage";
```

### _PeekMessages

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.PeekedMessageItem["x-ms-client-name"] = "PeekedQueueMessage";
      $.PeekedMessageItem.properties["InsertionTime"]["x-ms-client-name"] = "InsertedOn";
      $.PeekedMessageItem.properties["ExpirationTime"]["x-ms-client-name"] = "ExpiresOn";
  - from: swagger-document
    where: $["x-ms-paths"]["/{queueName}/messages?peekonly=true"].get.responses["200"]
    transform: >
      $.schema = {
        "type": "object",
        "x-ms-client-name": "PeekedMessages",
        "xml": {"name": "."},
        "x-ms-sealed": false,
        "properties": {
          "Messages": {"$ref": "#/definitions/PeekedMessagesList", "x-ms-xml": {"name": "QueueMessagesList"}}
        }
      }
```

### _UpdateMessage

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{queueName}/messages/{messageid}"].put
    transform: >
      $.responses["204"].headers["x-ms-time-next-visible"]["x-ms-client-name"] = "NextVisibleOn";
```

### _UpdateMessageVisibility

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]
    transform: >
      $["/{queueName}/messages/{messageid}?updatevisibilityonly=true"] = {};
      $["/{queueName}/messages/{messageid}?updatevisibilityonly=true"].put = JSON.parse(JSON.stringify($["/{queueName}/messages/{messageid}"].put));
  - from: swagger-document
    where: $["x-ms-paths"]["/{queueName}/messages/{messageid}?updatevisibilityonly=true"]
    transform: >
      $.put.operationId = "Queue_UpdateMessageVisibility";
      $.put.parameters.shift();
  - from: swagger-document
    where: $.definitions
    transform: >
      $.UpdateMessageResult.description = "Response type for #Azure::Storage::Queues::QueueClient::UpdateMessage.";
```