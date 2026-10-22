# Setup Storage Resources for Running Unit Tests

Storage accounts are required for running storage unit tests. We don't expect users or developers to do these configurations or run unit tests, unless you're making some major changes or working on new features.

## Synthetic playback tests

`SyntheticStorageTest.*_PLAYBACKONLY_` cases use fabricated test-proxy fixtures, not
captured Azure responses. They cover client-side request construction, response
handling, bearer challenges, SAS fields, CRC64 conditions, container rename, and
delete continuation without requiring service feature flags or cross-tenant identities.
They do not establish that Azure accepts these requests or enforces the simulated
authorization rules.

Every synthetic response, including token and error responses, contains
`Server: Azure-Storage-Cpp-Synthetic-Test-Server`. The fixture checks this marker
and refuses to run in LIVE or RECORD mode. It also refuses to send requests if
playback has not started. No Storage connection strings or Azure credentials are
needed; token requests use dummy credentials and are routed through test-proxy.

The fixtures use the existing test-proxy playback infrastructure and live in the
assets repository referenced by `assets.json`. Do not replace real service
recordings with synthetic responses.
`StructuredMessageTest_Large_LIVEONLY_` uses a 64 MiB + 512-byte body and has no
playback fixture.

Azure Storage libraries for C++ offers three types for authorizing access to data: `Shared Key(storage account key)`, `Shared access signature (SAS)` and `Azure Active Directory (Azure AD)`. 
The first two types can be obtained by `connection string`.

#### Set up storage account 
Before setting up storage account, you should have an overview of storage account: [Storage account overview][storage_account_overview].

Set up steps:
1. [Create storage account][create_storage_account]
2. [Get storage account connection string][get_connection_string]
3. Fill the connection strings in [source code](https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/azure-storage-common/test/ut/test_base.cpp) or environment variables.

| Environment Variable               | Variable in Source Code              | Storage Account Details                                                                                                                                                          |
|------------------------------------|--------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| STANDARD_STORAGE_CONNECTION_STRING | StandardStorageConnectionStringValue | Account Kind: General Purpose V2<br> Performance: Standard<br> Replication: RA-GRS<br> Enabled Features: soft delete for blobs, soft delete for containers, versioning for blobs |
| PREMIUM_FILE_CONNECTION_STRING     | PremiumStorageConnectionStringValue  | Account Kind: FileStorage<br> Performance: Premium<br> Replication: LRS                                                                                                          |
| ADLS_GEN2_CONNECTION_STRING        | AdlsGen2ConnectionStringValue        | Account Kind: General Purpose V2<br> Performance: Standard<br> Replication: LRS<br> Enabled Features: hierarchical namespace                                                     |

#### Set up Azure AD for storage account
Before setting it up, you should have an overview of accessing storage through Azure AD: [Authorize access to blobs using Azure Active Directory][access_azure_ad].

Set up steps:
1. [Create Azure Service Principal][create_service_principal].
2. [Assign an Azure role for access to blob data][assign_role].
	- You can find storage roles in [Azure built-in roles][azure_built_in_roles]
3. Fill the tenant id, client id and client secret in environment variables or in source code.


[storage_account_overview]: https://learn.microsoft.com/azure/storage/common/storage-account-overview
[create_storage_account]: https://learn.microsoft.com/azure/storage/common/storage-account-create?tabs=azure-portal
[get_connection_string]: https://learn.microsoft.com/azure/storage/common/storage-account-keys-manage?tabs=azure-portal#view-account-access-keys
[access_azure_ad]: https://learn.microsoft.com/azure/storage/blobs/authorize-access-azure-active-directory
[create_service_principal]: https://learn.microsoft.com/purview/create-service-principal-azure
[assign_role]: https://learn.microsoft.com/azure/storage/blobs/assign-azure-role-data-access?tabs=portal
[azure_built_in_roles]: https://learn.microsoft.com/azure/role-based-access-control/built-in-roles