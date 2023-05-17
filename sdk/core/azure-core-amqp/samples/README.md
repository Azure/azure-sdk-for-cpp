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
protocol. It is a wrapper around the [Azure uAMQP Library](https://github.com/Azure/azure-uamqp-c/).

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

### eventhub_async_writer_sample
Demonstrates writing messages to the Azure Event Hubs service using the AMQP protocol using queued send operations.

### eventhub_reader_sample
Demonstrates reading messages from the Azure Event Hubs service using the AMQP protocol.

### eventhub_sas_reader_sample
Demonstrates reading messages from the Azure Event Hubs service using the AMQP protocol with SAS authentication.

### eventhub_sas_writer_sample
Demonstrates writing messages to the Azure Event Hubs service using the AMQP protocol with SAS authentication.

### eventhub_token_writer_sample
Demonstrates writing messages to the Azure Event Hubs service using the AMQP protocol with an Azure bearer token authentication.

### eventhub_writer_sample
Demonstrates writing messages to the Azure Event Hubs service using the AMQP protocol.

### local_client_async_sample
Demonstrates sending messages to a local AMQP server using the AMQP protocol using queued send operations.

### local_client_sample
Demonstrates sending messages to a local AMQP server using the AMQP protocol.

### local_server_sample
Demonstrates receiving messages from a local AMQP server using the AMQP protocol.

