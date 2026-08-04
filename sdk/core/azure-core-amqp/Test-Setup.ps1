# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
# cspell: ignore JOBID depsfile

# Load common ES scripts
. "$PSScriptRoot\..\..\..\eng\common\scripts\common.ps1"

# Create the test binary *outside* the repo root to avoid polluting the repo.
$WorkingDirectory = ([System.IO.Path]::Combine($RepoRoot, "../TestArtifacts"))

# Create the working directory if it does not exist.
Write-Host "Using Working Directory $WorkingDirectory"

if (-not (Test-Path $WorkingDirectory)) {
  Write-Host "Working directory does not exist, creating working directory: $WorkingDirectory"
  New-Item -ItemType Directory -Path $WorkingDirectory
}

Write-Host "Setting current directory to working directory: $WorkingDirectory"
Push-Location -Path $WorkingDirectory

# Clone and build the Test Amqp Broker.
try {

  $repositoryUrl = "https://github.com/Azure/azure-amqp.git"
  $repositoryCommit = "d82a86455c3459c5628bc95b25511f6e8a065598"
  $cloneCommand = "git clone $repositoryUrl"

  Write-Host "Cloning repository from $repositoryUrl..."
  Invoke-LoggedCommand $cloneCommand
  Invoke-LoggedCommand "git -C azure-amqp checkout --detach $repositoryCommit"

  $clonedNuGetConfig = Join-Path $WorkingDirectory "azure-amqp/nuget.config"
  $azureSdkFeed = "https://pkgs.dev.azure.com/azure-sdk/public/_packaging/azure-sdk-for-net/nuget/v3/index.json"
  dotnet nuget remove source "NuGet official package source" --configfile $clonedNuGetConfig
  dotnet nuget add source $azureSdkFeed --name azure-sdk-for-net --configfile $clonedNuGetConfig

  Set-Location -Path "./azure-amqp/test/TestAmqpBroker"

  Invoke-LoggedCommand "dotnet build -p RollForward=LatestMajor --framework net8.0"
  if (!$? -ne 0) {
    Write-Error "Failed to build TestAmqpBroker."
    exit 1
  }

  Write-Host "Test broker built successfully."

  # now that the Test broker has been built, launch the broker on a local address.
  Write-Host "Starting test broker listening on ${env:TEST_BROKER_ADDRESS} ..."

  Set-Location -Path $WorkingDirectory/azure-amqp/bin/Debug/TestAmqpBroker/net8.0

#  $job = dotnet exec ./TestAmqpBroker.dll ${env:TEST_BROKER_ADDRESS} /headless &
  $Process = Start-Process -NoNewWindow -FilePath "dotnet" -ArgumentList "exec ./TestAmqpBroker.dll ${env:TEST_BROKER_ADDRESS} /headless" -PassThru  -RedirectStandardOutput $WorkingDirectory/test-broker.log -RedirectStandardError $WorkingDirectory/test-broker-error.log

  if (!$? -ne 0) {
    Write-Error "Failed to start TestAmqpBroker."
    exit 1
  }

  $Process

  $env:TEST_BROKER_JOBID = $Process.Id

  Write-Host "Waiting for test broker to start..."
  Start-Sleep -Seconds 3

#  Write-Host "Job Output after wait:"
#  Receive-Job $job.Id
#
#  $job = Get-Job -Id $env:TEST_BROKER_JOBID
#  if ($job.State -ne "Running") {
#    Write-Host "Test broker failed to start."
#    exit 1
#  }
}
finally {
  Pop-Location
}
