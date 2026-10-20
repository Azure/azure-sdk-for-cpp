# Data Lake protocol generation

The private protocol client is generated from the DataLakeStorage TypeSpec
specification pinned in `tsp-location.yaml`. `tspconfig.yaml` enables pipeline
reuse, typed response header models (`includeResponseHeadersInModels: true`),
and lenient JSON boolean parsing. Generated files live in `src/generated`,
under `Azure::Storage::Files::DataLake::_detail`.

`src/datalake_protocol.cpp` adapts generated options and results to the existing
SDK models, decodes checksum headers, and translates HTTP failures through the
existing Storage error parser. Public clients retain their paging, idempotency,
access conditions, service-version policy, retry behavior and caller-owned body
streams. A private pipeline policy preserves the legacy absence of `Accept` and
append `Content-Type` headers before user policies run.

The public headers, including `inc/azure/storage/files/datalake/rest_client.hpp`,
are retained for source compatibility. Do not replace them with generated
headers or manually edit generated C++.

## C++ customizations

`client.tsp` uses language-scoped decorators without editing the upstream
specification or other languages' projections:

- Operations already delegated to Blob Storage or not used by the SDK are hidden.
- A replacement path-item model reads the service's lowercase JSON `etag`.
- ETag and Last-Modified response headers are nullable because ListPaths does not
  return them. This applies to all C++ operations using the shared aliases;
  adapters still require them for create, flush and other legacy operations.
- Creation and expiry response headers are nullable, matching the public models
  and their absence from access-control responses.
- Binary checksum response headers are strings, decoded in the adapters.

The generated calls handle a single page; existing handwritten pagers remain
responsible for continuation tokens. Downloads continue to use Blob Storage.

## Regeneration

Use a clean checkout or snapshot of
[`Azure/typespec-cpp`](https://github.com/Azure/typespec-cpp) at commit
`8b90d07e9621bcb53bfc4082f9ddd472b7fca1e4`, or a revision containing those changes.
Uncommitted emitter experiments and the unmodified published emitter are not
supported inputs. Build `packages/codegen.cpp` first, then
`packages/typespec-cpp`, using their package manifests. The generation toolchain
uses TypeSpec compiler 1.12.0 and client-generator-core 0.68.4.

Provide the DataLakeStorage directory from the exact specification revision in
`tsp-location.yaml`. From this package directory, run:

```powershell
.\typespec\generate.ps1 `
  -EmitterPath C:\path\to\typespec-cpp\packages\typespec-cpp `
  -SpecDirectory C:\path\to\azure-rest-api-specs\specification\storage\data-plane\DataLakeStorage `
  -ClangFormatPath "C:\Program Files\LLVM\bin\clang-format.exe"
```

The script generates and formats the five files with clang-format 11.0.0 in a
temporary directory before copying them into the package. Use `-OutputDirectory`
to regenerate elsewhere for comparison.

After regeneration, build the package, run the credential-free
`DataLakeProtocolTest.*` tests, and run the existing Data Lake playback suite.
Verify that all public headers remain unchanged.
