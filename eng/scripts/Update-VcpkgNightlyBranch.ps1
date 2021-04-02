<#
.SYNOPSIS
Updates a Target branch with changes from a Main branch and a Source branch

.DESCRIPTION
Updat the long-lived nightly branch with changes from Main and a given Source 
branch. Changes are first merged from the Main branch and then from the Source 
branch. In both cases conflicts are resolved in favor of the branch which is 
being merged.

.PARAMETER SourceBranchName
Name of the branch with new changes to merge into the Target Branch

.PARAMETER TargetBranchName
Name of the long-lived branch receiving the changes

.PARAMETER MainBranchName
Name of the "Main" branch to merge into the Target Branch

#>

param(
    [string] $SourceBranchName,
    [string] $TargetBranchName,
    [string] $MainBranchName
)

$ErrorActionPreference = "Continue"

if (!(Get-Command git-ErrorAction SilentlyContinue)) { 
    LogError "Could not locate git. Install git https://git-scm.com/downloads"
    exit 1
}

Write-Host "git fetch --all"
git fetch --all

# Fail quickly if the $SourceBranchName of nightly changes cannot be found
Write-Host "git checkout $SourceBranchName"
git checkout $SourceBranchName

if ($LASTEXITCODE -ne 0) { 
    Write-Error "Cannot find branch: $SourceBranchName, nothing to merge"
    exit 1
}


Write-Host "git branch --show-current"
$currentBranch = git branch --show-current

if ($currentBranch -ne $TargetBranchName) {
    Write-Host "git checkout $TargetBranchName"
    git checkout $TargetBranchName

    if ($LASTEXITCODE -ne 0) { 
        Write-Host "git checkout -b $TargetBranchName"
        git checkout -b $TargetBranchName
    }
}

Write-Host "git merge $MainBranchName --strategy-option=theirs" 
git merge $MainBranchName --strategy-option=theirs

if ($LASTEXITCODE -ne 0) { 
    Write-Error "Could not merge $MainBranchName into $TargetBranchName"
    exit 1
}

Write-Host "git merge $SourceBranchName --strategy-option=theirs" 
git merge $SourceBranchName --strategy-option=theirs

if ($LASTEXITCODE -ne 0) {
    Write-Error "Could not merge $SourceBranchName into $TargetBranchName" 
    exit 1
}
