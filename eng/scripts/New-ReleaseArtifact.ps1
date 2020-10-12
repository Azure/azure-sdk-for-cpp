<#
.SYNOPSIS
Creates a .tar.gz archive of files in a given folder.

.DESCRIPTION
This is supposed to help zip up any location where artifacts are properly staged
for placement in an archive. It does this by reading in the package information
from the package.json file, then creating an archive named for the package and
version, and copying the package to an artifact location.

.PARAMETER SourceDirectory
Source directory to compress. Files and folders in that source directory
(except .git) will be compressed.

.PARAMETER PackageSpecPath
Location of the package.json file.

.PARAMETER Destination
Location to copy the artifact file to.

.PARAMETER Destination
Name of the destination to which the archive should be copied.

.PARAMETER Workspace
Workspace folder where assets are staged before creating.

#>


param (
    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $SourceDirectory,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $PackageSpecPath,

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $Destination,

    [Parameter()]
    [ValidateNotNullOrEmpty()]
    [string] $Workspace = "$env:TEMP/vcpkg-workspace"
)

$initialLocation = Get-Location

try {
    $packageSpec = Get-Content -Raw -Path $PackageSpecPath | ConvertFrom-Json
    $archiveName = $packageSpec.packageName

    Write-Verbose "Archive Name: $archiveName"

    # Set up workspace, copy source to workspace
    Remove-Item -Path $Workspace -Force -Recurse -ErrorAction Ignore
    New-Item -ItemType directory -Path "$Workspace/$archiveName" -Force
    Copy-Item `
        -Path $SourceDirectory/* `
        -Destination "$Workspace/$archiveName/" `
        -Recurse `
        -Exclude ".git"

    # Move outside of workspace so the archive root contains a folder named
    # after the package and its contents are the package contents. For example:
    # azure-template-1.2.3/
    #   CMakeLists.txt
    #   ...
    Set-Location $Workspace

    # Create the tar.gz file
    tar -cvz --exclude .git -f "$archiveName.tar.gz" $archiveName

    New-Item -ItemType Directory $Destination -Force

    # Copy release archive to the appropriate destination location
    Copy-Item `
    -Path "$archiveName.tar.gz" `
    -Destination $Destination `
    -Verbose

} finally {
    Set-Location $initialLocation
}
