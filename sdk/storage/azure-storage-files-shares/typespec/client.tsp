import "./service/client.tsp";
import "@azure-tools/typespec-client-generator-core";

using Azure.ClientGenerator.Core;
using Storage.File;

@@alternateType(MetadataHeaders.metadata, Record<string>, "cpp");
#suppress "@azure-tools/typespec-client-generator-core/client-option" "C++ prefix header collection support."
@@clientOption(MetadataHeaders.metadata, "collectionHeaderPrefix", "x-ms-meta-", "cpp");

@@alternateType(FileCreationTimeParameter.fileCreationTime, string, "cpp");
@@alternateType(FileLastWriteTimeParameter.fileLastWriteTime, string, "cpp");
@@alternateType(FileChangeTimeParameter.fileChangeTime, string, "cpp");
@@alternateType(KeyInfo.start, string, "cpp");
@@alternateType(KeyInfo.expiry, string, "cpp");

@@alternateType(ContentMD5ResponseHeader.contentMD5, string, "cpp");
@@alternateType(ContentCrc64ResponseHeader.contentCrc64, string, "cpp");
@@alternateType(FileHttpContentResponseHeaders.contentMD5, string, "cpp");
@@alternateType(FileContentMD5ResponseHeader.fileContentMD5, string, "cpp");

@@alternateType(ShareQuotaOptionalHeader.quota, int64, "cpp");
@@alternateType(ShareQuotaHeader.quota, int64, "cpp");
@@alternateType(SharePropertiesInternal.quota, int64, "cpp");
@@alternateType(ShareStats.shareUsageBytes, int64, "cpp");
@@alternateType(LinkCountResponseHeader.linkCount, int64, "cpp");

@@alternateType(EtagResponseHeader.etag, Azure.Core.eTag | null, "cpp");
@@alternateType(LastModifiedResponseHeader.lastModified, utcDateTime | null, "cpp");
@@TypeSpec.Http.header(RangeHeader.range, "x-ms-range");
@@TypeSpec.Http.header(RangeRequiredHeader.range, "x-ms-range");
