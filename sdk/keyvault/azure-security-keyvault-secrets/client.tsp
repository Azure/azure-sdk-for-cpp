// client.tsp
import "./main.tsp";
import "@azure-tools/typespec-client-generator-core";

using Azure.ClientGenerator.Core;

@@clientName(KeyVault, "SecretClient"); // This is the name of the client
