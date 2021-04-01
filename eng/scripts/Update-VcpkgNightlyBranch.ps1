param(
    [string] $SourceBranch,
    [string] $TargetBranch,
    [string] $MainBranchName
)

$ErrorActionPreference = "Continue"

if ((Get-Command git | Measure-Object).Count -eq 0) { 
    LogError "Could not locate git. Install git https://git-scm.com/downloads"
    exit 1
}

Write-Host "git fetch --all"
git fetch --all

# Fail quickly if the $SourceBranch of nightly changes cannot be found
Write-Host "git checkout $SourceBranch"
git checkout $SourceBranch

if ($LASTEXITCODE -ne 0) { 
    Write-Error "Cannot find branch: $SourceBranch, nothing to merge"
    exit 1
}


Write-Host "git branch --show-current"
$currentBranch = git branch --show-current

if ($currentBranch -ne $TargetBranch) {
    Write-Host "git checkout $TargetBranch"
    git checkout $TargetBranch

    if ($LASTEXITCODE -ne 0) { 
        Write-Host "git checkout -b $TargetBranch"
        git checkout -b $TargetBranch
    }
}

Write-Host "git merge $MainBranchName --strategy-option=theirs" 
git merge $MainBranchName --strategy-option=theirs

if ($LASTEXITCODE -ne 0) { 
    Write-Error "Could not merge $MainBranchName into $TargetBranch"
    exit 1
}

Write-Host "git merge $SourceBranch --strategy-option=theirs" 
git merge $SourceBranch --strategy-option=theirs

if ($LASTEXITCODE -ne 0) {
    Write-Error "Could not merge $SourceBranch into $TargetBranch" 
    exit 1
}
