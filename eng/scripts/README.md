# Repository Scripts

This directory contains utility scripts for repository maintenance and validation.

## validate-dependabot-config.sh

**Purpose**: Validates the Dependabot configuration in `.github/dependabot.yml`

**Description**: This script ensures that:
- The YAML syntax is valid
- Package ecosystems are correctly configured
- NPM packages under `eng/common/` are excluded from Dependabot updates
- Any NPM packages outside `eng/common/` are properly tracked

**Requirements**:
- Docker (for running validation tools)
- Bash shell

**Usage**:
```bash
./eng/scripts/validate-dependabot-config.sh
```

**Expected Output** (when configuration is correct):
```
=========================================
Dependabot Configuration Validation
=========================================

Step 1: Validating YAML syntax...
✓ YAML syntax is valid

Step 2: Current dependabot.yml configuration:
[displays current config]

Step 3: Configured package ecosystems:
[lists ecosystems]

Step 4: Checking for NPM ecosystem configuration...
✓ NPM ecosystem is NOT configured

Step 5: NPM packages in the repository:
[lists all NPM packages]

Step 6: NPM packages under eng/common/:
[lists eng/common NPM packages]

Step 7: NPM packages outside eng/common/:
No NPM packages found outside eng/common/
✓ This validates removing NPM ecosystem from dependabot.yml

Step 8: Final Validation:
---
✓ Configuration is correct:
  - NPM ecosystem removed from dependabot.yml
  - No NPM packages outside eng/common/ need tracking
  - NPM packages under eng/common/ will not receive Dependabot PRs
---
```

**Exit Codes**:
- `0`: Configuration is valid
- `1`: Configuration has issues or validation failed

**When to Run**:
- After modifying `.github/dependabot.yml`
- Before merging PRs that affect dependency management
- As part of CI validation (optional)
