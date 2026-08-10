# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

. "$PSScriptRoot\..\..\..\eng\common\scripts\common.ps1"
. "$PSScriptRoot\Test-Broker-Common.ps1"

if ($IsMacOS) {
  Write-Host "AMQP tests are not supported on macOS. Skipping test cleanup."
  exit 0
}

$paths = Get-TestBrokerPaths -RepositoryRoot $RepoRoot

Write-BrokerLogs `
  -StandardOutputPath $paths.StandardOutputPath `
  -StandardErrorPath $paths.StandardErrorPath

# This step runs after the tests, and it runs even when the tests fail. It must not turn a
# test failure into a cleanup failure, so it always reports success.
Stop-TestBroker -ProcessIdPath $paths.ProcessIdPath
exit 0
