# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
# cspell: ignore AIDEV cfsclean depsfile

. "$PSScriptRoot\..\..\..\eng\common\scripts\common.ps1"

if ($IsMacOS) {
  Write-Host "AMQP tests are not supported on macOS. Skipping test setup."
  exit 0
}

function Invoke-RequiredCommand {
  param(
    [Parameter(Mandatory = $true)]
    [string] $Description,

    [Parameter(Mandatory = $true)]
    [string] $FilePath,

    [Parameter(Mandatory = $true)]
    [string[]] $ArgumentList
  )

  Write-Host "> $FilePath $($ArgumentList -join ' ')"
  & $FilePath @ArgumentList
  if ($LASTEXITCODE -ne 0) {
    throw "$Description failed with exit code $LASTEXITCODE."
  }
}

function Write-BrokerLogs {
  param(
    [Parameter(Mandatory = $true)]
    [string] $StandardOutputPath,

    [Parameter(Mandatory = $true)]
    [string] $StandardErrorPath
  )

  foreach ($log in @(
      @{ Name = "standard output"; Path = $StandardOutputPath },
      @{ Name = "standard error"; Path = $StandardErrorPath }
    )) {
    Write-Host "Test broker $($log.Name):"
    if (Test-Path $log.Path) {
      $content = Get-Content -Path $log.Path -Raw
      if ($content) {
        Write-Host $content
      }
      else {
        Write-Host "<empty>"
      }
    }
    else {
      Write-Host "<not created>"
    }
  }
}

function Test-TcpEndpoint {
  param(
    [Parameter(Mandatory = $true)]
    [string] $HostName,

    [Parameter(Mandatory = $true)]
    [int] $Port,

    [int] $TimeoutMilliseconds = 1000
  )

  $client = [System.Net.Sockets.TcpClient]::new()
  try {
    $connection = $client.ConnectAsync($HostName, $Port)
    return $connection.Wait($TimeoutMilliseconds) -and $client.Connected
  }
  catch {
    return $false
  }
  finally {
    $client.Dispose()
  }
}

function Wait-BrokerReady {
  param(
    [Parameter(Mandatory = $true)]
    [System.Diagnostics.Process] $Process,

    [Parameter(Mandatory = $true)]
    [System.Uri] $Address,

    [int] $TimeoutSeconds = 30
  )

  $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
  while ((Get-Date) -lt $deadline) {
    $Process.Refresh()
    if ($Process.HasExited) {
      throw "Test broker exited before becoming ready with exit code $($Process.ExitCode)."
    }

    if (Test-TcpEndpoint -HostName $Address.DnsSafeHost -Port $Address.Port) {
      Write-Host "Test broker is accepting connections on $($Address.DnsSafeHost):$($Address.Port)."
      return
    }

    Start-Sleep -Milliseconds 500
  }

  throw "Test broker did not accept connections on $($Address.DnsSafeHost):$($Address.Port) within $TimeoutSeconds seconds."
}

$WorkingDirectory = [System.IO.Path]::GetFullPath(
  [System.IO.Path]::Combine($RepoRoot, "../TestArtifacts"))
$repositoryDir = [System.IO.Path]::Combine($WorkingDirectory, "azure-amqp")
$standardOutputPath = [System.IO.Path]::Combine($WorkingDirectory, "test-broker.log")
$standardErrorPath = [System.IO.Path]::Combine($WorkingDirectory, "test-broker-error.log")
$repositoryUrl = "https://github.com/Azure/azure-amqp.git"
$repositoryHash = "239aff0d87b2c19e1fa91636e0fc0f6ee6e9999a"

$env:TEST_BROKER_ADDRESS = if ($env:TEST_BROKER_ADDRESS) {
  $env:TEST_BROKER_ADDRESS
}
else {
  "amqp://127.0.0.1:25672"
}

$brokerAddress = [System.Uri] $env:TEST_BROKER_ADDRESS
if ($brokerAddress.Port -le 0) {
  throw "TEST_BROKER_ADDRESS must include a valid port: $($env:TEST_BROKER_ADDRESS)"
}

Write-Host "Using working directory $WorkingDirectory"
New-Item -ItemType Directory -Path $WorkingDirectory -Force | Out-Null

if (Test-Path $repositoryDir) {
  Write-Host "Removing previously cloned repository $repositoryDir"
  Remove-Item $repositoryDir -Force -Recurse
}
Remove-Item $standardOutputPath, $standardErrorPath -Force -ErrorAction SilentlyContinue

$process = $null
try {
  # AIDEV-NOTE: Keep the explicit fetch and checkout for agents whose Git does not support
  # clone --revision.
  Invoke-RequiredCommand `
    -Description "Test broker clone" `
    -FilePath "git" `
    -ArgumentList @(
      "clone",
      "--no-checkout",
      "--depth", "1",
      $repositoryUrl,
      $repositoryDir
    )

  Invoke-RequiredCommand `
    -Description "Test broker commit fetch" `
    -FilePath "git" `
    -ArgumentList @(
      "-C", $repositoryDir,
      "fetch",
      "--depth", "1",
      "origin",
      $repositoryHash
    )

  Invoke-RequiredCommand `
    -Description "Test broker commit checkout" `
    -FilePath "git" `
    -ArgumentList @(
      "-C", $repositoryDir,
      "checkout",
      "--detach",
      $repositoryHash
    )

  $actualHash = & git -C $repositoryDir rev-parse HEAD
  $actualHash = "$actualHash".Trim()
  if ($LASTEXITCODE -ne 0 -or $actualHash -ne $repositoryHash) {
    throw "Test broker clone resolved to '$actualHash' instead of '$repositoryHash'."
  }

  Push-Location $repositoryDir
  try {
    Invoke-RequiredCommand `
      -Description "Test broker restore" `
      -FilePath "dotnet" `
      -ArgumentList @(
        "restore",
        "./test/TestAmqpBroker/TestAmqpBroker.csproj",
        "--configfile", "./nuget.cfsclean.config"
      )

    Invoke-RequiredCommand `
      -Description "Test broker build" `
      -FilePath "dotnet" `
      -ArgumentList @(
        "build",
        "./test/TestAmqpBroker/TestAmqpBroker.csproj",
        "--configuration", "Debug",
        "--framework", "net10.0",
        "--no-restore"
      )
  }
  finally {
    Pop-Location
  }

  $brokerDirectory = [System.IO.Path]::Combine(
    $repositoryDir, "bin", "Debug", "TestAmqpBroker", "net10.0")
  $brokerDll = [System.IO.Path]::Combine($brokerDirectory, "TestAmqpBroker.dll")
  if (-not (Test-Path $brokerDll)) {
    throw "Test broker build did not produce $brokerDll."
  }

  if (Test-TcpEndpoint -HostName $brokerAddress.DnsSafeHost -Port $brokerAddress.Port) {
    throw "The test broker endpoint $($brokerAddress.DnsSafeHost):$($brokerAddress.Port) is already in use."
  }

  Write-Host "Starting test broker on $($env:TEST_BROKER_ADDRESS)"
  $process = Start-Process `
    -FilePath "dotnet" `
    -ArgumentList @(
      "exec",
      "./TestAmqpBroker.dll",
      $env:TEST_BROKER_ADDRESS,
      "/headless"
    ) `
    -WorkingDirectory $brokerDirectory `
    -RedirectStandardOutput $standardOutputPath `
    -RedirectStandardError $standardErrorPath `
    -PassThru

  if (-not $process) {
    throw "Start-Process did not return a test broker process."
  }

  $env:TEST_BROKER_JOBID = $process.Id
  Write-Host "Test broker process ID: $($process.Id)"
  Wait-BrokerReady -Process $process -Address $brokerAddress
}
catch {
  Write-Error "Test broker setup failed: $_"
  Write-BrokerLogs `
    -StandardOutputPath $standardOutputPath `
    -StandardErrorPath $standardErrorPath

  if ($process) {
    $process.Refresh()
    if (-not $process.HasExited) {
      Stop-Process -Id $process.Id -ErrorAction SilentlyContinue
    }
  }

  throw
}
