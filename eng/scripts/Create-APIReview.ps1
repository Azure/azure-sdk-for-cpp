Param(
    [Parameter(Mandatory=$True)]
    [string] $ArtifactName,
    [Parameter(Mandatory=$True)]
    [string] $SourceName,
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

Write-Host Invoking CreateApiReviewTool. ArtifactName: $ArtifactName, OutPath: $OutPath, ApiviewUri: $ApiviewUri, SourceBranch: $SourceBranch, DefaultBranch: $DefaultBranch, ConfigFileDir: $ConfigFileDir

Write-Host "Contents in $($ConfigFileDir)"
Get-ChildItem -Path $ConfigFileDir -Recurse

# The eng\common\Save-Package-Properties.ps1 script does not contemplate a scenario where the artifact name is different from the source name.
# Rename the incorrect artifact name in the $ConfigFileDir directory to match the correct package name.
if ($ArtifactName -NE $SourceName)
{
	Write-Host "Copying $SourceName to $ArtifactName"
	Rename-Item -Path $ConfigFileDir/$SourceName.json -NewName $ConfigFileDir/$ArtifactName.json
}

Write-Host "Contents in $($ConfigFileDir)" after rename.
Get-ChildItem -Path $ConfigFileDir -Recurse
    
Write-Host "Send request to APIView to create review for $ArtifactName"
&($createReviewScript) -ArtifactPath $OutPath -APIViewUri $ApiviewUri -APIKey $ApiKey -APILabel $ApiLabel -PackageName $ArtifactName -SourceBranch $SourceBranch -DefaultBranch $DefaultBranch -ConfigFileDir $ConfigFileDir
