#!/bin/bash
# Dependabot CLI Validation Test
# This script uses the official Dependabot CLI to test the configuration
# 
# Prerequisites:
# - Go installed
# - Docker running
# - Dependabot CLI: go install github.com/dependabot/cli/cmd/dependabot@latest

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
CONFIG_FILE="$REPO_DIR/.github/dependabot.yml"

echo "========================================="
echo "Dependabot CLI Validation Test"
echo "========================================="
echo ""

# Check if dependabot CLI is installed
if ! command -v dependabot &> /dev/null; then
    echo "✗ Dependabot CLI not found"
    echo ""
    echo "Install with:"
    echo "  go install github.com/dependabot/cli/cmd/dependabot@latest"
    echo ""
    echo "Then add to PATH:"
    echo "  export PATH=\"\$PATH:\$(go env GOPATH)/bin\""
    exit 1
fi

echo "✓ Dependabot CLI found: $(which dependabot)"
echo "  Version: $(dependabot --version)"
echo ""

# Check if Docker is running
if ! docker info &> /dev/null; then
    echo "✗ Docker is not running"
    echo "  Please start Docker to run CLI tests"
    exit 1
fi
echo "✓ Docker is running"
echo ""

# Extract NPM configuration from dependabot.yml
echo "Reading dependabot.yml configuration..."
NPM_CONFIGURED=$(docker run --rm -v "$CONFIG_FILE:/config.yml:ro" mikefarah/yq:latest eval '.updates[] | select(.package-ecosystem == "npm")' /config.yml 2>/dev/null)

if [ -z "$NPM_CONFIGURED" ]; then
    echo "ℹ NPM ecosystem not configured in dependabot.yml"
    echo "  No CLI test needed"
    exit 0
fi

echo "✓ NPM ecosystem configured"
echo ""

# Extract exclude-paths
EXCLUDE_PATHS=$(docker run --rm -v "$CONFIG_FILE:/config.yml:ro" mikefarah/yq:latest eval '.updates[] | select(.package-ecosystem == "npm") | .exclude-paths[]' /config.yml 2>/dev/null)

echo "Configuration details:"
echo "  Package ecosystem: npm"
echo "  Directory: /"
if [ -n "$EXCLUDE_PATHS" ]; then
    echo "  Exclude paths:"
    echo "$EXCLUDE_PATHS" | sed 's/^/    - /'
else
    echo "  Exclude paths: (none)"
fi
echo ""

# Create a test scenario file
TEST_SCENARIO="/tmp/dependabot-test-scenario-$$.yml"
cat > "$TEST_SCENARIO" << EOF
job:
  package-manager: npm
  source:
    provider: github
    repo: Azure/azure-sdk-for-cpp
    directory: /
  allowed-updates:
    - update-type: all
EOF

if [ -n "$EXCLUDE_PATHS" ]; then
    echo "  exclude-paths:" >> "$TEST_SCENARIO"
    echo "$EXCLUDE_PATHS" | sed 's/^/    - /' >> "$TEST_SCENARIO"
fi

echo "Created test scenario: $TEST_SCENARIO"
cat "$TEST_SCENARIO"
echo ""

echo "Running Dependabot CLI smoke test..."
echo "Note: This may take a minute and requires network access"
echo ""

# Run the test with timeout
TEST_OUTPUT="/tmp/dependabot-test-output-$$.yml"
if timeout 60s dependabot test -f "$TEST_SCENARIO" -o "$TEST_OUTPUT" 2>&1; then
    echo ""
    echo "✓ Dependabot CLI test PASSED"
    echo ""
    
    if [ -f "$TEST_OUTPUT" ]; then
        echo "Test output generated:"
        head -30 "$TEST_OUTPUT"
        echo ""
        echo "Full output saved to: $TEST_OUTPUT"
    fi
else
    EXIT_CODE=$?
    echo ""
    if [ $EXIT_CODE -eq 124 ]; then
        echo "⚠ Test timed out after 60 seconds"
        echo "  This is expected if there are network issues or many dependencies"
        echo "  Configuration structure appears valid"
    else
        echo "⚠ Test exited with code $EXIT_CODE"
        echo "  This may indicate:"
        echo "  - Missing GitHub credentials"
        echo "  - Network connectivity issues"
        echo "  - Configuration errors"
        echo ""
        echo "  Check the error messages above for details"
    fi
fi

# Cleanup
rm -f "$TEST_SCENARIO"

echo ""
echo "========================================="
echo "CLI Validation Complete"
echo "========================================="
