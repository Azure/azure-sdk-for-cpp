import "./service/client.tsp";
import "@azure-tools/typespec-client-generator-core";
import "./client-cpp.mjs";

using Azure.ClientGenerator.Core;

@@alternateType(Storage.Queues.MetadataHeaders.metadata, Record<string>, "cpp");
#suppress "@azure-tools/typespec-client-generator-core/client-option" "C++ prefix header collection support."
@@clientOption(Storage.Queues.MetadataHeaders.metadata, "collectionHeaderPrefix", "x-ms-meta-", "cpp");

@@alternateType(Storage.Queues.KeyInfo.start, string, "cpp");
@@alternateType(Storage.Queues.KeyInfo.expiry, string, "cpp");

namespace QueueCppCustomization {
  @encode("rfc7231")
  scalar LastSyncDateTime extends utcDateTime;

  extern dec alternateParameterType(
    target: TypeSpec.Reflection.Operation,
    parameter: valueof string,
    type: unknown,
    scope: valueof string
  );
}

@@alternateType(Storage.Queues.GeoReplication.lastSyncTime, QueueCppCustomization.LastSyncDateTime | null, "cpp");

@@QueueCppCustomization.alternateParameterType(Storage.Queues.Queue.receiveMessages, "numberOfMessages", int64, "cpp");
@@QueueCppCustomization.alternateParameterType(Storage.Queues.Queue.peekMessages, "numberOfMessages", int64, "cpp");

@@TypeSpec.Xml.name(Storage.Queues.QueueServiceStats, "StorageServiceStats");
