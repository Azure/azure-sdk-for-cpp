[CmdletBinding()]
param (
    [Parameter(Position=0)]
    [ValidateNotNullOrEmpty()]
    [string] $ProjectDirectory
)

$ErrorActionPreference = "Stop"
. $PSScriptRoot/Helpers/PSModule-Helpers.ps1
Install-ModuleIfNotInstalled "powershell-yaml" "0.4.1" | Import-Module

function NpmInstallForProject([string]$workingDirectory) {
    Push-Location $workingDirectory
    try {
        $currentDur = Resolve-Path "."
        Write-Host "Generating from $currentDur"
        if (Test-Path "package.json") {
            Remove-Item -Path "package.json" -Force
        }
        if (Test-Path ".npmrc") {
            Remove-Item -Path ".npmrc" -Force
        }
        $replacementPackageJson = "$PSScriptRoot/../../emitter-package.json"
        Write-Host("Copying package.json from $replacementPackageJson")
        Copy-Item -Path $replacementPackageJson -Destination "package.json" -Force
        npm install
        if ($LASTEXITCODE) { exit $LASTEXITCODE }
    }
    finally {
        Pop-Location
    }
}

Write-Host($GetEmitterNameFn)

$emitterName = &$GetEmitterNameFn
$cadlConfigurationFile = Resolve-Path "$ProjectDirectory/cadl-location.yaml"

Write-Host "Reading configuration from $cadlConfigurationFile"
$configuration = Get-Content -Path $cadlConfigurationFile -Raw | ConvertFrom-Yaml

$specSubDirectory = $configuration["directory"]
$innerFolder = Split-Path $specSubDirectory -Leaf

$tempFolder = "$ProjectDirectory/TempCadlFiles"
$npmWorkingDir = Resolve-Path $tempFolder/$innerFolder
$mainCadlFile = If (Test-Path "$npmWorkingDir/client.cadl") { Resolve-Path "$npmWorkingDir/client.cadl" } Else { Resolve-Path "$npmWorkingDir/main.cadl"}

try {
    Push-Location $npmWorkingDir
    NpmInstallForProject $npmWorkingDir

    if ($LASTEXITCODE) { exit $LASTEXITCODE }

    $emitterAdditionalOptions = &$GetEmitterAdditionalOptionsFn
    Write-Host("npx cadl compile $mainCadlFile --emit $emitterName $emitterAdditionalOptions")
    npx cadl compile $mainCadlFile --emit $emitterName $emitterAdditionalOptions

    if ($LASTEXITCODE) { exit $LASTEXITCODE }
}
finally {
    Pop-Location
}

$shouldCleanUp = $configuration["cleanup"] ?? $true
if ($shouldCleanUp) {
    Remove-Item $tempFolder -Recurse -Force
}