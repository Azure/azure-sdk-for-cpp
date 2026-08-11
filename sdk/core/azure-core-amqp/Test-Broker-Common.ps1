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

  # The file holds the process ID on the first line, and the start time in ticks on the second.
  # Older files hold the ID alone, and those still work with the name check below.
  $recorded = @(Get-Content -Path $ProcessIdPath -ErrorAction SilentlyContinue)
  Remove-Item $ProcessIdPath -Force -ErrorAction SilentlyContinue

  $processId = 0
  if ($recorded.Count -lt 1 -or
    -not [int]::TryParse(("" + $recorded[0]).Trim(), [ref] $processId) -or
    $processId -le 0) {
    Write-Host "The test broker process ID file did not hold a process ID."
    return
  }

  $recordedTicks = $null
  if ($recorded.Count -ge 2) {
    $parsedTicks = [long] 0
    if ([long]::TryParse(("" + $recorded[1]).Trim(), [ref] $parsedTicks)) {
      $recordedTicks = $parsedTicks
    }
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

  # A recycled process ID can belong to another dotnet process. Compare the start time, which
  # makes the pair unique. A start time that cannot be read falls back to the name check above,
  # because a broker that keeps running holds the port and breaks every later run.
  if ($null -ne $recordedTicks) {
    $actualTicks = $null
    try { $actualTicks = $process.StartTime.Ticks } catch { $actualTicks = $null }

    if ($null -eq $actualTicks) {
      Write-Host "The start time of process $processId is not readable. Stopping it on the name alone."
    }
    elseif ($actualTicks -ne $recordedTicks) {
      Write-Host ("Process $processId started at a different time than the recorded broker, so " +
        "the operating system gave this ID to another process. This script leaves it alone.")
      return
    }
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
