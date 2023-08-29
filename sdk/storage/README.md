# Azure Storage libraries for C++

Azure Storage is a Microsoft-managed service providing cloud storage that is highly available, secure, durable, scalable, and redundant. Azure Storage includes Blobs (objects), Queues, and Files.

- [Azure.Storage.Blobs](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-blobs/README.md) is Microsoft's object storage solution for the cloud. Blob storage is optimized for storing massive amounts of unstructured data that does not adhere to a particular data model or definition, such as text or binary data.

- [Azure.Storage.Queues](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-queues/README.md) is a service for storing large numbers of messages.  A queue message can be up to 64 KB in size and a queue may contain millions of messages, up to the total capacity limit of a storage account.

- [Azure.Storage.Files.Shares](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-files-shares/README.md) offers fully managed file shares in the cloud that are accessible via the industry standard Server Message Block (SMB) protocol.  Azure file shares can be mounted concurrently by cloud or on-premises deployments of Windows, Linux, and macOS.

- [Azure.Storage.Files.DataLake](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-files-datalake/README.md) includes all the capabilities required to make it easy for developers, data scientists, and analysts to store data of any size, shape, and speed, and do all types of processing and analytics across platforms and languages.

- [Azure.Storage.Common](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-common/README.md) provides infrastructure shared by the other Azure Storage client libraries like shared key authentication and exceptions.

## Contributing

See the [Storage CONTRIBUTING.md][storage_contrib] for details on building,
testing, and contributing to these libraries.

This project welcomes contributions and suggestions.  Most contributions require
you to agree to a Contributor License Agreement (CLA) declaring that you have
the right to, and actually do, grant us the rights to use your contribution. For
details, visit [cla.microsoft.com][cla].

This project has adopted the [Microsoft Open Source Code of Conduct][coc].
For more information see the [Code of Conduct FAQ][coc_faq]
or contact [opencode@microsoft.com][coc_contact] with any
additional questions or comments.

### Set up storage account

Storage account is required for storage unit test. There are several environment variables to set up storage account:

<center>

<table>
<tr>
<td>Environment Variable</td>
<td>Description</td>
<td>Test scope</td>
</tr>
<tr>
<td>STANDARD_STORAGE_CONNECTION_STRING</td>
<td>Standard storage account connection string for Blobs, standard Files Shares and Queues.</td>
<td>Blobs,<br>Standard Files Shares,<br>Queues </td>
</tr>
<tr>
<td>PREMIUM_FILE_CONNECTION_STRING</td>
<td>Premium Files Shares storage account connection string.</td>
<td>Premium Files Shares</td>
</tr>
<tr>
<td>ADLS_GEN2_CONNECTION_STRING</td>
<td>Data Lake storage account connection string.</td>
<td>Data Lake</td></tr>
<tr>
<td>AAD_TENANT_ID</td>
<td>Azure Active Directory tenant id of the service principal.</td>
<td rowspan="3">Blobs,<br>Data Lake,<br>Files Shares,<br>Queues </td>
</tr>
<tr>
<td>AAD_CLIENT_ID</td>
<td>Application(Client) id of the service principal.</td>
</tr>
<tr>
<td>AAD_CLIENT_SECRET</td>
<td>Client secret of the service principal.</td>
</tr>
</table>

</center>

Azure Storage libraries for C++ offers three types for authorizing access to data: `Shared Key(storage account key)`, `Shared access signature (SAS)` and `Azure Active Directory (Azure AD)`. 
The first two types can be obtained by `connection string`.

#### Set up storage account 
Before setting up storage account, you should have an overview of storage account: [Storage account overview][storage_account_overview].

Set up steps:
1. [Create storage account][create_storage_account]
2. [Get storage account connection string][get_connection_string]

#### Set up Azure AD for storage account
Before setting it up, you should have an overview of accessing storage through Azure AD: [Authorize access to blobs using Azure Active Directory][access_azure_ad].

Set up steps:
1. [Create Azure Service Principal][create_service_principal].
2. [Assign an Azure role for access to blob data][assign_role].
	- You can find storage roles in [Azure built-in roles][azure_built_in_roles]
3. Fill the tenant id, client id and client secret into environment variables.

<!-- LINKS -->
[storage_contrib]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[cla]: https://cla.microsoft.com
[coc]: https://opensource.microsoft.com/codeofconduct/
[coc_faq]: https://opensource.microsoft.com/codeofconduct/faq/
[coc_contact]: mailto:opencode@microsoft.com
[storage_account_overview]: https://learn.microsoft.com/en-us/azure/storage/common/storage-account-overview
[create_storage_account]: https://learn.microsoft.com/en-us/azure/storage/common/storage-account-create?tabs=azure-portal
[get_connection_string]: https://learn.microsoft.com/en-us/azure/storage/common/storage-account-keys-manage?tabs=azure-portal#view-account-access-keys
[create_storage_account]: https://learn.microsoft.com/en-us/azure/storage/common/storage-account-create?tabs=azure-portal
[access_azure_ad]: https://learn.microsoft.com/en-us/azure/storage/blobs/authorize-access-azure-active-directory
[create_service_principal]: https://learn.microsoft.com/en-us/purview/create-service-principal-azure
[assign_role]: https://learn.microsoft.com/en-us/azure/storage/blobs/assign-azure-role-data-access?tabs=portal
[azure_built_in_roles]: https://learn.microsoft.com/en-us/azure/role-based-access-control/built-in-roles