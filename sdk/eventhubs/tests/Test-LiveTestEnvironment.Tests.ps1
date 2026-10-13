# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

BeforeAll {
    $serviceDirectory = Split-Path $PSScriptRoot -Parent
    $scriptPath = Join-Path $serviceDirectory 'Test-LiveTestEnvironment.ps1'
    $template = Get-Content (Join-Path $serviceDirectory 'azure-messaging-eventhubs/test-resources.json') -Raw | ConvertFrom-Json
}

Describe 'Event Hubs test resources' -Tag 'UnitTest', 'Resources' {
    It 'uses a general-purpose v2 account for checkpoints' {
        $storageAccount = @($template.resources | Where-Object type -EQ 'Microsoft.Storage/storageAccounts')
        $storageAccount.Count | Should -Be 1
        $storageAccount[0].kind | Should -Be 'StorageV2'
        $storageAccount[0].sku.name | Should -Be 'Standard_LRS'
        $storageAccount[0].properties.allowSharedKeyAccess | Should -BeFalse
    }
}

Describe 'Event Hubs live-test environment' -Tag 'UnitTest' {
    BeforeAll {
        $requiredVariables = @(
            'EVENTHUB_NAME'
            'EVENTHUBS_NAMESPACE'
            'EVENTHUBS_HOST'
            'EVENTHUB_CONSUMER_GROUP'
            'CHECKPOINTSTORE_STORAGE_URL'
        )
    }

    BeforeEach {
        $savedVariables = @{}
        foreach ($name in $requiredVariables) {
            $savedVariables[$name] = [Environment]::GetEnvironmentVariable($name)
            [Environment]::SetEnvironmentVariable($name, 'test-value')
        }
    }

    AfterEach {
        foreach ($name in $requiredVariables) {
            [Environment]::SetEnvironmentVariable($name, $savedVariables[$name])
        }
    }

    It 'accepts a complete environment without changing its values' {
        { & $scriptPath } | Should -Not -Throw
        foreach ($name in $requiredVariables) {
            [Environment]::GetEnvironmentVariable($name) | Should -Be 'test-value'
        }
    }

    It 'requires variables that the deployment template publishes' {
        foreach ($name in $requiredVariables) {
            $template.outputs.PSObject.Properties.Name | Should -Contain $name
        }
    }

    It 'rejects a missing <Name>' -ForEach @(
        @{ Name = 'EVENTHUB_NAME' }
        @{ Name = 'EVENTHUBS_NAMESPACE' }
        @{ Name = 'EVENTHUBS_HOST' }
        @{ Name = 'EVENTHUB_CONSUMER_GROUP' }
        @{ Name = 'CHECKPOINTSTORE_STORAGE_URL' }
    ) {
        [Environment]::SetEnvironmentVariable($Name, $null)
        { & $scriptPath } | Should -Throw "*Missing required Event Hubs live-test environment variables: $Name.*"
    }

    It 'rejects whitespace in <Name>' -ForEach @(
        @{ Name = 'EVENTHUB_NAME' }
        @{ Name = 'EVENTHUBS_NAMESPACE' }
        @{ Name = 'EVENTHUBS_HOST' }
        @{ Name = 'EVENTHUB_CONSUMER_GROUP' }
        @{ Name = 'CHECKPOINTSTORE_STORAGE_URL' }
    ) {
        [Environment]::SetEnvironmentVariable($Name, " `t")
        { & $scriptPath } | Should -Throw "*Missing required Event Hubs live-test environment variables: $Name.*"
    }

    It 'reports all missing variables without printing their values' {
        foreach ($name in $requiredVariables) {
            [Environment]::SetEnvironmentVariable($name, $null)
        }
        [Environment]::SetEnvironmentVariable('EVENTHUB_NAME', 'do-not-log-this-value')

        $failure = { & $scriptPath } | Should -Throw -PassThru
        foreach ($name in $requiredVariables | Where-Object { $_ -ne 'EVENTHUB_NAME' }) {
            $failure.Exception.Message | Should -Match $name
        }
        $failure.Exception.Message | Should -Not -Match 'do-not-log-this-value'
    }
}
