# Azure Storage C++ Protocol Layer

> see https://aka.ms/autorest

## Configuration

```yaml
package-name: azure-storage-files-shares
namespace: Azure::Storage::Files::Shares
output-folder: generated
clear-output-folder: true
input-file: https://raw.githubusercontent.com/Azure/azure-rest-api-specs/refs/heads/main/specification/storage/data-plane/Microsoft.FileStorage/stable/2026-04-06/file.json
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

### Delete Unused Query Parameters and Headers and global changes for Parameters and Headers

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
        "enum": ["2026-04-06"]
      };
  - from: swagger-document
    where: $.parameters
    transform: >
      $.ApiVersionParameter.enum = ["2026-04-06"];
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
        "Service_SetProperties": "SetServicePropertiesResult",
        "Directory_SetMetadata": "SetDirectoryMetadataResult",
        "File_SetMetadata": "SetFileMetadataResult",
        "File_UploadRange":"UploadFileRangeResult",
        "File_UploadRangeFromUri":"UploadFileRangeFromUriResult",
        "File_AbortCopy":"AbortFileCopyResult",
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
      $.AccessRight["x-namespace"] = "_detail";
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
        "File_CreateSymbolicLink",
        "File_GetSymbolicLink",
        "File_CreateHardLink",
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
      $.KeyInfo["x-namespace"] = "_detail";
```

### Global Changes for Definitions, Types etc.

```yaml
directive:
  - from: swagger-document
    where: $.parameters
    transform: >
      $.ListSharesInclude.items["x-ms-enum"].name = "ListSharesIncludeFlags";
      $.ListFilesInclude.items["x-ms-enum"].name = "ListFilesIncludeFlags";
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
      $.FileRequestIntent["x-ms-enum"]["values"] = [{"value": "__placeHolder", "name": "__placeHolder"}, {"value": "backup", "name": "Backup"}];
      $.FilePermissionFormat["enum"] = ["sddl", "binary"];
      $.FileAttributes["required"] = true;
      delete $.EnableSmbDirectoryLease;
      delete $.FilePropertySemantics;
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
        }
      };
      $.FileSmbProperties = {
        "type": "object",
        "properties": {
          "PermissionKey": {"type": "string", "x-nullable": true},
          "Attributes": {"$ref": "#/definitions/FileAttributes"},
          "CreatedOn": {"type": "string", "format": "date-time", "x-nullable": true},
          "LastWrittenOn": {"type": "string", "format": "date-time", "x-nullable": true},
          "ChangedOn": {"type": "string", "format": "date-time", "x-nullable": true},
          "FileId": {"type": "string"},
          "ParentFileId": {"type": "string"}
        }
      };
      $.AccessTier = {
        "type": "string",
        "enum": ["TransactionOptimized", "Hot", "Cool", "Premium"],
        "x-ms-enum": {
          "name": "AccessTier",
          "modelAsString": false
        }
      };
      $.Metadata = {"type": "object", "x-ms-format": "caseinsensitivemap", properties: {"__placeHolder" : {"type": "integer"}}};
      $.CopyStatus = {
        "type": "string",
        "enum": ["pending", "success", "aborted", "failed"],
        "x-ms-enum": {
          "name": "CopyStatus",
          "modelAsString": false
        }
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
          "Content-Type": {"type": "string"},
          "Content-Encoding": {"type": "string"},
          "Content-Language": {"type": "string"},
          "Content-Hash": {"$ref": "#/definitions/ContentHash", "x-ms-xml": {"name": "."}},
          "Content-Disposition": {"type": "string"},
          "Cache-Control": {"type": "string"}
        }
      };
      $.SharePermission["x-namespace"] = "_detail";
      $.SharePermission["properties"]["format"]["enum"] = ["sddl", "binary"];
      $.ShareEnabledProtocols["enum"] = ["Smb", "Nfs"];
      $.ShareEnabledProtocols["x-ms-enum"] = {"name": "ShareProtocols", "modelAsString": false};
      $.ShareEnabledProtocols["x-ms-enum"]["values"] = [{"value": "SMB", "name": "Smb"},{"value": "NFS", "name": "Nfs"}];
      $.StringEncoded["x-namespace"] = "_detail";
      delete $.StringEncoded.properties["content"]["xml"];
      $.StringEncoded["xml"] = {"name": "Name"};
      $.StringEncoded.properties["content"]["x-ms-xml"] = {"name": "."};
      $.SmbEncryptionInTransit = {
          "description": "Enable or disable encryption in transit.",
          "type": "object",
          "properties": {
            "Required": {
              "description": "If encryption in transit is required",
              "type": "boolean"
          }
        }
      };
      $.NfsEncryptionInTransit = {
          "description": "Enable or disable encryption in transit.",
          "type": "object",
          "properties": {
            "Required": {
              "description": "If encryption in transit is required",
              "type": "boolean"
          }
        }
      };
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
        if (header === "x-ms-structured-body" || header === "x-ms-structured-content-length") {
          $[header]["x-nullable"] = true;
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
      $.SmbSettings.properties["Multichannel"]["x-nullable"] = true;
      delete $.SmbSettings.properties["EncryptionInTransit"];
      $.SmbSettings.properties["EncryptionInTransit"] = { "$ref": "#/definitions/SmbEncryptionInTransit" };
      $.SmbSettings.properties["EncryptionInTransit"]["x-ms-client-name"] = "EncryptionInTransit";
      $.SmbSettings.properties["EncryptionInTransit"]["x-nullable"] = true;
      $.ShareProtocolSettings.properties["Smb"]["$ref"] = "#/definitions/SmbSettings";
      $.ShareProtocolSettings.properties["Settings"] = $.ShareProtocolSettings.properties["Smb"];
      $.ShareProtocolSettings.properties["Settings"]["x-ms-xml"] = { "name": "SMB" };
      $.ShareProtocolSettings.properties["Settings"]["x-nullable"] = true;
      delete $.ShareProtocolSettings.properties["Smb"];
      $.NfsSettings = $.ShareNfsSettings;
      delete $.ShareNfsSettings;
      delete $.NfsSettings.properties["EncryptionInTransit"];
      $.NfsSettings.properties["EncryptionInTransit"] = { "$ref": "#/definitions/NfsEncryptionInTransit" };
      $.NfsSettings.properties["EncryptionInTransit"]["x-ms-client-name"] = "EncryptionInTransit";
      $.NfsSettings.properties["EncryptionInTransit"]["x-nullable"] = true;
      $.ShareProtocolSettings.properties["Nfs"]["$ref"] = "#/definitions/NfsSettings";
      $.ShareProtocolSettings.properties["NfsSettings"] = $.ShareProtocolSettings.properties["Nfs"];
      $.ShareProtocolSettings.properties["NfsSettings"]["x-ms-xml"] = { "name": "NFS" };
      $.ShareProtocolSettings.properties["NfsSettings"]["x-nullable"] = true;
      delete $.ShareProtocolSettings.properties["Nfs"];
      $.ProtocolSettings = $.ShareProtocolSettings;
      delete $.ShareProtocolSettings;
      $.StorageServiceProperties.properties["Protocol"]["$ref"] = "#/definitions/ProtocolSettings";
      $.StorageServiceProperties.properties["Protocol"]["x-ms-xml"] = { "name": "ProtocolSettings" };
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
      $.headers["ETag"]["x-ms-client-default"] = "";
      $.headers["ETag"]["x-nullable"] = true;
      $.headers["Last-Modified"]["x-ms-client-default"] = "";
      $.headers["Last-Modified"]["x-nullable"] = true;
      $.schema = {
        "type": "object",
        "x-ms-sealed": false,
        "x-ms-client-name": "ShareStatistics",
        "xml": {"name": "ShareStats"},
        "properties": {
          "ShareUsageBytes": {
            "type": "integer",
            "format": "int64",
            "x-ms-client-name": "ShareUsageInBytes"
          }
        }
      };
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
      delete $.ShareItemDetails.properties["EnableSmbDirectoryLease"];
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
      $.headers["x-ms-share-quota"]["x-nullable"] = true;
      $.headers["x-ms-share-provisioned-iops"]["x-nullable"] = true;
      $.headers["x-ms-share-provisioned-bandwidth-mibps"]["x-nullable"] = true;
      $.headers["x-ms-share-included-burst-iops"]["x-nullable"] = true;
      $.headers["x-ms-share-max-burst-credits-for-iops"]["x-nullable"] = true;
      $.schema = {
        "type": "object",
        "x-ms-client-name": "CreateShareResult",
        "x-ms-sealed": false,
        "properties": {
          "Created": {"type": "boolean", "x-ms-client-default": true, "x-ms-xml": {"name": ""}}
        }
      };
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}?restype=share"].put.parameters
    transform: >
      $ = $.filter(p => !p["$ref"] || !p["$ref"].endsWith("#/parameters/EnableSmbDirectoryLease"));
```

### SetShareProperties

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}?restype=share&comp=properties"].put.responses["200"]
    transform: >
      $.headers["x-ms-share-quota"]["x-nullable"] = true;
      $.headers["x-ms-share-provisioned-iops"]["x-nullable"] = true;
      $.headers["x-ms-share-provisioned-bandwidth-mibps"]["x-nullable"] = true;
      $.headers["x-ms-share-included-burst-iops"]["x-nullable"] = true;
      $.headers["x-ms-share-max-burst-credits-for-iops"]["x-nullable"] = true;
      $.headers["x-ms-share-next-allowed-quota-downgrade-time"]["x-nullable"] = true;
      $.headers["x-ms-share-next-allowed-provisioned-iops-downgrade-time"]["x-nullable"] = true;
      $.headers["x-ms-share-next-allowed-provisioned-bandwidth-downgrade-time"]["x-nullable"] = true;
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}?restype=share&comp=properties"].put.parameters
    transform: >
      $ = $.filter(p => !p["$ref"] || !p["$ref"].endsWith("#/parameters/EnableSmbDirectoryLease"));
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
      $["x-ms-share-provisioned-bandwidth-mibps"]["x-nullable"] = true;
      $["x-ms-share-provisioned-bandwidth-mibps"]["x-ms-client-name"] = "ProvisionedBandwidthMBps";
      $["x-ms-enabled-protocols"]["x-nullable"] = true;
      $["x-ms-root-squash"]["x-nullable"] = true;
      $["x-ms-enabled-protocols"]["enum"] = ["Smb", "Nfs"];
      $["x-ms-enabled-protocols"]["x-ms-enum"] = {"name": "ShareProtocols", "modelAsString": false};
      $["x-ms-enabled-protocols"]["x-ms-enum"]["values"] = [{"value": "SMB", "name": "Smb"},{"value": "NFS", "name": "Nfs"}];
      $["x-ms-enable-snapshot-virtual-directory-access"]["x-nullable"] = true;
      $["x-ms-share-paid-bursting-enabled"]["x-nullable"] = true;
      $["x-ms-share-paid-bursting-max-iops"]["x-nullable"] = true;
      $["x-ms-share-paid-bursting-max-bandwidth-mibps"]["x-nullable"] = true;
      $["x-ms-share-included-burst-iops"]["x-nullable"] = true;
      $["x-ms-share-max-burst-credits-for-iops"]["x-nullable"] = true;
      $["x-ms-share-next-allowed-provisioned-iops-downgrade-time"]["x-nullable"] = true;
      $["x-ms-share-next-allowed-provisioned-bandwidth-downgrade-time"]["x-nullable"] = true;
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}?restype=share"].get.responses["200"]
    transform: >
      delete $.headers["x-ms-enable-smb-directory-lease"];
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
      $.headers["x-ms-file-share-usage-bytes"]["x-ms-client-name"] = "ShareUsageBytes";
      $.headers["x-ms-file-share-usage-bytes"]["x-nullable"] = true;
      $.headers["x-ms-file-share-snapshot-usage-bytes"]["x-ms-client-name"] = "ShareSnapshotUsageBytes";
      $.headers["x-ms-file-share-snapshot-usage-bytes"]["x-nullable"] = true;
      $.schema = {
        "type": "object",
        "x-ms-client-name": "DeleteShareResult",
        "x-ms-sealed": false,
        "properties": {
          "Deleted": {"type": "boolean", "x-ms-client-default": true, "x-ms-xml": {"name": ""}},
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
      $.FileItem["x-namespace"] = "_detail";

      delete $.DirectoryItem.properties["Properties"];
      delete $.DirectoryItem.properties["FileId"];
      delete $.DirectoryItem.properties["Attributes"];
      delete $.DirectoryItem.properties["PermissionKey"];
      delete $.DirectoryItem.required;
      $.DirectoryItemDetails = JSON.parse(JSON.stringify($.FileItemDetails));
      delete $.DirectoryItemDetails.properties["Content-Length"];
      $.DirectoryItem.properties["Details"] = {"$ref": "#/definitions/DirectoryItemDetails", "x-ms-xml" : {"name": "Properties"}};
      $.DirectoryItem["x-namespace"] = "_detail";

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
      $.HandleItem["x-namespace"] = "_detail";
      delete $.ListHandlesResponse.properties.HandleList["xml"];
      $.ListHandlesResponse.properties.HandleList["x-ms-xml"] = {"name": "Entries", "wrapped": true};
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

### CreateDirectory

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}?restype=directory"].put.responses["201"]
    transform: >
      $.headers["x-ms-file-permission-key"]["x-ms-client-path"] = "SmbProperties.PermissionKey";
      $.headers["x-ms-file-attributes"]["x-ms-client-path"] = "SmbProperties.Attributes";
      $.headers["x-ms-file-attributes"]["x-nullable"] = true;
      $.headers["x-ms-file-attributes"]["x-ms-client-default"] = "None";
      $.headers["x-ms-file-creation-time"]["x-ms-client-path"] = "SmbProperties.CreatedOn";
      $.headers["x-ms-file-last-write-time"]["x-ms-client-path"] = "SmbProperties.LastWrittenOn";
      $.headers["x-ms-file-change-time"]["x-ms-client-path"] = "SmbProperties.ChangedOn";
      $.headers["x-ms-file-id"]["x-ms-client-path"] = "SmbProperties.FileId";
      $.headers["x-ms-file-parent-id"]["x-ms-client-path"] = "SmbProperties.ParentFileId";
      $.headers["x-ms-mode"]["x-nullable"] = true;
      $.headers["x-ms-owner"]["x-nullable"] = true;
      $.headers["x-ms-group"]["x-nullable"] = true;
      $.headers["x-ms-file-file-type"]["x-nullable"] = true;
      $.schema = {
        "type": "object",
        "x-ms-client-name": "CreateDirectoryResult",
        "x-ms-sealed": false,
        "properties": {
          "Created": {"type": "boolean", "x-ms-client-default": true, "x-ms-xml": {"name": ""}},
          "SmbProperties": {"$ref": "#/definitions/FileSmbProperties", "x-ms-xml": {"name": ""}}
        },
        "x-namespace" : "_detail"
      };
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}?restype=directory"].put.parameters
    transform: >
      $ = $.filter(p => !p["$ref"] || !p["$ref"].endsWith("#/parameters/FilePropertySemantics"));
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
          "Deleted": {"type": "boolean", "x-ms-client-default": true, "x-ms-xml": {"name": ""}}
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
      $.headers["x-ms-file-attributes"]["x-nullable"] = true;
      $.headers["x-ms-file-attributes"]["x-ms-client-default"] = "None";
      $.headers["x-ms-file-creation-time"]["x-ms-client-path"] = "SmbProperties.CreatedOn";
      $.headers["x-ms-file-last-write-time"]["x-ms-client-path"] = "SmbProperties.LastWrittenOn";
      $.headers["x-ms-file-change-time"]["x-ms-client-path"] = "SmbProperties.ChangedOn";
      $.headers["x-ms-file-id"]["x-ms-client-path"] = "SmbProperties.FileId";
      $.headers["x-ms-file-parent-id"]["x-ms-client-path"] = "SmbProperties.ParentFileId";
      $.headers["x-ms-mode"]["x-nullable"] = true;
      $.headers["x-ms-owner"]["x-nullable"] = true;
      $.headers["x-ms-group"]["x-nullable"] = true;
      $.headers["x-ms-file-file-type"]["x-nullable"] = true;
      $.schema = {
        "type": "object",
        "x-ms-client-name": "DirectoryProperties",
        "x-ms-sealed": false,
        "properties": {
          "SmbProperties": {"$ref": "#/definitions/FileSmbProperties", "x-ms-xml": {"name": ""}}
        },
        "x-namespace" : "_detail"
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
      $.headers["x-ms-file-attributes"]["x-nullable"] = true;
      $.headers["x-ms-file-attributes"]["x-ms-client-default"] = "None";
      $.headers["x-ms-file-creation-time"]["x-ms-client-path"] = "SmbProperties.CreatedOn";
      $.headers["x-ms-file-last-write-time"]["x-ms-client-path"] = "SmbProperties.LastWrittenOn";
      $.headers["x-ms-file-change-time"]["x-ms-client-path"] = "SmbProperties.ChangedOn";
      $.headers["x-ms-file-id"]["x-ms-client-path"] = "SmbProperties.FileId";
      $.headers["x-ms-file-parent-id"]["x-ms-client-path"] = "SmbProperties.ParentFileId";
      $.headers["x-ms-mode"]["x-nullable"] = true;
      $.headers["x-ms-owner"]["x-nullable"] = true;
      $.headers["x-ms-group"]["x-nullable"] = true;
      $.schema = {
        "type": "object",
        "x-ms-client-name": "SetDirectoryPropertiesResult",
        "x-ms-sealed": false,
        "properties": {
          "SmbProperties": {"$ref": "#/definitions/FileSmbProperties", "x-ms-xml": {"name": ""}}
        },
        "x-namespace" : "_detail"
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
      $.headers["x-ms-file-attributes"]["x-nullable"] = true;
      $.headers["x-ms-file-attributes"]["x-ms-client-default"] = "None";
      $.headers["x-ms-file-creation-time"]["x-ms-client-path"] = "SmbProperties.CreatedOn";
      $.headers["x-ms-file-last-write-time"]["x-ms-client-path"] = "SmbProperties.LastWrittenOn";
      $.headers["x-ms-file-change-time"]["x-ms-client-path"] = "SmbProperties.ChangedOn";
      $.headers["x-ms-file-id"]["x-ms-client-path"] = "SmbProperties.FileId";
      $.headers["x-ms-file-parent-id"]["x-ms-client-path"] = "SmbProperties.ParentFileId";
      $.headers["x-ms-mode"]["x-nullable"] = true;
      $.headers["x-ms-owner"]["x-nullable"] = true;
      $.headers["x-ms-group"]["x-nullable"] = true;
      $.headers["x-ms-file-file-type"]["x-nullable"] = true;
      delete $.headers["Content-MD5"];
      delete $.headers["Content-Length"];
      $.schema = {
        "type": "object",
        "x-ms-client-name": "CreateFileResult",
        "x-ms-sealed": false,
        "properties": {
          "Created": {"type": "boolean", "x-ms-client-default": true, "x-ms-xml": {"name": ""}},
          "SmbProperties": {"$ref": "#/definitions/FileSmbProperties", "x-ms-xml": {"name": ""}}
        },
        "x-namespace" : "_detail"
      };
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}"].put.parameters
    transform: >
      $ = $.filter(p => !(p["$ref"] && (p["$ref"].endsWith("#/parameters/ContentMD5") || p["$ref"].endsWith("#/parameters/FilePropertySemantics") || p["$ref"].endsWith("#/parameters/ContentLengthOptional") || p["$ref"].endsWith("#/parameters/OptionalBody"))));
```

### GetFileProperties

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}"].head.responses["200"]
    transform: >
      $.headers["x-ms-file-permission-key"]["x-ms-client-path"] = "SmbProperties.PermissionKey";
      $.headers["x-ms-file-attributes"]["x-ms-client-path"] = "SmbProperties.Attributes";
      $.headers["x-ms-file-attributes"]["x-nullable"] = true;
      $.headers["x-ms-file-attributes"]["x-ms-client-default"] = "None";
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
      $.headers["x-ms-mode"]["x-nullable"] = true;
      $.headers["x-ms-owner"]["x-nullable"] = true;
      $.headers["x-ms-group"]["x-nullable"] = true;
      $.headers["x-ms-file-file-type"]["x-nullable"] = true;
      $.headers["x-ms-link-count"]["x-nullable"] = true;
      delete $.headers["x-ms-type"];
      $.schema = {
        "type": "object",
        "x-ms-client-name": "FileProperties",
        "x-ms-sealed": false,
        "properties": {
          "SmbProperties": {"$ref": "#/definitions/FileSmbProperties", "x-ms-xml": {"name": ""}},
          "HttpHeaders": {"$ref": "#/definitions/FileHttpHeaders", "x-ms-xml": {"name": ""}}
        },
        "x-namespace" : "_detail"
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
      $.headers["x-ms-file-attributes"]["x-nullable"] = true;
      $.headers["x-ms-file-attributes"]["x-ms-client-default"] = "None";
      $.headers["x-ms-file-creation-time"]["x-ms-client-path"] = "SmbProperties.CreatedOn";
      $.headers["x-ms-file-last-write-time"]["x-ms-client-path"] = "SmbProperties.LastWrittenOn";
      $.headers["x-ms-file-change-time"]["x-ms-client-path"] = "SmbProperties.ChangedOn";
      $.headers["x-ms-file-id"]["x-ms-client-path"] = "SmbProperties.FileId";
      $.headers["x-ms-file-parent-id"]["x-ms-client-path"] = "SmbProperties.ParentFileId";
      $.headers["x-ms-mode"]["x-nullable"] = true;
      $.headers["x-ms-owner"]["x-nullable"] = true;
      $.headers["x-ms-group"]["x-nullable"] = true;
      $.headers["x-ms-link-count"]["x-nullable"] = true;
      $.schema = {
        "type": "object",
        "x-ms-client-name": "SetFilePropertiesResult",
        "x-ms-sealed": false,
        "properties": {
          "SmbProperties": {"$ref": "#/definitions/FileSmbProperties", "x-ms-xml": {"name": ""}}
        },
        "x-namespace" : "_detail"
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
          "ETag": {"type": "string", "format": "etag"},
          "LastModified": {"type": "string", "format": "date-time-rfc1123"},
          "Metadata": {"$ref": "#/definitions/Metadata"},
          "CopyId": {"type": "string"},
          "CopySource": {"type": "string"},
          "CopyStatus": {"$ref": "#/definitions/CopyStatus"},
          "CopyStatusDescription": {"type": "string"},
          "CopyProgress": {"type": "string"},
          "CopyCompletedOn": {"type": "string", "format": "date-time-rfc1123"},
          "IsServerEncrypted": {"type": "boolean"},
          "SmbProperties": {"$ref": "#/definitions/FileSmbProperties", "x-ms-xml": {"name": ""}},
          "LeaseDuration": {"$ref": "#/definitions/LeaseDuration"},
          "LeaseState": {"$ref": "#/definitions/LeaseState"},
          "LeaseStatus": {"$ref": "#/definitions/LeaseStatus"},
          "FileMode": {"type": "string", "x-nullable": true},
          "Owner": {"type": "string", "x-nullable": true},
          "Group": {"type": "string", "x-nullable": true},
          "LinkCount": {"type": "integer", "format": "int64", "x-nullable": true},
        },
        "x-namespace" : "_detail"
      };
      $.DownloadFileResult = {
        "type": "object",
        "x-ms-sealed": false,
        "properties": {
          "BodyStream": {"type": "object", "format": "file"},
          "ContentRange": {"$ref": "#/definitions/ContentRange", "x-ms-xml": {"name": ""}},
          "FileSize": {"type": "integer", "format": "int64", "x-ms-xml": {"name": ""}},
          "TransactionalContentHash": {"$ref": "#/definitions/ContentHash", "x-nullable": true, "x-ms-xml": {"name": ""}},
          "HttpHeaders": {"$ref": "#/definitions/FileHttpHeaders", "x-ms-xml": {"name": ""}},
          "Details": {"$ref": "#/definitions/DownloadFileDetails", "x-ms-xml": {"name": ""}}
        },
        "x-namespace" : "_detail"
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
        $[status_code].headers["x-ms-file-attributes"]["x-nullable"] = true;
        $[status_code].headers["x-ms-file-attributes"]["x-ms-client-default"] = "None";
        $[status_code].headers["x-ms-file-creation-time"]["x-ms-client-path"] = "Details.SmbProperties.CreatedOn";
        $[status_code].headers["x-ms-file-last-write-time"]["x-ms-client-path"] = "Details.SmbProperties.LastWrittenOn";
        $[status_code].headers["x-ms-file-change-time"]["x-ms-client-path"] = "Details.SmbProperties.ChangedOn";
        $[status_code].headers["x-ms-file-id"]["x-ms-client-path"] = "Details.SmbProperties.FileId";
        $[status_code].headers["x-ms-file-parent-id"]["x-ms-client-path"] = "Details.SmbProperties.ParentFileId";
        $[status_code].headers["x-ms-mode"]["x-ms-client-path"] = "Details.FileMode";
        $[status_code].headers["x-ms-owner"]["x-ms-client-path"] = "Details.Owner";
        $[status_code].headers["x-ms-group"]["x-ms-client-path"] = "Details.Group";
        $[status_code].headers["x-ms-link-count"]["x-ms-client-path"] = "Details.LinkCount";
        delete $[status_code].headers["Accept-Ranges"];
        delete $[status_code].headers["Content-Length"];
        delete $[status_code].headers["Content-Range"];
        delete $[status_code].headers["x-ms-structured-content-length"];
        delete $[status_code].headers["x-ms-structured-body"];
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
      $.headers["x-ms-link-count"]["x-nullable"] = true;
      $.schema = {
        "type": "object",
        "x-ms-client-name": "DeleteFileResult",
        "x-ms-sealed": false,
        "properties": {
          "Deleted": {"type": "boolean", "x-ms-client-default": true, "x-ms-xml": {"name": ""}}
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
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}?comp=range"].put.responses["201"]
    transform: >
      $.headers["Content-MD5"]["x-ms-client-name"] = "TransactionalContentHash";
      $.headers["Content-MD5"]["x-ms-client-default"] = "";
      $.headers["Content-MD5"]["x-nullable"] = true;
      $.headers["x-ms-request-server-encrypted"]["x-ms-client-default"] = false;
      $.headers["x-ms-request-server-encrypted"]["x-nullable"] = true;
      delete $.headers["x-ms-file-last-write-time"];
```

### UploadFileRangeFromUri

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}?comp=range&fromURL"].put.responses["201"]
    transform: >
      $.headers["x-ms-content-crc64"]["x-ms-client-name"] = "TransactionalContentHash";
      $.headers["x-ms-content-crc64"]["x-ms-client-default"] = "";
      $.headers["x-ms-content-crc64"]["x-nullable"] = true;
      $.headers["x-ms-request-server-encrypted"]["x-ms-client-default"] = false;
      $.headers["x-ms-request-server-encrypted"]["x-nullable"] = true;
      delete $.headers["x-ms-file-last-write-time"];
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

### CreateFileSymbolicLink

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}?restype=symboliclink"].put.responses["201"]
    transform: >
      $.headers["x-ms-file-creation-time"]["x-ms-client-path"] = "SmbProperties.CreatedOn";
      $.headers["x-ms-file-last-write-time"]["x-ms-client-path"] = "SmbProperties.LastWrittenOn";
      $.headers["x-ms-file-change-time"]["x-ms-client-path"] = "SmbProperties.ChangedOn";
      $.headers["x-ms-file-id"]["x-ms-client-path"] = "SmbProperties.FileId";
      $.headers["x-ms-file-parent-id"]["x-ms-client-path"] = "SmbProperties.ParentFileId";
      $.schema = {
        "type": "object",
        "x-ms-client-name": "CreateFileSymbolicLinkResult",
        "x-ms-sealed": false,
        "properties": {
          "Created": {"type": "boolean", "x-ms-client-default": true, "x-ms-xml": {"name": ""}},
          "SmbProperties": {"$ref": "#/definitions/FileSmbProperties", "x-ms-xml": {"name": ""}}
        },
        "x-namespace" : "_detail"
      };
```

### CreateFileHardLink

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}?restype=hardlink"].put.responses["201"]
    transform: >
      $.headers["x-ms-file-creation-time"]["x-ms-client-path"] = "SmbProperties.CreatedOn";
      $.headers["x-ms-file-last-write-time"]["x-ms-client-path"] = "SmbProperties.LastWrittenOn";
      $.headers["x-ms-file-change-time"]["x-ms-client-path"] = "SmbProperties.ChangedOn";
      $.headers["x-ms-file-id"]["x-ms-client-path"] = "SmbProperties.FileId";
      $.headers["x-ms-file-parent-id"]["x-ms-client-path"] = "SmbProperties.ParentFileId";
      $.schema = {
        "type": "object",
        "x-ms-client-name": "CreateFileHardLinkResult",
        "x-ms-sealed": false,
        "properties": {
          "Created": {"type": "boolean", "x-ms-client-default": true, "x-ms-xml": {"name": ""}},
          "SmbProperties": {"$ref": "#/definitions/FileSmbProperties", "x-ms-xml": {"name": ""}}
        },
        "x-namespace" : "_detail"
      };
```

### Description

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.ApiVersion.description = "The version used for the operations to Azure storage services.";
      $.FileAttributes.description = "Attributes for a file or directory.";
      $.FileSmbProperties.properties["PermissionKey"].description = "Permission key for the directory or file.";
      $.FileSmbProperties.properties["Attributes"].description = "Attributes for the file or directory.";
      $.FileSmbProperties.properties["CreatedOn"].description = "Creation time for the file or directory.";
      $.FileSmbProperties.properties["LastWrittenOn"].description = "Last write time for the file or directory.";
      $.FileSmbProperties.properties["ChangedOn"].description = "Changed time for the file or directory.";
      $.FileSmbProperties.properties["FileId"].description = "The fileId of the file.";
      $.FileSmbProperties.properties["ParentFileId"].description = "The parentId of the file.";
      $.FileSmbProperties.description = "The SMB related properties for the file.";
      $.AccessTier.description = "Specifies the access tier of the share.";
      $.Metadata.description = "A set of name-value pairs associated with the share or file.";
      $.CopyStatus.description = "Status of a copy operation.";
      $.FileHttpHeaders.properties["Content-Type"].description = "MIME content type of the file.";
      $.FileHttpHeaders.properties["Content-Encoding"].description = "Specifies which content encodings have been applied to the file.";
      $.FileHttpHeaders.properties["Content-Language"].description = "Specifies the natural languages used by this file.";
      $.FileHttpHeaders.properties["Content-Hash"].description = "Hash of the file content.";
      $.FileHttpHeaders.properties["Content-Disposition"].description = "Conveys additional information about how to process the resource payload, and also can be used to attach additional metadata.";
      $.FileHttpHeaders.properties["Cache-Control"].description = "Specifies directives for caching mechanisms.";
      $.FileHttpHeaders.description = "Standard HTTP properties supported files.";
      $.HandleItem.properties["Path"]["description"] = "File or directory name including full path starting from share root.";
      $.DownloadFileDetails.properties["ETag"].description = "The ETag contains a value that you can use to perform operations conditionally. If the request version is 2011-08-18 or newer, the ETag value will be in quotes.";
      $.DownloadFileDetails.properties["LastModified"].description = "Returns the date and time the file was last modified. Any operation that modifies the file, including an update of the file's metadata or properties, changes the last-modified time of the file.";
      $.DownloadFileDetails.properties["CopyId"].description = "String identifier for this copy operation. Use with Get File Properties to check the status of this copy operation, or pass to Abort Copy File to abort a pending copy.";
      $.DownloadFileDetails.properties["CopySource"].description = "URL up to 2 KB in length that specifies the source file or file used in the last attempted Copy File operation where this file was the destination file. This header does not appear if this file has never been the destination in a Copy File operation, or if this file has been modified after a concluded Copy File operation using Set File Properties, Put File, or Put Block List.";
      $.DownloadFileDetails.properties["CopyStatusDescription"].description = "Only appears when x-ms-copy-status is failed or pending. Describes the cause of the last fatal or non-fatal copy operation failure. This header does not appear if this file has never been the destination in a Copy File operation, or if this file has been modified after a concluded Copy File operation using Set File Properties, Put File, or Put Block List.";
      $.DownloadFileDetails.properties["CopyProgress"].description = "Contains the number of bytes copied and the total bytes in the source in the last attempted Copy File operation where this file was the destination file. Can show between 0 and Content-Length bytes copied. This header does not appear if this file has never been the destination in a Copy File operation, or if this file has been modified after a concluded Copy File operation using Set File Properties, Put File, or Put Block List.";
      $.DownloadFileDetails.properties["CopyCompletedOn"].description = "Conclusion time of the last attempted Copy File operation where this file was the destination file. This value can specify the time of a completed, aborted, or failed copy attempt. This header does not appear if a copy is pending, if this file has never been the destination in a Copy File operation, or if this file has been modified after a concluded Copy File operation using Set File Properties, Put File, or Put Block List.";
      $.DownloadFileDetails.properties["IsServerEncrypted"].description = "True if the file data and metadata are completely encrypted using the specified algorithm. Otherwise, the value is set to false (when the file is unencrypted, or if only parts of the file/application metadata are encrypted).";
      $.DownloadFileDetails.description = "Detailed information of the downloaded file.";
      $.DownloadFileResult.properties["BodyStream"].description = "Content of the file or file range.";
      $.DownloadFileResult.properties["ContentRange"].description = "Indicates the range of bytes returned.";
      $.DownloadFileResult.properties["TransactionalContentHash"].description = "MD5 hash for the downloaded range of data.";
      $.DownloadFileResult.properties["FileSize"].description = "Size of the file in bytes.";
      $.DownloadFileResult.description = "Response type for #Azure::Storage::Files::Shares::ShareFileClient::Download.";
      $.ShareItemDetails.properties["Etag"].description = "The ETag contains a value which represents the version of the share, in quotes.";
      $.ShareItemDetails.properties["Quota"].description = "The Quota for the item.";
      $.ShareItemDetails.properties["ProvisionedIops"].description = "Provisioned Iops";
      $.ShareItemDetails.properties["ProvisionedIngressMBps"].description = "Provisioned Ingress MBps";
      $.ShareItemDetails.properties["ProvisionedEgressMBps"].description = "Provisioned Egress MBps";
      $.ShareItemDetails.properties["ProvisionedBandwidthMiBps"].description = "Provisioned Bandwidth MBps";
      $.ShareItemDetails.properties["NextAllowedQuotaDowngradeTime"].description = "Next allowed Quota Downgrade Time";
      $.ShareItemDetails.properties["DeletedTime"].description = "Time the item was deleted.";
      $.ShareItemDetails.properties["RemainingRetentionDays"].description = "Remaining retention days.";
      $.ShareItemDetails.properties["AccessTierChangeTime"].description = "Indicates the time the access tier was last changed.";
      $.ShareItemDetails.properties["AccessTierTransitionState"].description = "Indicates the access tier transition state.";
      $.ShareItemDetails.properties["EnabledProtocols"].description = "The protocols which have been enabled on the share.";
      $.ShareItemDetails.properties["RootSquash"].description = "Root squash to set on the share.  Only valid for NFS shares.";
      $.ShareItemDetails.properties["Last-Modified"].description = "The date and time the share was last modified.";
      $.ShareItemDetails.properties["EnableSnapshotVirtualDirectoryAccess"].description = "Version 2023-08-03 and newer. Specifies whether the snapshot virtual directory should be accessible at the root of share mount point when NFS is enabled. This header is only returned for shares, not for snapshots.";
      $.ShareItemDetails.properties["PaidBurstingEnabled"].description = "Optional. Boolean. Default if not specified is false. This property enables paid bursting.";
      $.ShareItemDetails.properties["PaidBurstingMaxIops"].description = "Optional. Integer. Default if not specified is the maximum IOPS the file share can support. Current maximum for a file share is 102,400 IOPS.";
      $.ShareItemDetails.properties["PaidBurstingMaxBandwidthMibps"].description = "Optional. Integer. Default if not specified is the maximum throughput the file share can support. Current maximum for a file share is 10,340 MiB/sec.";
      $.ShareItemDetails.properties["IncludedBurstIops"].description = "Return the calculated burst IOPS of the share.";
      $.ShareItemDetails.properties["MaxBurstCreditsForIops"].description = "Return the calculated maximum burst credits. This is not the current burst credit level, but the maximum burst credits the share can have.";
      $.ShareItemDetails.properties["NextAllowedProvisionedIopsDowngradeTime"].description = "Return timestamp for provisioned IOPS following existing rules for provisioned storage GiB.";
      $.ShareItemDetails.properties["NextAllowedProvisionedBandwidthDowngradeTime"].description = "Return timestamp for provisioned throughput following existing rules for provisioned storage GiB.";
      $.ShareItemInternal.properties["Name"].description = "The name of the share.";
      $.ShareItemInternal.properties["Snapshot"].description = "The snapshot of the share.";
      $.ShareItemInternal.properties["Deleted"].description = "True if the share is deleted.";
      $.ShareItemInternal.properties["Version"].description = "The version of the share";
      $.FileItemDetails.properties["LastAccessTime"].description = "The time the file was last accessed.";
      $.FileItemDetails.properties["Last-Modified"].description = "The date and time the file was last modified.";
      $.FileItemDetails.properties["Etag"].description = "The ETag contains a value which represents the version of the file, in quotes.";
      $.DirectoryItemDetails.properties["LastAccessTime"].description = "The time the directory was last accessed.";
      $.DirectoryItemDetails.properties["Last-Modified"].description = "The date and time the directory was last modified.";
      $.DirectoryItemDetails.properties["Etag"].description = "The ETag contains a value which represents the version of the directory, in quotes.";
      $.SetServicePropertiesResult.description = "Response type for #Azure::Storage::Files::Shares::ShareServiceClient::SetProperties.";
      $.SetDirectoryMetadataResult.description = "Response type for #Azure::Storage::Files::Shares::ShareDirectoryClient::SetMetadata.";
      $.SetFileMetadataResult.description = "Response type for #Azure::Storage::Files::Shares::ShareFileClient::SetMetadata.";
      $.UploadFileRangeResult.description = "Response type for #Azure::Storage::Files::Shares::ShareFileClient::UploadRange.";
      $.UploadFileRangeFromUriResult.description = "Response type for #Azure::Storage::Files::Shares::ShareFileClient::UploadRangeFromUri.";
      $.AbortFileCopyResult.description = "Response type for #Azure::Storage::Files::Shares::ShareFileClient::AbortCopy.";
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}?restype=share&comp=stats"].get.responses["200"]
    transform: >
      $.schema.properties["ShareUsageBytes"].description = "The approximate size of the data stored in bytes. Note that this value may not include all recently created or recently resized files.";
      $.schema.description = "Stats for the share.";
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}?restype=share"].put.responses["201"]
    transform: >
      $.schema.properties["Created"].description = "Indicates if the share was successfully created by this operation.";
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}?restype=share"].get.responses["200"].headers
    transform: >
      $["x-ms-access-tier-transition-state"].description = "Returns the transition state between access tiers, when present.";
      $["x-ms-share-provisioned-iops"].description = "Returns the current share provisioned IOPS.";
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}?restype=share"].delete.responses["202"]
    transform: >
      $.schema.properties["Deleted"].description = "Indicates if the share was successfully deleted by this operation.";
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}?restype=directory"].put.responses["201"]
    transform: >
      $.schema.properties["Created"].description = "Indicates if the directory was successfully created by this operation.";
      $.schema.description = "Response type for #Azure::Storage::Files::Shares::ShareDirectoryClient::Create.";
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}?restype=directory"].delete.responses["202"]
    transform: >
      $.schema.properties["Deleted"].description = "Indicates if the directory was successfully deleted by this operation.";
      $.schema.description = "Response type for #Azure::Storage::Files::Shares::ShareDirectoryClient::Delete.";
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}?restype=directory"].get.responses["200"]
    transform: >
      $.schema.description = "Response type for #Azure::Storage::Files::Shares::ShareDirectoryClient::GetProperties.";
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}?restype=directory&comp=properties"].put.responses["200"]
    transform: >
      $.schema.description = "Response type for #Azure::Storage::Files::Shares::ShareDirectoryClient::SetProperties.";
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}"].put.responses["201"]
    transform: >
      $.schema.properties["Created"].description = "Indicates if the file was successfully created by this operation.";
      $.schema.description = "Response type for #Azure::Storage::Files::Shares::ShareFileClient::Create.";
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}"].delete.responses["202"]
    transform: >
      $.schema.properties["Deleted"].description = "Indicates if the file was successfully deleted by this operation.";
      $.schema.description = "Response type for #Azure::Storage::Files::Shares::ShareFileClient::Delete.";
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}"].head.responses["200"]
    transform: >
      $.schema.description = "Response type for #Azure::Storage::Files::Shares::ShareFileClient::GetProperties.";
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}?comp=properties"].put.responses["200"]
    transform: >
      $.schema.description = "Response type for #Azure::Storage::Files::Shares::ShareFileClient::SetProperties.";
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}/{directory}/{fileName}?comp=rangelist"].get.responses["200"]
    transform: >
      $.schema.description = "Response type for #Azure::Storage::Files::Shares::ShareFileClient::GetRangeList.";
  - from: swagger-document
    where: $["x-ms-paths"]["/{shareName}?restype=share&comp=properties"].put.responses["200"]
    transform: >
      $.headers["x-ms-share-provisioned-iops"].description = "Returns the current share provisioned IOPS.";
```
