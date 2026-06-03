# Spelling Check Scripts

This directory contains a script to run cspell (Code Spell Checker) on the repository using the dependencies defined in the adjacent `package*.json` files.

## Adding Legitimate Words

If the spell checker flags legitimate words as misspelled, you can add them to the dictionary configuration file located at `.vscode/cspell.json`.

### Where to Add Words

There are two main places to add legitimate words. Maintain alphabetical order when adding words to keep the dictionary organized:

1. **Root-level words array**: Add words to the `"words"` array at the root level of the configuration file. This is the preferred location for project-specific terms, technical vocabulary, and commonly used words.

2. **Baseline dictionary**: Add words to the `"baseline"` dictionary under `"dictionaryDefinitions"`. This is typically used for words that were already present in the codebase when the spell checker was first introduced.


### Example

To add new words, edit `.vscode/cspell.json` and add them to the `"words"` array:

```json
{
    "words": [
        "myprojectname",
        "customterm",
        "technicalword"
    ]
}
```

### Guidelines

- Use lowercase for words
- Consider whether the word is truly legitimate or if it might be a typo

## Available Scripts

### PowerShell Version
- **File**: `Invoke-Cspell.ps1`
- **Usage**: PowerShell 7+ (`pwsh`)

## Usage Examples

```powershell
# Check specific files
./eng/common/spelling/Invoke-Cspell.ps1 -FileList @('./README.md', './doc/DistributedTracing.md')

# Pipe a file list from git
git diff main --name-only | ./eng/common/spelling/Invoke-Cspell.ps1

# Check one file
./eng/common/spelling/Invoke-Cspell.ps1 -FileList @('./README.md')
```

## Parameters

- **Job Type**: The cspell command to run (default: `lint`)
- **File List**: File paths to check for spelling (`-FileList` or pipeline input)
- **Config Path**: Location of the cspell.json configuration file
- **Spell Check Root**: Root directory for relative paths
- **Package Cache**: Working directory for npm dependencies
- **Leave Cache**: Option to preserve the npm package cache

## Requirements

- Node.js (`>=20.18`) and npm must be installed
- The `.vscode/cspell.json` configuration file must exist

## How It Works

1. Creates (or reuses) a working directory for npm packages
2. Copies `package.json` and `package-lock.json` to the working directory
3. Installs npm dependencies using `npm ci` (or reuses an existing `node_modules` in the cache)
4. Pipes the file list into cspell via stdin (`--file-list stdin`)
5. Cleans up temporary files unless `-LeavePackageInstallCache` is set
