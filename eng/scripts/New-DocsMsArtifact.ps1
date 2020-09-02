[CmdletBinding()]
Param (
    [ValidateNotNullOrEmpty()]
    [string] $RepoRoot = "${PSScriptRoot}/../..",
    [Parameter(Mandatory=$True)]
    [string] $ServiceDirectory,
    [Parameter(Mandatory=$True)]
    [string] $PackageName
)

$docsPath = "$RepoRoot/build/sdk/$ServiceDirectory/$PackageName/docs"
New-Item -ItemType directory -Path "$docsPath/docs.ms" -Force
moxygen --anchors --output "$docsPath/docs.ms/api-docs.md" "$docsPath/xml"