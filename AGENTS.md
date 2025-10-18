# AGENTS.md

This file provides guidance for AI agents (e.g., GitHub Copilot, LLM-based assistants, and automation tools) interacting with the Azure SDK for C++ repository.

## Repository Purpose and Scope

The **Azure SDK for C++** provides modern C++ libraries for Azure services, following the [Azure SDK Design Guidelines for C++](https://azure.github.io/azure-sdk/cpp_introduction.html). This repository contains:

- **Client libraries** for Azure services (Storage, Key Vault, Identity, etc.)
- **Core libraries** (`azure-core`) providing shared functionality (HTTP pipeline, authentication, retry policies, etc.)
- **Build infrastructure** using CMake and vcpkg
- **Documentation** and samples for developers
- **Test infrastructure** including unit tests and integration tests

### Key Technologies
- **Language**: C++14 or higher
- **Build System**: CMake (version 3.13+)
- **Package Manager**: vcpkg (manifest mode)
- **Testing**: Google Test (gtest)
- **Documentation**: Doxygen
- **CI/CD**: Azure Pipelines

## Agent Capabilities and Boundaries

### Supported Agent Actions

AI agents working in this repository are encouraged to:

1. **Code Contributions**
   - Implement new features following the [C++ Guidelines](https://azure.github.io/azure-sdk/cpp_introduction.html)
   - Fix bugs with minimal, surgical changes
   - Add or update unit tests using Google Test framework
   - Improve code documentation using Doxygen-style comments

2. **Documentation**
   - Update README files
   - Improve inline code comments
   - Update CONTRIBUTING.md when development processes change
   - Generate or update API documentation

3. **Build and Test**
   - Run CMake builds locally
   - Execute unit tests via `ctest`
   - Generate code coverage reports (when BUILD_CODE_COVERAGE is ON)
   - Build samples (when BUILD_SAMPLES is ON)

4. **Code Review and Analysis**
   - Review pull requests for adherence to guidelines
   - Suggest improvements to code structure and style
   - Identify potential issues before CI runs

### Actions Requiring Caution

1. **Dependency Management**
   - Changing vcpkg.json should be done sparingly
   - New dependencies require justification and approval
   - OpenSSL version changes need careful consideration
   - Consult `vcpkg-custom-ports` for custom dependency versions

2. **Breaking Changes**
   - Public API changes must follow semver and SDK guidelines
   - Internal types (in `_internal` namespace) can change within constraints
   - Private types (in `_detail` namespace) have fewer restrictions

3. **CI/CD Configuration**
   - Changes to `.github/workflows/` require testing
   - Azure Pipeline configurations need validation
   - Test proxy configuration changes need verification

### Actions Outside Agent Scope

Agents should **NOT**:

1. **Modify Security-Critical Code** without explicit review
   - Authentication flows
   - Credential handling
   - Cryptographic operations

2. **Make Large Architectural Changes** without design review
   - Changing core abstractions
   - Modifying HTTP pipeline architecture
   - Restructuring repository layout

3. **Commit Secrets or Sensitive Data**
   - No credentials in code or config files
   - No API keys in tests (use environment variables)
   - No connection strings in source

## Key Workflows and Commands

### Building the Project

```bash
# Basic build
mkdir build && cd build
cmake ..
cmake --build .

# Build with tests
cmake -DBUILD_TESTING=ON ..
cmake --build .

# Build with samples
cmake -DBUILD_SAMPLES=ON ..
cmake --build .

# Build with specific transport adapter
cmake -DBUILD_TRANSPORT_CURL=ON ..
cmake --build .
```

### Running Tests

```bash
# Set test mode (LIVE, PLAYBACK, or RECORD)
export AZURE_TEST_MODE=PLAYBACK

# Run all tests
ctest -V

# Run specific package tests
ctest -R azure-core
ctest -R storage

# Run with test proxy (for recording/playback)
export AZURE_TEST_USE_TEST_PROXY=ON
ctest -V
```

### Code Coverage

```bash
# Build with coverage enabled (requires Debug mode and GNU compiler)
cmake -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug -DBUILD_CODE_COVERAGE=ON ..
cmake --build .

# Generate coverage reports
make azure-core_cov_xml    # XML report
make azure-core_cov_html   # HTML report
```

### Documentation Generation

```bash
# Build Doxygen documentation
cmake -DBUILD_DOCUMENTATION=ON ..
cmake --build .
```

### Code Formatting

```bash
# Format code using clang-format
# The repository includes a .clang-format configuration
find sdk/ -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
```

## Repository Structure

```
azure-sdk-for-cpp/
+-- .github/              # GitHub configuration and workflows
    +-- copilot-instructions.md  # Copilot-specific guidance
    +-- workflows/        # CI/CD workflows
+-- cmake-modules/        # CMake helper modules
+-- doc/                  # Documentation
+-- eng/                  # Engineering system scripts
    +-- common/           # Shared scripts across Azure SDKs
    +-- docs/             # Documentation generation
+-- samples/              # Sample applications
+-- sdk/                  # SDK service libraries
    +-- core/             # Core libraries (azure-core, azure-core-amqp, etc.)
    +-- storage/          # Storage services (blobs, files, queues)
    +-- identity/         # Authentication and identity
    +-- keyvault/         # Key Vault (keys, secrets, certificates)
    +-- template/         # Template for new services
+-- tools/                # Development tools
+-- vcpkg.json            # Package dependencies manifest
+-- CMakeLists.txt        # Root CMake configuration
+-- CONTRIBUTING.md       # Contribution guidelines
+-- AGENTS.md             # This file
```

## SDK-Specific Automation Workflows

### 1. Code Generation

Some SDK components use AutoRest or other code generators:

- **Swagger/OpenAPI**: Located in `sdk/{service}/swagger/README.md`
- **Protocol Layer**: Auto-generated code in `src/private/` directories
- **Customizations**: Defined via directives in swagger README files

**Agent Guidance**: Do not manually edit generated files. Modify swagger configurations and regenerate.

### 2. API Review Process

Public API changes require:

1. Update to public header files
2. Doxygen documentation for new APIs
3. Unit tests covering new functionality
4. Sample code demonstrating usage
5. API review approval (tracked externally)

**Agent Guidance**: Flag API changes for human review. Include APIView links when available.

### 3. Test Matrix

Tests run across multiple configurations:

- **Platforms**: Windows, Linux (Ubuntu), macOS
- **Compilers**: MSVC, GCC, Clang
- **Build Types**: Debug, Release
- **Transport Adapters**: libcurl, WinHTTP
- **Test Modes**: LIVE, PLAYBACK, RECORD

**Agent Guidance**: Ensure changes work across platforms. Test with both curl and WinHTTP when modifying HTTP layer.

### 4. Versioning and Releases

- **Semantic Versioning**: Libraries follow semver (MAJOR.MINOR.PATCH)
- **Version Files**: `src/private/package_version.hpp` in each package
- **Release Tags**: Format `{package-name}_{version}` (e.g., `azure-core_1.10.0`)
- **Changelogs**: `CHANGELOG.md` in each package directory

**Agent Guidance**: Update version files and changelogs when making changes. Follow [branching strategy](https://github.com/Azure/azure-sdk/blob/main/docs/policies/repobranching.md).

## Testing Guidelines

### Test Environment Setup

Tests require environment variables for authentication and configuration:

```bash
# Example for Storage tests
export STANDARD_STORAGE_CONNECTION_STRING="..."
export AZURE_STORAGE_ACCOUNT_URL="..."
export AZURE_STORAGE_ACCOUNT_NAME="..."
export AZURE_STORAGE_ACCOUNT_KEY="..."
```

See [sdk/core/ci.yml](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/core/ci.yml) for required configuration per package.

### Test Modes

- **PLAYBACK**: Use recorded responses (no network, no credentials needed for actual services)
- **LIVE**: Connect to real Azure services (requires valid credentials)
- **RECORD**: Capture responses for playback (requires test proxy)

**Agent Guidance**: Use PLAYBACK mode for local development. Use RECORD only when updating test recordings.

### Test Proxy

The test proxy (`test-proxy`) enables recording and playback:

```bash
# Start test proxy automatically
export AZURE_TEST_USE_TEST_PROXY=ON

# Or start manually
dotnet tool install azure.sdk.tools.testproxy --global
test-proxy start
```

See [doc/TestProxy.md](https://github.com/Azure/azure-sdk-for-cpp/blob/main/doc/TestProxy.md) for details.

## Common Development Scenarios

### Adding a New Service Library

1. Copy `sdk/template/azure-template/` as starting point
2. Update CMakeLists.txt with new package name
3. Implement service client following SDK guidelines
4. Add unit tests and samples
5. Generate documentation
6. Update root CMakeLists.txt to include new package

**Agent Guidance**: Follow the template structure. Maintain consistency with existing packages.

### Fixing a Bug

1. Add a failing unit test that reproduces the bug
2. Make minimal changes to fix the issue
3. Verify test passes and no regressions
4. Update CHANGELOG.md
5. Submit PR with clear description

**Agent Guidance**: Keep changes surgical. One bug = one PR.

### Adding a Feature

1. Review API design guidelines
2. Create issue or spec doc for large features
3. Implement with tests and documentation
4. Add samples demonstrating usage
5. Update package version and CHANGELOG

**Agent Guidance**: Get design approval before implementing. Include usage examples.

## Cross-References

- **Copilot-Specific Instructions**: [.github/copilot-instructions.md](https://github.com/Azure/azure-sdk-for-cpp/blob/main/.github/copilot-instructions.md)
- **Contribution Guide**: [CONTRIBUTING.md](https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md)
- **API Design Guidelines**: <https://azure.github.io/azure-sdk/cpp_introduction.html>
- **Test Proxy Documentation**: [doc/TestProxy.md](https://github.com/Azure/azure-sdk-for-cpp/blob/main/doc/TestProxy.md)
- **Performance Testing**: [doc/PerformanceTesting.md](https://github.com/Azure/azure-sdk-for-cpp/blob/main/doc/PerformanceTesting.md)
- **Distributed Tracing**: [doc/DistributedTracing.md](https://github.com/Azure/azure-sdk-for-cpp/blob/main/doc/DistributedTracing.md)

## Additional Resources

- **Developer Documentation**: <https://azure.github.io/azure-sdk-for-cpp>
- **Azure for C++ Developers**: <https://learn.microsoft.com/azure/>
- **SDK Design Guidelines**: <https://azure.github.io/azure-sdk/cpp_introduction.html>
- **Azure SDK Repository Policies**: <https://github.com/Azure/azure-sdk>
- **Filing Issues**: <https://github.com/Azure/azure-sdk-for-cpp/issues/new/choose>

## Agent Best Practices

1. **Understand Before Changing**: Review existing code and patterns before making changes
2. **Follow Conventions**: Maintain consistency with existing code style and structure
3. **Test Thoroughly**: Run tests locally before submitting changes
4. **Document Changes**: Update comments, docs, and changelogs
5. **Keep Changes Minimal**: Make surgical changes; avoid unnecessary refactoring
6. **Seek Review**: Complex changes should be reviewed by maintainers
7. **Check CI**: Ensure all CI checks pass before merging

## Metadata

- **Repository**: <https://github.com/Azure/azure-sdk-for-cpp>
- **Language**: C++14+
- **License**: MIT
- **Maintainers**: Azure SDK Team
- **Last Updated**: 2025-01-09

---

This file follows the [AGENTS.md specification](https://agents.md) for standardizing AI agent interactions with repositories.

For Copilot-specific instructions, see [.github/copilot-instructions.md](https://github.com/Azure/azure-sdk-for-cpp/blob/main/.github/copilot-instructions.md).
