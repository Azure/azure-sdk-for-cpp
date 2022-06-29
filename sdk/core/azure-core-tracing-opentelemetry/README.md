# Azure SDK Core Tracing Library for C++

Azure::Core::Tracing::OpenTelemetry (`azure-core-tracing-opentelemetry`) provides an implementation
to enable customers to implement tracing in the Azure SDK for C++ libraries.

## Getting started

### Include the package

The easiest way to acquire the OpenTelemetry library is leveraging vcpkg package manager. See the corresponding [Azure SDK for C++ readme section][azsdk_vcpkg_install].

To install Azure Core OpenTelemetry package via vcpkg:

```cmd
> vcpkg install azure-core-tracing-opentelemetry-cpp
```

Then, use in your CMake file:

```CMake
find_package(azure-core-tracing-opentelemetry-cpp CONFIG REQUIRED)
target_link_libraries(<your project name> PRIVATE Azure::azure-core-tracing-opentelemetry)
```

## Key concepts

The `azure-core-tracing-opentelemetry` package supports enabling tracing for Azure SDK packages, using an [OpenTelemetry](https://opentelemetry.io/) `Tracer`.

By default, all libraries log with a `NoOpTracer` that takes no action. To enable tracing, you will need to set a global tracer provider following the instructions in the [OpenTelemetry getting started guide](https://opentelemetry-cpp.readthedocs.io/en/latest/api/GettingStarted.html) or the [Enabling Tracing using OpenTelemetry example](#enabling-tracing-using-opentelemetry) below.

### Span Propagation

Core Tracing supports both automatic and manual span propagation. Automatic propagation is handled using OpenTelemetry's API and will work well in most scenarios.

For customers who require manual propagation, all client library operations accept an optional field in the `options` parameter where a tracingContext can 
be passed in and used as the currently active context. Please see the [Manual Span Propagation example](#manual-span-propagation-using-opentelemetry) 
below for more details.

### OpenTelemetry Compatibility

Most Azure SDKs use [OpenTelemetry](https://opentelemetry.io/) to support tracing. Specifically, we depend on 
the [opentelemetry-cpp](https://github.com/open-telemetry/opentelemetry-cpp/blob/main/docs/building-with-vcpkg.md) VCPKG package.


## Examples

### Enabling tracing using OpenTelemetry

```cpp
// Start by creating an OpenTelemetry Provider using the
// default OpenTelemetry tracer provider.
std::shared_ptr<Azure::Core::Tracing::TracerProvider> tracerProvider = Azure::Core::OpenTelemetry::TracerProvider::Create();

// Connect the tracerProvider to the current application context.
ApplicationContext().SetTracerProvider(tracerProvider);
```

After this, the SDK API implementations will be able to retrieve the tracer provider and produce tracing events automatically.

### Enabling tracing using a non-default TracerProvider

```cpp
// Start by creating an OpenTelemetry Provider.
auto exporter = std::make_unique<opentelemetry::exporter::memory::InMemorySpanExporter>();
m_spanData = exporter->GetData();

// simple processor
auto simple_processor = std::unique_ptr<opentelemetry::sdk::trace::SpanProcessor>(
    new opentelemetry::sdk::trace::SimpleSpanProcessor(std::move(exporter)));

auto always_on_sampler = std::unique_ptr<opentelemetry::sdk::trace::AlwaysOnSampler>(
    new opentelemetry::sdk::trace::AlwaysOnSampler);

auto resource_attributes = opentelemetry::sdk::resource::ResourceAttributes{
    {"service.name", "telemetryTest"}, {"service.instance.id", "instance-1"}};
auto resource = opentelemetry::sdk::resource::Resource::Create(resource_attributes);
auto openTelemetryProvider = opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider>(
        new opentelemetry::sdk::trace::TracerProvider(
            std::move(simple_processor), resource, std::move(always_on_sampler)));

// Use the default OpenTelemetry tracer provider.
std::shared_ptr<Azure::Core::Tracing::TracerProvider> tracerProvider = 
    Azure::Core::OpenTelemetry::TracerProvider::Create(openTelemetryProvider);

// Connect the tracerProvider to the current application context.
ApplicationContext().SetTracerProvider(tracerProvider);
```

### Manual Span Propagation using OpenTelemetry

In Azure Service methods, the `Azure::Context` value passed into the tracer optionally has an associated Span.

If there is a span associated with the `Azure::Context`, then calling `DiagnosticTracingFactory::CreateSpanFromContext` will
cause a new span to be created using the span in the provided `Azure::Context` object as the parent span.

```cpp
    auto contextAndSpan
        = Azure::Core::Tracing::_internal::DiagnosticTracingFactory::CreateSpanFromContext(
            "HTTP GET#2", context);
```


## Next steps

You can build and run the tests locally by executing `azure-core-tracing-opentelemetry-test`. Explore the `test` folder to see advanced usage and behavior of the public classes.

## Troubleshooting

If you run into issues while using this library, please feel free to [file an issue](https://github.com/Azure/azure-sdk-for-cpp/issues/new).

### OpenTelemetry Compatibility Errors


> Ideally you'd want to use OpenTelemetry 1.3.0 or higher.

<!-- ### Community-->

### Reporting security issues and security bugs

Security issues and bugs should be reported privately, via email, to the Microsoft Security Response Center (MSRC) <secure@microsoft.com>. You should receive a response within 24 hours. If for some reason you do not, please follow up via email to ensure we received your original message. Further information, including the MSRC PGP key, can be found in the [Security TechCenter](https://www.microsoft.com/msrc/faqs-report-an-issue).

### License

Azure SDK for C++ is licensed under the [MIT](https://github.com/Azure/azure-sdk-for-cpp/blob/main/LICENSE.txt) license.

<!-- LINKS -->
[azsdk_vcpkg_install]: https://github.com/Azure/azure-sdk-for-cpp#download--install-the-sdk
[azure_sdk_for_cpp_contributing]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[azure_sdk_for_cpp_contributing_developer_guide]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#developer-guide
[azure_sdk_for_cpp_contributing_pull_requests]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md#pull-requests
[azure_sdk_cpp_development_guidelines]: https://azure.github.io/azure-sdk/cpp_introduction.html
[azure_cli]: https://docs.microsoft.com/cli/azure
[azure_pattern_circuit_breaker]: https://docs.microsoft.com/azure/architecture/patterns/circuit-breaker
[azure_pattern_retry]: https://docs.microsoft.com/azure/architecture/patterns/retry
[azure_portal]: https://portal.azure.com
[azure_sub]: https://azure.microsoft.com/free/
[c_compiler]: https://visualstudio.microsoft.com/vs/features/cplusplus/
[cloud_shell]: https://docs.microsoft.com/azure/cloud-shell/overview
[cloud_shell_bash]: https://shell.azure.com/bash

![Impressions](https://azure-sdk-impressions.azurewebsites.net/api/impressions/azure-sdk-for-cpp%2Fsdk%2Fcore%2Fcore-opentelemetry%2FREADME.png)