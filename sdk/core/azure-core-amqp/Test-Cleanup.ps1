# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

. "$PSScriptRoot\..\..\..\eng\common\scripts\common.ps1"
. "$PSScriptRoot\Test-Broker-Common.ps1"

if ($IsMacOS) {
  Write-Host "AMQP tests are not supported on macOS. Skipping test cleanup."
  exit 0
}

$paths = Get-TestBrokerPaths -RepositoryRoot $RepoRoot

# Stop the broker first. It holds the log files open while it runs, and a read of an open file
# can fail on Windows. This step also runs after the tests, and it runs even when the tests
# fail, so it must not turn a test failure into a cleanup failure. It always reports success.
Stop-TestBroker -ProcessIdPath $paths.ProcessIdPath

Write-BrokerLogs `
  -StandardOutputPath $paths.StandardOutputPath `
  -StandardErrorPath $paths.StandardErrorPath

exit 0
