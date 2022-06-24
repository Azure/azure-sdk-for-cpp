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
    [string] $ConfigFileDir
)

Write-Host "$PSScriptRoot"
. (Join-Path $PSScriptRoot .. common scripts common.ps1)
$createReviewScript = (Join-Path $PSScriptRoot .. common scripts Create-APIReview.ps1)
Set-Location $PSScriptRoot

Write-Host "Creating API review artifact for $ArtifactName"
New-Item -ItemType Directory -Path $OutPath/$ArtifactName -force

$gitroot = Join-Path $PSScriptRoot .. ..
Write-Host "Get-ApiViewCommandLine.ps1 $gitroot $ArtifactName"
$cmdLine = & $PSScriptRoot/Get-ApiViewCommandLine.ps1 $gitroot $ArtifactName
Write-Host "Executing clang++ command:"
Write-Host $cmdLine
$cmd, $cmdArgs = $cmdLine -split ' '
# Get-ApiViewCommandLine.ps1 returns a string representing a clang++ command that needs to be run, e.g.
# clang++ <space separated list of header files> -Xclang -ast-dump -I <space separated list of header files>
# ApiView expects a zip of this ast as the format for a C++ language artifact.
& $cmd $cmdArgs > clangAstOutput

Compress-Archive -Path clangAstOutput -DestinationPath $OutPath/$ArtifactName/$ArtifactName
Rename-Item $OutPath/$ArtifactName/$ArtifactName.zip -NewName "$ArtifactName.cppast"

Write-Host "Send request to APIView to create review for $ArtifactName"
&($createReviewScript) -ArtifactPath $OutPath -APIViewUri $ApiviewUri -APIKey $ApiKey -APILabel $ApiLabel -PackageName $ArtifactName -SourceBranch $SourceBranch -DefaultBranch $DefaultBranch -ConfigFileDir $ConfigFileDir
