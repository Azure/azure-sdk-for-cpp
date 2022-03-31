param (
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $ReleaseArtifactSourceDirectory,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $PortDestinationDirectory,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $VcpkgPortName,

    [string] $GitCommitParameters,

    [switch] $DailyRelease
)

."$PSScriptRoot/../common/scripts/common.ps1"

# Validate prerequisites
if (!(Test-Path bootstrap-vcpkg.bat)) { 
    LogError "Could not locate bootstrap-vcpkg.bat current directory must be a clone of the vcpkg repo (https://github.com/microsoft/vcpkg)"
    exit 1
}

if ((Get-Command git | Measure-Object).Count -eq 0) {
    LogError "Could not locate git. Install git https://git-scm.com/downloads"
    exit 1
} 

if (!(Test-Path $ReleaseArtifactSourceDirectory/package-info.json)) {
    LogError "Could not locate package-info.json in -ReleaseArtifactSourceDirectory"
    exit 1
}

if (!(Test-Path $ReleaseArtifactSourceDirectory/CHANGELOG.md)) { 
    LogError "Could not locate CHANGELOG.md in -ReleaseArtifactSourceDirectory"
}

if (!(Test-Path $ReleaseArtifactSourceDirectory/vcpkg/port)) { 
    LogError "Could not locate vcpkg/port directory in -ReleaseArtifactSourceDirectory"
}

# Clean out the folder so that template files removed are not inadvertently
# re-added
if (Test-Path $PortDestinationDirectory) {
    Remove-Item -v -r $PortDestinationDirectory
}

New-Item -Type Directory $PortDestinationDirectory
Copy-Item `
    -Verbose `
    "$ReleaseArtifactSourceDirectory/vcpkg/port/*" `
    $PortDestinationDirectory

# Show artifacts copied into ports folder for PR
Write-Host "Files in destination directory:" 
Get-ChildItem -Recurse $PortDestinationDirectory | Out-String | Write-Host

Write-Host "./bootstrap-vcpkg.bat"
./bootstrap-vcpkg.bat

# Format the manifest file
Write-Host "./vcpkg.exe format-manifest $PortDestinationDirectory/vcpkg.json"
./vcpkg.exe format-manifest $PortDestinationDirectory/vcpkg.json

Write-Host "git status"
git status

# Temporarily commit changes to generate git tree objects required by 
# "vcpkg x-add-version <port-name>"
Write-Host "git add -A"
git add -A
Write-Host "git $GitCommitParameters commit -m 'Temporary commit to reset after x-add-version'"
"git $GitCommitParameters commit -m 'Temporary commit to reset after x-add-version'" | Invoke-Expression -Verbose | Write-Host

if ($LASTEXITCODE -ne 0) { 
    Write-Error "Failed to run bootstrap-vcpkg.bat"
    exit 1
}

$addVersionAdditionalParameters = ''
if ($DailyRelease) { 
    $addVersionAdditionalParameters = '--overwrite-version'
}


Write-Host "./vcpkg.exe x-add-version $VcpkgPortName $addVersionAdditionalParameters"
./vcpkg.exe x-add-version $VcpkgPortName $addVersionAdditionalParameters

if ($LASTEXITCODE -ne 0) { 
    Write-Error "Failed to run vcpkg x-add-version $VcpkgPortName"
    exit 1
}

# Reset to undo previous commit and put changes in the working directory.
Write-Host "git reset HEAD^"
git reset HEAD^

# Only perform the final commit if this is not a test release
if (!$DailyRelease) { 
    # Grab content needed for commit message and place in a temporary file
    $packageVersion = (Get-Content $ReleaseArtifactSourceDirectory/package-info.json -Raw | ConvertFrom-Json).version
    $commitMessageFile = New-TemporaryFile
    $changelogEntry = Get-ChangeLogEntryAsString `
        -ChangeLogLocation $ReleaseArtifactSourceDirectory/CHANGELOG.md `
        -VersionString $PackageVersion

    "[$VcpkgPortName] Update to $PackageVersion`n$changelogEntry" `
        | Set-Content $commitMessageFile

    Write-Host "Commit Message:"
    Write-host (Get-Content $commitMessageFile -Raw)

    Write-Host "git add -A"
    git add -A

    # Final commit using commit message from the temporary file. Using the file
    # enables the commit message to be formatted properly without having to write
    # code to escape certain characters that might appear in the changelog file.
    Write-Host "git $GitCommitParameters commit --file $commitMessageFile"
    "git $GitCommitParameters commit --file $commitMessageFile" `
        | Invoke-Expression -Verbose `
        | Write-Host

    # Set $(HasChanges) to $true so that create-pull-request.yml completes the
    # push and PR submission steps
    Write-Host "##vso[task.setvariable variable=HasChanges]$true"
}


<# 
.SYNOPSIS
Uses release artifacts to update a vcpkg port

.DESCRIPTION
This script updates a given vcpkg port using C++ release artifacts. It requires 
that the GitHub repo is tagged and a release is available at that tag for 
generating the SHA of the vcpkg artifact.

This script also uses the contents of the changelog at the release version in 
the commit message. 

Vcpkg requires the use of `vcpkg.exe x-add-version` to prepare ports going 
forward. This script handles adding this and making a single commit so that 
subsequent functions in the Azure DevOps pipeline can push the commit or rebase
as needed to get the commit into the PR branch.

.PARAMETER ReleaseArtifactSourceDirectory
Location of the release artifact. Must have package-info.json, CHANGELOG.md, and 
vcpkg/port/

.PARAMETER PortDestinationDirectory
Location of the vcpkg port folder where the port release artifacts are copied.
Generally "<vcpkg-repo>/ports/<port-name>"

.PARAMETER VcpkgPortName
Name of the vcpkg port (e.g. azure-template-cpp)

.PARAMETER GitCommitParameters
Additional parameters to supply to the `git commit` command. These are useful
in the context of Azure DevOps where the git client does not have a configured
user.name and user.email.

.PARAMETER DailyRelease
In the case of a test release set this to ensure that the x-add-version step
includes `--overwrite-version` to ensure daily packages are properly updated
in the vcpkg repo.

#>
