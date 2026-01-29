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
    echo "ℹ NPM ecosystem is NOT configured"
    NPM_CONFIGURED=false
else
    echo "✓ NPM ecosystem is configured"
    echo "  Checking exclude-paths..."
    EXCLUDE_PATHS=$(docker run --rm -v "$CONFIG_FILE:/config.yml:ro" mikefarah/yq:latest eval '.updates[] | select(.package-ecosystem == "npm") | .exclude-paths[]' /config.yml 2>/dev/null)
    if [ -n "$EXCLUDE_PATHS" ]; then
        echo "  Exclude paths configured:"
        echo "$EXCLUDE_PATHS" | sed 's/^/    - /'
    else
        echo "  ⚠ Warning: No exclude-paths configured"
    fi
    NPM_CONFIGURED=true
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
    if [ "$NPM_CONFIGURED" = true ]; then
        echo "  ℹ NPM ecosystem is configured for future packages or security updates"
    fi
else
    echo "$TRACKED_NPM" | sed 's|'"$REPO_DIR"'|  .|g'
    if [ "$NPM_CONFIGURED" = false ]; then
        echo "  ⚠ Warning: These packages will NOT be tracked without NPM ecosystem"
    fi
fi
echo ""

# Step 8: Final validation
echo "Step 8: Final Validation:"
echo "---"

# Check if exclude-paths properly covers eng/common
EXCLUDE_PATHS_COUNT=$(docker run --rm -v "$CONFIG_FILE:/config.yml:ro" mikefarah/yq:latest eval '.updates[] | select(.package-ecosystem == "npm") | .exclude-paths[]' /config.yml 2>/dev/null | wc -l)

if [ "$NPM_COUNT" -eq 0 ] && [ -z "$TRACKED_NPM" ]; then
    echo "✓ Configuration is correct (NPM ecosystem removed):"
    echo "  - NPM ecosystem removed from dependabot.yml"
    echo "  - No NPM packages outside eng/common/ need tracking"
    echo "  - NPM packages under eng/common/ will not receive Dependabot PRs"
    echo "---"
    exit 0
elif [ "$NPM_COUNT" -gt 0 ] && [ "$EXCLUDE_PATHS_COUNT" -gt 0 ] && [ -z "$TRACKED_NPM" ]; then
    echo "✓ Configuration is correct (NPM ecosystem with exclusions):"
    echo "  - NPM ecosystem is configured for future packages/security updates"
    echo "  - exclude-paths configured to skip eng/common/"
    echo "  - No NPM packages outside eng/common/ currently exist"
    echo "  - NPM packages under eng/common/ should not receive Dependabot PRs"
    echo "---"
    exit 0
elif [ "$NPM_COUNT" -gt 0 ] && [ -n "$TRACKED_NPM" ]; then
    echo "✓ Configuration tracks NPM packages:"
    echo "  - NPM ecosystem is configured"
    echo "  - $(echo "$TRACKED_NPM" | wc -l) NPM package(s) will be tracked"
    if [ "$EXCLUDE_PATHS_COUNT" -gt 0 ]; then
        echo "  - exclude-paths configured to skip eng/common/"
    fi
    echo "---"
    exit 0
elif [ "$NPM_COUNT" -gt 0 ] && [ "$EXCLUDE_PATHS_COUNT" -eq 0 ] && [ -z "$TRACKED_NPM" ]; then
    echo "⚠ Configuration may have issues:"
    echo "  - NPM ecosystem is configured"
    echo "  - No exclude-paths configured"
    echo "  - Dependabot may create PRs for eng/common/ packages"
    echo "---"
    exit 1
else
    echo "⚠ Configuration needs attention:"
    echo "  - NPM ecosystem: $([ "$NPM_COUNT" -gt 0 ] && echo "configured" || echo "NOT configured")"
    echo "  - Exclude paths: $([ "$EXCLUDE_PATHS_COUNT" -gt 0 ] && echo "$EXCLUDE_PATHS_COUNT configured" || echo "none")"
    echo "  - Tracked packages: $([ -n "$TRACKED_NPM" ] && echo "$(echo "$TRACKED_NPM" | wc -l) found" || echo "none")"
    echo "---"
    exit 1
fi
