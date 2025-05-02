<!-- cspell:words azurecli azureclicredential -->
# Troubleshoot Azure Identity authentication issues

This troubleshooting guide covers failure investigation techniques, common errors for the credential types in the Azure Identity library for C++, and mitigation steps to resolve these errors.

## Table of contents

- [Troubleshoot AzureCliCredential authentication issues](#troubleshoot-azureclicredential-authentication-issues)
- [Troubleshoot AzurePipelinesCredential authentication issues](#troubleshoot-azurepipelinescredential-authentication-issues)
- [Troubleshoot ClientAssertionCredential authentication issues](#troubleshoot-clientassertioncredential-authentication-issues)
- [Troubleshoot ClientCertificateCredential authentication issues](#troubleshoot-clientcertificatecredential-authentication-issues)
- [Troubleshoot ClientSecretCredential authentication issues](#troubleshoot-clientsecretcredential-authentication-issues)
- [Troubleshoot DefaultAzureCredential authentication issues](#troubleshoot-defaultazurecredential-authentication-issues)
- [Troubleshoot EnvironmentCredential authentication issues](#troubleshoot-environmentcredential-authentication-issues)
- [Troubleshoot ManagedIdentityCredential authentication issues](#troubleshoot-managedidentitycredential-authentication-issues)
- [Troubleshoot WorkloadIdentityCredential authentication issues](#troubleshoot-workloadidentitycredential-authentication-issues)
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
|401 (Unauthorized) response from OIDC endpoint|The `systemAccessToken` argument to the `AzurePipelinesCredential` constructor is incorrect|Check pipeline configuration. This value comes from the predefined variable `System.AccessToken` [as described in Azure Pipelines documentation](https://learn.microsoft.com/azure/devops/pipelines/build/variables?view=azure-devops&tabs=yaml#systemaccesstoken).|
|No value for environment variable `SYSTEM_OIDCREQUESTURI` needed by AzurePipelinesCredential. This should be set by Azure Pipelines. AuthenticationException: AzurePipelinesCredential authentication unavailable.|This code is not running inside of the Azure Pipelines environment. You may be running this code locally or on some other environment.|This credential is only designed to run inside the Azure Pipelines environment for the federated identity to work.|
|AADSTS700016: Application with identifier 'clientId' was not found in the directory 'Microsoft'. This can happen if the application has not been installed by the administrator of the tenant or consented to by any user in the tenant. You may have sent your authentication request to the wrong tenant.|The `<clientId>` provided is invalid|Verify the client ID argument is valid. If the service connection's federated identity was registered via a user-assigned managed identity, the client ID of the managed identity should be provided. If the service connection's federated identity is registered via an app registration, the Application (client) ID from your app registration should be provided.|

## Troubleshoot `ClientAssertionCredential` authentication issues

`AuthenticationException`

| Error Code | Description | Mitigation |
|---|---|---|
|AADSTS700021| Client assertion application identifier doesn't match 'client_id' parameter. Review the documentation at [Microsoft Identity platform application authentication certificate credentials](https://learn.microsoft.com/entra/identity-platform/certificate-credentials). | Ensure the JWT assertion created has the correct values specified for the `sub` and `issuer` value of the payload, both of these should have the value be equal to `clientId`. Refer to the documentation for [client assertion format](https://learn.microsoft.com/entra/identity-platform/certificate-credentials).|
|AADSTS700023| Client assertion audience claim doesn't match Realm issuer. Review the documentation at [Microsoft Identity platform application authentication certificate credentials](https://learn.microsoft.com/entra/identity-platform/certificate-credentials). | Ensure the audience `aud` field in the JWT assertion created has the correct value for the audience specified in the payload. This should be set to `https://login.microsoftonline.com/{tenantId}/v2`.|
|AADSTS50027| JWT token is invalid or malformed. | Ensure the JWT assertion token is in the valid format. Refer to the documentation for [client assertion format](https://learn.microsoft.com/entra/identity-platform/certificate-credentials).|

## Troubleshoot `ClientCertificateCredential` authentication issues

`AuthenticationException`

| Error Code | Description | Mitigation |
|---|---|---|
|AADSTS700027|Client assertion contains an invalid signature.|Ensure the specified certificate has been uploaded to the Microsoft Entra application registration. Instructions for uploading certificates to the application registration can be found [here](https://learn.microsoft.com/entra/identity-platform/howto-create-service-principal-portal#option-1-recommended-upload-a-trusted-certificate-issued-by-a-certificate-authority).|
|AADSTS700016|The specified application wasn't found in the specified tenant.| Ensure the specified `clientId` and `tenantId` are correct for your application registration. For multi-tenant apps, ensure the application has been added to the desired tenant by a tenant admin. To add a new application in the desired tenant, follow the instructions [here](https://learn.microsoft.com/entra/identity-platform/howto-create-service-principal-portal).

## Troubleshoot `ClientSecretCredential` authentication issues

`AuthenticationException`

| Error Code | Issue | Mitigation |
|---|---|---|
|AADSTS7000215|An invalid client secret was provided.|Ensure the `clientSecret` provided when constructing the credential is valid. If unsure, create a new client secret using the Azure portal. Details on creating a new client secret can be found [here](https://learn.microsoft.com/entra/identity-platform/howto-create-service-principal-portal#option-3-create-a-new-client-secret).|
|AADSTS7000222|An expired client secret was provided.|Create a new client secret using the Azure portal. Details on creating a new client secret can be found [here](https://learn.microsoft.com/entra/identity-platform/howto-create-service-principal-portal#option-3-create-a-new-client-secret).|
|AADSTS700016|The specified application wasn't found in the specified tenant.|Ensure the specified `clientId` and `tenantId` are correct for your application registration. For multi-tenant apps, ensure the application has been added to the desired tenant by a tenant admin. To add a new application in the desired tenant, follow the instructions [here](https://learn.microsoft.com/entra/identity-platform/howto-create-service-principal-portal).|

## Troubleshoot `DefaultAzureCredential` authentication issues

| Error |Description| Mitigation |
|---|---|---|
|`AuthenticationException` raised with message. "DefaultAzureCredential failed to retrieve a token from the included credentials."|All credentials in the `DefaultAzureCredential` chain failed to retrieve a token, each throwing a `CredentialUnavailableException`.|<ul><li>[Enable logging](#enable-and-configure-logging) to verify the credentials being tried, and get further diagnostic information.</li><li>Consult the troubleshooting guide for underlying credential types for more information.</li><ul><li>[EnvironmentCredential](#troubleshoot-environmentcredential-authentication-issues)</li><li>[WorkloadIdentityCredential](#troubleshoot-workloadidentitycredential-authentication-issues)</li><li>[ManagedIdentityCredential](#troubleshoot-managedidentitycredential-authentication-issues)</li><li>[VisualStudioCredential](#troubleshoot-visualstudiocredential-authentication-issues)</li><li>[AzureCliCredential](#troubleshoot-azureclicredential-authentication-issues)</li><li>[AzurePowerShellCredential](#troubleshoot-azurepowershellcredential-authentication-issues)</li></ul>|
|`AuthenticationException` raised from the client with a status code of 401 or 403|Authentication succeeded but the authorizing Azure service responded with a 401 (Authenticate) or 403 (Forbidden) status code. This error can often be caused by the `DefaultAzureCredential` authenticating an account other than the intended or that the intended account doesn't have the correct permissions or roles assigned.|<ul><li>[Enable logging](#enable-and-configure-logging) to determine which credential in the chain returned the authenticating token.</li><li>In the case a credential other than the expected is returning a token, bypass this by either signing out of the corresponding development tool, or excluding the credential with the ExcludeXXXCredential property in the `DefaultAzureCredentialOptions`</li><li>Ensure that the correct role is assigned to the account being used. For example, a service specific role rather than the subscription Owner role.</li></ul>|

## Troubleshoot `EnvironmentCredential` authentication issues

`AuthenticationException`

| Error Message |Description| Mitigation |
|---|---|---|
|Environment variables aren't fully configured.|A valid combination of environment variables wasn't set.|Ensure the appropriate environment variables are set **prior to application startup** for the intended authentication method.</p><ul><li>To authenticate a service principal using a client secret, ensure the variables `AZURE_CLIENT_ID`, `AZURE_TENANT_ID` and `AZURE_CLIENT_SECRET` are properly set.</li><li>To authenticate a service principal using a certificate, ensure the variables `AZURE_CLIENT_ID`, `AZURE_TENANT_ID`, `AZURE_CLIENT_CERTIFICATE_PATH`, and optionally `AZURE_CLIENT_CERTIFICATE_PASSWORD` are properly set.</li><li>To authenticate a user using a password, ensure the variables `AZURE_USERNAME` and `AZURE_PASSWORD` are properly set.</li><ul>|
|Password protection for PEM encoded certificates is not supported.|`AZURE_CLIENT_CERTIFICATE_PASSWORD` was set when using a PEM encoded certificate.|Re-encode the client certificate to a password protected PFX (PKCS12) certificate, or a PEM certificate without password protection.|

## Troubleshoot `ManagedIdentityCredential` authentication issues

The `ManagedIdentityCredential` is designed to work on various Azure hosts that provide managed identity. Configuring the managed identity and troubleshooting failures varies from hosts. The following table lists the Azure hosts that can be assigned a managed identity and are supported by the `ManagedIdentityCredential`.

|Host Environment| | |
|---|---|---|
|Azure App Service and Azure Functions|[Configuration](https://learn.microsoft.com/azure/app-service/overview-managed-identity)|[Troubleshooting](#azure-app-service-and-azure-functions-managed-identity)|
|Azure Arc|[Configuration](https://learn.microsoft.com/azure/azure-arc/servers/managed-identity-authentication)||
|Azure Virtual Machines and Scale Sets|[Configuration](https://learn.microsoft.com/entra/identity/managed-identities-azure-resources/qs-configure-portal-windows-vm)|[Troubleshooting](#azure-virtual-machine-managed-identity)|

### Azure Virtual Machine managed identity

`AuthenticationException`

| Error Message |Description| Mitigation |
|---|---|---|
|The requested identity hasn't been assigned to this resource.|The IMDS endpoint responded with a status code of 400, indicating the requested identity isn't assigned to the VM.|If using a user assigned identity, ensure the specified `clientId` is correct.<p/><p/>If using a system assigned identity, make sure it has been enabled properly. Instructions to enable the system assigned identity on an Azure VM can be found [here](https://learn.microsoft.com/entra/identity/managed-identities-azure-resources/qs-configure-portal-windows-vm#enable-system-assigned-managed-identity-on-an-existing-vm).|
|The request failed due to a gateway error.|The request to the IMDS endpoint failed due to a gateway error, 502 or 504 status code.|IMDS doesn't support calls via proxy or gateway. Disable proxies or gateways running on the VM for calls to the IMDS endpoint `http://169.254.169.254/`|
|No response received from the managed identity endpoint.|No response was received for the request to IMDS or the request timed out.|<ul><li>Ensure managed identity has been properly configured on the VM. Instructions for configuring the managed identity can be found [here](https://learn.microsoft.com/entra/identity/managed-identities-azure-resources/qs-configure-portal-windows-vm).</li><li>Verify the IMDS endpoint is reachable on the VM by following the instructions at [Verify IMDS is available on the VM](#verify-imds-is-available-on-the-vm).</li></ul>|
|Multiple attempts failed to obtain a token from the managed identity endpoint.|Retries to retrieve a token from the IMDS endpoint have been exhausted.|<ul><li>For more information on specific failures, see the inner exception messages. If the data has been truncated, more detail can be obtained by [collecting logs](https://github.com/Azure/azure-sdk-for-net/tree/main/sdk/identity/Azure.Identity#logging).</li><li>Ensure managed identity has been properly configured on the VM. Instructions for configuring the managed identity can be found [here](https://learn.microsoft.com/entra/identity/managed-identities-azure-resources/qs-configure-portal-windows-vm).</li><li>Verify the IMDS endpoint is reachable on the VM by following the instructions at [Verify IMDS is available on the VM](#verify-imds-is-available-on-the-vm).</li></ul>|

#### __Verify IMDS is available on the VM__

If you have access to the VM, you can verify the managed identity endpoint is available via the command line using curl.

```bash
curl 'http://169.254.169.254/metadata/identity/oauth2/token?resource=https://management.core.windows.net&api-version=2018-02-01' -H "Metadata: true"
```

> Note that output of this command will contain a valid access token, and SHOULD NOT BE SHARED to avoid compromising account security.

### Azure App Service and Azure Functions Managed Identity

`AuthenticationException`

| Error Message |Description| Mitigation |
|---|---|---|
|ManagedIdentityCredential authentication unavailable.|The environment variables configured by the App Services host weren't present.|<ul><li>Ensure the managed identity has been properly configured on the App Service. Instructions for configuring the managed identity can be found [here](https://learn.microsoft.com/azure/app-service/overview-managed-identity?tabs=dotnet).</li><li>Verify the App Service environment is properly configured and the managed identity endpoint is available by following the instructions at [Verify the App Service managed identity endpoint is available](#verify-the-app-service-managed-identity-endpoint-is-available).</li></ul>|

#### __Verify the App Service managed identity endpoint is available__

If you have access to SSH into the App Service, you can verify managed identity is available in the environment. First ensure the environment variables `MSI_ENDPOINT` and `MSI_SECRET` have been set in the environment. Then you can verify the managed identity endpoint is available using curl.

```bash
curl 'http://169.254.169.254/metadata/identity/oauth2/token?resource=https://management.core.windows.net&api-version=2018-02-01' -H "Metadata: true"
```

> Note that the output of this command will contain a valid access token, and SHOULD NOT BE SHARED to avoid compromising account security.

## Troubleshoot `WorkloadIdentityCredential` authentication issues

`AuthenticationException`

| Error Message |Description| Mitigation |
|---|---|---|
|`AuthenticationException` raised with message. "WorkloadIdentityCredential authentication unavailable. The workload options are not fully configured."|The `WorkloadIdentityCredential` requires `ClientId`, `TenantId` and `TokenFilePath` to authenticate with Microsoft Entra ID.| <ul><li>If using `DefaultAzureCredential` then:</li><ul><li>Ensure client ID is specified via `WorkloadIdentityClientId` property on `DefaultAzureCredentialOptions` or `AZURE_CLIENT_ID` env variable.</li><li>Ensure tenant ID is specified via `AZURE_TENANT_ID` env variable.</li><li>Ensure token file path is specified via `AZURE_FEDERATED_TOKEN_FILE` env variable.</li><li>Ensure authority host is specified via `AZURE_AUTHORITY_HOST` env variable.</ul><li>If using `WorkloadIdentityCredential` then:</li><ul><li>Ensure tenant ID is specified via the `TenantId` property on the `WorkloadIdentityCredentialOptions` or `AZURE_TENANT_ID` env variable.</li><li>Ensure client ID is specified via the `ClientId` property on the `WorkloadIdentityCredentialOptions` or `AZURE_CLIENT_ID` env variable.</li><li>Ensure token file path is specified via the `TokenFilePath` property on the `WorkloadIdentityCredentialOptions` instance or `AZURE_FEDERATED_TOKEN_FILE` environment variable. </li></ul></li><li>Consult the [product troubleshooting guide](https://azure.github.io/azure-workload-identity/docs/troubleshooting.html) for other issues.</li></ul>
|The workload options are not fully configured.|The workload identity configuration wasn't provided in environment variables or through `WorkloadIdentityCredentialOptions`.|Ensure the appropriate environment variables are set **prior to application startup** or are specified in code.</p><ul><li>To configure the `WorkloadIdentityCredential` via the environment, ensure the variables `AZURE_AUTHORITY_HOST`, `AZURE_CLIENT_ID`, `AZURE_TENANT_ID`, and `AZURE_FEDERATED_TOKEN_FILE` are set by the admission webhook.</li><li>To configure the `WorkloadIdentityCredential` in code, ensure `ClientId`, `TenantId`, and `TokenFilePath` are set on the `WorkloadIdentityCredentialOptions` passed to the `WorkloadIdentityCredential` constructor.</li><ul>|

## Get additional help

Additional information on ways to reach out for support can be found in the [SUPPORT.md](https://github.com/Azure/azure-sdk-for-cpp/blob/main/SUPPORT.md) at the root of the repo.