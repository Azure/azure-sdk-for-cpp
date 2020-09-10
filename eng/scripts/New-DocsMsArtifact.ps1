[CmdletBinding()]
Param (
    [ValidateNotNullOrEmpty()]
    [string] $RepoRoot = "${PSScriptRoot}/../..",
    [Parameter(Mandatory=$True)]
    [string] $ServiceDirectory,
    [Parameter(Mandatory=$True)]
    [string] $PackageName,
    [string] $TargetFolder = "$RepoRoot/build/sdk/$ServiceDirectory/$PackageName/docs/docs.ms"
)

npm install -g moxygen

$docsPath = "$RepoRoot/build/sdk/$ServiceDirectory/$PackageName/docs"
New-Item -ItemType directory -Path $TargetFolder -Force
moxygen --anchors --output "$TargetFolder/api-docs.md" "$docsPath/xml"
