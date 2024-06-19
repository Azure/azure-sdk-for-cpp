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
| AADSTS900023: Specified tenant identifier 'some tenant ID' is neither a valid DNS name, nor a valid external domain.|The `tenantId` argument to the `AzurePipelinesCredential` constructor is incorrect| Verify the tenant ID. It must identify the tenant of the user-assigned managed identity or service principal configured for the service connection.|
| No service connection found with identifier |The `serviceConnectionId` argument to the `AzurePipelinesCredential` constructor is incorrect| Verify the service connection ID. This parameter refers to the `resourceId` of the Azure Service Connection. It can also be found in the query string of the service connection's configuration in Azure DevOps. [Azure Pipelines documentation](https://learn.microsoft.com/azure/devops/pipelines/library/service-endpoints?view=azure-devops&tabs=yaml) has more information about service connections.|
|302 (Found) response from OIDC endpoint|The `systemAccessToken` argument to the `AzurePipelinesCredential` constructor is incorrect|Check pipeline configuration. This value comes from the predefined variable `System.AccessToken` [as described in Azure Pipelines documentation](https://learn.microsoft.com/azure/devops/pipelines/build/variables?view=azure-devops&tabs=yaml#systemaccesstoken).|

## Get additional help

Additional information on ways to reach out for support can be found in the [SUPPORT.md](https://github.com/Azure/azure-sdk-for-cpp/blob/main/SUPPORT.md) at the root of the repo.