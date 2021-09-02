# Azure Key Vault Certificates client library for C++

Azure Key Vault is a cloud service that provides secure storage and automated management of certificates used throughout a cloud application. Multiple certificates, and multiple versions of the same certificate, can be kept in the Azure Key Vault. Each certificate in the vault has a policy associated with it which controls the issuance and lifetime of the certificate, along with actions to be taken as certificates near expiry.

The Azure Key Vault certificates client library enables programmatically managing certificates, offering methods to get certificates, policies, issuers, and contacts.

[Source code][certificate_client_src] | [API reference documentation][api_reference] | [Product documentation][keyvault_docs]

### Additional concepts

<!-- CLIENT COMMON BAR -->

[Replaceable HTTP transport adapter](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/core/azure-core#http-transport-adapter) |
[Long-running operations](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/core/azure-core#long-running-operations) |

### Additional Documentation

- For more extensive documentation on Azure Key Vault, see the [API reference documentation][keyvault_rest].

## Contributing

See the [CONTRIBUTING.md][contributing] for details on building, testing, and contributing to these libraries.

This project welcomes contributions and suggestions. Most contributions require you to agree to a Contributor License Agreement (CLA)
declaring that you have the right to, and actually do, grant us the rights to use your contribution. For details, visit https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide a CLA and decorate the PR appropriately (e.g., label, comment).
Simply follow the instructions provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct][code_of_conduct].
For more information see the [Code of Conduct FAQ][coc_faq] or contact opencode@microsoft.com with any additional questions or comments.

<!-- LINKS -->
[api_reference]: https://azure.github.io/azure-sdk-for-cpp/keyvault.html
[code_of_conduct]: https://opensource.microsoft.com/codeofconduct/
[keyvault_docs]: https://docs.microsoft.com/azure/key-vault/
[keyvault_rest]: https://docs.microsoft.com/rest/api/keyvault/
[contributing]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[coc_faq]: https://opensource.microsoft.com/codeofconduct/faq/
