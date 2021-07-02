[CmdletBinding()]
Param (
    [Parameter()]
    [string] $TargetBranch,

    [Parameter()]
    [string] $SourceBranch,

    [Parameter()]
    [string] $CspellConfigPath = "./.vscode/cspell.json"
)

$ErrorActionPreference = "Continue"
. $PSScriptRoot/logging.ps1

if ((Get-Command git | Measure-Object).Count -eq 0) { 
    LogError "Could not locate git. Install git https://git-scm.com/downloads"
    exit 1
}

if ((Get-Command cspell | Measure-Object).Count -eq 0) { 
    LogError "Could not locate cspell. Install NodeJS (includes npm) https://nodejs.org/en/download/ and cspell (npm install --global cspell)"
    exit 1
}

if (!(Test-Path $CspellConfigPath)) {
    LogError "Could not locate config file $CspellConfigPath"
    exit 1
}

# Lists names of files that were in some way changed between the 
# current $SourceBranch and $TargetBranch. Excludes files that were deleted to
# prevent errors in Resolve-Path
Write-Host "git diff --diff-filter=d --name-only $TargetBranch $SourceBranch"
$changedFiles = git diff --diff-filter=d --name-only $TargetBranch $SourceBranch `
    | Resolve-Path

$changedFilesCount = ($changedFiles | Measure-Object).Count
Write-Host "Git Detected $changedFilesCount changed file(s). Files checked by cspell may exclude files according to cspell.json"

if ($changedFilesCount -eq 0) {
    Write-Host "No changes detected"
    exit 0
}

$changedFilesString = $changedFiles -join ' '

Write-Host "npx cspell --config $CspellConfigPath $changedFilesString"
$spellingErrors = cspell --config $CspellConfigPath @changedFiles

if ($spellingErrors) {
    foreach ($spellingError in $spellingErrors) { 
        LogWarning $spellingError
    }
    LogWarning "Spelling errors detected. To correct false positives or learn about spell checking see: https://aka.ms/azsdk/engsys/spellcheck"
    exit 1
}

exit 0

<#
.SYNOPSIS
Uses cspell (from NPM) to check spelling of recently changed files

.DESCRIPTION
This script checks files that have changed relative to a base branch (default 
branch) for spelling errors. Dictionaries and spelling configurations reside 
in a configurable `cspell.json` location.

This script assumes NodeJS and cspell are installed. NodeJS (which includes npm)
can be downloaded from https://nodejs.org/en/download/. Once NodeJS is installed
the cspell command can be installed with `npm install -g cspell`.

The entire file is scanned, not just changed sections. Spelling errors in parts 
of the file not touched will still be shown.

.PARAMETER TargetBranch
Git ref to compare changes. This is usually the "base" (GitHub) or "target" 
(DevOps) branch for which a pull request would be opened.

.PARAMETER SourceBranch
Git ref to use instead of changes in current repo state. Use `HEAD` here to 
check spelling of files that have been committed and exclude any new files or
modified files that are not committed. This is most useful in CI scenarios where
builds may have modified the state of the repo. Leaving this parameter blank  
includes files for whom changes have not been committed. 

.PARAMETER CspellConfigPath
Optional location to use for cspell.json path. Default value is 
`./.vscode/cspell.json`

.EXAMPLE
./eng/common/scripts/check-spelling-in-changed-files.ps1 -TargetBranch 'target_branch_name'

This will run spell check with changes in the current branch with respect to 
`target_branch_name`

#>
