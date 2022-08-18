# Azure Storage C++ Protocol Layer

> see https://aka.ms/autorest

## Configuration

```yaml
package-name: azure-storage-files-shares
namespace: Azure::Storage::Files::Shares
output-folder: generated
clear-output-folder: true
input-file: https://raw.githubusercontent.com/Azure/azure-rest-api-specs/main/specification/storage/data-plane/Microsoft.FileStorage/preview/2021-06-08/file.json
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
      delete $["/{shareName}?restype=share&comp=undelete"].put;
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
        "enum": ["2021-06-08"],
        "description": "The version used for the operations to Azure storage services."
      };
  - from: swagger-document
    where: $.parameters
    transform: >
      $.ApiVersionParameter.enum[0] = "2021-06-08";
```

### Rename Operations

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]
    transform: >
      $["/{shareName}/{directory}/{fileName}?comp=range&fromURL"].put.operationId = "File_UploadRangeFromUri";
```

### Define names for return types

```yaml
directive:
  - from: swagger-document
    where: $
    transform: >
      const operationReturnTypeNames = new Map(Object.entries({
        "Directory_ForceCloseHandles": "ForceCloseDirectoryHandlesResult",
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
    where: $.definitions
    transform: >
      $.ListHandlesResponse["x-namespace"] = "_detail";
      $.ListSharesResponse["x-namespace"] = "_detail";
      $.FilesAndDirectoriesListSegment["x-namespace"] = "_detail";
      $.ListFilesAndDirectoriesSegmentResponse["x-namespace"] = "_detail";
  - from: swagger-document
    where: $
    transform: >
      const operations = [
        "Share_AcquireLease",
        "Share_ReleaseLease",
        "Share_ChangeLease",
        "Share_RenewLease",
        "Share_BreakLease",
        "File_AcquireLease",
        "File_ReleaseLease",
        "File_ChangeLease",
        "File_BreakLease",
        "File_StartCopy",
        "Directory_ForceCloseHandles",
        "File_ForceCloseHandles",
        "Directory_Rename",
        "File_Rename",
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
```

### Global Changes for Definitions, Types etc.

```yaml
directive:
  - from: swagger-document
    where: $.parameters
    transform: >
      $.ListSharesInclude.items["x-ms-enum"].name = "ListSharesIncludeFlags";
      $.AccessTierOptional.enum.push("Premium");
      $.AccessTierOptional["x-ms-enum"].name = "AccessTier";
      $.AccessTierOptional["x-ms-enum"].modelAsString = false;
      $.DeleteSnapshots["x-ms-enum"].name = "DeleteSnapshotsOption";
      $.DeleteSnapshots.enum = ["__placeHolder", "include"];
      $.FileCopyPermissionCopyMode["x-ms-enum"].name = "PermissionCopyMode";
      $.MaxResults["x-ms-client-name"] = "MaxResults";
      delete $.FileCreationTime.format;
      delete $.FileLastWriteTime.format;
      $.ShareEnabledProtocols["enum"] = ["Smb", "Nfs"];
      $.ShareEnabledProtocols["x-ms-enum"] = {"name": "ShareProtocols", "modelAsString": false};
      $.ShareEnabledProtocols["x-ms-enum"]["values"] = [{"value": "SMB", "name": "Smb"},{"value": "NFS", "name": "Nfs"}];
      delete $.FileChangeTime.format;
      $.FileLastWriteTimeMode["x-ms-enum"]["values"] = [{"value": "now", "name": "Now"},{"value": "preserve", "name": "Preserve"}];
  - from: swagger-document
    where: $.definitions
    transform: >
      $.ContentRange = {"type": "object", "x-ms-format": "range", properties: {"__placeHolder" : {"type": "integer"}}};
      $.Range = $.ContentRange;
      $.ClearRange = $.Range;
      $.LeaseStatus["x-ms-enum"]["name"] = "LeaseStatus";
      $.LeaseState["x-ms-enum"]["name"] = "LeaseState";
      $.FileAttributes = {
        "type": "string",
        "x-ms-separator": " | ",
        "enum": ["ReadOnly", "Hidden", "System", "None", "Directory", "Archive", "Temporary", "Offline", "NotContentIndexed", "NoScrubData"],
        "x-ms-enum": {
          "name": "FileAttributes",
          "modelAsString": false
        },
        "description": "Attributes for a file or directory."
      };
      $.FileSmbProperties = {
        "type": "object",
        "properties": {
          "PermissionKey": {"type": "string", "x-nullable": true, "description": "Permission key for the directory or file."},
          "Attributes": {"$ref": "#/definitions/FileAttributes", "description": "Attributes for the file or directory."},
          "CreatedOn": {"type": "string", "format": "date-time", "x-nullable": true, "description": "Creation time for the file or directory."},
          "LastWrittenOn": {"type": "string", "format": "date-time", "x-nullable": true, "description": "Last write time for the file or directory."},
          "ChangedOn": {"type": "string", "format": "date-time", "x-nullable": true, "description": "Changed time for the file or directory."},
          "FileId": {"type": "string", "description": "The fileId of the file."},
          "ParentFileId": {"type": "string", "description": "The parentId of the file."}
        },
        "description": "The SMB related properties for the file."
      };
      $.AccessTier = {
        "type": "string",
        "enum": ["TransactionOptimized", "Hot", "Cool", "Premium"],
        "x-ms-enum": {
          "name": "AccessTier",
          "modelAsString": false
        },
        "description": "Specifies the access tier of the share."
      };
      $.Metadata = {"type": "object", "x-ms-format": "caseinsensitivemap", properties: {"__placeHolder" : {"type": "integer"}}, "description": "A set of name-value pairs associated with the share or file."};
      $.CopyStatus = {
        "type": "string",
        "enum": ["pending", "success", "aborted", "failed"],
        "x-ms-enum": {
          "name": "CopyStatus",
          "modelAsString": false
        },
        "description": "Status of a copy operation."
      };
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
      $.FileHttpHeaders = {
        "type": "object",
        "properties": {
          "Content-Type": {"type": "string", "description": "MIME content type of the file."},
          "Content-Encoding": {"type": "string", "description": "Specifies which content encodings have been applied to the file."},
          "Content-Language": {"type": "string", "description": "Specifies the natural languages used by this file."},
          "Content-Hash": {"$ref": "#/definitions/ContentHash", "x-ms-xml": {"name": "."}, "description": "Hash of the file content."},
          "Content-Disposition": {"type": "string", "description": "Conveys additional information about how to process the resource payload, and also can be used to attach additional metadata." },
          "Cache-Control": {"type": "string", "description": "Specifies directives for caching mechanisms." }
        },
        "description": "Standard HTTP properties supported files."
      };
      $.SharePermission["x-namespace"] = "_detail";
      $.ShareEnabledProtocols["enum"] = ["Smb", "Nfs"];
      $.ShareEnabledProtocols["x-ms-enum"] = {"name": "ShareProtocols", "modelAsString": false};
      $.ShareEnabledProtocols["x-ms-enum"]["values"] = [{"value": "SMB", "name": "Smb"},{"value": "NFS", "name": "Nfs"}];
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
        if (header === "x-ms-meta") {
          $[header]["x-ms-format"] = "caseinsensitivemap";
        }
      }
```

### GetFileServiceProperties

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.Metrics["type"] = "object";
      delete $.Metrics.required;
      $.Metrics.properties["IncludeAPIs"]["x-ms-client-name"] = "IncludeApis";
      $.Metrics.properties["IncludeAPIs"]["x-nullable"] = true;
      $.SmbSettings = $.ShareSmbSettings;
      delete $.ShareSmbSettings;
      $.ShareProtocolSettings.properties["Smb"]["$ref"] = "#/definitions/SmbSettings";
      $.ShareProtocolSettings.properties["Settings"] = $.ShareProtocolSettings.properties["Smb"];
      delete $.ShareProtocolSettings.properties["Smb"];
      $.ProtocolSettings = $.ShareProtocolSettings;
      delete $.ShareProtocolSettings;
      $.StorageServiceProperties.properties["Protocol"]["$ref"] = "#/definitions/ProtocolSettings";
      $.ShareServiceProperties = $.StorageServiceProperties;
      delete $.StorageServiceProperties;
      $.ShareServiceProperties.xml = { "name": "StorageServiceProperties" };
      $.ShareServiceProperties.properties["Protocol"]["x-nullable"] = true;
  - from: swagger-document
    where: $.parameters
    transform: >
      $.ShareServiceProperties = $.StorageServiceProperties;
      $.ShareServiceProperties.name = "ShareServiceProperties";
      $.ShareServiceProperties.schema["$ref"] = "#/definitions/ShareServiceProperties";
      delete $.StorageServiceProperties;
  - from: swagger-document
    where: $["x-ms-paths"]["/?restype=service&comp=properties"]
    transform: >
      $.put.parameters[0]["$ref"] = "#/parameters/ShareServiceProperties";
      $.get.responses["200"].schema["$ref"] = "#/definitions/ShareServiceProperties";
```

### GetShareStatistics

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      delete $.ShareStats;
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}?restype=share&comp=stats"].get.responses["200"]
    transform: >
      $.schema = {
        "description": "Stats for the share.",
        "type": "object",
        "x-ms-sealed": false,
        "x-ms-client-name": "ShareStatistics",
        "xml": {"name": "ShareStats"},
        "properties": {
          "ShareUsageBytes": {
            "description": "The approximate size of the data stored in bytes. Note that this value may not include all recently created or recently resized files.",
            "type": "integer",
            "format": "int64",
            "x-ms-client-name": "ShareUsageInBytes"
          }
        }
      };
```

### ListShares

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.ListSharesResponse.properties["ShareItems"]["x-ms-xml"] = {"name": "Shares"};
      $.ShareItemDetails = $.SharePropertiesInternal;
      delete $.SharePropertiesInternal;
      $.ShareItemDetails.properties["Quota"]["format"] = "int64";
      $.ShareItemDetails.properties["AccessTierChangeTime"]["x-ms-client-name"] = "AccessTierChangedOn";
      $.ShareItemDetails.properties["AccessTier"] = {"$ref": "#/definitions/AccessTier"};
      $.ShareItemDetails.properties["DeletedTime"]["x-ms-client-name"] = "DeletedOn";
      $.ShareItemDetails.required.push("RemainingRetentionDays", "LeaseStatus", "LeaseState", "LeaseDuration");
      $.ShareItemInternal.properties["Details"] = {"$ref": "#/definitions/ShareItemDetails", "x-ms-xml": {"name": "Properties"}};
      $.ShareItemInternal["x-ms-client-name"] = "ShareItem";
      $.ShareItemDetails.properties["ProvisionedBandwidthMiBps"]["x-ms-client-name"] = "ProvisionedBandwidthMBps";
      delete $.ShareItemInternal.properties["Properties"];
      delete $.ShareItemInternal.required;
```

### CreateShare

```yaml
directive:
  - from: swagger-document
    where: $.parameters
    transform: >
      $.ShareQuota["format"] = "int64";
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}?restype=share"].put.responses["201"]
    transform: >
      $.schema = {
        "type": "object",
        "x-ms-client-name": "CreateShareResult",
        "x-ms-sealed": false,
        "properties": {
          "Created": {"type": "boolean", "x-ms-client-default": true, "x-ms-xml": {"name": ""}, "description": "Indicates if the share was successfully created by this operation."}
        }
      };
```

### GetShareProperties

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}?restype=share"].get.responses["200"].headers
    transform: >
      $["x-ms-share-quota"]["format"] = "int64";
      $["x-ms-share-provisioned-iops"]["x-nullable"] = true;
      $["x-ms-share-provisioned-ingress-mbps"]["x-nullable"] = true;
      $["x-ms-share-provisioned-egress-mbps"]["x-nullable"] = true;
      $["x-ms-share-next-allowed-quota-downgrade-time"]["x-nullable"] = true;
      $["x-ms-lease-duration"]["x-nullable"] = true;
      $["x-ms-lease-state"]["x-nullable"] = true;
      $["x-ms-lease-status"]["x-nullable"] = true;
      $["x-ms-access-tier-change-time"]["x-nullable"] = true;
      $["x-ms-access-tier-change-time"]["x-ms-client-name"] = "AccessTierChangedOn";
      $["x-ms-access-tier-transition-state"]["x-nullable"] = true;
      $["x-ms-access-tier-transition-state"].description = "Returns the transition state between access tiers, when present.";
      $["x-ms-share-provisioned-iops"].description = "Returns the current share provisioned IOPS.";
      $["x-ms-share-provisioned-bandwidth-mibps"]["x-nullable"] = true;
      $["x-ms-share-provisioned-bandwidth-mibps"]["x-ms-client-name"] = "ProvisionedBandwidthMBps";
      $["x-ms-enabled-protocols"]["x-nullable"] = true;
      $["x-ms-root-squash"]["x-nullable"] = true;
      $["x-ms-enabled-protocols"]["enum"] = ["Smb", "Nfs"];
      $["x-ms-enabled-protocols"]["x-ms-enum"] = {"name": "ShareProtocols", "modelAsString": false};
      $["x-ms-enabled-protocols"]["x-ms-enum"]["values"] = [{"value": "SMB", "name": "Smb"},{"value": "NFS", "name": "Nfs"}];
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}?restype=share"].get.responses["200"]
    transform: >
      $.schema = {
        "type": "object",
        "x-ms-client-name": "ShareProperties",
        "x-ms-sealed": false,
        "properties": {
          "AccessTier": {"$ref": "#/definitions/AccessTier", "x-nullable": true, "x-ms-xml": {"name": ""}}
        }
      };
```

### GetShareAccessPolicy

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.AccessPolicy.required = ["Permission"];
      $.AccessPolicy.properties["Start"]["x-ms-client-name"] = "StartsOn";
      $.AccessPolicy.properties["Expiry"]["x-ms-client-name"] = "ExpiresOn";
      $.SignedIdentifier.properties["AccessPolicy"]["x-ms-client-name"] = "Policy";
      delete $.SignedIdentifier.required;
      delete $.SignedIdentifiers.items.xml;
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}?restype=share&comp=acl"].get.responses["200"]
    transform: >
      $.schema = {
        "type": "object",
        "x-ms-client-name": "ShareAccessPolicy",
        "x-ms-sealed": "false",
        "xml": {"name": "."},
        "properties": {
          "SignedIdentifiers": {"$ref": "#/definitions/SignedIdentifiers"}
        }
      };
```

### DeleteShare

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}?restype=share"].delete.responses["202"]
    transform: >
      $.schema = {
        "type": "object",
        "x-ms-client-name": "DeleteShareResult",
        "x-ms-sealed": false,
        "properties": {
          "Deleted": {"type": "boolean", "x-ms-client-default": true, "x-ms-xml": {"name": ""}, "description": "Indicates if the share was successfully deleted by this operation."},
        }
      };
```

### ListFilesAndDirectories

```yaml
directive:
  - from: swagger-document
    where: $.parameters
    transform: >
      $.ListFilesInclude["items"]["x-ms-enum"]["values"] = [{"name": "Timestamps", "value": "Timestamps"}, {"name": "ETag", "value": "Etag"}, {"name": "Attributes", "value": "Attributes"}, {"name": "PermissionKey", "value": "PermissionKey"},];
  - from: swagger-document
    where: $.definitions
    transform: >
      $.ListFilesAndDirectoriesSegmentResponse.properties["Segment"]["x-ms-xml"] = {"name": "Entries"};
      $.FileItemDetails = $.FileProperty;
      $.FileItemDetails.properties["Content-Length"]["x-ms-client-name"] = "FileSize";
      $.FileItemDetails.properties["SmbProperties"] = {"$ref": "#/definitions/FileSmbProperties", "x-ms-xml": {"name": "."}};
      $.FileItemDetails.properties["LastAccessTime"]["x-ms-client-name"] = "LastAccessedOn";
      $.FileItemDetails.properties["LastAccessTime"]["x-nullable"] = true;
      $.FileSmbProperties.properties["PermissionKey"]["x-ms-xml"] = {"name": "../PermissionKey"};
      $.FileSmbProperties.properties["Attributes"]["x-ms-xml"] = {"name": "../Attributes"};
      $.FileSmbProperties.properties["CreatedOn"]["x-ms-xml"] = {"name": "CreationTime"};
      $.FileSmbProperties.properties["LastWrittenOn"]["x-ms-xml"] = {"name": "LastWriteTime"};
      $.FileSmbProperties.properties["ChangedOn"]["x-ms-xml"] = {"name": "ChangeTime"};
      $.FileSmbProperties.properties["FileId"]["x-ms-xml"] = {"name": "../FileId"};
      $.FileSmbProperties.properties["ParentFileId"]["x-ms-xml"] = {"name": ""};
      delete $.FileItemDetails.properties["CreationTime"];
      delete $.FileItemDetails.properties["LastWriteTime"];
      delete $.FileItemDetails.properties["ChangeTime"];
      delete $.FileItemDetails.required;
      delete $.FileProperty;
      delete $.FileItem.properties["Properties"];
      delete $.FileItem.properties["FileId"];
      delete $.FileItem.properties["Attributes"];
      delete $.FileItem.properties["PermissionKey"];
      delete $.FileItem.required;
      $.FileItem.properties["Details"] = {"$ref": "#/definitions/FileItemDetails", "x-ms-xml" : {"name": "Properties"}};
      
      delete $.DirectoryItem.properties["Properties"];
      delete $.DirectoryItem.properties["FileId"];
      delete $.DirectoryItem.properties["Attributes"];
      delete $.DirectoryItem.properties["PermissionKey"];
      delete $.DirectoryItem.required;
      $.DirectoryItemDetails = JSON.parse(JSON.stringify($.FileItemDetails));
      delete $.DirectoryItemDetails.properties["Content-Length"];
      $.DirectoryItem.properties["Details"] = {"$ref": "#/definitions/DirectoryItemDetails", "x-ms-xml" : {"name": "Properties"}};
      
      $.FilesAndDirectoriesListSegment.properties["DirectoryItems"]["x-ms-xml"] = {"name": "."};
      $.FilesAndDirectoriesListSegment.properties["FileItems"]["x-ms-xml"] = {"name": "."};
```

### ListHandles

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      delete $.HandleItem.required;
      $.HandleItem.properties["OpenTime"]["x-ms-client-name"] = "OpenedOn";
      $.HandleItem.properties["LastReconnectTime"]["x-ms-client-name"] = "LastReconnectedOn";
```

### ForceCloseFileHandles

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}?comp=forceclosehandles"].put.responses["200"].headers
    transform: >
      $["x-ms-marker"]["x-ms-client-name"] = "ContinuationToken";
      $["x-ms-marker"]["x-nullable"] = true;
```

### ForceCloseDirectoryHandles

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}?comp=forceclosehandles"].put.responses["200"].headers
    transform: >
      $["x-ms-marker"]["x-ms-client-name"] = "ContinuationToken";
      $["x-ms-marker"]["x-nullable"] = true;
```

### CreateDirecotry

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}?restype=directory"].put.responses["201"]
    transform: >
      $.headers["x-ms-file-permission-key"]["x-ms-client-path"] = "SmbProperties.PermissionKey";
      $.headers["x-ms-file-attributes"]["x-ms-client-path"] = "SmbProperties.Attributes";
      $.headers["x-ms-file-creation-time"]["x-ms-client-path"] = "SmbProperties.CreatedOn";
      $.headers["x-ms-file-last-write-time"]["x-ms-client-path"] = "SmbProperties.LastWrittenOn";
      $.headers["x-ms-file-change-time"]["x-ms-client-path"] = "SmbProperties.ChangedOn";
      $.headers["x-ms-file-id"]["x-ms-client-path"] = "SmbProperties.FileId";
      $.headers["x-ms-file-parent-id"]["x-ms-client-path"] = "SmbProperties.ParentFileId";
      $.schema = {
        "type": "object",
        "x-ms-client-name": "CreateDirectoryResult",
        "x-ms-sealed": false,
        "properties": {
          "Created": {"type": "boolean", "x-ms-client-default": true, "x-ms-xml": {"name": ""}, "description": "Indicates if the directory was successfully created by this operation."},
          "SmbProperties": {"$ref": "#/definitions/FileSmbProperties", "x-ms-xml": {"name": ""}}
        }
      };
```

### DeleteDirectory

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}?restype=directory"].delete.responses["202"]
    transform: >
      $.schema = {
        "type": "object",
        "x-ms-client-name": "DeleteDirectoryResult",
        "x-ms-sealed": false,
        "properties": {
          "Deleted": {"type": "boolean", "x-ms-client-default": true, "x-ms-xml": {"name": ""}, "description": "Indicates if the directory was successfully deleted by this operation."}
        }
      };
```

### GetDirectoryProperties

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}?restype=directory"].get.responses["200"]
    transform: >
      $.headers["x-ms-file-permission-key"]["x-ms-client-path"] = "SmbProperties.PermissionKey";
      $.headers["x-ms-file-attributes"]["x-ms-client-path"] = "SmbProperties.Attributes";
      $.headers["x-ms-file-creation-time"]["x-ms-client-path"] = "SmbProperties.CreatedOn";
      $.headers["x-ms-file-last-write-time"]["x-ms-client-path"] = "SmbProperties.LastWrittenOn";
      $.headers["x-ms-file-change-time"]["x-ms-client-path"] = "SmbProperties.ChangedOn";
      $.headers["x-ms-file-id"]["x-ms-client-path"] = "SmbProperties.FileId";
      $.headers["x-ms-file-parent-id"]["x-ms-client-path"] = "SmbProperties.ParentFileId";
      $.schema = {
        "type": "object",
        "x-ms-client-name": "DirectoryProperties",
        "x-ms-sealed": false,
        "properties": {
          "SmbProperties": {"$ref": "#/definitions/FileSmbProperties", "x-ms-xml": {"name": ""}}
        }
      };
```

### SetDirectoryProperties

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}?restype=directory&comp=properties"].put.responses["200"]
    transform: >
      $.headers["x-ms-file-permission-key"]["x-ms-client-path"] = "SmbProperties.PermissionKey";
      $.headers["x-ms-file-attributes"]["x-ms-client-path"] = "SmbProperties.Attributes";
      $.headers["x-ms-file-creation-time"]["x-ms-client-path"] = "SmbProperties.CreatedOn";
      $.headers["x-ms-file-last-write-time"]["x-ms-client-path"] = "SmbProperties.LastWrittenOn";
      $.headers["x-ms-file-change-time"]["x-ms-client-path"] = "SmbProperties.ChangedOn";
      $.headers["x-ms-file-id"]["x-ms-client-path"] = "SmbProperties.FileId";
      $.headers["x-ms-file-parent-id"]["x-ms-client-path"] = "SmbProperties.ParentFileId";
      $.schema = {
        "type": "object",
        "x-ms-client-name": "SetDirectoryPropertiesResult",
        "x-ms-sealed": false,
        "properties": {
          "SmbProperties": {"$ref": "#/definitions/FileSmbProperties", "x-ms-xml": {"name": ""}}
        }
      };
```

### CreateFile

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}"].put.responses["201"]
    transform: >
      $.headers["x-ms-file-permission-key"]["x-ms-client-path"] = "SmbProperties.PermissionKey";
      $.headers["x-ms-file-attributes"]["x-ms-client-path"] = "SmbProperties.Attributes";
      $.headers["x-ms-file-creation-time"]["x-ms-client-path"] = "SmbProperties.CreatedOn";
      $.headers["x-ms-file-last-write-time"]["x-ms-client-path"] = "SmbProperties.LastWrittenOn";
      $.headers["x-ms-file-change-time"]["x-ms-client-path"] = "SmbProperties.ChangedOn";
      $.headers["x-ms-file-id"]["x-ms-client-path"] = "SmbProperties.FileId";
      $.headers["x-ms-file-parent-id"]["x-ms-client-path"] = "SmbProperties.ParentFileId";
      $.schema = {
        "type": "object",
        "x-ms-client-name": "CreateFileResult",
        "x-ms-sealed": false,
        "properties": {
          "Created": {"type": "boolean", "x-ms-client-default": true, "x-ms-xml": {"name": ""}, "description": "Indicates if the file was successfully created by this operation."},
          "SmbProperties": {"$ref": "#/definitions/FileSmbProperties", "x-ms-xml": {"name": ""}}
        }
      };
```

### GetFileProperties

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}"].head.responses["200"]
    transform: >
      $.headers["x-ms-file-permission-key"]["x-ms-client-path"] = "SmbProperties.PermissionKey";
      $.headers["x-ms-file-attributes"]["x-ms-client-path"] = "SmbProperties.Attributes";
      $.headers["x-ms-file-creation-time"]["x-ms-client-path"] = "SmbProperties.CreatedOn";
      $.headers["x-ms-file-last-write-time"]["x-ms-client-path"] = "SmbProperties.LastWrittenOn";
      $.headers["x-ms-file-change-time"]["x-ms-client-path"] = "SmbProperties.ChangedOn";
      $.headers["x-ms-file-id"]["x-ms-client-path"] = "SmbProperties.FileId";
      $.headers["x-ms-file-parent-id"]["x-ms-client-path"] = "SmbProperties.ParentFileId";
      $.headers["Content-Length"]["x-ms-client-name"] = "FileSize";
      $.headers["Content-Type"]["x-ms-client-path"] = "HttpHeaders.ContentType";
      $.headers["Content-Type"]["x-nullable"] = true;
      $.headers["Content-Encoding"]["x-ms-client-path"] = "HttpHeaders.ContentEncoding";
      $.headers["Content-Encoding"]["x-nullable"] = true;
      $.headers["Cache-Control"]["x-ms-client-path"] = "HttpHeaders.CacheControl";
      $.headers["Cache-Control"]["x-nullable"] = true;
      $.headers["Content-Disposition"]["x-ms-client-path"] = "HttpHeaders.ContentDisposition";
      $.headers["Content-Disposition"]["x-nullable"] = true;
      $.headers["Content-Language"]["x-ms-client-path"] = "HttpHeaders.ContentLanguage";
      $.headers["Content-Language"]["x-nullable"] = true;
      $.headers["Content-MD5"]["x-ms-client-path"] = "HttpHeaders.ContentHash";
      $.headers["Content-MD5"]["x-nullable"] = true;
      $.headers["x-ms-copy-completion-time"]["x-ms-client-name"] = "CopyCompletedOn";
      $.headers["x-ms-copy-completion-time"]["x-nullable"] = true;
      $.headers["x-ms-copy-status-description"]["x-nullable"] = true;
      $.headers["x-ms-copy-id"]["x-nullable"] = true;
      $.headers["x-ms-copy-progress"]["x-nullable"] = true;
      $.headers["x-ms-copy-source"]["x-nullable"] = true;
      $.headers["x-ms-copy-status"]["x-nullable"] = true;
      $.headers["x-ms-lease-duration"]["x-nullable"] = true;
      $.headers["x-ms-lease-state"]["x-nullable"] = true;
      $.headers["x-ms-lease-status"]["x-nullable"] = true;
      delete $.headers["x-ms-type"];
      $.schema = {
        "type": "object",
        "x-ms-client-name": "FileProperties",
        "x-ms-sealed": false,
        "properties": {
          "SmbProperties": {"$ref": "#/definitions/FileSmbProperties", "x-ms-xml": {"name": ""}},
          "HttpHeaders": {"$ref": "#/definitions/FileHttpHeaders", "x-ms-xml": {"name": ""}}
        }
      };
```

### SetFileProperties

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}?comp=properties"].put.responses["200"]
    transform: >
      $.headers["x-ms-file-permission-key"]["x-ms-client-path"] = "SmbProperties.PermissionKey";
      $.headers["x-ms-file-attributes"]["x-ms-client-path"] = "SmbProperties.Attributes";
      $.headers["x-ms-file-creation-time"]["x-ms-client-path"] = "SmbProperties.CreatedOn";
      $.headers["x-ms-file-last-write-time"]["x-ms-client-path"] = "SmbProperties.LastWrittenOn";
      $.headers["x-ms-file-change-time"]["x-ms-client-path"] = "SmbProperties.ChangedOn";
      $.headers["x-ms-file-id"]["x-ms-client-path"] = "SmbProperties.FileId";
      $.headers["x-ms-file-parent-id"]["x-ms-client-path"] = "SmbProperties.ParentFileId";
      $.schema = {
        "type": "object",
        "x-ms-client-name": "SetFilePropertiesResult",
        "x-ms-sealed": false,
        "properties": {
          "SmbProperties": {"$ref": "#/definitions/FileSmbProperties", "x-ms-xml": {"name": ""}}
        }
      };
```

### DownloadFile

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.DownloadFileDetails = {
        "type": "object",
        "required":  ["ETag", "LastModified", "IsServerEncrypted", "SmbProperties"],
        "properties": {
          "ETag": {"type": "string", "format": "etag", "description": "The ETag contains a value that you can use to perform operations conditionally. If the request version is 2011-08-18 or newer, the ETag value will be in quotes."},
          "LastModified": {"type": "string", "format": "date-time-rfc1123", "description": "Returns the date and time the file was last modified. Any operation that modifies the file, including an update of the file's metadata or properties, changes the last-modified time of the file."},
          "Metadata": {"$ref": "#/definitions/Metadata"},
          "CopyId": {"type": "string", "description": " String identifier for this copy operation. Use with Get File Properties to check the status of this copy operation, or pass to Abort Copy File to abort a pending copy."},
          "CopySource": {"type": "string", "description": "URL up to 2 KB in length that specifies the source file or file used in the last attempted Copy File operation where this file was the destination file. This header does not appear if this file has never been the destination in a Copy File operation, or if this file has been modified after a concluded Copy File operation using Set File Properties, Put File, or Put Block List."},
          "CopyStatus": {"$ref": "#/definitions/CopyStatus"},
          "CopyStatusDescription": {"type": "string", "description": "Only appears when x-ms-copy-status is failed or pending. Describes the cause of the last fatal or non-fatal copy operation failure. This header does not appear if this file has never been the destination in a Copy File operation, or if this file has been modified after a concluded Copy File operation using Set File Properties, Put File, or Put Block List"},
          "CopyProgress": {"type": "string", "description": "Contains the number of bytes copied and the total bytes in the source in the last attempted Copy File operation where this file was the destination file. Can show between 0 and Content-Length bytes copied. This header does not appear if this file has never been the destination in a Copy File operation, or if this file has been modified after a concluded Copy File operation using Set File Properties, Put File, or Put Block List"},
          "CopyCompletedOn": {"type": "string", "format": "date-time-rfc1123", "description": "Conclusion time of the last attempted Copy File operation where this file was the destination file. This value can specify the time of a completed, aborted, or failed copy attempt. This header does not appear if a copy is pending, if this file has never been the destination in a Copy File operation, or if this file has been modified after a concluded Copy File operation using Set File Properties, Put File, or Put Block List."},
          "IsServerEncrypted": {"type": "boolean", "description": "True if the file data and metadata are completely encrypted using the specified algorithm. Otherwise, the value is set to false (when the file is unencrypted, or if only parts of the file/application metadata are encrypted)."},
          "SmbProperties": {"$ref": "#/definitions/FileSmbProperties", "x-ms-xml": {"name": ""}},
          "LeaseDuration": {"$ref": "#/definitions/LeaseDuration"},
          "LeaseState": {"$ref": "#/definitions/LeaseState"},
          "LeaseStatus": {"$ref": "#/definitions/LeaseStatus"}
        }
      };
      $.DownloadFileResult = {
        "type": "object",
        "x-ms-sealed": false,
        "properties": {
          "BodyStream": {"type": "object", "format": "file", "description": "Content of the file or file range."},
          "ContentRange": {"$ref": "#/definitions/ContentRange", "x-ms-xml": {"name": ""}, "description": "Indicates the range of bytes returned."},
          "FileSize": {"type": "integer", "format": "int64", "x-ms-xml": {"name": ""}},
          "TransactionalContentHash": {"$ref": "#/definitions/ContentHash", "x-nullable": true, "x-ms-xml": {"name": ""}, "description": "MD5 hash for the downloaded range of data."},
          "HttpHeaders": {"$ref": "#/definitions/FileHttpHeaders", "x-ms-xml": {"name": ""}},
          "Details": {"$ref": "#/definitions/DownloadFileDetails", "x-ms-xml": {"name": ""}}
        }
      };
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}"].get.responses
    transform: >
      for (const status_code of ["200", "206"]) {
        $[status_code].headers["ETag"]["x-ms-client-path"] = "Details.ETag";
        $[status_code].headers["Last-Modified"]["x-ms-client-path"] = "Details.LastModified";
        $[status_code].headers["x-ms-meta"]["x-ms-client-path"] = "Details.Metadata";
        $[status_code].headers["Content-Type"]["x-ms-client-path"] = "HttpHeaders.ContentType";
        $[status_code].headers["Content-Type"]["x-nullable"] = true;
        $[status_code].headers["Content-Encoding"]["x-ms-client-path"] = "HttpHeaders.ContentEncoding";
        $[status_code].headers["Content-Encoding"]["x-nullable"] = true;
        $[status_code].headers["Cache-Control"]["x-ms-client-path"] = "HttpHeaders.CacheControl";
        $[status_code].headers["Cache-Control"]["x-nullable"] = true;
        $[status_code].headers["Content-Disposition"]["x-ms-client-path"] = "HttpHeaders.ContentDisposition";
        $[status_code].headers["Content-Disposition"]["x-nullable"] = true;
        $[status_code].headers["Content-Language"]["x-ms-client-path"] = "HttpHeaders.ContentLanguage";
        $[status_code].headers["Content-Language"]["x-nullable"] = true;
        $[status_code].headers["x-ms-copy-completion-time"]["x-ms-client-path"] = "Details.CopyCompletedOn";
        $[status_code].headers["x-ms-copy-status-description"]["x-ms-client-path"] = "Details.CopyStatusDescription";
        $[status_code].headers["x-ms-copy-id"]["x-ms-client-path"] = "Details.CopyId";
        $[status_code].headers["x-ms-copy-progress"]["x-ms-client-path"] = "Details.CopyProgress";
        $[status_code].headers["x-ms-copy-source"]["x-ms-client-path"] = "Details.CopySource";
        $[status_code].headers["x-ms-copy-status"]["x-ms-client-path"] = "Details.CopyStatus";
        $[status_code].headers["x-ms-lease-duration"]["x-ms-client-path"] = "Details.LeaseDuration";
        $[status_code].headers["x-ms-lease-state"]["x-ms-client-path"] = "Details.LeaseState";
        $[status_code].headers["x-ms-lease-status"]["x-ms-client-path"] = "Details.LeaseStatus";
        $[status_code].headers["x-ms-server-encrypted"]["x-ms-client-path"] = "Details.IsServerEncrypted";
        $[status_code].headers["x-ms-file-permission-key"]["x-ms-client-path"] = "Details.SmbProperties.PermissionKey";
        $[status_code].headers["x-ms-file-attributes"]["x-ms-client-path"] = "Details.SmbProperties.Attributes";
        $[status_code].headers["x-ms-file-creation-time"]["x-ms-client-path"] = "Details.SmbProperties.CreatedOn";
        $[status_code].headers["x-ms-file-last-write-time"]["x-ms-client-path"] = "Details.SmbProperties.LastWrittenOn";
        $[status_code].headers["x-ms-file-change-time"]["x-ms-client-path"] = "Details.SmbProperties.ChangedOn";
        $[status_code].headers["x-ms-file-id"]["x-ms-client-path"] = "Details.SmbProperties.FileId";
        $[status_code].headers["x-ms-file-parent-id"]["x-ms-client-path"] = "Details.SmbProperties.ParentFileId";
        delete $[status_code].headers["Accept-Ranges"];
        delete $[status_code].headers["Content-Length"];
        delete $[status_code].headers["Content-Range"];
        $[status_code].schema = {"$ref": "#/definitions/DownloadFileResult"};
      }
      $["200"].headers["Content-MD5"] = {"type": "string", "format": "byte", "x-ms-client-name": "TransactionalContentHash", "x-ms-client-path": "HttpHeaders.ContentHash", "x-nullable": true};
      $["206"].headers["Content-MD5"] = {"type": "string", "format": "byte", "x-ms-client-name": "TransactionalContentHash", "x-nullable": true};
      $["200"].headers["x-ms-content-md5"] = {"type": "string", "format": "byte", "x-ms-client-path": "HttpHeaders.ContentHash", "x-nullable": true};
      $["206"].headers["x-ms-content-md5"] = $["200"].headers["x-ms-content-md5"];
```

### DeleteFile

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}"].delete.responses["202"]
    transform: >
      $.schema = {
        "type": "object",
        "x-ms-client-name": "DeleteFileResult",
        "x-ms-sealed": false,
        "properties": {
          "Deleted": {"type": "boolean", "x-ms-client-default": true, "x-ms-xml": {"name": ""}, "description": "Indicates if the file was successfully deleted by this operation."}
        }
      };
```

### UploadFileRange

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}?comp=range"].put.parameters
    transform: >
      for (const p of $) {
        if (p["x-ms-client-name"] && p["x-ms-client-name"] === "FileRangeWrite") {
          delete p["x-ms-enum"];
          delete p["enum"];
          delete p["default"];
        }
      }
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}?comp=range"].put.responses["201"].headers
    transform: >
      $["Content-MD5"]["x-ms-client-name"] = "TransactionalContentHash";
      $["Content-MD5"]["x-ms-client-default"] = "";
      $["Content-MD5"]["x-nullable"] = true;
      $["x-ms-request-server-encrypted"]["x-ms-client-default"] = false;
      $["x-ms-request-server-encrypted"]["x-nullable"] = true;
      delete $["x-ms-file-last-write-time"];
```

### UploadFileRangeFromUri

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}?comp=range&fromURL"].put.responses["201"].headers
    transform: >
      $["x-ms-content-crc64"]["x-ms-client-name"] = "TransactionalContentHash";
      $["x-ms-content-crc64"]["x-ms-client-default"] = "";
      $["x-ms-content-crc64"]["x-nullable"] = true;
      $["x-ms-request-server-encrypted"]["x-ms-client-default"] = false;
      $["x-ms-request-server-encrypted"]["x-nullable"] = true;
      delete $["x-ms-file-last-write-time"];
```

### GetFileRangeList

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}?comp=rangelist"].get.responses["200"]
    transform: >
      $.headers["x-ms-content-length"]["x-ms-client-name"] = "FileSize";
      $.schema = {
      "x-ms-client-name": "GetFileRangeListResult",
        "x-ms-sealed": false,
        "xml": {"name": "Ranges"},
        "type": "object",
        "properties": {
          "Range": {
            "type": "array",
            "x-ms-client-name": "Ranges",
            "x-ms-xml": {"name": "."},
            "items": {"$ref": "#/definitions/Range"}
          },
          "ClearRange": {
            "type": "array",
            "x-ms-client-name": "ClearRanges",
            "x-ms-xml": {"name": "."},
            "items": {"$ref": "#/definitions/ClearRange"}
          }
        }
      };
```

### BreakFileLease

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}?comp=lease&break"].put.responses["202"].headers
    transform: >
      delete $["x-ms-lease-id"];
```

### BreakShareLease

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}?restype=share&comp=lease&break"].put.responses["202"].headers
    transform: >
      delete $["x-ms-lease-id"];
```

### RenameFile/Directory

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}?comp=rename"].put.responses["200"].headers
    transform: >
      $["x-ms-file-creation-time"].format = "date-time";
      $["x-ms-file-last-write-time"].format = "date-time";
      $["x-ms-file-change-time"].format = "date-time";
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}?restype=directory&comp=rename"].put.responses["200"].headers
    transform: >
      $["x-ms-file-creation-time"].format = "date-time";
      $["x-ms-file-last-write-time"].format = "date-time";
      $["x-ms-file-change-time"].format = "date-time";
```
