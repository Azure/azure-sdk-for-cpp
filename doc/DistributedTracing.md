---
# cspell:words openetelemetry
---
# Distributed Tracing in the C++ SDK

Azure has adopted [W3C Distributed Tracing](https://www.w3.org/TR/trace-context/) as a paradigm for correlating
requests from clients across multiple services.

This document explains how the Azure C++ SDK implements distributed tracing, how clients integrate with distributed tracing, how
services should integrate with distributed tracing and finally how the network pipeline and other functionality should
integrate with distributed tracing.

## Tracing Overview

The Azure SDK for C++ Tracing APIs are modeled after the opentelemetry-cpp API surface defined in the [OpenTelemetry Tracing Specification](https://github.com/open-telemetry/opentelemetry-specification/blob/main/specification/trace/api.md).
Additional architectural information about OpenTelemetry can be found in [OpenTelemetry Concepts](https://opentelemetry.io/docs/concepts/).

There are three major components which the Azure SDK components interact with:

- `TracerProvider` - this is a factory which creates `Tracer` objects.
- `Tracer` - this is a factory which creates `Span` objects.
- `Span` - Span objects are the APIs which allow tracing an operation.
Each `span` has a name, a type and a "status". `Spans` also contain "attributes" and "events" which describe an operation.

There is typically a single `TracerProvider` for each application, and for the Azure SDK, each
service will have a `Tracer` implementation which creates `Span` objects for each service client.

A `Span` can be considered a "unit of work" for a service. Each service method (method which calls into the service) will have a single `Span` reflecting the client method which
was called.

`Span`'s are hierarchical and each span can have multiple children (each `Span` can only have a single parent). The typical way that this manifests itself during a
service method call is:

- Service Method "MyServiceMethod" creates a span named "MyServiceMethod" and starts an HTTP request to communicate with the service.
  - The HTTP pipeline (specifically the `RequestActivityPolicy`) will create a child `span` under the service method `span` named `"HTTP <verb> #0"`. This span

  reflects the HTTP call into the service.
  - If the HTTP call needs to be retried, the existing `span` will be closed an a new span named `HTTP <verb> #1` will be created for the retry.

## Distributed Tracing Client Integration

Applications which wish to integrate Distributed Tracing are strongly encouraged
to use the [opentelemetry-cpp](https://github.com/open-telemetry/opentelemetry-cpp) vcpkg package.

There are numerous examples on the OpenTelemetry web site which demonstrate how to integrate
opentelemetry into a customer application and integrate the generated traces
with Azure monitoring infrastructure such as Geneva Monitoring.

Following the examples from opentelemetry-cpp, the following can be used
to establish an OpenTelemetry exporter which logs to the console or to an
in-memory logger.

```c++
opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider>
CreateOpenTelemetryProvider()
{
#if USE_MEMORY_EXPORTER
    auto exporter = std::make_unique<opentelemetry::exporter::memory::InMemorySpanExporter>();
#else
    auto exporter = std::make_unique<opentelemetry::exporter::trace::OStreamSpanExporter>();
#endif

    // simple processor
    auto simple_processor = std::unique_ptr<opentelemetry::sdk::trace::SpanProcessor>(
        new opentelemetry::sdk::trace::SimpleSpanProcessor(std::move(exporter)));

    auto always_on_sampler = std::unique_ptr<opentelemetry::sdk::trace::AlwaysOnSampler>(
        new opentelemetry::sdk::trace::AlwaysOnSampler);

    auto resource_attributes = opentelemetry::sdk::resource::ResourceAttributes{
        {"service.name", "telemetryTest"}, {"service.instance.id", "instance-1"}};
    auto resource = opentelemetry::sdk::resource::Resource::Create(resource_attributes);
    // Create using SDK configurations as parameter
    return opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider>(
        new opentelemetry::sdk::trace::TracerProvider(
            std::move(simple_processor), resource, std::move(always_on_sampler)));
}
```

Other exporters exist to export to [Jaeger](https://github.com/open-telemetry/opentelemetry-cpp/tree/main/exporters/jaeger),
[Windows ETW](https://github.com/open-telemetry/opentelemetry-cpp/tree/main/exporters/etw) and others.

Once the `opentelemetry::trace::TracerProvider` has been created, The client needs to create a new `Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider` which
functions as an abstract class integration between OpenTelemetry and Azure Core:

```c++
std::shared_ptr<Azure::Core::Tracing::TracerProvider> traceProvider
      = std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider>(CreateOpenTelemetryProvider());
```

To finish the integration with Azure clients, there are two mechanisms to integrate OpenTelemetry into a client application:

1) `Azure::Core::Context` integration.
1) Service Client Options integration.

### Integrate an OpenTelemetryProvider via the ApplicationContext

To integrate OpenTelemetry for all Azure Clients in the application, the customer can call `Azure::Core::Context::ApplicationContext.SetTracerProvider` to establish the
tracer provider for the application.

```c++
    Azure::Core::Context::ApplicationContext.SetTracerProvider(provider);
```

### Integrate an OpenTelemetryProvider via Service ClientOptions

While using the ApplicationContext is the simplest mechanism for integration OpenTelemetry with a customer application, there may be times the customer needs more flexibility when creating service clients.
To enable customers to further customize how tracing works, the application can set the `Telemetry.TracingProvider` field in the service client options, which will establish the tracer provider used by
the service client.

```c++
auto tracerProvider(CreateOpenTelemetryProvider());
auto provider(std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider>(tracerProvider));

ServiceClientOptions clientOptions;
clientOptions.Telemetry.TracingProvider = provider;
clientOptions.Telemetry.ApplicationId = "MyApplication";
ServiceClient myServiceClient(clientOptions);
```

## Distributed Tracing Service Integration

There are two steps needed to integrate Distributed Tracing with a Service Client.

1. Add a `DiagnosticTracingFactory` object to the ServiceClient object
1. Update each service method as follows:
    1. Add a call to the `CreateSpan` method on the diagnostic tracing factory. This will create a new span for the client operation.
    1. Call `SetStatus` on the created span when the service method successfully completes.
    1. Wrap the client method code with a try/catch handler which catches exceptions and call AddEvent with the value of the exception.

### Add a `DiagnosticTracingFactory` to the serviceClient class

To add a new `DiagnosticTracingFactory` to the client, simply add the class as a member:

```c++
  Azure::Core::Tracing::_internal::TracingContextFactory m_tracingFactory;

```

And construct the new tracing factory in the service constructor:

```c++
  explicit ServiceClient(ServiceClientOptions const& clientOptions = ServiceClientOptions{})
      : m_tracingFactory(clientOptions, "Azure.Core.OpenTelemetry.Test.Service", PackageVersion::ToString())
 ```

### Update Each Service Method

 There are three methods of interest when updating the service method:

 1. `DiagnosticTracingFactory::CreateSpan` - this creates and returns a `Span` and `Context` object for the service method. The returned Context object must be used for subsequent service operations.
 1. `Span::AddEvent(std::exception&)` - This registers the exception with the distributed tracing infrastructure.
 1. `Span::SetStatus` - This sets the status of the operation in the trace.

 ```c++
 Azure::Response<std::string> ServiceMethod(
      std::string const&,
      Azure::Core::Context const& context = Azure::Core::Context{})
  {
    // Create a new context and span for this request.
    auto contextAndSpan = m_tracingFactory.CreateSpan("ServiceMethod", context);

    // contextAndSpan.Context is the new context for the operation.
    // contextAndSpan.Span is the new span for the operation.

    try
    {
      // <Call Into Service via an HTTP pipeline>
      Azure::Core::Http::Request requestToSend(
          HttpMethod::Get, Azure::Core::Url("<Service URL>"));

      std::unique_ptr<Azure::Core::Http::RawResponse> response
          = m_pipeline->Send(requestToSend, contextAndSpan.Context);
      contextAndSpan.Span.SetStatus(Azure::Core::Tracing::_internal::SpanStatus::Ok);
      return Azure::Response<std::string>("", std::move(response));
    }
    catch (std::exception const& ex)
    {
      // Register that the exception has happened and that the span is now in error.
      contextAndSpan.Span.AddEvent(ex);
      throw;
    }

    // When contextAndSpan.second goes out of scope, it ends the span, which will record it.
  }
};
```

## Implementation Details

### Distributed Tracing components

In order to maintain flexibility, the opentelemetry-cpp APIs are implemented in a separate package - azure-core-tracing-opentelemetry.
This is consistent with how opentelemetry is distributed for
the other Azure SDKs.

The Azure Core API surface interacts with a set of pure virtual base classes (aka "interfaces") in
the `Azure::Core::Tracing` and `Azure::Core::Tracing::_internal` namespace. These allow a level of separation
between the Azure Core API surface and the OpenTelemetry API surface - an alternative tracing mechanism needs
to provide APIs consistent with the `Azure::Core::Tracing` APIs.

The azure-core-tracing-openetelemetry-cpp package implements a set of APIs in the `Azure::Core::Tracing::OpenTelemetry`
and `Azure::Core::Tracing::OpenTelemetry::_detail` namespace. These provide an Azure Core compatable API surface for distributed tracing.

The core service client interface is the `DiagnosticTracingFactory` class which implements two APIs: `CreateSpan` and
`CreateSpanFromContext`. `CreateSpan` is intended to be used by service methods which have direct access to a
`DiagnosticTracingFactory` object, `CreateSpanFromContext` in intended to be used from code which does NOT have
direct access to the `DiagnosticTracingFactory`.

The final significant piece of the distributed tracing infrastructure is the `RequestActivityPolicy` - this policy MUST be
inserted into the HTTP pipeline AFTER the `RetryPolicy`. It is responsible for creating the span associated with the HTTP request, it will
also propagate the W3C distributed tracing headers from the span into the HTTP request.

### Generated traces

The Azure standards for distributed tracing are define in [Azure Distributed Tracing Conventions](https://github.com/Azure/azure-sdk/blob/main/docs/tracing/distributed-tracing-conventions.md).
The actual tracing elements generated by Azure services are defined in [Azure Tracing Conventions YAML](https://github.com/Azure/azure-sdk/blob/main/docs/tracing/distributed-tracing-conventions.yml).

In summary, these are the traces and attributes which should be generated
for azure services:

#### Spans

The distributed tracing standards define the following traces:

##### Public APIs

All public APIs MUST create a span which will describes the API.
The name of the span MUST be the API name.

##### HTTP Calls

Each HTTP request sent to the service MUST create a span describing the request to the service.
The name of the span MUST be of the form `HTTP <HTTP VERB> #<HTTP RETRY>`.

#### Attributes

Generated traces have the following attributes:

| Attribute Name | Semantics | Where Used
|-----------|--------|-------
| `az.namespace` |Namespace of the azure service request| All spans.
| `http.method`| HTTP Method ("GET", "PUT", etc)| HTTP Spans.
| `http.url`| URL being retrieved (sanitized)| HTTP Spans.
| `http.status_code` | HTTP status code returned by the service | HTTP Spans.
| `http.user_agent` | The value of the `User-Agent` HTTP header sent to the service | HTTP Spans.
| `requestId` | The value of the `x-ms-client-request-id` header sent by the client | HTTP Spans.
| `serviceRequestId` | The value -f the `x-ms-request-id` sent by the server | HTTP Spans.
