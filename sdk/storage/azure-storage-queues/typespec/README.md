# Queue protocol generation

Queue operations are generated directly from TypeSpec using
[`Azure/typespec-cpp`](https://github.com/Azure/typespec-cpp).
`tsp-location.yaml` pins the service specification and `tspconfig.yaml` selects
the service version and internal protocol generation.

`includeResponseHeadersInModels: true` explicitly enables the typed response
headers consumed by the compatibility layer.

All generated code is in `src/generated`, under
`Azure::Storage::Queues::_detail`. The public clients reuse their existing
Storage pipelines and convert generated models to the existing public models.
The handwritten layer translates Core HTTP exceptions into `StorageException`
using the existing Storage error parser; no emitter-specific error factory is
needed.
An internal pipeline policy also removes the emitter's default `Accept` header
and retains `application/xml; charset=UTF-8` for XML requests before user policies
run. Requests without an XML body omit `Content-Type`, including visibility-only
message updates. This preserves the previous Queue wire behavior and compatibility
with existing recordings. User policies can still override these headers.
The public `inc/azure/storage/queues/rest_client.hpp` is retained for source
compatibility, including existing include paths, fields, defaults, and enums.
Do not replace it with the generated model header or edit generated C++ files.

## Regeneration

The emitter needs the Queue migration changes: XML metadata and nullable values,
header collections and scalar headers, SAS-safe query parameters, pipeline reuse,
and optional request bodies. Use committed emitter revision
`f81e14cd`, including fixed-precision XML date-times, or a revision containing those
changes. An unmodified published emitter is not sufficient. Generate from a clean
snapshot of that revision, not from uncommitted emitter experiments.

Build `packages/codegen.cpp` first, then `packages/typespec-cpp`, using the
emitter repository's package manifests. Use its compiler and dependencies rather
than independently installing the latest TypeSpec packages. The migration was
generated with compiler 1.12.0 and client-generator-core 0.68.4.

Provide a QueueStorage specification directory from the exact revision in
`tsp-location.yaml`. `typespec/client.tsp` adds C++-specific customizations without
changing those upstream inputs. From this package directory, run:

```powershell
.\typespec\generate.ps1 `
  -EmitterPath C:\path\to\typespec-cpp\packages\typespec-cpp `
  -SpecDirectory C:\path\to\azure-rest-api-specs\specification\storage\data-plane\QueueStorage `
  -ClangFormatPath "C:\Program Files\LLVM\bin\clang-format.exe"
```

Use clang-format 11.0.0. The script generates and formats in a temporary directory
before copying the five generated files into the package. `-OutputDirectory`
can redirect the output for comparison without changing the package.

After regeneration, build the package and run the offline `QueueProtocolTest.*`
tests, then the existing playback tests when test-proxy assets are available.
The offline tests cover public model conversions, metadata, service versions,
SAS URLs, XML, paging, error handling, and visibility-only updates.

The C++ input overlay preserves nullable RFC1123 statistics dates and 64-bit
receive/peek count options, supplies the `StorageServiceStats` XML root, and
enables metadata header collections. ACL dates retain all seven fractional-second
digits. Delegation-key request dates are projected as strings and formatted by
the handwritten client without fractional seconds, as required by that endpoint.
Keep `client-cpp.mjs` alongside `client.tsp`:
the helper applies the existing language-scoped `alternateType` decorator to
expanded operation parameters that the pinned compiler's `::parameters`
selector does not expose. It does not modify the upstream specification.
