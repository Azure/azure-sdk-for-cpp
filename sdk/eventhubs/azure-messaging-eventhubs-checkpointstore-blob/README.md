<!-- cspell:words azeventhubs  -->
# Azure Event Hubs Blob Storage Checkpoint Store for C++

The EventHubs Blob Storage Checkpoint Store is a checkpoint store for the 
[Azure Event Hubs](https://azure.microsoft.com/services/event-hubs/) service, used to enable an 
[Event Processor](https://learn.microsoft.com/azure/event-hubs/event-hubs-event-processor-host) to store checkpoints and partition ownership information in Azure Blob Storage.

For information on how to use an EventHubs processor, see the [Azure SDK for C++ EventHubs documentation](https://azure.github.io/azure-sdk-for-cpp/eventhubs.html).

Key links:
- [Source code][source]
- [API Reference Documentation][cppdoc]
- [Product documentation](https://azure.microsoft.com/services/event-hubs/)
- [Samples][cppdoc_examples]

## Getting started

### Install the package

Install the Azure Event Hubs Blob Storage Checkpoint Store for C++ with `vcpkg`:

```bash
vcpkg install azure-messaging-eventhubs-checkpointstore-blob-cpp
```

### Prerequisites

- A C++ Compiler with C++14 support
- An [Azure subscription](https://azure.microsoft.com/free/)
- An [Event Hub namespace](https://learn.microsoft.com/azure/event-hubs/).
- An Event Hub. You can create an event hub in your Event Hubs Namespace using the [Azure Portal](https://learn.microsoft.com/azure/event-hubs/event-hubs-create), or the [Azure CLI](https://learn.microsoft.com/azure/event-hubs/event-hubs-quickstart-cli).

## Contributing
For details on contributing to this repository, see the [contributing guide][azure_sdk_for_cpp_contributing].

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

### Additional Helpful Links for Contributors  
Many people all over the world have helped make this project better.  You'll want to check out:

* [What are some good first issues for new contributors to the repo?](https://github.com/azure/azure-sdk-for-cpp/issues?q=is%3Aopen+is%3Aissue+label%3A%22up+for+grabs%22)
* [How to build and test your change][azure_sdk_for_cpp_contributing_developer_guide]
* [How you can make a change happen!][azure_sdk_for_cpp_contributing_pull_requests]
* Frequently Asked Questions (FAQ) and Conceptual Topics in the detailed [Azure SDK for C++ wiki](https://github.com/azure/azure-sdk-for-cpp/wiki).

<!-- ### Community-->
### Reporting security issues and security bugs

Security issues and bugs should be reported privately, via email, to the Microsoft Security Response Center (MSRC) <secure@microsoft.com>. You should receive a response within 24 hours. If for some reason you do not, please follow up via email to ensure we received your original message. Further information, including the MSRC PGP key, can be found in the [Security TechCenter](https://www.microsoft.com/msrc/faqs-report-an-issue).

### License

Azure SDK for C++ is licensed under the [MIT](https://github.com/Azure/azure-sdk-for-cpp/blob/main/LICENSE.txt) license.

<!-- LINKS -->
[azure_sdk_for_cpp_contributing]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[azure_sdk_for_cpp_contributing_developer_guide]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#developer-guide
[azure_sdk_for_cpp_contributing_pull_requests]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#pull-requests

[consumer_client]: https://azuresdkdocs.z19.web.core.windows.net/cpp/azure-messaging-eventhubs/latest/class_azure_1_1_messaging_1_1_event_hubs_1_1_consumer_client.html
[producer_client]: https://azuresdkdocs.z19.web.core.windows.net/cpp/azure-messaging-eventhubs/latest/class_azure_1_1_messaging_1_1_event_hubs_1_1_producer_client.html

[source]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/eventhubs
[azure_identity_pkg]: https://azuresdkdocs.z19.web.core.windows.net/cpp/azure-identity/latest/index.html
[default_azure_credential]: https://azuresdkdocs.z19.web.core.windows.net/cpp/azure-identity/latest/index.html#defaultazurecredential

[cppdoc]: https://azuresdkdocs.z19.web.core.windows.net/cpp/azure-messaging-eventhubs-checkpointstore-blob/latest/index.html
[cppdoc_examples]: https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/eventhubs/azure-messaging-eventhubs/samples


