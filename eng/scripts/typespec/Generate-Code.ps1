# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

[CmdletBinding()]
param(
  [Parameter(Mandatory=$false)]
  [string] $ClangFormatPath = "clang-format",

  [Parameter(Mandatory=$false)]
  [string] $outputPath = ".\generated"
)

$ErrorActionPreference = "Stop"
$PSNativeCommandUseErrorActionPreference = $true

pushd
cd $PSScriptRoot
cd ../../../
$typespecCppDir = Get-Location
popd

pushd
Write-Host "Invoking: tsp compile ."
tsp compile --emit $typespecCppDir client.tsp --output-dir $outputPath

if (-not (Test-Path ".clang-format")) {
  $oldProgressPreference = $ProgressPreference
  $ProgressPreference = "SilentlyContinue"
  Invoke-WebRequest -Uri "https://raw.githubusercontent.com/Azure/azure-sdk-for-cpp/azure-core_1.13.0/.clang-format" -OutFile ".clang-format"
  $ProgressPreference = $oldProgressPreference
}

try {
  & "$ClangFormatPath" -version
} catch {
  Write-Error "Clang-Format not found at: $ClangFormatPath"
  popd
  exit 1
}

Write-Host "Formatting generated code with clang-format"
Get-ChildItem $outputPath -Include *.cpp, *.hpp -Recurse | ForEach-Object -Process{ 
  "  Processing: $_"
  & "$ClangFormatPath" -i $_
}

popd
