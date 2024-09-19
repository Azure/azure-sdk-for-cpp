Param(
    [Parameter(Mandatory=$True)]
    [array] $ArtifactList,
    [Parameter(Mandatory=$True)]
    [string] $OutPath,
    [Parameter(Mandatory=$True)]
    [string] $ParserPath,
    [Parameter(Mandatory=$True)]
    [string] $ServicePath
)

Write-Host "$PSScriptRoot"
. (Join-Path $PSScriptRoot .. common scripts common.ps1)

foreach ($artifact in $ArtifactList)
{
    $ArtifactName = $artifact.name
    Write-Host "Generating artifact for $ArtifactName"
    $SourcePath = Join-Path $ServicePath $ArtifactName "inc"
    $apiviewSettings = Join-Path $SourcePath "ApiViewSettings.json"
    if (!(Test-Path $apiviewSettings))
    {
        Write-Host "ApiViewSettings.json file is not found in $($SourcePath). APIView settings file is required to generate API review file."
        exit 1
    }

    Write-Host "Creating API review artifact for $ArtifactName"
    New-Item -ItemType Directory -Path "$OutPath/$ArtifactName" -Force
    $parentPath = Split-Path $ParserPath  -Parent
    Write-Host "Contents in ${parentPath}:"
    Get-ChildItem -Path $parentPath -Recurse
    & $ParserPath -o "$OutPath/$ArtifactName/${ArtifactName}_cpp.api.json" $SourcePath
    if ($LASTEXITCODE -ne 0)
	{
		Write-Host "Failed to generate API review file for $($ArtifactName)"
		exit 1
	}
}
