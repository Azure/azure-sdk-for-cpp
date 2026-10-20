# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

#Requires -Version 7.0
[CmdletBinding()]
param(
  [Parameter(Mandatory)]
  [string] $EmitterPath,
  [Parameter(Mandatory)]
  [string] $SpecDirectory,
  [string] $ClangFormatPath = "clang-format",
  [string] $OutputDirectory = (Split-Path $PSScriptRoot -Parent)
)

$ErrorActionPreference = "Stop"
$emitter = (Resolve-Path -LiteralPath $EmitterPath).Path
$spec = (Resolve-Path -LiteralPath $SpecDirectory).Path
$compiler = Join-Path $emitter "node_modules\@typespec\compiler\cmd\tsp.js"
if (!(Test-Path -LiteralPath $compiler) -or !(Test-Path (Join-Path $emitter "dist\src\emitter.js"))) {
  throw "Build packages\codegen.cpp and packages\typespec-cpp before generating."
}
$version = & $ClangFormatPath --version
if ($LASTEXITCODE -ne 0 -or $version -notmatch "clang-format version 11\.0\.0\b") {
  throw "Generation requires clang-format 11.0.0."
}
$work = Join-Path ([System.IO.Path]::GetTempPath()) ("blobs-typespec-" + [guid]::NewGuid())
$modules = Join-Path $work "node_modules"
New-Item -ItemType Directory -Path (Join-Path $work "service\Common") | Out-Null
try {
  foreach ($file in @("client.tsp", "main.tsp", "Common\service.tsp", "Common\models.tsp", "Common\routes.tsp")) {
    Copy-Item -LiteralPath (Join-Path $spec $file) -Destination (Join-Path $work "service\$file")
  }
  Copy-Item -LiteralPath (Join-Path $PSScriptRoot "client.tsp") -Destination $work
  Copy-Item -LiteralPath (Join-Path $PSScriptRoot "..\tspconfig.yaml") -Destination $work
  Copy-Item -LiteralPath (Join-Path $PSScriptRoot "..\..\..\..\.clang-format") -Destination $work
  $linkType = if ($IsWindows) { "Junction" } else { "SymbolicLink" }
  New-Item -ItemType $linkType -Path $modules -Target (Join-Path $emitter "node_modules") | Out-Null
  & node $compiler compile (Join-Path $work "client.tsp") `
    --config (Join-Path $work "tspconfig.yaml") --emit $emitter `
    --output-dir (Join-Path $work "output") --pretty false
  if ($LASTEXITCODE -ne 0) {
    throw "Blob TypeSpec generation failed with exit code $LASTEXITCODE."
  }
  $generated = Join-Path $work "output\src\generated"
  $files = @("blobs_client.cpp", "blobs.cpp", "blobs_client.hpp", "blobs_client_options.hpp", "blobs_models.hpp")
  foreach ($file in $files) {
    & $ClangFormatPath -i --style=file (Join-Path $generated $file)
    if ($LASTEXITCODE -ne 0) { throw "Formatting $file failed." }
  }
  $destination = Join-Path ([System.IO.Path]::GetFullPath($OutputDirectory)) "src\generated"
  New-Item -ItemType Directory -Path $destination -Force | Out-Null
  foreach ($file in $files) {
    Copy-Item -LiteralPath (Join-Path $generated $file) -Destination $destination -Force
  }
}
finally {
  if (Test-Path -LiteralPath $modules) { [System.IO.Directory]::Delete($modules) }
  Remove-Item -LiteralPath $work -Recurse -Force
}
