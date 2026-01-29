#!/bin/bash
# Dependabot Configuration Validation Script
# This script validates that the dependabot.yml configuration is correct
# and that NPM packages under eng/common/ are properly excluded

set -e

# Determine repository root dynamically
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
CONFIG_FILE="$REPO_DIR/.github/dependabot.yml"

# Check if Docker is available
if ! command -v docker &> /dev/null; then
    echo "Error: Docker is required but not installed or not in PATH"
    echo "Please install Docker to run this validation script"
    echo "See: https://docs.docker.com/get-docker/"
    exit 1
fi

# Check if Docker daemon is running
if ! docker info &> /dev/null; then
    echo "Error: Docker daemon is not running"
    echo "Please start Docker to run this validation script"
    exit 1
fi

echo "========================================="
echo "Dependabot Configuration Validation"
echo "========================================="
echo ""

# Step 1: Validate YAML syntax
echo "Step 1: Validating YAML syntax..."
if docker run --rm -v "$CONFIG_FILE:/config.yml:ro" mikefarah/yq:latest eval '.' /config.yml > /dev/null 2>&1; then
    echo "✓ YAML syntax is valid"
else
    echo "✗ YAML syntax is invalid"
    exit 1
fi
echo ""

# Step 2: Check current configuration
echo "Step 2: Current dependabot.yml configuration:"
echo "---"
cat "$CONFIG_FILE"
echo "---"
echo ""

# Step 3: List package ecosystems
echo "Step 3: Configured package ecosystems:"
docker run --rm -v "$CONFIG_FILE:/config.yml:ro" mikefarah/yq:latest eval '.updates[].package-ecosystem' /config.yml
echo ""

# Step 4: Check for NPM ecosystem
echo "Step 4: Checking for NPM ecosystem configuration..."
NPM_COUNT=$(docker run --rm -v "$CONFIG_FILE:/config.yml:ro" mikefarah/yq:latest eval '.updates[] | select(.package-ecosystem == "npm") | .package-ecosystem' /config.yml 2>/dev/null | wc -l)

if [ "$NPM_COUNT" -eq 0 ]; then
    echo "✓ NPM ecosystem is NOT configured (expected - no NPM packages outside eng/common/)"
else
    echo "✗ NPM ecosystem is still configured (will process eng/common/ packages)"
    echo "  Found $NPM_COUNT NPM ecosystem configuration(s)"
fi
echo ""

# Step 5: List all NPM packages in the repository
echo "Step 5: NPM packages in the repository:"
ALL_NPM=$(find "$REPO_DIR" -name "package.json" -type f ! -path "*/node_modules/*" 2>/dev/null | sort)
if [ -z "$ALL_NPM" ]; then
    echo "  No NPM packages found"
else
    echo "$ALL_NPM" | sed 's|'"$REPO_DIR"'|  .|g'
fi
echo ""

# Step 6: List NPM packages under eng/common (should be excluded)
echo "Step 6: NPM packages under eng/common/ (should be excluded from Dependabot):"
ENG_COMMON_NPM=$(find "$REPO_DIR/eng/common" -name "package.json" -type f 2>/dev/null | sort)
if [ -z "$ENG_COMMON_NPM" ]; then
    echo "  No NPM packages found in eng/common/"
else
    echo "$ENG_COMMON_NPM" | sed 's|'"$REPO_DIR"'|  .|g'
fi
echo ""

# Step 7: List NPM packages outside eng/common (should be tracked)
echo "Step 7: NPM packages outside eng/common/ (should be tracked by Dependabot):"
TRACKED_NPM=$(find "$REPO_DIR" -name "package.json" -type f ! -path "*/node_modules/*" ! -path "*/eng/common/*" 2>/dev/null | sort)
if [ -z "$TRACKED_NPM" ]; then
    echo "  No NPM packages found outside eng/common/"
    echo "  ✓ This validates removing NPM ecosystem from dependabot.yml"
else
    echo "$TRACKED_NPM" | sed 's|'"$REPO_DIR"'|  .|g'
    echo "  ⚠ Warning: These packages will NOT be tracked without NPM ecosystem"
fi
echo ""

# Step 8: Final validation
echo "Step 8: Final Validation:"
echo "---"
if [ "$NPM_COUNT" -eq 0 ] && [ -z "$TRACKED_NPM" ]; then
    echo "✓ Configuration is correct:"
    echo "  - NPM ecosystem removed from dependabot.yml"
    echo "  - No NPM packages outside eng/common/ need tracking"
    echo "  - NPM packages under eng/common/ will not receive Dependabot PRs"
    echo "---"
    exit 0
elif [ "$NPM_COUNT" -gt 0 ] && [ -n "$TRACKED_NPM" ]; then
    echo "✓ Configuration tracks NPM packages"
    echo "  - NPM ecosystem is configured"
    echo "  - $(echo "$TRACKED_NPM" | wc -l) NPM package(s) will be tracked"
    echo "---"
    exit 0
elif [ "$NPM_COUNT" -gt 0 ] && [ -z "$TRACKED_NPM" ]; then
    echo "⚠ Configuration may have issues:"
    echo "  - NPM ecosystem is configured"
    echo "  - But no NPM packages outside eng/common/ exist"
    echo "  - Dependabot may still create PRs for eng/common/ packages"
    echo "---"
    exit 1
else
    echo "⚠ Configuration needs attention:"
    echo "  - NPM ecosystem is NOT configured"
    echo "  - But $(echo "$TRACKED_NPM" | wc -l) NPM package(s) exist that should be tracked"
    echo "---"
    exit 1
fi
