# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# Usage: Get-ApiViewCommandLine.ps1 .\azure-sdk-for-cpp azure-identity
# Or: ..\Get-ApiViewCommandLine.ps1 . azure-core
# Or: Get-ApiViewCommandLine.ps1 ..\.. azure-security-attestation
# Or: c:\src\azure-sdk-for-cpp\eng\scripts\Get-ApiViewCommandLine.ps1 c:\src\azure-sdk-for-cpp azure-identity

param([String]$RepoPath, [String]$LibName)

[String]$SdkRoot = Resolve-Path ($RepoPath + "\sdk")

[String[]]$AllIncDirs = Get-ChildItem -Directory -Filter "inc" -Recurse $SdkRoot | Select-Object -ExpandProperty FullName

[String[]]$AllIncDirsWithoutInc = $AllIncDirs | Select-Object @{ Label="Substr"; Expression = { $_.Substring(0, $_.Length - "inc".Length) } } | Select-Object -ExpandProperty Substr

[String[]]$AllLibIncDirs = @()
for($i = 0; $i -lt $AllIncDirsWithoutInc.Length; $i++) {
    $isLibDir = $true
    $libDir = $AllIncDirsWithoutInc[$i]
    for($j = 0; $j -lt $AllIncDirsWithoutInc.Length; $j++) {
        if ($i -eq $j) {
            continue
        }

        $StartsWith = $AllIncDirsWithoutInc[$j] + "*"
        if ($libDir -Like $StartsWith) {
            $isLibDir = $false
            break
        }
    }

    if ($isLibDir){
        $AllLibIncDirs += $libDir + "inc"
    }
}

[String]$LibIncDir = $AllLibIncDirs | Where-Object {$_ -Match ("\\" + $LibName + "\\inc") } | Select-Object -First 1

[String[]]$LibHeaders = Get-ChildItem -File -Recurse $LibIncDir | Select-Object -ExpandProperty FullName

$CmdLine = "clang++"
foreach ($header in $LibHeaders) {
    $CmdLine += " " + $header
}

$CmdLine += " -Xclang -ast-dump"

foreach ($incDir in $AllLibIncDirs) {
    $CmdLine += " -I " + $incDir
}

$CmdLine
