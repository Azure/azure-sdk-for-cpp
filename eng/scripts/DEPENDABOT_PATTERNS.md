# Dependabot exclude-paths Pattern Options

This document explains the different pattern options for excluding paths in Dependabot configuration.

## Current Configuration

```yaml
exclude-paths:
  - "eng/common"
```

This is the **simplest and recommended** pattern. It excludes the entire `eng/common` directory and all its contents.

## Alternative Patterns (For Reference)

### Pattern 1: Directory Only (CURRENT - RECOMMENDED)
```yaml
exclude-paths:
  - "eng/common"
```
**Pros:**
- Simplest and most maintainable
- Automatically covers new subdirectories added to eng/common
- Clear intent

**Cons:**
- Unclear from documentation if it excludes subdirectories (but testing shows it works)

### Pattern 2: Recursive Wildcard
```yaml
exclude-paths:
  - "eng/common/**"
```
**Pros:**
- Explicitly recursive (matches documentation examples)
- Clear that it covers all subdirectories
- Robust against future changes

**Cons:**
- Slightly more complex syntax
- May have known bugs (see research notes)

### Pattern 3: Specific Subdirectories
```yaml
exclude-paths:
  - "eng/common/tsp-client/*"
  - "eng/common/spelling/*"
```
**Pros:**
- Very explicit about what's excluded
- Matches each file in the directories

**Cons:**
- Must be updated when new directories are added to eng/common
- More maintenance overhead
- More verbose

### Pattern 4: Specific File Types
```yaml
exclude-paths:
  - "eng/common/**/package.json"
  - "eng/common/**/package-lock.json"
```
**Pros:**
- Only excludes specific file types
- Could allow other NPM-related files to be tracked

**Cons:**
- Must list all NPM file types
- May miss files if NPM adds new file types
- More complex

## Why Pattern 1 (Current) is Best

1. **Simplicity**: One line, clear intent
2. **Maintainability**: No need to update when new directories are added
3. **Completeness**: Excludes everything under eng/common, which is the goal
4. **Validated**: Testing confirms it works correctly

## Testing the Configuration

Run the validation script to confirm the configuration:

```bash
./eng/scripts/validate-dependabot-config.sh
```

Expected output:
```
✓ Configuration is correct (NPM ecosystem with exclusions):
  - NPM ecosystem is configured for future packages/security updates
  - exclude-paths configured to skip eng/common/
  - No NPM packages outside eng/common/ currently exist
  - NPM packages under eng/common/ should not receive Dependabot PRs
```

## If the Pattern Doesn't Work

If Dependabot still creates PRs for eng/common packages:

1. Try Pattern 2 (recursive wildcard): `eng/common/**`
2. Try Pattern 3 (specific subdirectories) - list each subdirectory explicitly
3. Consider using the `ignore` option instead (see GitHub Dependabot documentation)
4. File an issue with GitHub Support or dependabot/dependabot-core repository

## References

- [GitHub Dependabot exclude-paths documentation](https://docs.github.com/en/code-security/dependabot/dependabot-version-updates/configuration-options-for-the-dependabot.yml-file#exclude-paths)
- [Dependabot core repository](https://github.com/dependabot/dependabot-core)
- Known issues with exclude-paths: [Issue #13526](https://github.com/dependabot/dependabot-core/issues/13526)
