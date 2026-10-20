import "./service/client.tsp";
import "@azure-tools/typespec-client-generator-core";

using Azure.ClientGenerator.Core;

@@access(Storage.DataLake.Service.listFileSystems, Access.internal, "cpp");
@@access(Storage.DataLake.FileSystem.create, Access.internal, "cpp");
@@access(Storage.DataLake.FileSystem.setProperties, Access.internal, "cpp");
@@access(Storage.DataLake.FileSystem.getProperties, Access.internal, "cpp");
@@access(Storage.DataLake.FileSystem.delete, Access.internal, "cpp");
@@access(Storage.DataLake.FileSystem.listBlobHierarchySegment, Access.internal, "cpp");
@@access(Storage.DataLake.Path.update, Access.internal, "cpp");
@@access(Storage.DataLake.Path.lease, Access.internal, "cpp");
@@access(Storage.DataLake.Path.read, Access.internal, "cpp");
@@access(Storage.DataLake.Path.setExpiry, Access.internal, "cpp");

@@alternateType(Storage.DataLake.ContentMD5ResponseHeader.contentMD5, string, "cpp");
@@alternateType(Storage.DataLake.ContentMD5ResponseHeaderBytes.contentMD5, string, "cpp");
@@alternateType(Storage.DataLake.ContentCrc64ResponseHeader.contentCrc64, string, "cpp");
@@alternateType(Storage.DataLake.EtagResponseHeader.eTag, Azure.Core.eTag | null, "cpp");
@@alternateType(Storage.DataLake.LastModifiedResponseHeader.lastModified, utcDateTime | null, "cpp");
@@alternateType(Storage.DataLake.CreationTimeResponseHeader.creationTime, utcDateTime | null, "cpp");
@@alternateType(Storage.DataLake.ExpiryTimeResponseHeader.expiresOn, utcDateTime | null, "cpp");

namespace DataLakeCppCustomization {
  model PathItem {
    ...OmitProperties<Storage.DataLake.PathItem, "eTag">;

    @encodedName("application/json", "etag")
    eTag?: Azure.Core.eTag;
  }
}

@@alternateType(Storage.DataLake.PathItem, DataLakeCppCustomization.PathItem, "cpp");
