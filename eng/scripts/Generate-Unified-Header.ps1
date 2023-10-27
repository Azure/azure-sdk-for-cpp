param(
    [Parameter(Mandatory, HelpMessage="Package name - 'azure-core' for example")][string]$PackageName,
    [Parameter(Mandatory, HelpMessage='Output header name. Will be generated in the <root>\sdk\ServiceName\PackageName\inc\azure directory.')][string]$OutputHeaderName,
    [Parameter(Mandatory, HelpMessage='Azure service name - "storage", or "keyvault" for example')][string]$ServiceName,
    [Parameter(HelpMessage='Display name for service used in header - "Key Vault Keys" for example"'][string]$PackageDisplayName="",
    [Parameter(HelpMessage="Allow headers in an 'internal' directory.")][bool]$AllowInternal=$False,
    [Parameter(HelpMessage='Output file name. If not specified, -OutputHeaderName will be used to construct the output file')][string]$OutputFile
)

if ($OutputFile.Length -EQ 0) {
  $OutputFile = Join-Path -Path $PWD.Path -ChildPath "sdk/$ServiceName/$PackageName/inc/azure/$OutputHeaderName"
}

# Check if the package directory exists
# Concatenate the service name and package directory to form the package path
$packagePath = Join-Path -Path $PWD.Path -ChildPath "sdk/$ServiceName/$PackageName/inc".

Write-Output  "Generate inclusive header for package $PackageName in service $ServiceName."
Write-Output "Inclusive header will be generated in: $OutputFile."


if (-not (Test-Path -Path $packagePath -PathType Container)) {
    Write-Error "Package directory does not exist: $packagePath"
    return
}

# Get the .hpp files in the package directory
$hppFiles = Get-ChildItem -Path $packagePath -Filter *.hpp -Recurse

@("Found {0} .hpp files in {1}" -f $hppFiles.Count, $packagePath)

# Create the output file
New-Item -Path $OutputFile -ItemType File -Force | Out-Null
Add-Content -Path $OutputFile -Value '// Copyright (c) Microsoft Corporation.'
Add-Content -Path $OutputFile -Value '// Licensed under the MIT License.'
if ($PackageDisplayName.Length -NE 0) {
  Add-Content -Path $OutputFile -Value '','/**'," * @brief Includes all public headers from the Azure $PackageDisplayName SDK library.",' *',' */',''
}
Add-Content -Path $OutputFile -Value "#pragma once","" -Encoding UTF8

# Sort the .hpp files by their full path, and convert to an array of strings for easier management.
$hppFiles = $hppFiles | Sort-Object -Property FullName
$sortedFiles = $hppFiles | ForEach-Object { $_.FullName }

# Loop through the .hpp files and output their paths to the output file
foreach ($file in $sortedFiles) {
  if ($file -NE $OutputFile)
  {
    if ($file -like "*_transport.hpp")
    {
      continue;
    }
    if ($file -like "*keyvault_keys.hpp" -and $PackageName -EQ "azure-security-keyvault-keys")
    {
      continue;
    }
    if ($AllowInternal -OR $file -notlike "*internal*" -AND $file -notlike "*private*") {
      $file = $file.Remove(0, $packagePath.Length-1)
      $file = $file.Replace("\", "/")
      Add-Content -Path $OutputFile -Value "#include ""$file""" -Encoding UTF8
    }
  }
}
