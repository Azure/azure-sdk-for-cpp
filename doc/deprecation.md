# Deprecating Azure SDK for C++ Packages

This document provides guidance on how to deprecate packages, APIs, and features in the Azure SDK for C++.

## Overview

The Azure SDK follows a structured lifecycle management approach to ensure predictable support and migration paths for customers. Understanding and following the deprecation process is crucial for maintaining a stable and reliable SDK ecosystem.

## Support Lifecycle and Deprecation Policy

The Azure SDK for C++ adheres to the Azure SDK-wide support and deprecation policies. For comprehensive information about package lifecycle stages (Beta, Active, Deprecated, Community), support timelines, and deprecation policies, please refer to:

- [Azure SDK Support Policy](https://azure.github.io/azure-sdk/policies_support.html) - Defines the support lifecycle stages and timelines
- [Azure SDK Releases Policy](https://azure.github.io/azure-sdk/policies_releases.html) - Covers versioning and breaking change guidelines
- [Azure SDK Deprecated Releases](https://azure.github.io/azure-sdk/releases/deprecated/index.html) - Lists currently deprecated SDK packages

### Key Points

- **Deprecated packages receive only critical bug and security fixes for at least 12 months** after deprecation
- **Breaking changes may extend the support window to 3 years** for critical fixes
- **Migration guides must be provided** when deprecating packages or introducing breaking changes
- **Advance notice is required** before deprecating any stable package or API

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

// Deprecating a type alias
using OldType [[deprecated("Use NewType instead")]] = NewType;

// Deprecating a member
struct MyStruct {
    [[deprecated("Use NewField instead")]]
    int OldField;
};
```

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
 * @deprecated This function is deprecated as of version 2.0.0.
 * Use NewFunction() instead for improved performance and better error handling.
 * 
 * @param input The input parameter.
 * @return The result of the operation.
 */
[[deprecated("Use NewFunction() instead. This function will be removed in version 3.0.0.")]]
int OldFunction(int input);
```

## Deprecating an Entire Package

When an entire package needs to be deprecated (usually because it's being replaced by a new package or the underlying service is being retired):

### 1. Update Package Metadata

Update the package's vcpkg port information to mark it as deprecated:

- Add deprecation notice to `vcpkg.json` if applicable
- Update the package description to indicate deprecation status

### 2. Update Documentation

Update the following documentation:

- **README.md**: Add a prominent deprecation notice at the top of the package README
- **CHANGELOG.md**: Add an entry documenting the deprecation, the reason, and the migration path
- Package documentation on [azure.github.io](https://azure.github.io/azure-sdk-for-cpp)

Example README notice:

```markdown
# Azure [Service] SDK for C++

> **IMPORTANT: This package has been deprecated and will no longer receive updates after [DATE].**
>
> Please migrate to [azure-new-package-cpp] which provides [benefits].
> See the [Migration Guide](link-to-migration-guide.md) for detailed instructions.
```

### 3. Create a Migration Guide

Provide a comprehensive migration guide that includes:

- **Why the package is being deprecated**: Explain the business or technical reasons
- **What replaces it**: Clearly identify the replacement package or alternative approach
- **Migration steps**: Provide step-by-step instructions for migrating code
- **Breaking changes**: Document all API changes and incompatibilities
- **Code examples**: Show side-by-side comparisons of old vs. new code
- **Timeline**: Specify the deprecation date and end-of-support date

See [sdk/storage/MigrationGuide.md](../sdk/storage/MigrationGuide.md) for an example of a migration guide.

### 4. Communicate the Deprecation

- Announce the deprecation through appropriate channels (GitHub discussions, blog posts, etc.)
- Update the [Azure SDK Deprecated Releases](https://azure.github.io/azure-sdk/releases/deprecated/index.html) page
- Consider adding the deprecation notice to package manager listings

### 5. Maintain During Deprecation Period

During the deprecation period (minimum 12 months for stable packages):

- Continue to address critical security vulnerabilities
- Apply critical bug fixes as needed
- Do not add new features
- Direct users to the replacement package in issue responses

## Versioning Considerations

When deprecating APIs or packages, follow semantic versioning principles:

- **Marking an API as deprecated is not a breaking change** (can be done in a minor version)
- **Removing a deprecated API is a breaking change** (requires a major version bump)
- **Deprecating an entire package typically occurs when releasing a new major version** of its replacement

## Best Practices

1. **Provide ample notice**: Announce deprecations well in advance of the end-of-support date
2. **Offer migration paths**: Never deprecate without providing a clear alternative
3. **Minimize disruption**: Keep deprecated APIs functional as long as possible
4. **Document thoroughly**: Ensure all deprecation information is clear and accessible
5. **Be consistent**: Follow the same deprecation patterns across all packages
6. **Gather feedback**: Listen to customer concerns and adjust timelines if reasonable
7. **Test migrations**: Verify that migration guides are accurate and complete

## Example: Deprecation Workflow

Here's a typical workflow for deprecating an API:

1. **Identify the API to deprecate** and determine the replacement
2. **Add `[[deprecated]]` attribute** with a helpful message
3. **Update Doxygen comments** with `@deprecated` tag
4. **Update CHANGELOG.md** with deprecation notice
5. **Create or update migration documentation**
6. **Release in a minor version** (marking as deprecated)
7. **Wait for the deprecation period** (typically one major version cycle)
8. **Remove the API in the next major version**
9. **Update CHANGELOG.md** to note the removal

## References

- [Azure SDK Design Guidelines for C++](https://azure.github.io/azure-sdk/cpp_introduction.html)
- [Azure SDK Support Policy](https://azure.github.io/azure-sdk/policies_support.html)
- [Azure SDK Releases Policy](https://azure.github.io/azure-sdk/policies_releases.html)
- [Azure SDK Repository Branching and Tagging](https://github.com/Azure/azure-sdk/blob/main/docs/policies/repobranching.md)
- [Microsoft Lifecycle Policy](https://learn.microsoft.com/lifecycle/policies/modern)

## Getting Help

If you have questions about deprecation policies or need assistance with deprecating a package:

- File an issue in the [Azure SDK for C++ repository](https://github.com/Azure/azure-sdk-for-cpp/issues)
- Consult the [Azure SDK Architecture Board](https://github.com/Azure/azure-sdk) for guidance
- Review the [Contributing Guide](../CONTRIBUTING.md) for more information
