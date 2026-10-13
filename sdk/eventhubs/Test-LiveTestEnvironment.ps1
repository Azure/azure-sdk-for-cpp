# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

<#
.SYNOPSIS
Validates the resource settings required by Event Hubs live tests and samples.
#>
[CmdletBinding()]
param()

Set-StrictMode -Version 4
$ErrorActionPreference = 'Stop'

$requiredVariables = @(
    'EVENTHUB_NAME'
    'EVENTHUBS_NAMESPACE'
    'EVENTHUBS_HOST'
    'EVENTHUB_CONSUMER_GROUP'
    'CHECKPOINTSTORE_STORAGE_URL'
)
$missingVariables = @(
    foreach ($name in $requiredVariables) {
        if ([string]::IsNullOrWhiteSpace([Environment]::GetEnvironmentVariable($name))) {
            $name
        }
    }
)

if ($missingVariables.Count -gt 0) {
    throw "Missing required Event Hubs live-test environment variables: $($missingVariables -join ', '). Make sure the test resource deployment succeeded."
}

Write-Host 'Event Hubs live-test resource settings are present.'
