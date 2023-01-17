# Azure Storage C++ Protocol Layer

> see https://aka.ms/autorest

## Configuration

```yaml
package-name: azure-storage-blobs
namespace: Azure::Storage::Blobs
output-folder: generated
clear-output-folder: true
input-file: https://raw.githubusercontent.com/Azure/azure-rest-api-specs/main/specification/storage/data-plane/Microsoft.BlobStorage/preview/2021-12-02/blob.json
```

## ModelFour Options

```yaml
modelerfour:
  naming:
    property: pascal
    parameter: pascal
```

## Customizations for Track 2 Generator

See the [AutoRest samples](https://github.com/Azure/autorest/tree/master/Samples/3b-custom-transformations)
for more about how we're customizing things.

### Fix Generator Warnings

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      if ($.DataLakeStorageError) {
        $.DataLakeStorageError.properties["error"]["type"] = "object";
      }
      delete $.QueryRequest.properties["InputSerialization"]["xml"];
      delete $.QueryRequest.properties["OutputSerialization"]["xml"];
      delete $.QuerySerialization.properties["Format"]["xml"];
  - from: swagger-document
    where: $.info
    transform: >
      delete $["x-ms-code-generation-settings"];
```

### Delete Unused Query Parameters and Headers

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

### Delete Unused Operations

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]
    transform: >
      delete $["/{containerName}?restype=account&comp=properties"];
      delete $["/{containerName}/{blob}?restype=account&comp=properties"];
      delete $["/{filesystem}/{path}?action=setAccessControl&blob"];
      delete $["/{filesystem}/{path}?action=getAccessControl&blob"];
      delete $["/{filesystem}/{path}?FileRename"];
      
      for (const operation in $) {
        for (const verb in $[operation]) {
          const tag = $[operation][verb]["tags"][0];
          if (["directory"].includes(tag)) {
            delete $[operation];
          }
          break;
        }
      }
```

### API Version

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
        "enum": ["2021-12-02"],
        "description": "The version used for the operations to Azure storage services."
      };
```

### Rename Operations

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]
    transform: >
      $["/?comp=list"].get.operationId = "Service_ListBlobContainers";
      $["/?comp=blobs"].get.operationId = "Service_FindBlobsByTags";
      $["/{containerName}?restype=container&comp=blobs"].get.operationId = "BlobContainer_FindBlobsByTags";
      $["/{containerName}/{blob}?comp=incrementalcopy"].put.operationId = "PageBlob_StartCopyIncremental";
      $["/{containerName}?restype=container&comp=list&flat"].get.operationId = "BlobContainer_ListBlobs";
      $["/{containerName}?restype=container&comp=list&hierarchy"].get.operationId = "BlobContainer_ListBlobsByHierarchy";
      $["/{containerName}?restype=container&comp=undelete"].put.operationId = "BlobContainer_Undelete";
      $["/{containerName}/{blob}?comp=copy"].put.operationId = "Blob_StartCopyFromUri";
      $["/{containerName}/{blob}?comp=copy&sync"].put.operationId = "Blob_CopyFromUri";
      $["/{containerName}/{blob}?comp=copy&copyid"].put.operationId = "Blob_AbortCopyFromUri";
      $["/{containerName}/{blob}?comp=block&fromURL"].put.operationId = "BlockBlob_StageBlockFromUri";
      $["/{containerName}/{blob}?comp=page&update&fromUrl"].put.operationId = "PageBlob_UploadPagesFromUri";
      $["/{containerName}/{blob}?comp=appendblock&fromUrl"].put.operationId = "AppendBlob_AppendBlockFromUri";
      $["/{containerName}/{blob}?BlockBlob&fromUrl"].put.operationId = "BlockBlob_UploadFromUri";
      for (const operation in $) {
        for (const verb in $[operation]) {
          if ($[operation][verb].operationId && $[operation][verb].operationId.startsWith("Container_")) {
            $[operation][verb].operationId = "Blob" + $[operation][verb].operationId;
          }
        }
      }
```

### Define names for return types

```yaml
directive:
  - from: swagger-document
    where: $
    transform: >
      const operationReturnTypeNames = new Map(Object.entries({
        "Service_GetAccountInfo": "AccountInfo",
        "BlobContainer_GetProperties": "BlobContainerProperties",
        "Blob_SetTier": "SetBlobAccessTierResult",
        "PageBlob_UploadPages": "UploadPagesResult",
        "PageBlob_ClearPages": "ClearPagesResult",
        "PageBlob_UploadPagesFromUri": "UploadPagesFromUriResult",
        "PageBlob_UpdateSequenceNumber": "UpdateSequenceNumberResult",
        "PageBlob_StartCopyIncremental": "StartBlobCopyIncrementalResult",
        "AppendBlob_AppendBlock": "AppendBlockResult",
        "AppendBlob_AppendBlockFromUri": "AppendBlockFromUriResult",
        "BlockBlob_StageBlock": "StageBlockResult",
        "BlockBlob_StageBlockFromUri": "StageBlockFromUriResult",
        "BlockBlob_CommitBlockList": "CommitBlockListResult",
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

### Return Type namespace

```yaml
directive:
  - from: swagger-document
    where: $
    transform: >
      const operations = [
        "BlobContainer_Undelete",
        "BlobContainer_AcquireLease",
        "BlobContainer_ReleaseLease",
        "BlobContainer_RenewLease",
        "BlobContainer_BreakLease",
        "BlobContainer_ChangeLease",
        "BlobContainer_Rename",
        "Blob_AcquireLease",
        "Blob_ReleaseLease",
        "Blob_RenewLease",
        "Blob_BreakLease",
        "Blob_ChangeLease",
        "Blob_StartCopyFromUri",
        "PageBlob_StartCopyIncremental",
      ];
      for (const url in $["x-ms-paths"]) {
        for (const verb in $["x-ms-paths"][url]) {
          if (!operations.includes($["x-ms-paths"][url][verb].operationId)) continue;
          const operation = $["x-ms-paths"][url][verb];

          const status_codes = Object.keys(operation.responses).filter(s => s !== "default");
          status_codes.forEach((status_code, i) => {
            if (!operation.responses[status_code].schema) {
              const operationId = operation.operationId;
              const clientName = operationId.substr(0, operationId.indexOf("_"));
              const operationName = operationId.substr(operationId.indexOf("_") + 1);
              let operationWords = operationName.split(/(?=[A-Z])/);
              operationWords.splice(1, 0, clientName);
              const defaultReturnTypeName = operationWords.join("") + "Result";
              operation.responses[status_code].schema = {
                "type": "object",
                "x-ms-sealed": false,
                "x-ms-client-name": defaultReturnTypeName,
                "x-namespace": "_detail",
                "properties": {
                  "__placeHolder": {"type": "integer"}
                }
              };
            } else if (operation.responses[status_code].schema["$ref"]) {
              let obj = $;
              for (const p of operation.responses[status_code].schema["$ref"].split("/").slice(1)) {
                obj = obj[p];
              }
              obj["x-namespace"] = "_detail";
            } else {
              operation.responses[status_code].schema["x-namespace"] = "_detail";
            }
          });
        }
      }
  - from: swagger-document
    where: $.definitions
    transform: >
      $.ListContainersSegmentResponse["x-namespace"] = "_detail";
      $.FilterBlobSegment["x-namespace"] = "_detail";
      $.ListBlobsFlatSegmentResponse["x-namespace"] = "_detail";
      $.ListBlobsHierarchySegmentResponse["x-namespace"] = "_detail";
      $.KeyInfo["x-namespace"] = "_detail";
      $.BlockLookupList["x-namespace"] = "_detail";
```

### Rename enums

```yaml
directive:
  - from: swagger-document
    where: $.parameters
    transform: >
      $.ListContainersInclude.items["x-ms-enum"]["name"] = "ListBlobContainersIncludeFlags";
      $.ListBlobsInclude.items["x-ms-enum"]["name"] = "ListBlobsIncludeFlags";
      $.ListBlobsInclude.items["x-ms-enum"]["values"] = [
          {"value": "copy", "name": "Copy"},
          {"value": "deleted", "name": "Deleted"},
          {"value": "metadata", "name": "Metadata"},
          {"value": "snapshots", "name": "Snapshots"},
          {"value": "uncommittedblobs", "name": "UncomittedBlobs"},
          {"value": "versions", "name": "Versions"},
          {"value": "tags", "name": "Tags"},
          {"value": "immutabilitypolicy", "name": "ImmutabilityPolicy"},
          {"value": "legalhold", "name": "LegalHold"},
          {"value": "deletedwithversions", "name": "DeletedWithVersions"}
      ];
      $["ListBlobsShowOnly"]= {
        "name": "showonly",
        "x-ms-client-name": "ShowOnly",
        "in": "query",
        "required": false,
        "type": "string",
        "x-ms-parameter-location": "method",
        "description": "Include this parameter to specify one or more datasets to include in the response."
      };
      $.DeleteSnapshots["x-ms-enum"]["name"] = "DeleteSnapshotsOption";
      $.DeleteSnapshots["x-ms-enum"]["values"] = [{"value": "include", "name": "IncludeSnapshots"},{"value":"only", "name": "OnlySnapshots"}];
      $.BlobExpiryOptions["x-ms-enum"]["name"] = "ScheduleBlobExpiryOriginType";
      $.SequenceNumberAction["x-ms-enum"]["name"] = "SequenceNumberAction";
      delete $.EncryptionAlgorithm["enum"];
      delete $.EncryptionAlgorithm["x-ms-enum"];
      $.ImmutabilityPolicyMode.enum = $.ImmutabilityPolicyMode.enum.map(e => e.toLowerCase());
      $.CopySourceTags["x-ms-enum"]["name"] = "BlobCopySourceTagsMode";
      delete $.FilterBlobsInclude;
  - from: swagger-document
    where: $.definitions
    transform: >
      $.GeoReplication.properties["Status"]["x-ms-enum"]["name"] = "GeoReplicationStatus";
      $.LeaseStatus["x-ms-enum"]["name"] = "LeaseStatus";
      $.LeaseState["x-ms-enum"]["name"] = "LeaseState";
      $.CopyStatus["x-ms-enum"]["name"] = "CopyStatus";
      $.PublicAccessType["x-ms-enum"]["values"] = [{"value": "container", "name": "BlobContainer"}, {"value": "blob", "name": "Blob"}, {"value": "", "name": "None"}];
      $.PublicAccessType.description = "Specifies whether data in the container may be accessed publicly and the level of access";
      $.AccessTier["x-ms-enum"]["values"] = [
          {"value": "p1", "name": "P1"},
          {"value": "p2", "name": "P2"},
          {"value": "p3", "name": "P3"},
          {"value": "p4", "name": "P4"},
          {"value": "p6", "name": "P6"},
          {"value": "p10", "name": "P10"},
          {"value": "p15", "name": "P15"},
          {"value": "p20", "name": "P20"},
          {"value": "p30", "name": "P30"},
          {"value": "p40", "name": "P40"},
          {"value": "p50", "name": "P50"},
          {"value": "p60", "name": "P60"},
          {"value": "p70", "name": "P70"},
          {"value": "p80", "name": "P80"},
          {"value": "Hot", "name": "Hot"},
          {"value": "Cool", "name": "Cool"},
          {"value": "Archive", "name": "Archive"},
          {"value": "Premium", "name": "Premium"},
          {"value": "Cold", "name": "Cold"}
      ];
      $.AccessTier.description = "The tier of page blob on a premium storage account or tier of block blob on blob storage or general purpose v2 account.";
      $.EncryptionAlgorithm = {
        "type": "string",
        "enum": ["AES256"],
        "x-ms-enum": {
          "name": "EncryptionAlgorithmType",
          "modelAsString": false,
          "values": [{"value": "__placeHolder", "name": "__placeHolder"}, {"value": "AES256", "name": "Aes256"}]
        },
        "x-ms-export": true,
        "description": "The algorithm used to produce the encryption key hash. Currently, the only accepted value is \"AES256\". Must be provided if the x-ms-encryption-key header is provided."
      };
      $.BlockType = {
        "type": "string",
        "enum": ["Committed", "Uncommitted", "Latest"],
        "x-ms-enum": {
          "name": "BlockType",
          "modelAsString": false
        },
        "x-ms-export": true,
        "description": "Extensible enum used to specify how the service should look for a block ID."
      };
      $.ImmutabilityPolicyMode = {
        "type": "string",
        "enum": ["unlocked", "locked"],
        "x-ms-enum": {
          "name": "BlobImmutabilityPolicyMode",
          "modelAsString": false
        },
        "description": "Specifies the immutability policy mode set on the blob."
      }
  - from: swagger-document
    where: $["x-ms-paths"].*.*.responses.*.headers
    transform: >
      for (var header in $) {
        if (header === "x-ms-lease-status") {
          $[header]["x-ms-enum"]["name"] = "LeaseStatus";
        }
        if (header === "x-ms-lease-state") {
          $[header]["x-ms-enum"]["name"] = "LeaseState";
        }
        if (header === "x-ms-copy-status") {
          $[header]["x-ms-enum"]["name"] = "CopyStatus";
        }
        if (header === "x-ms-immutability-policy-mode" && $[header].enum) {
          $[header].enum = $[header].enum.filter(e => e !== "Mutable").map(e => e.toLowerCase());
        }
      }
```

### Global Changes for Definitions, Types etc.

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"].*.*.responses.*.headers
    transform: >
      for (const h in $) {
        if (h === "x-ms-encryption-key-sha256") {
          $[h]["format"] = "byte";
        }
        if (h === "x-ms-meta") {
          $[h]["x-ms-format"] = "caseinsensitivemap";
        }
        if (h === "x-ms-lease-id" && $[h].description === "Uniquely identifies a blobs' lease") {
          $[h].description = "Uniquely identifies a blob's lease";
        }
      }
  - from: swagger-document
    where: $.parameters
    transform: >
      $.EncryptionKeySha256["format"] = "byte";
      $.BlobContentType["required"] = true;
      $.BlobContentEncoding["required"] = true;
      $.BlobContentLanguage["required"] = true;
      $.BlobContentMD5["required"] = true;
      $.BlobContentDisposition["required"] = true;
      $.BlobCacheControl["required"] = true;
      $.MaxResults["x-ms-client-name"] = "MaxResults";
      $.BlobPublicAccess["required"] = true;
  - from: swagger-document
    where: $.definitions
    transform: >
      $.HashAlgorithm = {
        "type": "string",
        "x-ms-external": true,
        "x-namespace": "::Azure::Storage",
        "enum": ["Md5", "Crc64"],
        "x-ms-enum": {
          "name": "HashAlgorithm",
          "modelAsString": false
        }
      };
      $.ContentHash = {
        "type": "object",
        "x-ms-external": true,
        "x-namespace": "::Azure::Storage",
        "properties": {
           "Value": {"type": "string", "format": "byte", "x-ms-xml": {"name": "Content-MD5"}},
           "Algorithm": {"$ref": "#/definitions/HashAlgorithm", "x-ms-xml": {"name": ""}}
        }
      };

      $.BlobHttpHeaders = {
        "type": "object",
        "properties": {
          "Content-Type": {"type": "string", "description": "MIME content type of the blob."},
          "Content-Encoding": {"type": "string", "description": "Specifies which content encodings have been applied to the blob."},
          "Content-Language": {"type": "string", "description": "Specifies the natural languages used by this blob."},
          "Content-Hash": {"$ref": "#/definitions/ContentHash", "x-ms-xml": {"name": "."}, "description": "Hash of the blob content."},
          "Content-Disposition": {"type": "string", "description": "Conveys additional information about how to process the resource payload, and also can be used to attach additional metadata." },
          "Cache-Control": {"type": "string", "description": "Specifies directives for caching mechanisms." }
        },
        "description": "Standard HTTP properties supported by containers and blobs."
      };

      delete $.BlobTags;
      delete $.BlobTagSet;
      delete $.BlobTag;
      $.BlobTags = {"type": "object", "x-ms-format": "map", "xml": {"name": "Tags/TagSet"}, "properties": {"Key": {"type": "string", "x-ms-xml": {"name": "Tag/Key"}}, "Value": {"type": "string", "x-ms-xml": {"name": "Tag/Value"}}}, "description": "User-defined tags for this blob."};

      $.Metadata = {"type": "object", "x-ms-format": "caseinsensitivemap", properties: {"__placeHolder" : {"type": "integer"}}, "description": "A set of name-value pairs associated with this blob or blob container."};
      $.ContainerMetadata = $.Metadata;
      $.BlobMetadata = $.Metadata;

      delete $.ClearRange;
      delete $.PageList;
      $.ContentRange = {"type": "object", "x-ms-format": "range", properties: {"__placeHolder" : {"type": "integer"}}};
      $.PageRange = $.ContentRange;
      $.ClearRange = $.ContentRange;

      $.ObjectReplicationStatus  = {
        "type": "string",
        "enum": ["complete", "failed"],
        "x-ms-enum": {
          "name": "ObjectReplicationStatus",
          "modelAsString": false
        },
        "description": "The replication status of blob with the given policy and rule identifiers."
      };

      $.ObjectReplicationRule = {
        "type": "object",
        "x-ms-xml": {"name": "*", "tag": true},
        "properties": {
          "RuleId": {"type": "string", "x-ms-xml": {"name": "../*", "tag": true}, "description": "Object replication rule ID."},
          "ReplicationStatus": {"$ref": "#/definitions/ObjectReplicationStatus", "x-ms-xml": {"name": "../*"}, "description": "Object replication status."}
        },
        "description": "Contains the object replication rule ID and replication status of a blob."
      };

      $.ObjectReplicationPolicy = {
        "type": "object",
        "x-ms-xml": {"name": "*", "tag": true},
        "properties": {
          "PolicyId": {"type": "string", "x-ms-xml": {"name": "../*", "tag": true}, "description": "Object replication policy ID."},
          "Rules": {"type": "array", "items": {"$ref": "#/definitions/ObjectReplicationRule"}, "x-ms-xml": {"name": ".."}, "description": "The Rule IDs and respective replication status that are under the policy ID."}
        },
        "description": "Contains object replication policy ID and the respective list of #ObjectReplicationRule s. This is used when retrieving the object replication properties on the source blob."
      };
      delete $.ObjectReplicationMetadata;

      $.BlobImmutabilityPolicy = {
        "type": "object",
        "properties": {
          "ImmutabilityPolicyUntilDate": {"type": "string", "format": "date-time-rfc1123", "x-ms-client-name": "ExpiresOn", "description": "The date until which the blob can be protected from being modified or deleted."},
          "PolicyMode": {"$ref": "#/definitions/ImmutabilityPolicyMode", "x-ms-xml": {"name": "ImmutabilityPolicyMode"}}
        },
        "description": "Immutability policy associated with the blob."
      };

      $.LeaseStatus.description = "The current lease status of the blob.";
      $.LeaseState.description = "The current lease state of the blob.";
      $.LeaseDuration.description = "When a blob is leased, specifies whether the lease is of infinite or fixed duration.";
      $.CopyStatus.description = "Status of the copy operation.";
      $.ArchiveStatus.description = "For blob storage LRS accounts, valid values are rehydrate-pending-to-hot/rehydrate-pending-to-cool. If the blob is being rehydrated and is not complete then this value indicates that rehydrate is pending and also tells the destination tier.";
```

### Striped Blob Support

```yaml
directive:
  - from: swagger-document
    where: $
    transform: >
      const operations = [
        "Blob_GetProperties",
        "Blob_Download",
        "Blob_SetExpiry",
        "Blob_SetHTTPHeaders",
        "Blob_SetMetadata",
        "Blob_AcquireLease",
        "Blob_ReleaseLease",
        "Blob_RenewLease",
        "Blob_ChangeLease",
        "Blob_BreakLease",
        "Blob_CreateSnapshot",
        "Blob_StartCopyFromUri",
        "Blob_CopyFromUri",
        "Blob_Query",
        "PageBlob_Create",
        "PageBlob_UploadPages",
        "PageBlob_ClearPages",
        "PageBlob_UploadPagesFromUri",
        "PageBlob_GetPageRanges",
        "PageBlob_GetPageRangesDiff",
        "PageBlob_Resize",
        "PageBlob_UpdateSequenceNumber",
        "PageBlob_StartCopyIncremental",
      ];
      for (const url in $["x-ms-paths"]) {
        for (const verb in $["x-ms-paths"][url]) {
          if (!operations.includes($["x-ms-paths"][url][verb].operationId)) continue;
          const operation = $["x-ms-paths"][url][verb];

          const status_codes = Object.keys(operation.responses).filter(s => s !== "default");
          status_codes.forEach((status_code, i) => {
            operation.responses[status_code].headers["Last-Modified"]["x-ms-client-default"] = "";
            operation.responses[status_code].headers["Last-Modified"]["x-nullable"] = true;
            operation.responses[status_code].headers["ETag"]["x-ms-client-default"] = "";
            operation.responses[status_code].headers["ETag"]["x-nullable"] = true;
          });
        }
      }
  - from: swagger-document
    where: $
    transform: >
      const operations = [
        "PageBlob_UploadPages",
        "PageBlob_ClearPages",
        "PageBlob_UploadPagesFromUri",
      ];
      for (const url in $["x-ms-paths"]) {
        for (const verb in $["x-ms-paths"][url]) {
          if (!operations.includes($["x-ms-paths"][url][verb].operationId)) continue;
          const operation = $["x-ms-paths"][url][verb];

          const status_codes = Object.keys(operation.responses).filter(s => s !== "default");
          status_codes.forEach((status_code, i) => {
            operation.responses[status_code].headers["x-ms-blob-sequence-number"]["x-ms-client-default"] = "int64_t()";
            operation.responses[status_code].headers["x-ms-blob-sequence-number"]["x-nullable"] = true;
          });
        }
      }
```

### GetBlobServiceProperties

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.BlobServiceProperties = $.StorageServiceProperties;
      delete $.StorageServiceProperties;
      $.BlobServiceProperties.xml = { "name": "StorageServiceProperties" };
      
      $.AnalyticsLogging = $.Logging;
      delete $.Logging;
      $.AnalyticsLogging.xml = { "name": "Logging" };
      $.BlobServiceProperties.properties["Logging"]["$ref"] = "#/definitions/AnalyticsLogging";
      $.RetentionPolicy.properties["Enabled"]["x-ms-client-name"] = "IsEnabled";
      delete $.RetentionPolicy.properties["AllowPermanentDelete"];
      $.Metrics["type"] = "object";
      $.Metrics.properties["Enabled"]["x-ms-client-name"] = "IsEnabled";
      $.Metrics.properties["IncludeAPIs"]["x-ms-client-name"] = "IncludeApis";
      delete $.Metrics.required;
      $.Metrics.properties["IncludeAPIs"]["x-nullable"] = true;
      $.StaticWebsite.properties["Enabled"]["x-ms-client-name"] = "IsEnabled";
      $.BlobServiceProperties.properties.DefaultServiceVersion["x-nullable"] = true;
  - from: swagger-document
    where: $.parameters
    transform: >
      $.BlobServiceProperties = $.StorageServiceProperties;
      $.BlobServiceProperties.name = "BlobServiceProperties";
      $.BlobServiceProperties.schema["$ref"] = "#/definitions/BlobServiceProperties";
      delete $.StorageServiceProperties;
  - from: swagger-document
    where: $["x-ms-paths"]["/?restype=service&comp=properties"]
    transform: >
      $.put.parameters[0]["$ref"] = "#/parameters/BlobServiceProperties";
      $.get.responses["200"].schema["$ref"] = "#/definitions/BlobServiceProperties";
```

### GetServiceStatistics 

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
  - from: swagger-document
    where: $["x-ms-paths"]["/?restype=service&comp=stats"]
    transform: >
      $.get.responses["200"].schema["$ref"] = "#/definitions/ServiceStatistics";
```

### ListBlobContainers

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.ListContainersSegmentResponse["x-ms-client-name"] = "ListBlobContainersResult";
      $.ListContainersSegmentResponse.required.push("Prefix");
      $.ListContainersSegmentResponse.properties["Containers"] = $.ListContainersSegmentResponse.properties["ContainerItems"];
      $.ListContainersSegmentResponse.properties["Containers"]["x-ms-client-name"] = "Items";
      delete $.ListContainersSegmentResponse.properties["ContainerItems"];;
      $.ListContainersSegmentResponse.properties["NextMarker"]["x-ms-client-name"] = "ContinuationToken";
      delete $.ListContainersSegmentResponse.properties["Marker"];
      delete $.ListContainersSegmentResponse.properties["MaxResults"];

      $.ContainerItem["x-ms-client-name"] = "BlobContainerItem";
      $.ContainerItem.properties["Deleted"]["x-ms-client-name"] = "IsDeleted";
      $.ContainerItem.properties["Version"]["x-ms-client-name"] = "VersionId";
      $.ContainerItem.properties["Properties"]["x-ms-client-name"] = "Details";
      $.ContainerItem.required.push("Deleted");
      $.ContainerItem.properties["Name"].description = "Blob container name.";
      $.ContainerItem.properties["Deleted"].description = "Indicates whether this container was deleted.";
      $.ContainerItem.properties["Version"].description = "Version ID of a deleted container.";

      $.ContainerProperties.properties["Etag"]["x-ms-client-name"] = "ETag";
      $.ContainerProperties["x-ms-client-name"] = "BlobContainerItemDetails";
      $.ContainerProperties.properties["PublicAccess"]["x-ms-client-name"] = "AccessType";
      $.ContainerProperties.properties["Metadata"] = $.ContainerItem.properties["Metadata"];
      delete $.ContainerItem.properties["Metadata"];
      $.ContainerProperties.properties["Metadata"]["x-ms-xml"] = {"name": "../Metadata"};
      $.ContainerProperties.properties["DeletedTime"]["x-ms-client-name"] = "DeletedOn";
      $.ContainerProperties.properties["ImmutableStorageWithVersioningEnabled"]["x-ms-client-name"] = "HasImmutableStorageWithVersioning ";
      $.ContainerProperties.properties["ImmutableStorageWithVersioningEnabled"]["x-ms-client-default"] = false;
      delete $.ContainerProperties.required;
      $.ContainerProperties.properties["LeaseDuration"]["x-nullable"] = true;
      $.ContainerProperties.properties["DeletedTime"]["x-nullable"] = true;
      $.ContainerProperties.properties["RemainingRetentionDays"]["x-nullable"] = true;
      $.ContainerProperties.properties["DefaultEncryptionScope"]["x-ms-client-default"] = "$account-encryption-key";
      $.ContainerProperties.properties["DenyEncryptionScopeOverride"]["x-ms-client-default"] = "false";
      $.ContainerProperties.properties["Last-Modified"].description = "Returns the date and time the container was last modified. Any operation that modifies the blob, including an update of the blob's metadata or properties, changes the last-modified time of the blob.";
      $.ContainerProperties.properties["Etag"].description = "The ETag contains a value that you can use to perform operations conditionally. If the request version is 2011-08-18 or newer, the ETag value will be in quotes.";
      $.ContainerProperties.properties["HasImmutabilityPolicy"].description = "Indicates whether the container has an immutability policy set on it.";
      $.ContainerProperties.properties["HasLegalHold"].description = "Indicates whether the container has a legal hold.";
      $.ContainerProperties.properties["DefaultEncryptionScope"].description = "The default encryption scope for the container.";
      $.ContainerProperties.properties["DenyEncryptionScopeOverride"].description = "Indicates whether the container's default encryption scope can be overriden.";
      $.ContainerProperties.properties["DeletedTime"].description = "Data and time at which this container was deleted. Only valid when this container was deleted.";
      $.ContainerProperties.properties["RemainingRetentionDays"].description = "Remaining days before this container will be permanantely deleted. Only valid when this container was deleted.";
```

### GetUserDelegationKey

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.UserDelegationKey.properties["SignedOid"]["x-ms-client-name"] = "SignedObjectId";
      $.UserDelegationKey.properties["SignedTid"]["x-ms-client-name"] = "SignedTenantId";
      $.UserDelegationKey.properties["SignedStart"]["x-ms-client-name"] = "SignedStartsOn";
      $.UserDelegationKey.properties["SignedExpiry"]["x-ms-client-name"] = "SignedExpiresOn";
```

### GetAccountInfo

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/?restype=account&comp=properties"].get.responses["200"].headers["x-ms-sku-name"]
    transform: >
      $["x-ms-enum"]["values"] = [
        {"value": "Standard_LRS", "name":"Standard_Lrs"},
        {"value": "Standard_GRS", "name":"StandardGrs"},
        {"value": "Standard_RAGRS", "name":"StandardRagrs"},
        {"value": "Standard_ZRS", "name":"StandardZrs"},
        {"value": "Premium_LRS", "name":"PremiumLrs"},
        {"value": "Premium_ZRS", "name":"PremiumZrs"},
        {"value": "Standard_GZRS", "name":"StandardGzrs"},
        {"value": "Standard_RAGZRS", "name":"StandardRagzrs"}
      ];
```

### FindBlobsByTags

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/?comp=blobs"].get.parameters
    transform: >
      $ = $.filter(p => !p["$ref"] || !p["$ref"].endsWith("#/parameters/FilterBlobsInclude"));
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}?restype=container&comp=blobs"].get.parameters
    transform: >
      $ = $.filter(p => !p["$ref"] || !p["$ref"].endsWith("#/parameters/FilterBlobsInclude"));
  - from: swagger-document
    where: $.definitions
    transform: >
      $.FilterBlobItem["x-ms-client-name"] = "TaggedBlobItem";
      $.FilterBlobItem.properties["Name"]["x-ms-client-name"] = "BlobName";
      $.FilterBlobItem.properties["ContainerName"]["x-ms-client-name"] = "BlobContainerName";
      delete $.FilterBlobItem.properties["TagValue"];
      delete $.FilterBlobItem.properties["VersionId"];
      delete $.FilterBlobItem.properties["IsCurrentVersion"];
      $.FilterBlobItem.properties["Name"].description = "Blob name.";
      $.FilterBlobItem.properties["ContainerName"].description = "Blob container name.";
      $.FilterBlobItem.properties["Tags"]["x-ms-xml"] = {"name": "Tags/TagSet"};
      $.FilterBlobSegment["x-ms-client-name"] = "FindBlobsByTagsResult";
      $.FilterBlobSegment.properties["NextMarker"]["x-ms-client-name"] = "ContinuationToken";
      $.FilterBlobSegment.properties["Blobs"]["x-ms-client-name"] = "Items";
      delete $.FilterBlobSegment.properties["Where"];
```

### CreateBlobContainer

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}?restype=container"].put.responses["201"]
    transform: >
      $.schema = {
        "type": "object",
        "x-ms-client-name": "CreateBlobContainerResult",
        "x-ms-sealed": false,
        "properties": {
          "Created": {"type": "boolean", "x-ms-client-default": true, "x-ms-xml": {"name": ""}, "description": "Indicates if the container was successfully created by this operation."}
        }
      };
```

### GetBlobContainerProperties

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}?restype=container"].get.responses["200"].headers
    transform: >
      $["x-ms-blob-public-access"]["x-ms-client-name"] = "AccessType";
      $["x-ms-blob-public-access"]["x-nullable"] = true;
      $["x-ms-blob-public-access"]["x-ms-client-default"] = "None";
      $["x-ms-deny-encryption-scope-override"]["x-ms-client-name"] = "PreventEncryptionScopeOverride";
      $["x-ms-lease-duration"]["x-nullable"] = true;
      $["x-ms-default-encryption-scope"]["x-nullable"] = true;
      $["x-ms-default-encryption-scope"]["x-ms-client-default"] = "$account-encryption-key";
      $["x-ms-deny-encryption-scope-override"]["x-nullable"] = true;
      $["x-ms-deny-encryption-scope-override"]["x-ms-client-default"] = "false";
      $["x-ms-meta"].description = "A set of name-value pair associated with this blob container.";
      $["x-ms-immutable-storage-with-versioning-enabled"]["x-ms-client-name"] = "HasImmutableStorageWithVersioning";
      $["x-ms-immutable-storage-with-versioning-enabled"]["x-ms-client-default"] = false;
      $["x-ms-immutable-storage-with-versioning-enabled"]["x-nullable"] = true;
```

### GetBlobContainerAccessPolicy

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}?restype=container&comp=acl"].get.responses["200"]
    transform: >
      $.headers["x-ms-blob-public-access"]["x-nullable"] = true;
      $.headers["x-ms-blob-public-access"]["x-ms-client-default"] = "None";
      $.schema = {
        "type": "object",
        "x-ms-client-name": "BlobContainerAccessPolicy",
        "xml": {"name": "."},
        "x-ms-sealed": false,
        "properties": {
          "SignedIdentifiers": {"$ref": "#/definitions/SignedIdentifiers"}
        }
      }
```

### DeleteBlobContainer

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}?restype=container"].delete.responses["202"]
    transform: >
      $.schema = {
        "type": "object",
        "x-ms-client-name": "DeleteBlobContainerResult",
        "x-ms-sealed": false,
        "properties": {
          "Deleted": {"type": "boolean", "x-ms-client-default": true, "x-ms-xml": {"name": ""}, "description": "Indicates if the container was successfully deleted by this operation."}
        }
      };
```

### GetBlobContainerAccessPolicy

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}?restype=container&comp=acl"].get.responses["200"].headers
    transform: >
      delete $["ETag"];
      delete $["Last-Modified"];
      $["x-ms-blob-public-access"]["x-ms-client-name"] = "AccessType";
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
```

### ListBlobsFlat

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.ListBlobsFlatSegmentResponse["x-ms-client-name"] = "ListBlobsResult";
      $.ListBlobsFlatSegmentResponse.properties["ContainerName"]["x-ms-client-name"] = "BlobContainerName";
      $.ListBlobsFlatSegmentResponse.properties["NextMarker"]["x-ms-client-name"] = "ContinuationToken";
      $.ListBlobsFlatSegmentResponse.properties["Blobs"] = $.BlobFlatListSegment.properties["BlobItems"];
      $.ListBlobsFlatSegmentResponse.properties["Blobs"]["x-ms-client-name"] = "Items";
      delete $.BlobFlatListSegment;
      delete $.ListBlobsFlatSegmentResponse.properties["Marker"];
      delete $.ListBlobsFlatSegmentResponse.properties["MaxResults"];
      delete $.ListBlobsFlatSegmentResponse.properties["Segment"];
      delete $.ListBlobsFlatSegmentResponse.required;
      $.ListBlobsFlatSegmentResponse.properties["NextMarker"]["x-nullable"] = true;

      $.BlobName["x-namespace"] = "_detail";
      delete $.BlobName.properties["content"]["xml"];
      $.BlobName["xml"] = {"name": "Name"};
      $.BlobName.properties["content"]["x-ms-xml"] = {"name": "."};
      $.BlobItemInternal["x-ms-client-name"] = "BlobItem";
      $.BlobItemInternal["x-namespace"] = "_detail";
      $.BlobItemInternal.properties["Deleted"]["x-ms-client-name"] = "IsDeleted";
      $.BlobItemInternal.properties["Properties"]["x-ms-client-name"] = "Details";
      $.BlobItemInternal.properties["BlobSize"] = $.BlobPropertiesInternal.properties["Content-Length"];
      $.BlobItemInternal.properties["BlobSize"]["x-ms-xml"] = {"name": "Properties/Content-Length"};
      delete $.BlobPropertiesInternal.properties["Content-Length"];
      $.BlobItemInternal.properties["BlobType"] = $.BlobPropertiesInternal.properties["BlobType"];
      $.BlobItemInternal.properties["BlobType"]["x-ms-xml"] = {"name": "Properties/BlobType"};
      delete $.BlobPropertiesInternal.properties["BlobType"];
      $.BlobItemInternal.properties["DeletionId"] = {"type": "string"};
      $.BlobItemInternal.required.push("BlobType", "BlobSize");
      $.BlobItemInternal.properties["Name"].description = "Blob name.";
      $.BlobItemInternal.properties["Deleted"].description = "Indicates whether this blob was deleted.";
      $.BlobItemInternal.properties["Snapshot"].description = "A string value that uniquely identifies a blob snapshot.";
      $.BlobItemInternal.properties["VersionId"].description = "A string value that uniquely identifies a blob version.";
      $.BlobItemInternal.properties["IsCurrentVersion"].description = "Indicates if this is the current version of the blob.";
      $.BlobItemInternal.properties["BlobType"].description = "Type of the blob.";
      $.BlobItemInternal.properties["HasVersionsOnly"].description = "Indicates that this root blob has been deleted, but it has versions that are active.";
      $.BlobItemInternal.properties["DeletionId"].description = "The deletion ID associated with the deleted path.";

      $.BlobPropertiesInternal.properties["Etag"]["x-ms-client-name"] = "ETag";
      $.BlobPropertiesInternal["x-ms-client-name"] = "BlobItemDetails";
      $.BlobPropertiesInternal.properties["Creation-Time"]["x-ms-client-name"] = "CreatedOn";
      $.BlobPropertiesInternal.properties["HttpHeaders"] = {"$ref": "#/definitions/BlobHttpHeaders", "x-ms-xml": {"name": "."}};
      delete $.BlobPropertiesInternal.properties["TagCount"];
      delete $.BlobPropertiesInternal.properties["Content-Type"];
      delete $.BlobPropertiesInternal.properties["Content-Encoding"];
      delete $.BlobPropertiesInternal.properties["Content-Language"];
      delete $.BlobPropertiesInternal.properties["Content-MD5"];
      delete $.BlobPropertiesInternal.properties["Content-Disposition"];
      delete $.BlobPropertiesInternal.properties["Cache-Control"];
      $.BlobPropertiesInternal.properties["Metadata"] = $.BlobItemInternal.properties["Metadata"];
      $.BlobPropertiesInternal.properties["Metadata"]["x-ms-xml"] = {"name": "../Metadata"};
      $.BlobPropertiesInternal.properties["Tags"] = $.BlobItemInternal.properties["BlobTags"];
      $.BlobPropertiesInternal.properties["Tags"]["x-ms-xml"] = {"name": "../Tags/TagSet"};
      $.BlobPropertiesInternal.properties["ObjectReplicationSourceProperties"] = {"type": "array", "items": {"$ref": "#/definitions/ObjectReplicationPolicy"}};
       $.BlobPropertiesInternal.properties["ObjectReplicationSourceProperties"]["x-ms-xml"] = {"name": "../OrMetadata"};
      delete $.BlobItemInternal.properties["Metadata"];
      delete $.BlobItemInternal.properties["BlobTags"];
      delete $.BlobItemInternal.properties["ObjectReplicationMetadata"];
      $.BlobPropertiesInternal.properties["AccessTierInferred"]["x-ms-client-name"] = "IsAccessTierInferred";
      $.BlobPropertiesInternal.properties["AccessTierChangeTime"]["x-ms-client-name"] = "AccessTierChangedOn";
      $.BlobPropertiesInternal.properties["ServerEncrypted"]["x-ms-client-name"] = "IsServerEncrypted";
      $.BlobPropertiesInternal.properties["CustomerProvidedKeySha256"]["x-ms-client-name"] = "EncryptionKeySha256";
      $.BlobPropertiesInternal.properties["CustomerProvidedKeySha256"]["format"] = "byte";
      $.BlobPropertiesInternal.properties["x-ms-blob-sequence-number"]["x-ms-client-name"] = "SequenceNumber";
      $.BlobPropertiesInternal.properties["IncrementalCopy"]["x-ms-client-name"] = "IsIncrementalCopy";
      $.BlobPropertiesInternal.properties["DestinationSnapshot"]["x-ms-client-name"] = "IncrementalCopyDestinationSnapshot";
      $.BlobPropertiesInternal.properties["DestinationSnapshot"]["x-ms-xml"] = {"name": "CopyDestinationSnapshot"};
      $.BlobPropertiesInternal.properties["CopyCompletionTime"]["x-ms-client-name"] = "CopyCompletedOn";
      $.BlobPropertiesInternal.properties["DeletedTime"]["x-ms-client-name"] = "DeletedOn";
      $.BlobPropertiesInternal.properties["LegalHold"]["x-ms-client-name"] = "HasLegalHold";
      $.BlobPropertiesInternal.properties["LegalHold"]["x-ms-client-default"] = false;
      $.BlobPropertiesInternal.properties["ImmutabilityPolicy"] = {"$ref": "#/definitions/BlobImmutabilityPolicy", "x-ms-xml": {"name": "."}, "x-nullable": true};
      delete $.BlobPropertiesInternal.properties["ImmutabilityPolicyUntilDate"];
      delete $.BlobPropertiesInternal.properties["ImmutabilityPolicyMode"];
      $.BlobPropertiesInternal.required.push("Creation-Time", "LeaseStatus", "LeaseState", "ServerEncrypted", "HttpHeaders", "LegalHold");
      $.BlobPropertiesInternal.properties["Creation-Time"].description = "The date and time at which the blob was created.";
      $.BlobPropertiesInternal.properties["Last-Modified"].description = "The date and time the blob was last modified.";
      $.BlobPropertiesInternal.properties["Etag"].description = "The ETag contains a value that you can use to perform operations conditionally. If the request version is 2011-08-18 or newer, the ETag value will be in quotes.";
      $.BlobPropertiesInternal.properties["x-ms-blob-sequence-number"].description = "The current sequence number for a page blob.";
      $.BlobPropertiesInternal.properties["CopyId"].description = "String identifier for this copy operation. Use with Get Blob Properties to check the status of this copy operation, or pass to Abort Copy Blob to abort a pending copy.";
      $.BlobPropertiesInternal.properties["CopySource"].description = "URL up to 2 KB in length that specifies the source blob or file used in the last attempted Copy Blob operation where this blob was the destination blob. This header does not appear if this blob has never been the destination in a Copy Blob operation, or if this blob has been modified after a concluded Copy Blob operation using Set Blob Properties, Put Blob, or Put Block List.";
      $.BlobPropertiesInternal.properties["CopyProgress"].description = "Contains the number of bytes copied and the total bytes in the source in the last attempted Copy Blob operation where this blob was the destination blob. Can show between 0 and Content-Length bytes copied. This header does not appear if this blob has never been the destination in a Copy Blob operation, or if this blob has been modified after a concluded Copy Blob operation using Set Blob Properties, Put Blob, or Put Block List.";
      $.BlobPropertiesInternal.properties["CopyCompletionTime"].description = "Conclusion time of the last attempted Copy Blob operation where this blob was the destination blob. This value can specify the time of a completed, aborted, or failed copy attempt. This header does not appear if a copy is pending, if this blob has never been the destination in a Copy Blob operation, or if this blob has been modified after a concluded Copy Blob operation using Set Blob Properties, Put Blob, or Put Block List.";
      $.BlobPropertiesInternal.properties["CopyStatusDescription"].description = "Only appears when x-ms-copy-status is failed or pending. Describes the cause of the last fatal or non-fatal copy operation failure. This header does not appear if this blob has never been the destination in a Copy Blob operation, or if this blob has been modified after a concluded Copy Blob operation using Set Blob Properties, Put Blob, or Put Block List.";
      $.BlobPropertiesInternal.properties["ServerEncrypted"].description = "The value of this header is set to true if the blob data and application metadata are completely encrypted using the specified algorithm. Otherwise, the value is set to false (when the blob is unencrypted, or if only parts of the blob/application metadata are encrypted).";
      $.BlobPropertiesInternal.properties["IncrementalCopy"].description = "Included if the blob is incremental copy blob.";
      $.BlobPropertiesInternal.properties["DestinationSnapshot"].description = "Included if the blob is incremental copy blob or incremental copy snapshot, if x-ms-copy-status is success. Snapshot time of the last successful incremental copy snapshot for this blob.";
      $.BlobPropertiesInternal.properties["DeletedTime"].description = "Data and time at which this blob was deleted. Only valid when this blob was deleted.";
      $.BlobPropertiesInternal.properties["RemainingRetentionDays"].description = "Remaining days before this blob will be permanantely deleted. Only valid when this blob was deleted.";
      $.BlobPropertiesInternal.properties["AccessTierInferred"].description = "True if the access tier is not explicitly set on the blob.";
      $.BlobPropertiesInternal.properties["CustomerProvidedKeySha256"].description = "SHA-256 hash of the encryption key.";
      $.BlobPropertiesInternal.properties["AccessTierChangeTime"].description = "The time the tier was changed on the object. This is only returned if the tier on the block blob was ever set.";
      $.BlobPropertiesInternal.properties["Expiry-Time"].description = "The time this blob will expire.";
      $.BlobPropertiesInternal.properties["Sealed"].description = "If this blob has been sealed.";
      $.BlobPropertiesInternal.properties["LastAccessTime"].description = "UTC date/time value generated by the service that indicates the time at which the blob was last read or written to.";
      $.BlobPropertiesInternal.properties["LegalHold"].description = "Indicates whether the blob has a legal hold.";
```

### ListBlobsByHierarchy

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.ListBlobsHierarchySegmentResponse["x-ms-client-name"] = "ListBlobsByHierarchyResult";
      $.ListBlobsHierarchySegmentResponse.properties["ContainerName"]["x-ms-client-name"] = "BlobContainerName";
      $.ListBlobsHierarchySegmentResponse.properties["NextMarker"]["x-ms-client-name"] = "ContinuationToken";
      $.ListBlobsHierarchySegmentResponse.properties["Blobs"] = $.ListBlobsFlatSegmentResponse.properties["Blobs"];
      $.ListBlobsHierarchySegmentResponse.properties["Blobs"]["x-ms-client-name"] = "Items";
      $.ListBlobsHierarchySegmentResponse.properties["BlobPrefixes"] = {"type": "array", "items": {"$ref": "#/definitions/BlobName"}, "x-ms-xml": {"wrapped": true, "name": "Blobs/BlobPrefix"}};
      delete $.ListBlobsHierarchySegmentResponse.properties["Marker"];
      delete $.ListBlobsHierarchySegmentResponse.properties["MaxResults"];
      delete $.ListBlobsHierarchySegmentResponse.properties["Segment"];
      delete $.ListBlobsHierarchySegmentResponse.required;
      $.ListBlobsHierarchySegmentResponse.properties["NextMarker"]["x-nullable"] = true;
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}?restype=container&comp=list&hierarchy"].get.parameters
    transform: >
      $.push({"$ref": "#/parameters/ListBlobsShowOnly"});
```

### DownloadBlob

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.DownloadBlobDetails = {
        "type": "object",
        "required": ["ETag", "LastModified", "CreatedOn", "HttpHeaders", "IsServerEncrypted", "HasLegalHold"],
        "properties": {
          "ETag": {"type": "string", "format": "etag", "description": "The ETag contains a value that you can use to perform operations conditionally. If the request version is 2011-08-18 or newer, the ETag value will be in quotes."},
          "LastModified": {"type": "string", "format": "date-time-rfc1123", "description": "Returns the date and time the container was last modified. Any operation that modifies the blob, including an update of the blob's metadata or properties, changes the last-modified time of the blob."},
          "CreatedOn": {"type": "string", "format": "date-time-rfc1123", "description": "The date and time at which the blob was created."},
          "ExpiresOn": {"type": "string", "format": "date-time-rfc1123", "description": "The time this blob will expire."},
          "LastAccessedOn": {"type": "string", "format": "date-time-rfc1123", "description": "UTC date/time value generated by the service that indicates the time at which the blob was last read or written to."},
          "HttpHeaders": {"$ref": "#/definitions/BlobHttpHeaders"},
          "Metadata": {"$ref": "#/definitions/BlobMetadata"},
          "SequenceNumber": {"type": "integer", "format": "int64", "description": "The current sequence number for a page blob."},
          "CommittedBlockCount": {"type": "integer", "format": "int32", "description": "The number of committed blocks present in the blob."},
          "IsSealed": {"type": "boolean", "description": "If the blob has been sealed. This value is null for block blobs or page blobs."},
          "LeaseDuration": {"$ref": "#/definitions/LeaseDuration"},
          "LeaseState": {"$ref": "#/definitions/LeaseState"},
          "LeaseStatus": {"$ref": "#/definitions/LeaseStatus"},
          "IsServerEncrypted": {"type": "boolean", "description": "True if the blob data and metadata are completely encrypted using the specified algorithm. Otherwise, the value is set to false (when the blob is unencrypted, or if only parts of the blob/application metadata are encrypted)."},
          "EncryptionKeySha256": {"type": "string", "format": "byte", "description": "SHA-256 hash of the encryption key used to encrypt the blob data and metadata."},
          "EncryptionScope": {"type": "string", "description": "Name of the encryption scope used to encrypt the blob data and metadata."},
          "ObjectReplicationDestinationPolicyId": {"type": "string", "description": "Only valid when Object Replication is enabled and current blob is the destination."},
          "ObjectReplicationSourceProperties": {"type": "array", "items": {"$ref": "#/definitions/ObjectReplicationPolicy"}, "x-ms-xml": {"name": ""}, "description": "Only valid when Object Replication is enabled and current blob is the source."},
          "TagCount": {"type": "integer", "format": "int32", "description": "The number of tags associated with the blob."},
          "CopyId": {"type": "string", "description": " String identifier for this copy operation. Use with Get Blob Properties to check the status of this copy operation, or pass to Abort Copy Blob to abort a pending copy."},
          "CopySource": {"type": "string", "description": "URL up to 2 KB in length that specifies the source blob or file used in the last attempted Copy Blob operation where this blob was the destination blob. This header does not appear if this blob has never been the destination in a Copy Blob operation, or if this blob has been modified after a concluded Copy Blob operation using Set Blob Properties, Put Blob, or Put Block List."},
          "CopyStatus": {"$ref": "#/definitions/CopyStatus"},
          "CopyStatusDescription": {"type": "string", "description": "Only appears when x-ms-copy-status is failed or pending. Describes the cause of the last fatal or non-fatal copy operation failure. This header does not appear if this blob has never been the destination in a Copy Blob operation, or if this blob has been modified after a concluded Copy Blob operation using Set Blob Properties, Put Blob, or Put Block List"},
          "CopyProgress": {"type": "string", "description": "Contains the number of bytes copied and the total bytes in the source in the last attempted Copy Blob operation where this blob was the destination blob. Can show between 0 and Content-Length bytes copied. This header does not appear if this blob has never been the destination in a Copy Blob operation, or if this blob has been modified after a concluded Copy Blob operation using Set Blob Properties, Put Blob, or Put Block List"},
          "CopyCompletedOn": {"type": "string", "format": "date-time-rfc1123", "description": "Conclusion time of the last attempted Copy Blob operation where this blob was the destination blob. This value can specify the time of a completed, aborted, or failed copy attempt. This header does not appear if a copy is pending, if this blob has never been the destination in a Copy Blob operation, or if this blob has been modified after a concluded Copy Blob operation using Set Blob Properties, Put Blob, or Put Block List."},
          "VersionId": {"type": "string", "description": "A string value returned by the service that uniquely identifies the blob version."},
          "IsCurrentVersion": {"type": "boolean", "description": "Indicates whether version of this blob is the current version."},
          "ImmutabilityPolicy": {"$ref": "#/definitions/BlobImmutabilityPolicy", "x-nullable": true},
          "HasLegalHold": {"type": "boolean", "x-ms-client-default": false, "description": "Indicates whether the blob has a legal hold."}
        },
        "description": "Detailed information of the downloaded blob."
      };
      $.DownloadBlobResult = {
        "type": "object",
        "x-ms-sealed": false,
        "properties": {
          "Details": {"$ref": "#/definitions/DownloadBlobDetails", "x-ms-xml": {"name": ""}},
          "BlobSize": {"type": "integer", "format": "int64", "x-ms-xml": {"name": ""}, "description": "Size of the blob in bytes."},
          "ContentRange": {"$ref": "#/definitions/ContentRange", "x-ms-xml": {"name": ""}, "description": "Indicates the range of bytes returned."},
          "TransactionalContentHash": {"$ref": "#/definitions/ContentHash", "x-nullable": true, "x-ms-xml": {"name": ""}, "description": "CRC64 or MD5 hash for the downloaded range of data."},
          "BodyStream": {"type": "object", "format": "file", "description": "Content of the blob or blob range."}
        }
      };
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}"].get.responses
    transform: >
      $["200"].schema = {"$ref": "#/definitions/DownloadBlobResult"};
      $["206"].schema = {"$ref": "#/definitions/DownloadBlobResult"};
      for (const status_code of ["200", "206"]) {
        $[status_code].headers["Last-Modified"]["x-ms-client-path"] = "Details.LastModified";
        $[status_code].headers["x-ms-meta"]["x-ms-client-path"] = "Details.Metadata";
        $[status_code].headers["x-ms-or-policy-id"]["x-ms-client-path"] = "Details.ObjectReplicationDestinationPolicyId";
        $[status_code].headers["x-ms-or"]["x-ms-client-path"] = "Details.ObjectReplicationSourceProperties";
        $[status_code].headers["ETag"]["x-ms-client-path"] = "Details.ETag";
        $[status_code].headers["Content-Type"]["x-ms-client-path"] = "Details.HttpHeaders.ContentType";
        $[status_code].headers["Content-Type"]["x-nullable"] = true;
        $[status_code].headers["Content-Encoding"]["x-ms-client-path"] = "Details.HttpHeaders.ContentEncoding";
        $[status_code].headers["Content-Encoding"]["x-nullable"] = true;
        $[status_code].headers["Cache-Control"]["x-ms-client-path"] = "Details.HttpHeaders.CacheControl";
        $[status_code].headers["Cache-Control"]["x-nullable"] = true;
        $[status_code].headers["Content-Disposition"]["x-ms-client-path"] = "Details.HttpHeaders.ContentDisposition";
        $[status_code].headers["Content-Disposition"]["x-nullable"] = true;
        $[status_code].headers["Content-Language"]["x-ms-client-path"] = "Details.HttpHeaders.ContentLanguage";
        $[status_code].headers["Content-Language"]["x-nullable"] = true;
        $[status_code].headers["x-ms-blob-sequence-number"]["x-ms-client-path"] = "Details.SequenceNumber";
        $[status_code].headers["x-ms-copy-completion-time"]["x-ms-client-path"] = "Details.CopyCompletedOn";
        $[status_code].headers["x-ms-copy-status-description"]["x-ms-client-path"] = "Details.CopyStatusDescription";
        $[status_code].headers["x-ms-copy-id"]["x-ms-client-path"] = "Details.CopyId";
        $[status_code].headers["x-ms-copy-progress"]["x-ms-client-path"] = "Details.CopyProgress";
        $[status_code].headers["x-ms-copy-source"]["x-ms-client-path"] = "Details.CopySource";
        $[status_code].headers["x-ms-copy-status"]["x-ms-client-path"] = "Details.CopyStatus";
        $[status_code].headers["x-ms-lease-duration"]["x-ms-client-path"] = "Details.LeaseDuration";
        $[status_code].headers["x-ms-lease-state"]["x-ms-client-path"] = "Details.LeaseState";
        $[status_code].headers["x-ms-lease-status"]["x-ms-client-path"] = "Details.LeaseStatus";
        $[status_code].headers["x-ms-version-id"]["x-ms-client-path"] = "Details.VersionId";
        $[status_code].headers["x-ms-is-current-version"]["x-ms-client-path"] = "Details.IsCurrentVersion";
        $[status_code].headers["x-ms-blob-committed-block-count"]["x-ms-client-path"] = "Details.CommittedBlockCount";
        $[status_code].headers["x-ms-server-encrypted"]["x-ms-client-path"] = "Details.IsServerEncrypted";
        $[status_code].headers["x-ms-encryption-key-sha256"]["x-ms-client-path"] = "Details.EncryptionKeySha256";
        $[status_code].headers["x-ms-encryption-scope"]["x-ms-client-path"] = "Details.EncryptionScope";
        $[status_code].headers["x-ms-tag-count"]["x-ms-client-path"] = "Details.TagCount";
        $[status_code].headers["x-ms-blob-sealed"]["x-ms-client-path"] = "Details.IsSealed";
        $[status_code].headers["x-ms-last-access-time"]["x-ms-client-path"] = "Details.LastAccessedOn";
        $[status_code].headers["x-ms-creation-time"] = {"type": "string", "format": "date-time-rfc1123", "description": "Returns the date and time the blob was created.", "x-ms-client-path": "Details.CreatedOn"};
        $[status_code].headers["x-ms-legal-hold"]["x-ms-client-path"] = "Details.HasLegalHold";
        $[status_code].headers["x-ms-legal-hold"]["x-nullable"] = true;
        $[status_code].headers["x-ms-immutability-policy-until-date"]["x-ms-client-path"] = "Details.ImmutabilityPolicy.ExpiresOn";
        $[status_code].headers["x-ms-immutability-policy-mode"]["x-ms-client-path"] = "Details.ImmutabilityPolicy.PolicyMode";
        delete $[status_code].headers["Accept-Ranges"];
        delete $[status_code].headers["Content-Length"];
        delete $[status_code].headers["Content-Range"];
        delete $[status_code].headers["Content-MD5"];
        delete $[status_code].headers["x-ms-blob-content-md5"];
        delete $[status_code].headers["x-ms-content-crc64"];
        delete $[status_code].headers["x-ms-or"];
      }
      $["200"].headers["Content-MD5"] = {"type": "string", "format": "byte", "x-ms-client-name": "TransactionalContentHash", "x-ms-client-path": "Details.HttpHeaders.ContentHash", "x-nullable": true};
      $["206"].headers["Content-MD5"] = {"type": "string", "format": "byte", "x-ms-client-name": "TransactionalContentHash", "x-nullable": true};
      $["200"].headers["x-ms-blob-content-md5"] = {"type": "string", "format": "byte", "x-ms-client-path": "Details.HttpHeaders.ContentHash", "x-nullable": true};
      $["206"].headers["x-ms-blob-content-md5"] = $["200"].headers["x-ms-blob-content-md5"];
      $["206"].headers["x-ms-content-crc64"] = {"type": "string", "format": "byte", "x-ms-client-name": "TransactionalContentHash"};
```

### GetBlobProperties

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}"].head.responses
    transform: >
      $["200"].schema = {
        "type": "object",
        "x-ms-client-name": "BlobProperties",
        "x-ms-sealed": false,
        "properties": {
          "ObjectReplicationSourceProperties": {"type": "array", "items": {"$ref": "#/definitions/ObjectReplicationPolicy"}, "x-ms-xml": {"name": ""}},
          "ImmutabilityPolicy": {"$ref": "#/definitions/BlobImmutabilityPolicy", "x-nullable": true, "x-ms-xml": {"name": ""}},
          "HttpHeaders": {"$ref": "#/definitions/BlobHttpHeaders", "x-ms-xml": {"name": ""}}
        }
      };
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}"].head.responses["200"].headers
    transform: >
      $["x-ms-creation-time"]["x-ms-client-name"] = "CreatedOn";
      $["x-ms-or-policy-id"]["x-ms-client-name"] = "ObjectReplicationDestinationPolicyId";
      $["x-ms-or-policy-id"]["x-nullable"] = true;
      $["x-ms-or"]["x-ms-client-name"] = "ObjectReplicationSourceProperties";
      $["x-ms-copy-completion-time"]["x-ms-client-name"] = "CopyCompletedOn";
      $["x-ms-copy-completion-time"]["x-nullable"] = true;
      $["x-ms-copy-status-description"]["x-nullable"] = true;
      $["x-ms-copy-id"]["x-nullable"] = true;
      $["x-ms-copy-progress"]["x-nullable"] = true;
      $["x-ms-copy-source"]["x-nullable"] = true;
      $["x-ms-copy-status"]["x-nullable"] = true;
      $["x-ms-incremental-copy"]["x-nullable"] = true;
      $["x-ms-copy-destination-snapshot"]["x-ms-client-name"] = "IncrementalCopyDestinationSnapshot";
      $["x-ms-copy-destination-snapshot"]["x-nullable"] = true;
      $["x-ms-lease-duration"]["x-nullable"] = true;
      $["x-ms-lease-state"]["x-nullable"] = true;
      $["x-ms-lease-status"]["x-nullable"] = true;
      $["Content-Length"]["x-ms-client-name"] = "BlobSize";
      $["Content-Length"].description = "Size of the blob in bytes.";
      $["Content-Type"]["x-ms-client-path"] = "HttpHeaders.ContentType";
      $["Content-Type"]["x-nullable"] = true;
      $["Content-MD5"]["x-ms-client-path"] = "HttpHeaders.ContentHash";
      $["Content-MD5"]["x-nullable"] = true;
      $["Content-Encoding"]["x-ms-client-path"] = "HttpHeaders.ContentEncoding";
      $["Content-Encoding"]["x-nullable"] = true;
      $["Content-Disposition"]["x-ms-client-path"] = "HttpHeaders.ContentDisposition";
      $["Content-Disposition"]["x-nullable"] = true;
      $["Content-Language"]["x-ms-client-path"] = "HttpHeaders.ContentLanguage";
      $["Content-Language"]["x-nullable"] = true;
      $["Cache-Control"]["x-ms-client-path"] = "HttpHeaders.CacheControl";
      $["Cache-Control"]["x-nullable"] = true;
      $["x-ms-blob-sequence-number"]["x-ms-client-name"] = "SequenceNumber";
      $["x-ms-blob-sequence-number"]["x-nullable"] = true;
      $["x-ms-blob-committed-block-count"]["x-ms-client-name"] = "CommittedBlockCount";
      $["x-ms-blob-committed-block-count"]["x-nullable"] = true;
      $["x-ms-encryption-key-sha256"]["x-nullable"] = true;
      $["x-ms-encryption-scope"]["x-nullable"] = true;
      $["x-ms-access-tier"]["x-nullable"] = true;
      $["x-ms-access-tier"]["enum"] = ["P1", "P2", "P3", "P4", "P6", "P10", "P15", "P20", "P30", "P40", "P50", "P60", "P70", "P80", "Hot", "Cool", "Archive"];
      $["x-ms-access-tier"]["x-ms-enum"] = {"name": "AccessTier", "modelAsString": true};
      $["x-ms-access-tier-inferred"]["x-ms-client-name"] = "IsAccessTierInferred";
      $["x-ms-access-tier-inferred"]["x-nullable"] = true;
      $["x-ms-archive-status"]["x-nullable"] = true;
      $["x-ms-archive-status"]["enum"] = ["rehydrate-pending-to-hot", "rehydrate-pending-to-cool"];
      $["x-ms-archive-status"]["x-ms-enum"] = {"name": "ArchiveStatus", "modelAsString": true};
      $["x-ms-access-tier-change-time"]["x-ms-client-name"] = "AccessTierChangedOn";
      $["x-ms-access-tier-change-time"]["x-nullable"] = true;
      $["x-ms-version-id"]["x-nullable"] = true;
      $["x-ms-is-current-version"]["x-nullable"] = true;
      $["x-ms-tag-count"]["x-nullable"] = true;
      $["x-ms-tag-count"]["format"] = "int32";
      $["x-ms-expiry-time"]["x-nullable"] = true;
      $["x-ms-blob-sealed"]["x-nullable"] = true;
      $["x-ms-rehydrate-priority"]["x-nullable"] = true;
      $["x-ms-rehydrate-priority"]["enum"] = ["High", "Standard"];
      $["x-ms-rehydrate-priority"]["x-ms-enum"] =  {"name": "RehydratePriority", "modelAsString": true};
      $["x-ms-last-access-time"]["x-ms-client-name"] = "LastAccessedOn";
      $["x-ms-last-access-time"]["x-nullable"] = true;
      $["x-ms-immutability-policy-mode"]["x-ms-client-path"] = "ImmutabilityPolicy.PolicyMode";
      $["x-ms-immutability-policy-until-date"]["x-ms-client-path"] = "ImmutabilityPolicy.ExpiresOn";
      $["x-ms-legal-hold"]["x-ms-client-name"] = "HasLegalHold";
      $["x-ms-legal-hold"]["x-ms-client-default"] = false;
      $["x-ms-legal-hold"]["x-nullable"] = true;
      delete $["Accept-Ranges"];
      delete $["x-ms-or"];
      $["x-ms-meta"].description = "A set of name-value pair associated with this blob.";
```

### DeleteBlob

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}"].delete.parameters
    transform: >
      $ = $.filter(p => !p["$ref"] || !p["$ref"].endsWith("#/parameters/BlobDeleteType"));
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}"].delete.responses["202"]
    transform: >
      $.schema = {
        "type": "object",
        "x-ms-client-name": "DeleteBlobResult",
        "x-ms-sealed": false,
        "properties": {
          "Deleted": {"type": "boolean", "x-ms-client-default": true, "x-ms-xml": {"name": ""}, "description": "Indicates if the blob was successfully deleted by this operation."}
        }
      };
```

### SetBlobHttpHeaders

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=properties&SetHTTPHeaders"].put.responses["200"].headers
    transform: >
      $["x-ms-blob-sequence-number"]["x-ms-client-name"] = "SequenceNumber";
      $["x-ms-blob-sequence-number"]["x-nullable"] = true;
```

### SetBlobMetadata

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=metadata"].put.responses["200"]
    transform: >
      $.headers["x-ms-version-id"]["x-nullable"] = true;
      $.headers["x-ms-encryption-key-sha256"]["x-nullable"] = true;
      $.headers["x-ms-encryption-scope"]["x-nullable"] = true;
      $.schema = {
        "type": "object",
        "x-ms-client-name": "SetBlobMetadataResult",
        "x-ms-sealed": false,
        "properties": {
          "SequenceNumber": {
            "type": "integer",
            "format": "int64",
            "x-nullable": true,
            "x-ms-xml": {"name": ""},
            "description": "The field is deprecated and is always null. Use GetProperties() instead to check sequence number for a page blob."
          }
        }
      };
```

### CreateBlobSnapshot

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=snapshot"].put.responses["201"]
    transform: >
      $.headers["x-ms-version-id"]["x-nullable"] = true;
      $.schema = {
        "type": "object",
        "x-ms-client-name": "CreateBlobSnapshotResult",
        "x-ms-sealed": false,
        "properties": {
          "EncryptionKeySha256": {
            "type": "string",
            "format": "byte",
            "x-nullable": true,
            "x-ms-xml": {"name": ""},
            "description": "The field is deprecated and is always null. Use GetProperties() instead to get SHA256 of the encryption key."
          },
          "EncryptionScope" : {
            "type": "string",
            "x-nullable": true,
            "x-ms-xml": {"name": ""},
            "description": "The field is deprecated and is always null. Use GetProperties() instead to check the encryption scope."
          }
        }
      };
```

### StartBlobCopyFromUri

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=copy"].parameters
    transform: >
      $.push({"$ref": "#/parameters/SourceLeaseId"});
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=copy"].put.responses["202"].headers
    transform: >
      $["x-ms-version-id"]["x-nullable"] = true;
```

### CopyBlobFromUri

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=copy&sync"].put.parameters
    transform: >
      $.push({"$ref": "#/parameters/SourceContentCRC64"});
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=copy&sync"].put.responses["202"].headers
    transform: >
      $["x-ms-version-id"]["x-nullable"] = true;
      $["Content-MD5"]["x-ms-client-name"] = "TransactionalContentHash";
      $["Content-MD5"]["x-nullable"] = true;
      $["x-ms-content-crc64"]["x-ms-client-name"] = "TransactionalContentHash";
      $["x-ms-content-crc64"]["x-nullable"] = true;
      $["x-ms-encryption-scope"]["x-nullable"] = true;
```

### QueryBlobContent

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.BlobQueryArrowFieldType = {
        "type": "string",
        "enum": ["Int64", "Bool", "Timestamp", "String", "Double", "Decimal"],
        "x-ms-enum": {
          "name": "BlobQueryArrowFieldType",
          "modelAsString": false,
          "values": [
            {"value": "int64", "name": "Int64"},
            {"value": "bool", "name": "Bool"},
            {"value": "timestamp[ms]", "name": "Timestamp"},
            {"value": "string", "name": "String"},
            {"value": "double", "name": "Double"},
            {"value": "decimal", "name": "Decimal"}
          ]
        },
        "description": "Type of blob query arrow field."
      };
      if ($.ParquetConfiguration) {
        $.ParquetConfiguration.properties = {"__placeHolder" : { "type": "integer"}};
      }
      $.QuerySerialization["x-namespace"] = "_detail";
      $.QueryFormat["x-namespace"] = "_detail";
      $.QueryType["x-namespace"] = "_detail";
      $.DelimitedTextConfiguration["x-namespace"] = "_detail";
      $.JsonTextConfiguration["x-namespace"] = "_detail";
      $.ArrowConfiguration["x-namespace"] = "_detail";
      $.ParquetConfiguration["x-namespace"] = "_detail";
      $.QueryRequest["x-namespace"] = "_detail";
      $.QueryRequest.properties["QueryType"]["x-namespace"] = "_detail";
      $.ArrowField["x-ms-client-name"] = "BlobQueryArrowField";
      $.ArrowField.properties["Type"] = {"$ref": "#/definitions/BlobQueryArrowFieldType"};
      $.DelimitedTextConfiguration.properties["HeadersPresent"]["x-ms-xml"] = $.DelimitedTextConfiguration.properties["HeadersPresent"]["xml"];
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=query"].post.parameters
    transform: >
      $.push({"$ref": "#/parameters/EncryptionScope"});
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=query"].post.responses
    transform: >  
      for (const status_code of ["200", "206"]) {
        delete $[status_code].headers["x-ms-meta"];
        delete $[status_code].headers["Content-Length"];
        delete $[status_code].headers["Content-Type"];
        delete $[status_code].headers["Content-Range"];
        delete $[status_code].headers["Content-MD5"];
        delete $[status_code].headers["Content-Encoding"];
        delete $[status_code].headers["Cache-Control"];
        delete $[status_code].headers["Content-Disposition"];
        delete $[status_code].headers["Content-Language"];
        delete $[status_code].headers["x-ms-blob-sequence-number"];
        delete $[status_code].headers["x-ms-blob-type"];
        delete $[status_code].headers["x-ms-copy-completion-time"];
        delete $[status_code].headers["x-ms-copy-status-description"];
        delete $[status_code].headers["x-ms-copy-id"];
        delete $[status_code].headers["x-ms-copy-progress"];
        delete $[status_code].headers["x-ms-copy-source"];
        delete $[status_code].headers["x-ms-copy-status"];
        delete $[status_code].headers["Accept-Ranges"];
        delete $[status_code].headers["x-ms-blob-committed-block-count"];
        delete $[status_code].headers["x-ms-encryption-key-sha256"];
        delete $[status_code].headers["x-ms-encryption-scope"];
        delete $[status_code].headers["x-ms-blob-content-md5"];
        delete $[status_code].headers["x-ms-content-crc64"];
        $[status_code].headers["x-ms-lease-duration"]["x-nullable"] = true;
        $[status_code].headers["x-ms-lease-state"]["x-ms-client-default"] = "";
        $[status_code].headers["x-ms-lease-state"]["x-nullable"] = true;
        $[status_code].headers["x-ms-lease-status"]["x-ms-client-default"] = "";
        $[status_code].headers["x-ms-lease-status"]["x-nullable"] = true;
      }
```

### PutBlockList

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.BlockLookupList.properties["Committed"]["x-ms-xml"] = {"name": "."};
      $.BlockLookupList.properties["Uncommitted"]["x-ms-xml"] = {"name": "."};
      $.BlockLookupList.properties["Latest"]["x-ms-xml"] = {"name": "."};
```

### CreatePageBlob

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?PageBlob"].put
    transform: >
      for (const p of $.parameters) {
          if (p["$ref"].endsWith("#/parameters/PremiumPageBlobAccessTierOptional"))
          {
              p["$ref"] = "#/parameters/AccessTierOptional";
          }
      }
      $.responses["201"].schema = {
        "type": "object",
        "x-ms-client-name": "CreatePageBlobResult",
        "x-ms-sealed": false,
        "properties": {
          "Created": {
            "type": "boolean",
            "x-ms-client-default": true,
            "x-ms-xml": {"name": ""},
            "description": "Indicates if the page blob was successfully created by this operation."
          },
          "SequenceNumber": {
            "type": "integer",
            "format": "int64",
            "x-nullable": true,
            "x-ms-xml": {"name": ""},
            "description": "The field is deprecated and is always null. Use GetProperties() instead to check sequence number for a page blob."
          }
        }
      };
      $.responses["201"].headers["x-ms-version-id"]["x-nullable"] = true;
      $.responses["201"].headers["x-ms-encryption-key-sha256"]["x-nullable"] = true;
      $.responses["201"].headers["x-ms-encryption-scope"]["x-nullable"] = true;
      delete $.responses["201"].headers["Content-MD5"];
```

### UploadPages

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=page&update"].put.responses["201"].headers
    transform: >
      $["Content-MD5"]["x-ms-client-name"] = "TransactionalContentHash";
      $["Content-MD5"]["x-nullable"] = true;
      $["x-ms-content-crc64"]["x-ms-client-name"] = "TransactionalContentHash";
      $["x-ms-content-crc64"]["x-nullable"] = true;
      $["x-ms-blob-sequence-number"]["x-ms-client-name"] = "SequenceNumber";
      $["x-ms-encryption-key-sha256"]["x-nullable"] = true;
      $["x-ms-encryption-scope"]["x-nullable"] = true;
```

### UploadPagesFromUri

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=page&update&fromUrl"].put.responses["201"].headers
    transform: >
      $["Content-MD5"]["x-ms-client-name"] = "TransactionalContentHash";
      $["Content-MD5"]["x-nullable"] = true;
      $["x-ms-content-crc64"]["x-ms-client-name"] = "TransactionalContentHash";
      $["x-ms-content-crc64"]["x-nullable"] = true;
      $["x-ms-blob-sequence-number"]["x-ms-client-name"] = "SequenceNumber";
      $["x-ms-encryption-key-sha256"]["x-nullable"] = true;
      $["x-ms-encryption-scope"]["x-nullable"] = true;
```

### ClearPages

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=page&clear"].put.responses["201"].headers
    transform: >
      delete $["Content-MD5"];
      delete $["x-ms-content-crc64"];
      $["x-ms-blob-sequence-number"]["x-ms-client-name"] = "SequenceNumber";
```

### GetPageRanges

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=pagelist"].get.responses["200"]
    transform: >
      $.headers["x-ms-blob-content-length"]["x-ms-client-name"] = "BlobSize";
      $.schema = {
        "type": "object",
        "x-ms-client-name": "GetPageRangesResult",
        "x-namespace": "_detail",
        "xml": {"name": "."},
        "properties": {
          "ETag": {"type": "string", "format": "etag", "x-ms-xml": {"name": ""}},
          "LastModified": {"type": "string", "format": "date-time-rfc1123", "x-ms-xml": {"name": ""}},
          "BlobSize": {"type": "integer", "format": "int64", "x-ms-xml": {"name": ""}, "description": "Size of the blob in bytes."},
          "PageRange": {
            "type": "array",
            "x-ms-client-name": "PageRanges",
            "x-ms-xml": {"name": "PageList"},
            "items": {"$ref": "#/definitions/PageRange"}
          },
          "ClearRange": {
            "type": "array",
            "x-ms-client-name": "ClearRanges",
            "x-ms-xml": {"name": "PageList"},
            "items": {"$ref": "#/definitions/ClearRange"}
          },
          "ContinuationToken": {
            "type": "string",
            "x-ms-xml": {"name": "PageList/NextMarker"}
          }
        }
      }
```

### GetPageRangesDiff

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=pagelist&diff"].get.responses["200"]
    transform: >
      $.headers["x-ms-blob-content-length"]["x-ms-client-name"] = "BlobSize";
      $.schema = {
        "type": "object",
        "x-ms-client-name": "GetPageRangesDiffResult",
        "x-namespace": "_detail",
        "xml": {"name": "."},
        "properties": {
          "ETag": {"type": "string", "format": "etag", "x-ms-xml": {"name": ""}},
          "LastModified": {"type": "string", "format": "date-time-rfc1123", "x-ms-xml": {"name": ""}},
          "BlobSize": {"type": "integer", "format": "int64", "x-ms-xml": {"name": ""}, "description": "Size of the blob in bytes."},
          "PageRange": {
            "type": "array",
            "x-ms-client-name": "PageRanges",
            "x-ms-xml": {"name": "PageList"},
            "items": {"$ref": "#/definitions/PageRange"}
          },
          "ClearRange": {
            "type": "array",
            "x-ms-client-name": "ClearRanges",
            "x-ms-xml": {"name": "PageList"},
            "items": {"$ref": "#/definitions/ClearRange"}
          },
          "ContinuationToken": {
            "type": "string",
            "x-ms-xml": {"name": "PageList/NextMarker"}
          }
        }
      }
```

### ResizePageBlob

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=properties&Resize"].put.responses["200"].headers
    transform: >
      $["x-ms-blob-sequence-number"]["x-ms-client-name"] = "SequenceNumber";
```

### UpdatePageBlobSequenceNumber

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=properties&UpdateSequenceNumber"].put.responses["200"].headers
    transform: >
      $["x-ms-blob-sequence-number"]["x-ms-client-name"] = "SequenceNumber";
```

### StartCopyPageBlobIncremental

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=incrementalcopy"].put.responses["202"].headers
    transform: >
      $["x-ms-version-id"] = {"x-ms-client-name": "VersionId", "type": "string", "x-nullable": true, "description": "A DateTime value returned by the service that uniquely identifies the blob. The value of this header indicates the blob version, and may be used in subsequent requests to access this version of the blob."};
```

### CreateAppendBlob

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?AppendBlob"].put.responses["201"]
    transform: >
      $.schema = {
        "type": "object",
        "x-ms-client-name": "CreateAppendBlobResult",
        "x-ms-sealed": false,
        "properties": {
          "Created": {"type": "boolean", "x-ms-client-default": true, "x-ms-xml": {"name": ""}, "description": "Indicates if the append blob was successfully created by this operation."}
        }
      };
      $.headers["x-ms-version-id"]["x-nullable"] = true;
      $.headers["x-ms-encryption-key-sha256"]["x-nullable"] = true;
      $.headers["x-ms-encryption-scope"]["x-nullable"] = true;
      delete $.headers["Content-MD5"];
```

### AppendBlock

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=appendblock"].put.responses["201"].headers
    transform: >
      $["x-ms-blob-append-offset"]["x-ms-client-name"] = "AppendOffset";
      $["x-ms-blob-append-offset"]["type"] = "integer";
      $["x-ms-blob-append-offset"]["format"] = "int64";
      $["x-ms-blob-committed-block-count"]["x-ms-client-name"] = "CommittedBlockCount";
      $["x-ms-encryption-key-sha256"]["x-nullable"] = true;
      $["x-ms-encryption-scope"]["x-nullable"] = true;
      $["Content-MD5"]["x-ms-client-name"] = "TransactionalContentHash";
      $["Content-MD5"]["x-nullable"] = true;
      $["x-ms-content-crc64"]["x-ms-client-name"] = "TransactionalContentHash";
      $["x-ms-content-crc64"]["x-nullable"] = true;
```

### AppendBlockFromUri

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=appendblock&fromUrl"].put.responses["201"].headers
    transform: >
      $["x-ms-blob-append-offset"]["x-ms-client-name"] = "AppendOffset";
      $["x-ms-blob-append-offset"]["type"] = "integer";
      $["x-ms-blob-append-offset"]["format"] = "int64";
      $["x-ms-blob-committed-block-count"]["x-ms-client-name"] = "CommittedBlockCount";
      $["x-ms-encryption-key-sha256"]["x-nullable"] = true;
      $["x-ms-encryption-scope"]["x-nullable"] = true;
      $["Content-MD5"]["x-ms-client-name"] = "TransactionalContentHash";
      $["Content-MD5"]["x-nullable"] = true;
      $["x-ms-content-crc64"]["x-ms-client-name"] = "TransactionalContentHash";
      $["x-ms-content-crc64"]["x-nullable"] = true;
```

### UploadBlockBlob

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?BlockBlob"].put.responses["201"].headers
    transform: >
      $["x-ms-version-id"]["x-nullable"] = true;
      $["x-ms-encryption-key-sha256"]["x-nullable"] = true;
      $["x-ms-encryption-scope"]["x-nullable"] = true;
      $["Content-MD5"]["x-ms-client-name"] = "TransactionalContentHash";
      $["Content-MD5"]["x-nullable"] = true;
      $["x-ms-content-crc64"] = {"type": "string", "format": "byte", "x-ms-client-name": "TransactionalContentHash", "x-nullable": true};
```

### StageBlock

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=block"].put.responses["201"].headers
    transform: >
      $["x-ms-encryption-key-sha256"]["x-nullable"] = true;
      $["x-ms-encryption-scope"]["x-nullable"] = true;
      $["Content-MD5"]["x-ms-client-name"] = "TransactionalContentHash";
      $["Content-MD5"]["x-nullable"] = true;
      $["x-ms-content-crc64"]["x-ms-client-name"] = "TransactionalContentHash";
      $["x-ms-content-crc64"]["x-nullable"] = true;
```

### StageBlockFromUri

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=block&fromURL"].put.responses["201"].headers
    transform: >
      $["x-ms-encryption-key-sha256"]["x-nullable"] = true;
      $["x-ms-encryption-scope"]["x-nullable"] = true;
      $["Content-MD5"]["x-ms-client-name"] = "TransactionalContentHash";
      $["Content-MD5"]["x-nullable"] = true;
      $["x-ms-content-crc64"]["x-ms-client-name"] = "TransactionalContentHash";
      $["x-ms-content-crc64"]["x-nullable"] = true;
```

### CommitBlockList

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=blocklist"].put.responses["201"].headers
    transform: >
      $["x-ms-version-id"]["x-nullable"] = true;
      $["Content-MD5"]["x-ms-client-name"] = "TransactionalContentHash";
      $["Content-MD5"]["x-nullable"] = true;
      $["x-ms-content-crc64"]["x-ms-client-name"] = "TransactionalContentHash";
      $["x-ms-content-crc64"]["x-nullable"] = true;
      $["x-ms-encryption-key-sha256"]["x-nullable"] = true;
      $["x-ms-encryption-scope"]["x-nullable"] = true;
```

### GetBlockList

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.Block["x-ms-client-name"] = "BlobBlock";
      $.Block["xml"] = {"name": "Block"};
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=blocklist"].get.responses
    transform: >
      $["200"].headers["x-ms-blob-content-length"]["x-ms-client-name"] = "BlobSize";
      $["200"].headers["x-ms-blob-content-length"]["x-nullable"] = true;
      $["200"].headers["ETag"]["x-ms-client-name"] = "ETag";
      $["200"].headers["ETag"]["x-nullable"] = true;
      $["200"].headers["Last-Modified"]["x-ms-client-name"] = "LastModified";
      $["200"].headers["Last-Modified"]["x-nullable"] = true;
      delete $["200"].headers["Content-Type"];
      $["200"].schema = {
        "x-ms-client-name": "GetBlockListResult",
        "xml": {"name": "BlockList"},
        "type": "object",
        "properties": {
          "ETag": {"type": "string", "format": "etag", "x-ms-xml": {"name": ""}, "description": "The ETag contains a value that you can use to perform operations conditionally. If the request version is 2011-08-18 or newer, the ETag value will be in quotes."},
          "LastModified": {"type": "string", "format": "date-time-rfc1123", "x-ms-xml": {"name": ""}, "description": "Returns the date and time the container was last modified. Any operation that modifies the blob, including an update of the blob's metadata or properties, changes the last-modified time of the blob."},
          "BlobSize": {"type": "integer", "format": "int64", "x-ms-client-default": "0", "x-ms-xml": {"name": ""}, "description": "Size of the blob in bytes."},
          "CommittedBlocks": {"type": "array", "items": {"$ref": "#/definitions/Block"}, "description": "List of committed blocks."},
          "UncommittedBlocks": {"type": "array", "items": {"$ref": "#/definitions/Block"}, "description": "List of uncommitted blocks."}
        }
      };
```

### UploadBlockBlobFromUri

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?BlockBlob&fromUrl"].put.parameters
    transform: >
      $ = $.filter(p => !p["$ref"] || !p["$ref"].endsWith("#/parameters/ContentMD5"));
      $.push({"$ref": "#/parameters/SourceContentCRC64"});
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?BlockBlob&fromUrl"].put.responses["201"].headers
    transform: >
      $["Content-MD5"]["x-ms-client-name"] = "TransactionalContentHash";
      $["Content-MD5"]["x-nullable"] = true;
      $["x-ms-content-crc64"] = {"type": "string", "format": "byte", "x-ms-client-name": "TransactionalContentHash", "x-nullable": true};
      $["x-ms-version-id"]["x-nullable"] = true;
      $["x-ms-encryption-key-sha256"]["x-nullable"] = true;
      $["x-ms-encryption-scope"]["x-nullable"] = true;
```

### SetBlobImmutabilityPolicy

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=immutabilityPolicies"].put.responses["200"]
    transform: >
      $.schema = {
        "x-ms-client-name": "SetBlobImmutabilityPolicyResult",
        "xml": {"name": ""},
        "type": "object",
        "properties": {
          "ImmutabilityPolicy": {"$ref": "#/definitions/BlobImmutabilityPolicy", "x-ms-xml": {"name":""}}
        }
      };
      $.headers["x-ms-immutability-policy-until-date"]["x-ms-client-path"] = "ImmutabilityPolicy.ExpiresOn";
      $.headers["x-ms-immutability-policy-mode"]["x-ms-client-path"] = "ImmutabilityPolicy.PolicyMode";
```

### SetLegalHold

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}/{blob}?comp=legalhold"].put.responses["200"].headers
    transform: >
      $["x-ms-legal-hold"]["x-ms-client-name"] = "HasLegalHold";
```


### SubmitBatch

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.SubmitBatchResult = {
        "type": "object",
        "x-ms-sealed": false,
        "x-namespace": "_detail",
        "properties": {
          "BodyStream": {"type": "object", "format": "file"}
        }
      };
  - from: swagger-document
    where: $["x-ms-paths"]["/?comp=batch"].post.responses
    transform: >
      $["202"] = $["200"];
      delete $["200"];
      $["202"].schema =  {"$ref": "#/definitions/SubmitBatchResult"};
  - from: swagger-document
    where: $["x-ms-paths"]["/{containerName}?restype=container&comp=batch"].post.responses["202"]
    transform: >
      $.schema =  {"$ref": "#/definitions/SubmitBatchResult"};
```