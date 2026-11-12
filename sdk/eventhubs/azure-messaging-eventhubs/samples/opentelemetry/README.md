# Export Event Hubs traces and logs with OpenTelemetry

This executable configures OTLP/HTTP exporters for two independent signals:

- **Traces:** an OpenTelemetry `TracerProvider`, `BatchSpanProcessor`, and the
  `azure-core-tracing-opentelemetry` adapter assigned to `ProducerClientOptions.TracingProvider`.
- **Logs:** an OpenTelemetry `LoggerProvider` and `BatchLogRecordProcessor`, connected to the
  process-wide `Azure::Core::Diagnostics::Logger` listener by application code.

Event Hubs does **not** emit native OpenTelemetry metrics. Configuring a metrics exporter alone
will not produce SDK metrics. Applications can create their own OpenTelemetry counters and
histograms, or configure a backend to derive metrics from spans and lifecycle logs. Those are
application/backend metrics, not an SDK metrics contract. Metrics derived from spans or logs are
affected by trace sampling and log-level filtering; they may not represent every operation or failure.
See the OpenTelemetry C++
[metrics exporter example](https://github.com/open-telemetry/opentelemetry-cpp/blob/v1.28.0/examples/otlp/http_metric_main.cc)
for an application-owned metrics pipeline.

## Build

Build from this repository revision: the Event Hubs tracing options used here are not available
in older package releases. The sample is opt-in and does not add dependencies to Event Hubs.
The repository's vcpkg baseline pins **opentelemetry-cpp 1.28.0** with the `otlp-http` feature,
which supplies the trace and log HTTP exporters. It also supplies their curl/protobuf dependencies.
Use a C++14 compiler, CMake, and vcpkg as described in the repository
[development guide](https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md). On Windows, use a static triplet such as
`x64-windows-static` because the Azure OpenTelemetry adapter requires it.

From the repository root, with `VCPKG_ROOT` set to your vcpkg checkout:

```sh
cmake -S . -B build-otel \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
  -DBUILD_SAMPLES=ON \
  -DDISABLE_RUST_IN_BUILD=ON \
  -D AZURE_EVENTHUBS_BUILD_OPENTELEMETRY_SAMPLE=ON
cmake --build build-otel --target eventhubs-opentelemetry --config Debug --parallel
```

The commands select the uAMQP backend for this example; the exporter setup also works with the
Rust AMQP backend. For a single-configuration generator the executable is
`build-otel/sdk/eventhubs/azure-messaging-eventhubs/samples/opentelemetry/eventhubs-opentelemetry`.
For a multi-configuration generator, append the configuration directory (`Debug`) before the
executable filename; Windows also adds `.exe`.

For an application outside this repository, link the Azure Event Hubs, Azure Identity, and
Azure OpenTelemetry adapter packages plus `opentelemetry-cpp::otlp_http_exporter` and
`opentelemetry-cpp::otlp_http_log_record_exporter`. See this directory's [CMakeLists.txt](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/eventhubs/azure-messaging-eventhubs/samples/opentelemetry/CMakeLists.txt)
for the complete target list. Enable the `otlp-http` feature when installing opentelemetry-cpp
with vcpkg. The sample uses Azure Identity only for the optional live send.

## Verify export locally, without Azure credentials

Start an OpenTelemetry Collector in a separate terminal, from the repository root:

```sh
docker run --rm --name eventhubs-otel-collector \
  -p 127.0.0.1:4318:4318 \
  -v "$PWD/sdk/eventhubs/azure-messaging-eventhubs/samples/opentelemetry/collector.yaml:/etc/otelcol/config.yaml:ro" \
  otel/opentelemetry-collector:0.109.0
```

Set the full signal-specific OTLP/HTTP URLs and run the executable:

```sh
export OTEL_EXPORTER_OTLP_TRACES_ENDPOINT=http://localhost:4318/v1/traces
export OTEL_EXPORTER_OTLP_LOGS_ENDPOINT=http://localhost:4318/v1/logs
./build-otel/sdk/eventhubs/azure-messaging-eventhubs/samples/opentelemetry/eventhubs-opentelemetry --smoke
```

The collector should print a trace named `sample.exporter-check` and a log whose body is
`Close producer client.`, both with `service.name=eventhubs-opentelemetry-sample`. The span is
created by the application to verify its exporter; it is **not an Event Hubs operation span**.
The log is emitted by the SDK when the unused producer is destroyed and goes through the
Azure Logger bridge. This mode makes no Azure requests and does not test AMQP lifecycle recovery
or live send/receive. It never requests a token. Check collector output to establish delivery:
a successful local flush is not proof that a backend stored the records.

The sample uses binary protobuf over HTTP, not OTLP/gRPC. Port `4318` is the collector's HTTP
port; do not point these exporters at gRPC port `4317`. Signal-specific URLs include `/v1/traces`
or `/v1/logs`. Alternatively, set `OTEL_EXPORTER_OTLP_ENDPOINT=http://localhost:4318` and unset the
two signal-specific variables; the exporter appends the paths. See the OpenTelemetry
[OTLP exporter configuration](https://opentelemetry.io/docs/languages/sdk-configuration/otlp-exporter/)
for HTTPS, headers, and timeouts. Keep collector credentials in environment configuration,
not source files. The supplied collector prints telemetry locally; replace its debug exporter
with your backend exporter for production.

## Send an event to Azure

Sign in with `az login`. The signed-in principal needs **Azure Event Hubs Data Sender** access
to an existing Event Hub. Set the host (without a URL scheme) and Event Hub name:

```sh
export EVENTHUBS_HOST=your-namespace.servicebus.windows.net
export EVENTHUB_NAME=your-event-hub
./build-otel/sdk/eventhubs/azure-messaging-eventhubs/samples/opentelemetry/eventhubs-opentelemetry --send
```

This sends one `Hello OpenTelemetry!` event with a 30-second operation deadline. The collector should show `ProducerClient.Send`
and `ProducerClient.AmqpSend` spans, together with SDK logs. This requires Azure connectivity
and permission to send data; it is separate from the local exporter check.

For receive tracing, assign the same adapter to `ConsumerClientOptions.TracingProvider` before
constructing a consumer. Partition clients created by that consumer inherit the provider;
`PartitionClient.ReceiveEvents` creates operation spans. The
[package README](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/eventhubs/azure-messaging-eventhubs/README.md#distributed-tracing) describes the emitted spans and attributes.

## Ownership, severity, and shutdown

The application owns both OpenTelemetry providers. Registering an OpenTelemetry global provider
alone does not enable Event Hubs tracing: set the Azure adapter on each client options object.
Likewise, configuring a trace provider does not export Azure Logger records; install the separate
listener bridge. This example uses a single process-wide listener and replaces any previous
listener. Coordinate ownership with any existing logging integration in your application.

The bridge preserves the SDK message as the log body and maps levels explicitly:

| Azure Logger level | OpenTelemetry severity |
| --- | --- |
| Verbose | DEBUG |
| Informational | INFO |
| Warning | WARN |
| Error | ERROR |

The callback captures an owning logger pointer and queues records through a batch processor.
It must not call Azure SDK APIs or recursively write through Azure Logger. Azure Logger supplies
only level and message: the bridge does not invent trace/span IDs or parse lifecycle text into
structured attributes. Async SDK logs may have no active operation context. This sample therefore
does not promise automatic trace/log correlation; use the component IDs already in lifecycle
messages when investigating reconnects. Verbose logging is enabled to make the smoke check visible;
choose Informational or Warning for normal production use and apply your log-data handling policy.

Batch processors export on background workers using bounded queues; tune their batch settings
for your workload, and account for records being dropped when queues fill. At exit the sample:

1. Destroys the producer, allowing its final SDK logs to be queued.
2. Clears the Azure Logger listener, waiting for active callbacks to finish.
3. Calls `ForceFlush` on both providers and then `Shutdown`, with a five-second timeout per call.
4. Reports failure if any flush/shutdown call fails. Exception unwinding also performs cleanup.

In a service, first stop all work and destroy all clients using the listener, then follow this
sequence before process exit. Do not tear down providers while callbacks or client operations
are still running.
