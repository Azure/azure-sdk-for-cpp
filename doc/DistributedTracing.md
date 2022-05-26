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
* Service Method "MyServiceMethod" creates a span named "MyServiceMethod" and starts an HTTP request to communicate with the service.
  * The HTTP pipeline (specifically the `RequestActivityPolicy`) will create a child `span` under the service method `span` named `"HTTP <verb> #0"`. This span 
  reflects the HTTP call into the service.
  * If the HTTP call needs to be retried, the existing `span` will be closed an a new span named `HTTP <verb> #1` will be created for the retry. 



## Distributed Tracing Client Integration.
Applications which wish to integrate Distributed Tracing are strongly encouraged
to use the [opentelemetry-cpp](https://github.com/open-telemetry/opentelemetry-cpp) vcpkg package.

There are numerous examples on the OpenTelemetry web site which demonstrate how to integrate
opentelemetry into a customer application and integrate the generated traces
with Azure monitoring infrastructure such as Geneva Monitoring.

# Distributed Tracing components.
In order to maintain flexibility, the actual distributed tracing mechanism is 