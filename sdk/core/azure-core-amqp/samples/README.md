---
page_type: sample
languages:
- C++
products:
- azure
- azure-core-amqp
urlFragment: azure-core-amqp

---

# Samples for the Microsoft Azure SDK for C++ AMQP client library

These code samples show common scenario operations for the Azure SDK for C++ AMQP support.

The AMQP client library is a C++ library for communicating with entities that use the AMQP 
protocol.

These samples are intended to demonstrate common AMQP use scenarios for Azure Service authors and are 
not intended to be a comprehensive collection of all possible uses for the library. The samples are
grouped into folders by feature set. Each sample includes a README that describes the scenario.

> **NOTE** AMQP Support in the Azure SDK for C++ is intended *only* for use with Azure services. 
It is *not* a general purpose AMQP library. Because the library is intended for internal consumption only, 
the contents of this library are NOT subject to the normal breaking change policies for the Azure SDK and 
may change at any time without warning

## Prerequisites

The samples are compatible with C++ 14 and later.

## Sample Overview

All samples are standalone console applications that can be built and run independently. Each sample has similar dependencies: They assume that an existing 
Azure EventHubs instance has been created and the following environment variables are set:

- `EVENTHUB_CONNECTION_STRING`: The connection string for the EventHubs instance.
- `EVENTHUB_NAME`: The name of the EventHubs instance.
- `SAMPLES_CLIENT_ID`: The client ID for the EventHubs instance.
- `SAMPLES_CLIENT_SECRET`: The client secret for the EventHubs instance.
- `SAMPLES_TENANT_ID`: The tenant ID for the EventHubs instance.

If you have cloned the azure-sdk-for-cpp repo, this can be done using the [Azure SDK Live Test Resource Management tools](https://github.com/Azure/azure-sdk-tools/blob/main/eng/common/TestResources/README.md).

```pwsh
Connect-AzAccount -Subscription 'YOUR SUBSCRIPTION ID'
eng\common\testResources\New-TestResources.ps1 -Location Westus -ServiceDirectory core\azure-core-amqp\samples
```

This will establish a connection to your Azure account and create a resource group and an EventHubs instance. The connection string and other information will be displayed in the console.


## Building and Running the Samples
To build the samples, create a build directory and use CMake to generate the build files. For example:

```bash
mkdir build
cd build
cmake ..
```

Then use CMake to build the samples:

```bash
cmake --build .
```

Each sample is built into its own directory. For example, the `eventhub_sas_reader_sample` is built into the `eventhub_sas_reader_sample` directory.

### eventhub_sas_reader_sample
Demonstrates reading messages from the Azure Event Hubs service using the AMQP protocol with SAS authentication.

### eventhub_sas_writer_sample
Demonstrates writing messages to the Azure Event Hubs service using the AMQP protocol with SAS authentication.

<!-- @insert_snippet: CreateSender -->
```cpp
Azure::Core::Amqp::_internal::MessageSenderOptions senderOptions;
senderOptions.Name = "sender-link";
senderOptions.MessageSource = "source";
senderOptions.SettleMode = Azure::Core::Amqp::_internal::SenderSettleMode::Unsettled;
senderOptions.MaxMessageSize = (std::numeric_limits<uint16_t>::max)();

Azure::Core::Amqp::_internal::MessageSender sender(
    session, credentials->GetEntityPath(), senderOptions, nullptr);
```

<!-- @insert_snippet: create_connection -->
```cpp
Azure::Core::Amqp::_internal::ConnectionOptions connectionOptions;
connectionOptions.ContainerId = "whatever";
connectionOptions.EnableTrace = true;
connectionOptions.Port = credential->GetPort();
Azure::Core::Amqp::_internal::Connection connection(
    credential->GetHostName(), credential, connectionOptions);
```

### eventhub_token_reader_sample
Demonstrates reading messages from the Azure Event Hubs service using the AMQP protocol with an Azure bearer token authentication.

### eventhub_token_writer_sample
Demonstrates writing messages to the Azure Event Hubs service using the AMQP protocol with an Azure bearer token authentication.

### local_client_async_sample
Demonstrates sending messages to a local AMQP server using the AMQP protocol using queued send operations.

### local_client_sample
Demonstrates sending messages to a local AMQP server using the AMQP protocol.

### local_server_sample
Demonstrates receiving messages from a local AMQP server using the AMQP protocol.

### eventhub_get_eventhub_properties_sample
Demonstrates receiving messages from the Azure Event Hubs service using an AMQP Management API.
