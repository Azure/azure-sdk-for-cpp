Param(
    [Parameter(Mandatory=$True)]
    [array] $ArtifactList,
    [Parameter(Mandatory=$True)]
    [string] $OutPath,
    [Parameter(Mandatory=$True)]
    [string] $ApiKey,
    [Parameter(Mandatory=$True)]
    [string] $SourceBranch,
    [Parameter(Mandatory=$True)]
    [string] $DefaultBranch,
    [Parameter(Mandatory=$True)]
    [string] $ParserPath,
    [Parameter(Mandatory=$True)]
    [string] $ServicePath,
    [Parameter(Mandatory=$True)]
    [string] $RepoName,
    [Parameter(Mandatory=$True)]
    [string] $BuildId
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

    Write-Host "Creating API review artifact for $($ArtifactName)"
    New-Item -ItemType Directory -Path $OutPath/$ArtifactName -force
    $parentPath = Split-Path $ParserPath  -Parent
    Write-Host "Contents in $($parentPath)"
    Get-ChildItem -Path $parentPath -Recurse
    $tokenFilePath = "$OutPath/$ArtifactName/$ArtifactName.json"
    & $ParserPath -o $tokenFilePath $SourcePath    
    if (!(Test-Path $tokenFilePath))
    {
        Write-Host "Failed to generate token file for package [$(ArtifactName)]. Token file is not found in $(tokenFilePath). API review cannot be generated for C++ package without a token file."
        exit 1
    }
}

$createReviewScript = (Join-Path $PSScriptRoot .. common scripts Create-APIReview.ps1)
Write-Host "Running script to create review for all artifacts."
&($createReviewScript) -ArtifactList $ArtifactList -ArtifactPath $OutPath -APIKey $ApiKey -SourceBranch $SourceBranch -DefaultBranch $DefaultBranch -RepoName $RepoName -BuildId $BuildId
