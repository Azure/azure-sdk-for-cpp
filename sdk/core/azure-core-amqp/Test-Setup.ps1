# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
# cspell: ignore cfsclean NETSDK

. "$PSScriptRoot\..\..\..\eng\common\scripts\common.ps1"
. "$PSScriptRoot\Test-Broker-Common.ps1"

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

function Test-BrokerPinReachable {
  param(
    [Parameter(Mandatory = $true)]
    [string] $RepositorySlug,

    [Parameter(Mandatory = $true)]
    [string] $CommitHash,

    [Parameter(Mandatory = $true)]
    [string] $BranchName,

    [int] $TimeoutSeconds = 15
  )

  # Returns "reachable", "unreachable", or "unknown".
  #
  # This script makes a --depth 1 clone, which holds no commit graph, so
  # git merge-base --is-ancestor cannot answer this question. The GitHub compare API answers it
  # in one request, and that request works without a token on a public repository.
  #
  # Read ahead_by, not status. A reachable commit gives status "identical" or "behind", and both
  # of those give ahead_by 0. A commit that is not on the branch gives ahead_by above 0.
  $uri = "https://api.github.com/repos/$RepositorySlug/compare/$BranchName...$CommitHash"
  Write-Host "> GET $uri"

  try {
    $comparison = Invoke-RestMethod `
      -Uri $uri `
      -Method Get `
      -TimeoutSec $TimeoutSeconds `
      -Headers @{
        "Accept"     = "application/vnd.github+json"
        "User-Agent" = "azure-sdk-for-cpp-test-setup"
      }
  }
  catch {
    # A network error, a non-200 answer, and the 60 request per hour anonymous rate limit all
    # arrive here. None of them says anything about the pin.
    Write-Host "The compare request failed: $($_.Exception.Message)"
    return "unknown"
  }

  # Test the raw field for absence FIRST. `$null -as [int]` gives 0, not $null, so a coercion
  # on its own would turn a missing field into 0 and read as "reachable".
  if ($null -eq $comparison -or $null -eq $comparison.ahead_by) {
    Write-Host "The compare answer held no ahead_by field."
    return "unknown"
  }

  # Then coerce. A non-numeric ahead_by must count as "the test did not run", and must not
  # read as "not reachable" and fail a build.
  $aheadBy = $comparison.ahead_by -as [int]
  if ($null -eq $aheadBy) {
    Write-Host "The compare answer held a non-numeric ahead_by field."
    return "unknown"
  }

  Write-Host "Compare answer: status=$($comparison.status) ahead_by=$aheadBy"
  if ($aheadBy -eq 0) {
    return "reachable"
  }

  return "unreachable"
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

$paths = Get-TestBrokerPaths -RepositoryRoot $RepoRoot
$WorkingDirectory = $paths.WorkingDirectory
$repositoryDir = $paths.RepositoryDir
$standardOutputPath = $paths.StandardOutputPath
$standardErrorPath = $paths.StandardErrorPath
$processIdPath = $paths.ProcessIdPath
$repositorySlug = "Azure/azure-amqp"
$repositoryUrl = "https://github.com/${repositorySlug}.git"
$repositoryBranch = "master"

# The broker pin. This is a full 40 character commit SHA, so the build stays reproducible.
# A tag is not a safe substitute, because azure-amqp uses lightweight tags that a maintainer
# can move.
#
# This SHA comes from refs/pull/318/head of Azure/azure-amqp. It is not on master yet, and
# that is intentional for now. The reachability test below reports this as a warning.
#
# To update the pin:
#   1. Pick the new commit from Azure/azure-amqp. Prefer a commit on master.
#      Azure/azure-amqp squash-merges, so the commit on master is the merge_commit_sha of the
#      merged pull request, not the head commit of that pull request.
#   2. Run the manual restore and build steps in README.md against that commit.
#   3. Run the C++ AMQP tests against the broker that the commit builds.
#   4. Make sure that the commit contains nuget.cfsclean.config.
#   5. Replace the SHA below with the full 40 character SHA, and update README.md.
#
# Set TEST_BROKER_COMMIT to point a pipeline at a different broker commit without a code change.
$defaultRepositoryHash = "239aff0d87b2c19e1fa91636e0fc0f6ee6e9999a"

$repositoryHash = if ($env:TEST_BROKER_COMMIT) {
  $env:TEST_BROKER_COMMIT.Trim().ToLowerInvariant()
}
else {
  $defaultRepositoryHash
}

if ($repositoryHash -notmatch '^[0-9a-f]{40}$') {
  throw "TEST_BROKER_COMMIT must be a full 40 character commit SHA: '$repositoryHash'"
}

if ($repositoryHash -ne $defaultRepositoryHash) {
  Write-Host "TEST_BROKER_COMMIT overrides the broker pin with $repositoryHash."
}

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

# Stop a broker that an earlier run left behind. A pipeline agent can be reused, and a stage
# re-run starts this script again on the same machine. Without this step the old broker keeps
# the port, and this run cannot start its own broker.
Stop-TestBroker -ProcessIdPath $processIdPath

if (Test-Path $repositoryDir) {
  Write-Host "Removing previously cloned repository $repositoryDir"
  Remove-Item $repositoryDir -Force -Recurse
}
Remove-Item $standardOutputPath, $standardErrorPath -Force -ErrorAction SilentlyContinue

$process = $null
try {
  # Keep the clone, the fetch, and the checkout as three steps. A single clone --revision call
  # would do the same work, but that option needs Git 2.49 or later, and some pipeline agents
  # carry an older Git.
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

  # Report a pin that does not sit on the upstream branch. A pull request head can disappear,
  # and a pin that is only on a pull request head cannot be rebuilt after the branch is deleted.
  $pinState = Test-BrokerPinReachable `
    -RepositorySlug $repositorySlug `
    -CommitHash $repositoryHash `
    -BranchName $repositoryBranch

  switch ($pinState) {
    "reachable" {
      Write-Host "Broker pin $repositoryHash is reachable from $repositorySlug $repositoryBranch."
    }

    "unreachable" {
      $pinMessage = @(
        "Broker pin $repositoryHash is not reachable from $repositorySlug $repositoryBranch.",
        "$repositorySlug squash-merges its pull requests, so the head commit of a pull request",
        "never lands on $repositoryBranch.",
        "If the source pull request has merged, set the pin to its merge_commit_sha, which is",
        "the squash commit on $repositoryBranch.",
        "Do not use the merge_commit_sha of an open pull request, because that commit is a",
        "temporary test merge."
      ) -join " "

      if ($env:TEST_BROKER_REQUIRE_MERGED) {
        throw "$pinMessage TEST_BROKER_REQUIRE_MERGED is set, so this is an error."
      }

      Write-Warning ("$pinMessage The setup continues. Set TEST_BROKER_REQUIRE_MERGED to make " +
        "this an error.")
    }

    default {
      # "unknown" means the test itself did not run. Continue always, even when
      # TEST_BROKER_REQUIRE_MERGED is set. A shared CI address can exhaust the 60 request per
      # hour anonymous rate limit, and that says nothing about the pin. A failure to test must
      # never become a new source of flaky builds.
      Write-Warning ("The script could not test whether broker pin $repositoryHash is reachable " +
        "from $repositorySlug $repositoryBranch. The setup continues.")
    }
  }

  # The restore config belongs to this repository, and not to the broker clone. The restricted
  # feed policy is this pipeline's requirement, so the file that satisfies it sits next to this
  # script. Pass an absolute path, because the dotnet calls run from the clone root.
  $nugetConfig = [System.IO.Path]::Combine($PSScriptRoot, "nuget.cfsclean.config")
  if (-not (Test-Path $nugetConfig)) {
    throw "This repository does not contain $nugetConfig."
  }

  # Push-Location is load-bearing. The dotnet calls below must run from the clone root, because
  # the clone root holds the global.json that selects the .NET SDK, and because the paths below
  # are relative to it. Do not replace these paths with absolute paths and drop the location
  # change. That silently loses the SDK pin.
  Push-Location $repositoryDir
  try {
    # The restore covers every target framework, and the build below covers only net10.0. That
    # is intentional. A -p:TargetFramework argument is an MSBuild global property, so it flows
    # into the netstandard2.0 Microsoft.Azure.Amqp project reference, and the build then fails
    # with NETSDK1005. Do not add a framework filter here.
    Invoke-RequiredCommand `
      -Description "Test broker restore" `
      -FilePath "dotnet" `
      -ArgumentList @(
        "restore",
        "./test/TestAmqpBroker/TestAmqpBroker.csproj",
        "--configfile", $nugetConfig
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

  # A broker from an earlier run of this script is already gone, because Stop-TestBroker ran
  # above. A process that still holds the port belongs to somebody else. This script did not
  # start it, so this script does not stop it. Run the tests against it and report the reuse.
  # A busy port is not a reason to fail the run.
  if (Test-TcpEndpoint -HostName $brokerAddress.DnsSafeHost -Port $brokerAddress.Port) {
    Write-Warning ("A process already listens on " +
      "$($brokerAddress.DnsSafeHost):$($brokerAddress.Port). This script did not start it, so " +
      "this script will not stop it. The tests will use that endpoint. Stop that process and " +
      "run this script again if the tests fail in an unexpected way.")
    exit 0
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

  # Record the process ID for Test-Cleanup.ps1. That script runs in a separate pwsh step, so an
  # environment variable set here does not reach it.
  Set-Content -Path $processIdPath -Value $process.Id -NoNewline
  Write-Host "Test broker process ID: $($process.Id) (recorded in $processIdPath)"
  Wait-BrokerReady -Process $process -Address $brokerAddress
}
catch {
  # Stop the broker and drop its record BEFORE anything that can terminate this block. The
  # pipeline pwsh task sets $ErrorActionPreference to "stop", which makes Write-Error a
  # terminating error, so a Write-Error here would skip every line after it and leave a broker
  # running that no later step can find. `throw` at the end reports the original error, so no
  # Write-Error is needed.
  if ($process) {
    $process.Refresh()
    if (-not $process.HasExited) {
      Stop-Process -Id $process.Id -ErrorAction SilentlyContinue
    }
  }

  # This run owns no broker now, so the record must go. Otherwise Test-Cleanup.ps1 or the next
  # run reads a stale process ID.
  Remove-Item $processIdPath -Force -ErrorAction SilentlyContinue

  # Read the logs after the stop, so the broker no longer holds the files open.
  Write-BrokerLogs `
    -StandardOutputPath $standardOutputPath `
    -StandardErrorPath $standardErrorPath

  throw
}
