# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

# Shared state for the AMQP test broker.
#
# Test-Setup.ps1 starts the broker and writes its process ID to a file. Test-Cleanup.ps1
# reads that file and stops the process. The two scripts run in separate pwsh steps, so an
# environment variable does not carry the process ID between them. A file in the working
# directory does, and it also works for a developer who runs the scripts by hand.
#
# Both scripts must agree on these paths, so the paths live here.

function Get-TestBrokerPaths {
  param(
    [Parameter(Mandatory = $true)]
    [string] $RepositoryRoot
  )

  $workingDirectory = [System.IO.Path]::GetFullPath(
    [System.IO.Path]::Combine($RepositoryRoot, "../TestArtifacts"))

  return [pscustomobject]@{
    WorkingDirectory   = $workingDirectory
    RepositoryDir      = [System.IO.Path]::Combine($workingDirectory, "azure-amqp")
    StandardOutputPath = [System.IO.Path]::Combine($workingDirectory, "test-broker.log")
    StandardErrorPath  = [System.IO.Path]::Combine($workingDirectory, "test-broker-error.log")
    ProcessIdPath      = [System.IO.Path]::Combine($workingDirectory, "test-broker.pid")
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

function Stop-TestBroker {
  param(
    [Parameter(Mandatory = $true)]
    [string] $ProcessIdPath
  )

  # This function must be idempotent and must never throw. Test-Setup.ps1 calls it to clear a
  # broker that an earlier run left behind, and Test-Cleanup.ps1 calls it after the tests. A
  # broker that already exited is a normal state, not an error.
  if (-not (Test-Path $ProcessIdPath)) {
    Write-Host "No test broker process ID file at $ProcessIdPath. There is nothing to stop."
    return
  }

  $recordedId = (Get-Content -Path $ProcessIdPath -Raw).Trim()
  Remove-Item $ProcessIdPath -Force -ErrorAction SilentlyContinue

  $processId = 0
  if (-not [int]::TryParse($recordedId, [ref] $processId) -or $processId -le 0) {
    Write-Host "The test broker process ID file held '$recordedId', which is not a process ID."
    return
  }

  $process = Get-Process -Id $processId -ErrorAction SilentlyContinue
  if (-not $process) {
    Write-Host "Test broker process $processId already exited."
    return
  }

  # The operating system reuses process IDs. Make sure that this process is still the broker
  # before you stop it. Test-Setup.ps1 starts the broker with the dotnet host.
  if ($process.ProcessName -ne "dotnet") {
    Write-Host ("Process $processId is '$($process.ProcessName)', not the test broker. " +
      "This script leaves it alone.")
    return
  }

  Write-Host "Stopping test broker process $processId."
  Stop-Process -Id $processId -Force -ErrorAction SilentlyContinue
  if ($process.WaitForExit(10000)) {
    Write-Host "Test broker stopped."
  }
  else {
    Write-Host "Test broker process $processId did not exit within 10 seconds."
  }
}
