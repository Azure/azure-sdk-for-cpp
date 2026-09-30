---
name: code-review
description: "Review Azure SDK for C++ pull requests for actionable regressions and C++ SDK API or implementation guideline violations. Use for GitHub Copilot Code Review (CCR) of SDK code and its related tests, CMake files, samples, and documentation."
license: MIT
metadata:
  version: "1.0.0"
---

# Azure SDK for C++ code review

Review changes to packages under `sdk/` and their related tests, build files, samples, and documentation. Apply the checks below only where they fit the changed component: REST service-client requirements do not automatically apply to AMQP, other non-HTTP protocols, or repository tooling. Use `AGENTS.md` and nearby package code to establish existing conventions. The relevant rules from both C++ guideline pages are included here so the review does not depend on following external links.

## Review approach

1. Trace each changed behavior through its callers, public headers, implementation, and tests. Distinguish new defects from pre-existing behavior and intentional API changes from accidental breaking changes.
2. Comment only on an introduced, demonstrable bug or a clear, consequential API/design guideline violation. Point to the changed line, describe the affected scenario and impact, and suggest a feasible correction. Prioritize compatibility, correctness, reliability, and credential safety over formatting or minor style.
3. Do not raise speculative, duplicate, or style-only findings. Do not apply checklist items blindly to generated code or require a new SDK design when the package already follows a different established pattern.

## Public API and client design

- For changed public headers, check source compatibility and semantic versioning, namespace and type stability, and appropriate API review for intentional breaking changes. Service-specific public types belong under `Azure::<Group>::<Service>`; follow existing package namespaces for shared libraries. Keep private implementation in `_detail`, and use `_internal` for shared Azure Core internals rather than exposing implementation types as public API.
- For new HTTP service clients, prefer a discoverable `*Client` class in the package namespace with thread-safe, effectively immutable state and `const` service methods. Provide a minimal constructor with only required parameters (no default arguments), an options overload for configuration, and `GetUrl()` where applicable. Do not allow subsequent configuration changes or build-time differences to alter a constructed client's behavior.
- For new REST service methods, check the synchronous `Response<T>` or `Response` contract for single-response calls; put `const Azure::Core::Context& context = {}` last and pass that context by reference to downstream calls. Do not cancel the caller's context. Paged results must indicate when iteration ends and expose returned items/count; methods starting long-running operations use the `Start` prefix and return an `Operation<T>` subtype.
- For changed models and options, use `Azure::Core::ETag` and `Azure::Core::Uri` where appropriate. Use `enum class` for fixed sets, but preserve unknown values for service-extensible wire enumerations with the package's extendable-enum pattern. Flag changes that prevent round-tripping future service values.
- Document new or changed public/protected types and members with Doxygen comments, including meaningful error and cancellation behavior. Check that new customer-facing operations have usable samples or examples.

## Implementation and C++ correctness

- Route REST requests through the Azure Core HTTP pipeline so standard retry, authentication, logging, tracing, telemetry, and proxy behavior remain intact. Reuse Core policies where possible; custom policies must be thread-safe, with per-request state kept out of shared policy instances.
- Validate arguments the SDK itself consumes (for example, a locally used path or buffer); let the service validate parameters passed through to it. Check that failed HTTP requests and long-running operations produce actionable Azure Core exceptions rather than success-shaped results, and that exceptions from user callbacks are not swallowed.
- Preserve cancellation propagation through nested calls. Check new retry or paging paths for unintended duplicate operations, missed termination, and incorrect response metadata.
- Use Azure Core mechanisms for logging and credentials. Do not expose secrets or personal data in logs or telemetry: log header/query values only when explicitly allow-listed, and ensure new credential handling does not persist sensitive material or break refresh.
- Check ownership and lifetimes, initialized invariants, const-correctness, and concurrent client use. Keep library code C++14-compatible and portable across MSVC, GCC, Clang, Windows, Linux, and macOS; check HTTP changes against both curl and WinHTTP transports where relevant. Use 64-bit representations for file sizes and service integers, and suitable buffer-size types for in-memory buffers when conversions could lose data.
- Avoid new third-party types in public Azure headers. Scrutinize new runtime dependencies and CMake/export changes for consumer compatibility; nonstandard dependencies require architecture review rather than silently broadening the package's dependency surface.

## Tests, generated code, and documentation

- Look for focused Google Test coverage of changed behavior, especially a concrete failure case, cancellation/retry path, or previously broken edge case. Prefer existing PLAYBACK infrastructure for service tests; do not require live credentials or demand tests for unchanged behavior.
- Check affected CMake targets and package documentation when behavior or APIs change: Doxygen, README, samples, and CHANGELOG should agree with the shipped behavior. Raise a missing-coverage finding only when it conceals a specific regression or makes a new API unusable.
- Review generated SDK diffs for real behavioral or public API regressions, but direct proposed fixes to their TypeSpec source or generator/customization workflow; do not suggest hand-editing files identified as generated, whether headers or `src/private/` output.

## Guideline sources

- [Azure SDK C++ API design guidelines](https://azure.github.io/azure-sdk/cpp_introduction.html)
- [Azure SDK C++ implementation guidelines](https://azure.github.io/azure-sdk/cpp_implementation.html)
