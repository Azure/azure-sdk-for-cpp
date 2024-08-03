<!-- cspell:words azurecli azureclicredential -->
# Troubleshoot Azure Identity authentication issues

This troubleshooting guide covers failure investigation techniques, common errors for the credential types in the Azure Identity library for C++, and mitigation steps to resolve these errors.

## Table of contents

- [Troubleshoot AzureCliCredential authentication issues](#troubleshoot-azureclicredential-authentication-issues)
- [Troubleshoot AzurePipelinesCredential authentication issues](#troubleshoot-azurepipelinescredential-authentication-issues)
- [Get additional help](#get-additional-help)

## Troubleshoot `AzureCliCredential` authentication issues

`AuthenticationException`

| Error Message |Description| Mitigation |
|---|---|---|
|Azure CLI not installed|The Azure CLI isn't installed or couldn't be found.|<ul><li>Ensure the Azure CLI is properly installed. Installation instructions can be found [here](https://learn.microsoft.com/cli/azure/install-azure-cli).</li><li>Validate the installation location has been added to the `PATH` environment variable.</li></ul>|
|Please run 'az login' to set up account|No account is currently logged into the Azure CLI, or the login has expired.|<ul><li>Log in to the Azure CLI using the `az login` command. More information on authentication in the Azure CLI can be found [here](https://learn.microsoft.com/cli/azure/authenticate-azure-cli).</li><li>Validate that the Azure CLI can obtain tokens. For instructions, see [below](#verify-the-azure-cli-can-obtain-tokens).</li></ul>|

### Verify the Azure CLI can obtain tokens

You can manually verify that the Azure CLI is properly authenticated and can obtain tokens. First, use the `account` command to verify the account that is currently logged in to the Azure CLI.

```azurecli
az account show
```

Once you've verified the Azure CLI is using the correct account, you can validate that it's able to obtain tokens for that account.

```azurecli
az account get-access-token --output json --resource https://management.core.windows.net
```

> **Note:** The output of this command will contain an access token and SHOULD NOT BE SHARED, to avoid compromising account security.

## Troubleshoot `AzurePipelinesCredential` authentication issues

| Error Message |Description| Mitigation |
|---|---|---|
| AADSTS900023: Specified tenant identifier `<some tenant ID>` is neither a valid DNS name, nor a valid external domain.|The `tenantId` argument to the `AzurePipelinesCredential` constructor is incorrect| Verify the tenant ID. It must identify the tenant of the user-assigned managed identity or service principal configured for the service connection.|
| No service connection found with identifier `<GUID>` |The `serviceConnectionId` argument to the `AzurePipelinesCredential` constructor is incorrect| Verify the service connection ID. This parameter refers to the `resourceId` of the Azure Service Connection. It can also be found in the query string of the service connection's configuration in Azure DevOps. [Azure Pipelines documentation](https://learn.microsoft.com/azure/devops/pipelines/library/service-endpoints?view=azure-devops&tabs=yaml) has more information about service connections.|
|302 (Found) response from OIDC endpoint|The `systemAccessToken` argument to the `AzurePipelinesCredential` constructor is incorrect|Check pipeline configuration. This value comes from the predefined variable `System.AccessToken` [as described in Azure Pipelines documentation](https://learn.microsoft.com/azure/devops/pipelines/build/variables?view=azure-devops&tabs=yaml#systemaccesstoken).|
|No value for environment variable `SYSTEM_OIDCREQUESTURI` needed by AzurePipelinesCredential. This should be set by Azure Pipelines. AuthenticationException: AzurePipelinesCredential authentication unavailable.|This code is not running inside of the Azure Pipelines environment. You may be running this code locally or on some other environment.|This credential is only designed to run inside the Azure Pipelines environment for the federated identity to work.|
|AADSTS700016: Application with identifier 'clientId' was not found in the directory 'Microsoft'. This can happen if the application has not been installed by the administrator of the tenant or consented to by any user in the tenant. You may have sent your authentication request to the wrong tenant.|The `<clientId>` provided is invalid|Verify the client ID argument is valid. If the service connection's federated identity was registered via a user-assigned managed identity, the client ID of the managed identity should be provided. If the service connection's federated identity is registered via an app registration, the Application (client) ID from your app registration should be provided.|

## Get additional help

Additional information on ways to reach out for support can be found in the [SUPPORT.md](https://github.com/Azure/azure-sdk-for-cpp/blob/main/SUPPORT.md) at the root of the repo.