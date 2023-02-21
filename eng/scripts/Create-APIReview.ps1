Param(
    [Parameter(Mandatory=$True)]
    [string] $ArtifactName,
    [Parameter(Mandatory=$True)]
    [string] $OutPath,
    [Parameter(Mandatory=$True)]
    [string] $ApiviewUri,
    [Parameter(Mandatory=$True)]
    [string] $ApiKey,
    [Parameter(Mandatory=$True)]
    [string] $ApiLabel,
    [Parameter(Mandatory=$True)]
    [string] $SourceBranch,
    [Parameter(Mandatory=$True)]
    [string] $DefaultBranch,
    [Parameter(Mandatory=$True)]
    [string] $ConfigFileDir,
    [Parameter(Mandatory=$True)]
    [string] $ParserPath,
    [Parameter(Mandatory=$True)]
    [string] $SourcePath
)

Write-Host "$PSScriptRoot"
. (Join-Path $PSScriptRoot .. common scripts common.ps1)
$createReviewScript = (Join-Path $PSScriptRoot .. common scripts Create-APIReview.ps1)

$apiviewSettings = Join-Path $SourcePath "ApiViewSettings.json"
if (!(Test-Path $apiviewSettings))
{
    Write-Host "ApiViewSettings.json file is not found in $($SourcePath). APIView settings file is required to generate API review file."
    exit 1
}

Write-Host "Creating API review artifact for $($ArtifactName)"
New-Item -ItemType Directory -Path $OutPath/$ArtifactName -force
$parentPath = Split-Path $ParserPath  -Parent
Write-Host "Contents in $($parentPath)"
Get-ChildItem -Path $parentPath -Recurse

& $ParserPath -o $OutPath/$ArtifactName/$ArtifactName.json $SourcePath

Write-Host "Send request to APIView to create review for $ArtifactName"
&($createReviewScript) -ArtifactPath $OutPath -APIViewUri $ApiviewUri -APIKey $ApiKey -APILabel $ApiLabel -PackageName $ArtifactName -SourceBranch $SourceBranch -DefaultBranch $DefaultBranch -ConfigFileDir $ConfigFileDir
