# Azure Storage C++ Protocol Layer

> see https://aka.ms/autorest

## Configuration

```yaml
package-name: azure-storage-files-datalake
namespace: Azure::Storage::Files::DataLake
output-folder: generated
clear-output-folder: true
input-file: https://raw.githubusercontent.com/Azure/azure-rest-api-specs/main/specification/storage/data-plane/Azure.Storage.Files.DataLake/preview/2021-06-08/DataLakeStorage.json
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
      delete $["/"];
      delete $["/{filesystem}"].put;
      delete $["/{filesystem}"].patch;
      delete $["/{filesystem}"].head;
      delete $["/{filesystem}"].delete;
      delete $["/{filesystem}/{path}"].post;
      delete $["/{filesystem}/{path}"].get;
      delete $["/{filesystem}/{path}"].patch;
      delete $["/{filesystem}/{path}?comp=expiry"];
      delete $["/{filesystem}?restype=container&comp=list&hierarchy"];
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
      $["/{filesystem}/{path}?action=setAccessControl"].patch.operationId = "Path_SetAccessControlList";
      $["/{filesystem}/{path}?action=setAccessControlRecursive"].patch.operationId = "Path_SetAccessControlListRecursive";
      $["/{filesystem}/{path}?action=flush"].patch.operationId = "File_Flush";
      $["/{filesystem}/{path}?action=append"].patch.operationId = "File_Append";
      for (const operation in $) {
        for (const verb in $[operation]) {
          if ($[operation][verb].operationId && $[operation][verb].operationId.startsWith("Container_")) {
            $[operation][verb].operationId = "Blob" + $[operation][verb].operationId;
          }
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
        "Path_Undelete",
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
    where: $["x-ms-paths"].*.*.responses.*.headers
    transform: >
      for (const h in $) {
        if (h === "x-ms-encryption-key-sha256") {
          $[h]["format"] = "byte";
        }
      }
  - from: swagger-document
    where: $.parameters
    transform: >
      $.Continuation["x-ms-client-name"] = "ContinuationToken";
      $.EncryptionKeySha256["format"] = "byte";
      delete $.EncryptionAlgorithm["enum"];
      delete $.EncryptionAlgorithm["x-ms-enum"];
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
           "Value": {"type": "string", "format": "byte"},
           "Algorithm": {"$ref": "#/definitions/HashAlgorithm"}
        }
      };
      $.PublicAccessType = {
        "x-ms-export": true,
        "type": "string",
        "enum": ["fileSystem", "path", "none"],
        "x-ms-enum": {
          "values": [
            {"name": "none", "value": ""},
            {"name": "fileSystem", "value": "container"},
            {"name": "path", "value": "blob"}
          ],
          "name": "PublicAccessType",
          "modelAsString": true
        }
      };
```

### ListPaths

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.Path["x-ms-client-name"] = "PathItem";
      $.Path["x-namespace"] = "_detail";
      $.Path.properties["lastModified"]["format"] = "date-time-rfc1123";
      $.Path.properties["contentLength"]["x-ms-client-name"] = "FileSize";
      $.Path.properties["isDirectory"]["x-ms-client-default"] = false;
      $.Path.properties["EncryptionScope"]["x-nullable"] = true;
      $.Path.properties["creationTime"] = {"type": "string", "x-ms-client-name": "CreatedOn", "x-nullable": true};
      $.Path.properties["expiryTime"] = {"type": "string", "x-ms-client-name": "ExpiresOn", "x-nullable": true};
      $.Path.properties["etag"] = {"type": "string", "x-ms-format": "string", "x-ms-client-default": "", "x-ms-client-name": "ETag"};
      delete $.Path.properties["eTag"];
      $.PathList["x-namespace"] = "_detail";
      $.PathList["x-ms-sealed"] = false;
  - from: swagger-document
    where: $["x-ms-paths"]["/{filesystem}?resource=filesystem"].get.responses["200"].headers
    transform: >
      $["x-ms-continuation"]["x-ms-client-name"] = "ContinuationToken";
      $["x-ms-continuation"]["x-nullable"] = true;
      delete $["ETag"];
      delete $["Last-Modified"];
```

### CreatePath

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{filesystem}/{path}"].put.responses
    transform: >
      $["201"].headers["Content-Length"]["x-ms-client-name"] = "FileSize";
      $["201"].headers["Content-Length"]["x-nullable"] = true;
      $["201"].headers["x-ms-request-server-encrypted"]["x-nullable"] = true;
      $["201"].headers["x-ms-request-server-encrypted"]["x-ms-client-default"] = "bool()";
      $["201"].headers["x-ms-encryption-key-sha256"]["x-nullable"] = true;
      delete $["201"].headers["x-ms-continuation"];
      $["201"].schema = {
        "type": "object",
        "x-ms-client-name": "CreatePathResult",
        "x-ms-sealed": false,
        "properties": {
          "Created": {"type": "boolean", "x-ms-client-default": true, "x-ms-json": "", "description": "Indicates if the file or directory was successfully created by this operation."}
        }
      };
```


### SetExpiry

```yaml
directive:
  - from: swagger-document
    where: $.parameters
    transform: >
      delete $["PathExpiryOptions"];
      delete $.PathExpiryOptionsOptional["enum"];
      delete $.PathExpiryOptionsOptional["x-ms-enum"];

```

### DeletePath

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{filesystem}/{path}"].delete.responses
    transform: >
      delete $["200"].headers["x-ms-continuation"];
      delete $["200"].headers["x-ms-deletion-id"];
      $["200"].schema = {
        "type": "object",
        "x-ms-client-name": "DeletePathResult",
        "x-ms-sealed": false,
        "properties": {
          "Deleted": {"type": "boolean", "x-ms-client-default": true, "x-ms-json": "", "description": "Indicates if the file or directory was successfully deleted by this operation."}
        }
      };
```

### RenamePath

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.PathRenameMode = {
        "x-namespace": "_detail",
        "x-ms-export": true,
        "type": "string",
        "enum": ["legacy", "posix"],
        "x-ms-enum":  {
          "name": "PathRenameMode",
          "modelAsString": false
        }
      };
  - from: swagger-document
    where: $["x-ms-paths"]["/{filesystem}/{path}"].put.parameters.*
    transform: >
      if ($["x-ms-enum"] && $["x-ms-enum"]["name"] === "PathRenameMode") {
        delete $["x-ms-enum"];
        delete $["enum"];
      }
```

### UndeletePath

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{filesystem}/{path}?comp=undelete"].put.responses
    transform: >
      $["200"].headers["x-ms-resource-type"]["x-nullable"] = true;
```

### GetPathAccessControlList

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]
    transform: >
      $["/{filesystem}/{path}?getAccessControlList"] = {};
      $["/{filesystem}/{path}?getAccessControlList"].head = JSON.parse(JSON.stringify($["/{filesystem}/{path}"].head));
      delete $["/{filesystem}/{path}"].head;
  - from: swagger-document
    where: $["x-ms-paths"]["/{filesystem}/{path}?getAccessControlList"].head
    transform: >
      $.operationId = "Path_GetAccessControlList";
      $.parameters[0]["enum"] = ["getAccessControl"];
      $.parameters.push({"$ref": "#/parameters/ApiVersionParameter"});
      delete $.responses["200"].headers["Accept-Ranges"];
      delete $.responses["200"].headers["Cache-Control"];
      delete $.responses["200"].headers["Content-Disposition"];
      delete $.responses["200"].headers["Content-Encoding"];
      delete $.responses["200"].headers["Content-Language"];
      delete $.responses["200"].headers["Content-Length"];
      delete $.responses["200"].headers["Content-Range"];
      delete $.responses["200"].headers["Content-Type"];
      delete $.responses["200"].headers["Content-MD5"];
      delete $.responses["200"].headers["ETag"];
      delete $.responses["200"].headers["Last-Modified"];
      delete $.responses["200"].headers["x-ms-resource-type"];
      delete $.responses["200"].headers["x-ms-properties"];
      delete $.responses["200"].headers["x-ms-lease-duration"];
      delete $.responses["200"].headers["x-ms-lease-state"];
      delete $.responses["200"].headers["x-ms-lease-status"];
      $.responses["200"].headers["x-ms-acl"]["x-ms-client-name"] = "Acl";
      $.responses["200"].schema = {
        "type": "object",
        "x-ms-sealed": false,
        "x-ms-client-name": "GetPathAccessControlListResult",
        "x-namespace": "_detail",
        "properties": {
          "__placeHolder": {"type": "integer"}
        }
      };
```

### SetAccessControlListRecursive

```yaml
directive:
  - from: swagger-document
    where: $.definitions
    transform: >
      $.AclFailedEntry.description = "The failed entry when setting the Acl.";
      $.AclFailedEntry.properties["name"].description = "Name of the failed entry.";
      $.AclFailedEntry.properties["type"].description = "Type of the entry.";
      $.AclFailedEntry.properties["errorMessage"].description = "Error message for the failure.";

      $.SetAccessControlRecursiveResponse["x-ms-client-name"] = "SetAccessControlListRecursiveResult";
      $.SetAccessControlRecursiveResponse["x-namespace"] = "_detail";
      $.SetAccessControlRecursiveResponse["x-ms-sealed"] = false;
      $.SetAccessControlRecursiveResponse.properties["directoriesSuccessful"]["x-ms-client-name"] = "NumberOfSuccessfulDirectories";
      $.SetAccessControlRecursiveResponse.properties["filesSuccessful"]["x-ms-client-name"] = "NumberOfSuccessfulFiles";
      $.SetAccessControlRecursiveResponse.properties["failureCount"]["x-ms-client-name"] = "NumberOfFailures";

      $.PathSetAccessControlListRecursiveMode = {
        "x-namespace": "_detail",
        "x-ms-export": true,
        "type": "string",
        "enum": ["set", "modify", "remove"],
        "x-ms-enum":  {
          "name": "PathSetAccessControlListRecursiveMode",
          "modelAsString": false
        }
      };
  - from: swagger-document
    where: $.parameters
    transform: >
      delete $.PathSetAccessControlRecursiveMode["enum"];
      delete $.PathSetAccessControlRecursiveMode["x-ms-enum"];
  - from: swagger-document
    where: $["x-ms-paths"]["/{filesystem}/{path}?action=setAccessControlRecursive"].patch
    transform: >
      $.responses["200"].headers["x-ms-continuation"]["x-ms-client-name"] = "ContinuationToken";
      $.responses["200"].headers["x-ms-continuation"]["x-nullable"] = true;
```

### AppendFile

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{filesystem}/{path}?action=append"].patch.responses["202"].headers
    transform: >
      $["Content-MD5"]["x-ms-client-name"] = "TransactionalContentHash";
      $["Content-MD5"]["x-nullable"] = true;
      $["x-ms-content-crc64"]["x-ms-client-name"] = "TransactionalContentHash";
      $["x-ms-content-crc64"]["x-nullable"] = true;
      $["x-ms-encryption-key-sha256"]["x-nullable"] = true;
      delete $["ETag"];
```

### FlushFile

```yaml
directive:
  - from: swagger-document
    where: $["x-ms-paths"]["/{filesystem}/{path}?action=flush"].patch.responses["200"].headers
    transform: >
      $["Content-Length"]["x-ms-client-name"] = "FileSize";
      $["x-ms-encryption-key-sha256"]["x-nullable"] = true;
```