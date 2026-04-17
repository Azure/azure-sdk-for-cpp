# Deprecating APIs and Packages in the Azure SDK for C++

This document provides C++-specific procedural guidance for deprecating packages, APIs, and features in the Azure SDK for C++. The normative policy is defined by the Azure SDK-wide documents linked below; this document describes *how* to apply that policy in this repository.

## Overview

The Azure SDK follows a structured lifecycle management approach to ensure predictable support and migration paths for customers. Understanding and following the deprecation process is crucial for maintaining a stable and reliable SDK ecosystem.

## Policy Summary

The authoritative policy lives in the Azure SDK-wide documents. Read these first:

- [Azure SDK Support Policy](https://azure.github.io/azure-sdk/policies_support.html) — support lifecycle stages (Beta, Active, Deprecated, Community) and timelines
- [Azure SDK Releases Policy](https://azure.github.io/azure-sdk/policies_releases.html) — versioning and breaking change guidelines
- [Azure SDK Deprecated Releases](https://azure.github.io/azure-sdk/releases/deprecated/index.html) — currently deprecated SDK packages

Key points that drive the C++-specific process below:

- Deprecated packages receive only critical bug and security fixes for at least 12 months after deprecation.
- Breaking changes may extend the support window to 3 years for critical fixes.
- A migration guide must accompany any package deprecation or breaking API change.
- Stable packages and APIs require advance notice before deprecation; see the support policy for the current minimum notice period.

## Scope: what this process applies to

This process applies to **public** APIs — types, functions, and headers that customers are expected to consume directly.

- Types in an `_internal` namespace are intended for use by other Azure SDK packages. They follow a lighter-weight contract: breaking changes are allowed between minor versions but should still be announced in the package `CHANGELOG.md`.
- Types in a `_detail` namespace are implementation details and may change at any time without deprecation.

When in doubt, treat a symbol as public.

## Deprecating APIs in C++

When deprecating individual APIs, functions, classes, or other code elements within an active package, use C++ standard deprecation attributes.

### Using [[deprecated]] Attribute

The C++14 `[[deprecated]]` attribute is the standard way to mark APIs as deprecated:

```cpp
// Simple deprecation
[[deprecated]]
void OldFunction();

// Deprecation with message
[[deprecated("Use NewFunction() instead")]]
void OldFunction();

// Deprecating a class
class [[deprecated("Use NewClass instead")]] OldClass {
    // ...
};

// Deprecating a type alias. Note: the alias still resolves to the *existing*
// legacy type so that existing callers continue to compile; consumers should
// migrate to NewType.
using OldType [[deprecated("Use NewType instead")]] = LegacyType;

// Deprecating a member
struct MyStruct {
    [[deprecated("Use NewField instead")]]
    int OldField;
};
```

> [!NOTE]
> Adding `[[deprecated]]` is not a breaking change under the Azure SDK release
> policy, but it emits a warning. Customers who build with warnings-as-errors
> (`-Werror` / `/WX`) may need to suppress `-Wdeprecated-declarations` (GCC,
> Clang) or C4996 (MSVC) until they migrate. Call this out in the changelog
> entry for the release that introduces the deprecation.

### Handling deprecation warnings inside the SDK

Deprecating a public API will cause the SDK's own unit tests and samples that
exercise that API to emit warnings, which can fail CI builds that treat
warnings as errors. Prefer, in order:

1. Migrate the internal callers (tests, samples, sibling packages) to the
   replacement API.
2. If a test must continue to exercise the deprecated API to verify behavior,
   wrap *only* that call site in a diagnostic push/pop:

   ```cpp
   #if defined(_MSC_VER)
   #pragma warning(push)
   #pragma warning(disable : 4996)
   #elif defined(__GNUC__) || defined(__clang__)
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
   #endif
       auto result = OldFunction(input);
   #if defined(_MSC_VER)
   #pragma warning(pop)
   #elif defined(__GNUC__) || defined(__clang__)
   #pragma GCC diagnostic pop
   #endif
   ```

Do not disable the warning globally for the package or the test binary.

### Doxygen Documentation

When deprecating an API, always add a `@deprecated` Doxygen comment that:
1. Explains why the API is deprecated
2. Provides the replacement API or alternative approach
3. Indicates when the API was deprecated (version number)

Example:

```cpp
/**
 * @brief Performs an old operation.
 *
 * @deprecated Deprecated in 1.8.0. Use NewFunction() instead for improved
 * performance and better error handling. This function will be removed in
 * 2.0.0.
 *
 * @param input The input parameter.
 * @return The result of the operation.
 */
[[deprecated("Use NewFunction() instead. Will be removed in 2.0.0.")]]
int OldFunction(int input);
```

Use the standard `[[deprecated]]` attribute directly. Do not introduce
package-specific deprecation macros.

## Versioning and ABI Considerations

When deprecating APIs or packages, follow semantic versioning principles:

- Marking an API as deprecated is not a breaking change and can ship in a minor version.
- Removing a deprecated API is a breaking change and requires a major version bump.
- Deprecating an entire package typically occurs alongside the first major release of its replacement.

Because the Azure SDK for C++ is consumed as both static and shared libraries,
removing or changing the signature of an exported symbol is an ABI break even
if source compatibility is preserved via overloads. Treat signature changes on
exported (`AZ_*_DLLEXPORT`) symbols as breaking changes.

## Deprecating an Entire Package

When an entire package needs to be deprecated (usually because it is being replaced by a new package or the underlying service is being retired), complete the code and documentation changes below, then follow the communication steps.

### Code and documentation changes

1. **Package metadata**
   - Update the `description` field in the package's `vcpkg.json` to begin with `[DEPRECATED]` and point to the replacement.
   - Open a PR against the [vcpkg registry](https://github.com/microsoft/vcpkg) to mark the port deprecated and/or redirect consumers to the replacement port. vcpkg does not have a standalone `deprecated` manifest field; the convention is to communicate status via the port description and the registry PR.
2. **README.md** — add a prominent deprecation notice at the top of the package README. Example:

   ```markdown
   # Azure [Service] SDK for C++

   > [!IMPORTANT]
   > This package has been deprecated and will no longer receive updates after [DATE].
   > Please migrate to [azure-new-package-cpp] which provides [benefits].
   > See the [Migration Guide](link-to-migration-guide.md) for detailed instructions.
   ```

3. **CHANGELOG.md** — add an entry in the next release section. Use this template:

   ```markdown
   ## <next-version> (YYYY-MM-DD)

   ### Other Changes

   - This package is deprecated as of <version> and will receive only critical
     bug and security fixes until <end-of-support-date>. Migrate to
     [`<replacement-package>`](<link>). See the
     [migration guide](<link>) for details.
   ```

4. **Migration guide** — create or update a `MigrationGuide.md` in the package
   directory that includes:

   - Why the package is being deprecated (business or technical rationale).
   - The replacement package or alternative approach.
   - Step-by-step migration instructions.
   - All breaking changes and incompatibilities.
   - Side-by-side before/after code examples.
   - The deprecation date and the end-of-support date.

   See [sdk/storage/MigrationGuide.md](../sdk/storage/MigrationGuide.md) for an
   example.

5. **Published documentation** — ensure the deprecation notice surfaces on
   [azure.github.io/azure-sdk-for-cpp](https://azure.github.io/azure-sdk-for-cpp)
   once the release ships.

### Process and communication

1. Announce the deprecation through the appropriate channels (release notes,
   GitHub Discussions, blog posts).
2. Submit the package to the
   [Azure SDK Deprecated Releases](https://azure.github.io/azure-sdk/releases/deprecated/index.html)
   list.
3. Direct users to the replacement package in issue responses for the duration
   of the deprecation window.

### Maintenance during the deprecation period

During the deprecation period (minimum 12 months for stable packages):

- Address critical security vulnerabilities.
- Apply critical bug fixes as needed.
- Do not add new features.
- Do not accept non-critical PRs that extend the API surface.

## Best Practices

1. **Offer a migration path.** Never deprecate without identifying a concrete replacement.
2. **Minimize disruption.** Keep deprecated APIs functional for the full support window.
3. **Document thoroughly.** Deprecation notices belong in the attribute message, the Doxygen comment, the `CHANGELOG.md`, and the package `README.md`.
4. **Be consistent.** Follow the same patterns across every package in this repo.
5. **Migrate internal callers first.** Build the deprecated package with `-Wdeprecated-declarations` treated as an error in CI for at least one release cycle so that stray internal usages surface before customers see them.
6. **Verify migration guides.** Compile the before/after examples as part of CI or samples where feasible.
7. **Listen to feedback.** Adjust timelines when customer input justifies it, within the bounds of the Azure SDK support policy.

## Worked Example: Deprecating an Overload

Suppose a service client exposes a method that takes a raw connection string,
and a new overload accepts a strongly-typed options struct. The old overload is
being deprecated in 1.8.0 and will be removed in 2.0.0.

**Before (1.7.x):**

```cpp
class WidgetClient {
public:
  Response<Widget> GetWidget(std::string const& connectionString, std::string const& id);
};
```

**After (1.8.0, deprecation released):**

```cpp
class WidgetClient {
public:
  /**
   * @brief Gets a widget by id.
   *
   * @deprecated Deprecated in 1.8.0. Use the overload that takes
   * `GetWidgetOptions` instead. Will be removed in 2.0.0.
   */
  [[deprecated("Use GetWidget(std::string const&, GetWidgetOptions const&). "
               "Will be removed in 2.0.0.")]]
  Response<Widget> GetWidget(std::string const& connectionString, std::string const& id);

  Response<Widget> GetWidget(std::string const& id, GetWidgetOptions const& options = {});
};
```

**Checklist for the 1.8.0 release:**

1. Add `[[deprecated]]` attribute and `@deprecated` Doxygen tag.
2. Migrate in-repo callers (tests, samples, sibling packages) to the new overload.
3. Add a `### Other Changes` entry to `CHANGELOG.md` describing the deprecation and pointing at the migration guide.
4. Ship as a minor release.

**Checklist for the 2.0.0 release:**

1. Remove the deprecated overload.
2. Add a `### Breaking Changes` entry to `CHANGELOG.md`.
3. Update the migration guide to mark the API removed.
4. Bump the package major version via the scripts in [eng/scripts/](../eng/scripts/).

## References

- [Azure SDK Design Guidelines for C++](https://azure.github.io/azure-sdk/cpp_introduction.html)
- [Azure SDK Support Policy](https://azure.github.io/azure-sdk/policies_support.html)
- [Azure SDK Releases Policy](https://azure.github.io/azure-sdk/policies_releases.html)
- [Azure SDK Repository Branching and Tagging](https://github.com/Azure/azure-sdk/blob/main/docs/policies/repobranching.md)
- [Microsoft Lifecycle Policy](https://learn.microsoft.com/lifecycle/policies/modern)

## Getting Help

If you have questions about deprecation policies or need assistance with deprecating a package:

- File an issue in the [Azure SDK for C++ repository](https://github.com/Azure/azure-sdk-for-cpp/issues).
- Open a discussion in the cross-language [azure-sdk](https://github.com/Azure/azure-sdk) repository for policy questions.
- Review the [Contributing Guide](../CONTRIBUTING.md) for repository conventions.
