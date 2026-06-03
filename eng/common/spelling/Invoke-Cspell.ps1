#!/usr/bin/env pwsh

<#
.SYNOPSIS
Invokes cspell using dependencies defined in adjacent ./package*.json

.PARAMETER JobType
Maps to cspell command (e.g. `lint`, `trace`, etc.). Default is `lint`

.PARAMETER FileList
List of file paths to be scanned. This is piped into cspell via stdin.

.PARAMETER CSpellConfigPath
Location of cspell.json file to use when scanning. Defaults to
`.vscode/cspell.json` at the root of the repo.

.PARAMETER SpellCheckRoot
Location of root folder for generating readable relative file paths. Defaults to
the root of the repo relative to the script.

.PARAMETER PackageInstallCache
Location of a working directory. If no location is provided a folder will be
created in the temp folder, package*.json files will be placed in that folder.

.PARAMETER LeavePackageInstallCache
If set the PackageInstallCache will not be deleted. Use if there are multiple
calls to Invoke-Cspell.ps1 to prevent creating multiple working directories and
redundant calls `npm ci`.

.EXAMPLE
./eng/common/spelling/Invoke-Cspell.ps1 -FileList @('./README.md', 'file2.txt')

.EXAMPLE
git diff main --name-only | ./eng/common/spelling/Invoke-Cspell.ps1

#>
[CmdletBinding()]
param(
  [Parameter()]
  [string] $JobType = 'lint',

  [Parameter(ValueFromPipeline)]
  [array]$FileList,

  [Parameter()]
  [string] $CSpellConfigPath = (Resolve-Path "$PSScriptRoot/../../../.vscode/cspell.json"),

  [Parameter()]
  [string] $SpellCheckRoot = (Resolve-Path "$PSScriptRoot/../../.."),

  [Parameter()]
  [string] $PackageInstallCache = (Join-Path ([System.IO.Path]::GetTempPath()) "cspell-tool-path"),

  [Parameter()]
  [switch] $LeavePackageInstallCache
)

begin {
  Set-StrictMode -Version 3.0
  . (Join-Path $PSScriptRoot "../scripts/logging.ps1")

  if (!(Get-Command npm -ErrorAction SilentlyContinue)) {
    LogError "Could not locate npm. Install NodeJS (includes npm) https://nodejs.org/en/download/"
    exit 1
  }

  if (!(Get-Command node -ErrorAction SilentlyContinue)) {
    LogError "Could not locate node. Install NodeJS https://nodejs.org/en/download/"
    exit 1
  }

  $nodeVersionRaw = (& node --version)
  if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($nodeVersionRaw)) {
    LogError "Unable to determine NodeJS version. Node >=20.18.0 is required."
    exit 1
  }

  $nodeVersionText = $nodeVersionRaw.Trim().TrimStart('v')
  $nodeVersion = $null
  if (-not [System.Version]::TryParse($nodeVersionText, [ref]$nodeVersion)) {
    LogError "Unable to parse NodeJS version '$nodeVersionText'. Node >=20.18.0 is required."
    exit 1
  }

  if ($nodeVersion -lt [System.Version]'20.18.0') {
    LogError "Unsupported NodeJS version ($nodeVersionText); >=20.18.0 is required."
    exit 1
  }

  if (!(Test-Path $CSpellConfigPath)) {
    LogError "Could not locate config file $CSpellConfigPath"
    exit 1
  }

  # Prepare the working directory if it does not already have requirements in
  # place.
  if (!(Test-Path $PackageInstallCache)) {
    New-Item -ItemType Directory -Path $PackageInstallCache | Out-Null
  }

  if (!(Test-Path "$PackageInstallCache/package.json")) {
    Copy-Item "$PSScriptRoot/package.json" $PackageInstallCache
  }

  if (!(Test-Path "$PackageInstallCache/package-lock.json")) {
    Copy-Item "$PSScriptRoot/package-lock.json" $PackageInstallCache
  }


  $filesToCheck = @()
 }
process {
  if ($null -ne $FileList) {
    $filesToCheck += $FileList
  }
 }
end {
  if (($filesToCheck | Measure-Object).Count -eq 0) {
    LogError "No files provided. Pass -FileList or pipe file paths into Invoke-Cspell.ps1."
    exit 1
  }

  $nodeModulesPath = Join-Path $PackageInstallCache "node_modules"
  if (!(Test-Path $nodeModulesPath)) {
    npm --prefix $PackageInstallCache ci | Write-Host
    if ($LASTEXITCODE -ne 0) {
      LogError "npm ci failed with exit code $LASTEXITCODE"
      exit $LASTEXITCODE
    }
  }
  else {
    Write-Host "Reusing package install cache at $PackageInstallCache"
  }

  $command = "npm --prefix $PackageInstallCache exec --no -- cspell $JobType --config $CSpellConfigPath --no-must-find-files --root $SpellCheckRoot --file-list stdin"
  Write-Host $command
  $cspellOutput = $filesToCheck | npm --prefix $PackageInstallCache `
    exec  `
    --no `
    '--' `
    cspell `
    $JobType `
    --config $CSpellConfigPath `
    --no-must-find-files `
    --root $SpellCheckRoot `
    --file-list stdin

  if (!$LeavePackageInstallCache) {
    Write-Host "Cleaning up package install cache at $PackageInstallCache"
    Remove-Item -Path $PackageInstallCache -Recurse -Force | Out-Null
  }

  return $cspellOutput
}
