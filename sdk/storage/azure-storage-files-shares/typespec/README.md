# File Shares protocol generation

The private protocol is generated from the FileStorage TypeSpec revision pinned
in `tsp-location.yaml`. The C++ projection selects API version `2026-10-06`,
matching the existing SDK, including paginated range listing.

`includeResponseHeadersInModels: true` explicitly enables the typed response
headers consumed by the compatibility layer.

Generated files live in `src/generated` under
`Azure::Storage::Files::Shares::_detail`. The generated client is named
`FileSharesClient` to avoid colliding with the retained legacy private `FileClient`.
`src/shares_protocol.cpp` implements the existing private protocol signatures as
adapters to generated operations and results. The borrowed pipeline is used only
during each synchronous generated call; the adapters never take ownership of it.

Public headers, including `inc/azure/storage/files/shares/rest_client.hpp`, are
retained unchanged. Public enum constants and `FileAttributes` implementations
live in `src/share_responses.cpp`. Do not replace public headers with generated
headers or manually edit generated C++.

## Compatibility customizations

`typespec/client.tsp` is a package-local C++ projection, without modifying the
upstream specification:

- Metadata is a case-insensitive prefix header collection (`x-ms-meta-`).
- Request timestamps remain strings, preserving `now`, `preserve`, and the exact
  precision supplied by the handwritten clients.
- Quotas, usage bytes, and link counts retain their existing 64-bit representation.
- Binary checksum response headers are strings, decoded in the adapters.
- ETag and Last-Modified response headers are nullable because some operations,
  including ACL and statistics queries, do not require them. Adapters still
  require these headers wherever the legacy operation required them.
- Range requests use the existing `x-ms-range` header.

`pipelineMode: reuse` preserves the existing authentication, retries, service
version, and user policies. A private policy removes generated `Accept` and binary
`Content-Type` defaults and preserves the XML charset before user policies run.
HTTP errors are translated through the Storage error parser.

`binaryResponseBodyMode: stream` keeps downloads unbuffered. Uploads borrow the
caller-owned stream, including retries. XML and JSON bodies are still buffered
and parsed in the generated layer. The adapters retain SMB/NFS properties,
snapshot targeting, checksum handling, and the existing handwritten pagers.

## Regeneration

Use a clean checkout or snapshot of
[`Azure/typespec-cpp`](https://github.com/Azure/typespec-cpp) containing commit
`f81e14cd`, including fixed-precision XML date-times and the XML unwrapped-scalar
response parsing fix introduced during this migration. Older published emitters
and unrelated local experiments are not supported generation inputs.

Build `packages/codegen.cpp` and then `packages/typespec-cpp` using their package
manifests. The generation toolchain uses TypeSpec compiler 1.12.0 and
client-generator-core 0.68.4. Supply the FileStorage directory from the exact
specification revision in `tsp-location.yaml`, then run from this package:

```powershell
.\typespec\generate.ps1 `
  -EmitterPath C:\path\to\typespec-cpp\packages\typespec-cpp `
  -SpecDirectory C:\path\to\azure-rest-api-specs\specification\storage\data-plane\FileStorage `
  -ClangFormatPath "C:\Program Files\LLVM\bin\clang-format.exe"
```

The script generates and formats the five files with clang-format 11.0.0 in a
temporary directory before copying them into the package. Use `-OutputDirectory`
to regenerate elsewhere for comparison.

After regeneration, build `azure-storage-files-shares-test`, run the
credential-free `SharesProtocolTest.*` cases and the existing File Shares
PLAYBACK suite, and verify that public headers remain unchanged.
