# Repository Scripts

This directory contains utility scripts for repository maintenance and validation.

## validate-dependabot-config.sh

**Purpose**: Validates the Dependabot configuration in `.github/dependabot.yml`

**Description**: This script ensures that:
- The YAML syntax is valid
- Package ecosystems are correctly configured
- NPM packages under `eng/common/` are excluded from Dependabot updates
- Any NPM packages outside `eng/common/` are properly tracked
- (Optional) Validates with Dependabot CLI if installed

**Requirements**:
- Docker (for running validation tools)
- Bash shell
- Optional: Dependabot CLI for advanced validation

**Usage**:
```bash
./eng/scripts/validate-dependabot-config.sh
```

**Expected Output** (when configuration is correct with NPM ecosystem and exclusions):
```
=========================================
Dependabot Configuration Validation
=========================================

Step 1: Validating YAML syntax...
✓ YAML syntax is valid

Step 1.5: Validating configuration with Dependabot CLI...
  Testing NPM ecosystem with exclude-paths...
  Exclude paths: eng/common
  Note: Full CLI test requires GitHub credentials and network access
  ℹ Skipping live test, configuration structure validated

Step 2: Current dependabot.yml configuration:
[displays current config]

Step 3: Configured package ecosystems:
[lists ecosystems]

Step 4: Checking for NPM ecosystem configuration...
✓ NPM ecosystem is configured
  Checking exclude-paths...
  Exclude paths configured:
    - eng/common

Step 5: NPM packages in the repository:
[lists all NPM packages]

Step 6: NPM packages under eng/common/:
[lists eng/common NPM packages]

Step 7: NPM packages outside eng/common/:
No NPM packages found outside eng/common/
ℹ NPM ecosystem is configured for future packages or security updates

Step 8: Final Validation:
---
✓ Configuration is correct (NPM ecosystem with exclusions):
  - NPM ecosystem is configured for future packages/security updates
  - exclude-paths configured to skip eng/common/
  - No NPM packages outside eng/common/ currently exist
  - NPM packages under eng/common/ should not receive Dependabot PRs
---
```

**Exit Codes**:
- `0`: Configuration is valid
- `1`: Configuration has issues or validation failed

**When to Run**:
- After modifying `.github/dependabot.yml`
- Before merging PRs that affect dependency management
- As part of CI validation (optional)

## test-dependabot-cli.sh

**Purpose**: Advanced validation using the official Dependabot CLI

**Description**: This script uses the official Dependabot CLI to run smoke tests against the configuration. This provides deeper validation than YQ-based checks.

**Requirements**:
- Go installed (`go version`)
- Docker running
- Dependabot CLI: `go install github.com/dependabot/cli/cmd/dependabot@latest`

**Installation**:
```bash
# Install Dependabot CLI
go install github.com/dependabot/cli/cmd/dependabot@latest

# Add to PATH (add to your shell profile)
export PATH="$PATH:$(go env GOPATH)/bin"

# Verify installation
dependabot --version
```

**Usage**:
```bash
./eng/scripts/test-dependabot-cli.sh
```

**What It Does**:
- Checks if Dependabot CLI is installed
- Extracts configuration from dependabot.yml
- Creates a test scenario matching the configuration
- Runs `dependabot test` to validate the setup
- Reports any configuration errors

**When to Use**:
- To validate changes before pushing to GitHub
- When troubleshooting Dependabot issues
- As a comprehensive pre-merge check
- To ensure exclude-paths work correctly

**Note**: This test requires network access and Docker images. It may take 1-2 minutes to complete.

## DEPENDABOT_PATTERNS.md

**Purpose**: Documentation for Dependabot exclude-paths pattern options

**Description**: Explains different pattern options for excluding paths in Dependabot configuration, including:
- Current recommended pattern
- Alternative patterns and their trade-offs
- Testing and troubleshooting guidance

**See**: [DEPENDABOT_PATTERNS.md](./DEPENDABOT_PATTERNS.md) for detailed information.
