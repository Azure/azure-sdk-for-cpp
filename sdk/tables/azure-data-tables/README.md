# Azure Tables client library for C++

Azure Data Tables is a NoSQL data storage service that can be accessed from anywhere in the world via authenticated calls using HTTP or HTTPS.
Tables scales as needed to support the amount of data inserted, and allows for the storing of data with non-complex accessing.
The Azure Tables client can be used to access Azure Storage or Cosmos accounts.

[Source code][source_code] | [Package (vcpkg)](https://vcpkg.io/en/package/azure-data-tables-cpp) | [API reference documentation](https://azuresdkdocs.blob.core.windows.net/$web/cpp/azure-data-tables/latest/index.html) | [Product documentation](https://learn.microsoft.com/azure/storage/tables/) | [Samples](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/tables/azure-data-tables/samples)

## Getting started

### Prerequisites
- [vcpkg](https://learn.microsoft.com/vcpkg/get_started/overview) for package acquisition and dependency management
- [CMake](https://cmake.org/download/) for project build
- An [Azure subscription][azure_sub]
- An existing Azure Storage Tables resource. If you need to create an Azure Storage Tables, you can use the Azure Portal or [Azure CLI][azure_cli].
* You must have an [Azure subscription][azure_subscription] and either
    * an [Azure Storage account][azure_storage_account] or
    * an [Azure Cosmos Account][azure_cosmos_account].

If you use the Azure CLI, replace `<your-resource-group-name>` and `<your-<service-name>-name>` with your own, unique names:

```PowerShell
az login
az <service-name> create --resource-group <your-resource-group-name> --name <your-<service-name>-name>
```
### Create account
* To create a new Storage account, you can use [Azure Portal][azure_portal_create_account], [Azure PowerShell][azure_powershell_create_account], or [Azure CLI][azure_cli_create_account]:
* To create a new Cosmos storage account, you can use the [Azure CLI][azure_cli_create_cosmos] or [Azure Portal][azure_portal_create_cosmos].

### Install the package
The easiest way to acquire the C++ SDK is leveraging the vcpkg package manager and CMake. See the corresponding [Azure SDK for C++ readme section][azsdk_vcpkg_install]. We'll use vcpkg in manifest mode. To start a vcpkg project in manifest mode use the following command at the root of your project: 

```batch
vcpkg new --application
```

To install the Azure \<Service-Name> package via vcpkg:
To add the Azure \<Service-Name> package to your vcpkg enter the following command (We'll also add the Azure Identity library for authentication):

```batch
vcpkg add port azure-<service-name>-cpp azure-identity-cpp
```

Then, add the following in your CMake file:

```CMake
find_package(azure-<service-name>-cpp CONFIG REQUIRED)
target_link_libraries(<your project name> PRIVATE Azure::azure-<service-name> Azure::azure-identity)
```

Remember to set `CMAKE_TOOLCHAIN_FILE` to the path to `vcpkg.cmake` either by adding the following to your `CMakeLists.txt` file before your project statement:

```CMake
set(CMAKE_TOOLCHAIN_FILE "vcpkg-root/scripts/buildsystems/vcpkg.cmake")
```

Or by specifying it in your CMake commands with the `-DCMAKE_TOOLCHAIN_FILE` argument.

There is more than one way to acquire and install this library. Check out [our samples on different ways to set up your Azure C++ project][project_set_up_examples].

## Key concepts
Common uses of the table service include:
* Storing TBs of structured data capable of serving web scale applications
* Storing datasets that do not require complex joins, foreign keys, or stored procedures and can be de-normalized for fast access
* Quickly querying data using a clustered index
* Accessing data using the OData protocol filter expressions

The following components make up the Azure Tables Service:
* The account
* A table within the account, which contains a set of entities
* An entity within a table, as a dictionary

The Azure Tables client library for C++ allows you to interact with each of these components through the
use of a dedicated client object.

### Create the client
The Azure Tables library allows you to interact with two types of resources:
* the tables in your account
* the entities within those tables.
Interaction with these resources starts with an instance of a [client](#clients). To create a client object, you will need the account's table service endpoint URL and a credential that allows you to access the account. The `endpoint` can be found on the page for your storage account in the [Azure Portal][azure_portal_account_url] under the "Access Keys" section or by running the following Azure CLI command:

```bash
# Log in to Azure CLI first, this opens a browser window
az login
# Get the table service URL for the account
az storage account show -n mystorageaccount -g MyResourceGroup --query "primaryEndpoints.table"
```

### Client (Instantiation & List of any Sub-Clients)

We'll be using the `DefaultAzureCredential` to authenticate which will pick up the credentials we used when logging in with the Azure CLI earlier. `DefaultAzureCredential` can pick up on a number of Credential types from your environment and is ideal when getting started and developing. Check out our section on [DefaultAzureCredentials](https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/identity/azure-identity#defaultazurecredential) to learn more.

```cpp
// Add all necessary includes for client creation and samples below
#include <azure/identity.hpp> 
#include <azure/<service-name>.hpp

// Add all using statements for client creation and samples below
using namespace Azure::<Service::Name>;

int main (){
  // Add all variables that are needed for creating the client
  static const char* url = "AZURE_SERVICENAME_URL";

  // Use Azure Default Credential wherever possible
  auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();

   // Create client code. Try to avoid using auto on the first use of any type from our libraries that a customer might not be familiar with. 
   AzureClientType client = clientCtor(url, credential); 
}
```

### Clients
Two different clients are provided to interact with the various components of the Table Service:
1. **`TableServiceClient`** -
    * Get and set account settings
    * Query tables within the account.
    * Create or delete the specified table.
2. **`TableClient`** -
    * Interacts with a specific table (which need not exist yet).
    * Create, delete, query, and upsert entities within the specified table.
    * Submit transactional batch operations.


#### TableServiceClient

##### Creating and deleting a table

In order to Create/Delete a table we need to create a TablesClient first.

```cpp
#include <azure/data/tables.hpp>
...
using namespace Azure::Data::Tables;
const std::string TableName = "sample1";
...
auto tableServiceClient = TableServiceClient::CreateFromConnectionString(GetConnectionString());

// create new table
tableServiceClient.CreateTable(TableName);
```

In order to Delete a table we need to call the delete method on the previously created client.
```cpp
// delete existing table
tableServiceClient.DeleteTable(TableName);
```
##### Table Service Operations

In order to get the service properties we need to create a TableServiceClient first.

```cpp
#include <azure/data/tables.hpp>
...
using namespace Azure::Data::Tables;
...
auto tableServiceClient = TableServiceClient::CreateFromConnectionString(...);
```

To get the service properties we call the GetProperties method on the table service client.
```cpp
  auto properties = tableServiceClient.GetProperties();
```

To list the tables in the account we call the ListTables method on the table service client.
```cpp
  auto tables = tableServiceClient.ListTables();
```

To get the statistics of the account we call the GetStatistics method on the table service client.
```cpp
  auto statistics = tableServiceClient.GetStatistics();
```

##### Table Transactions Success

In order to get the service properties we need to create a TableServiceClient first.

```cpp
#include <azure/data/tables.hpp>
...
using namespace Azure::Data::Tables;
...
auto tableServiceClient = TableServiceClient::CreateFromConnectionString(...);
```

We create a table on which we run the transaction and get a table client. 
```cpp
// create table
tableServiceClient.CreateTable(TableName);
// get table client from table service client
auto tableClient = tableServiceClient.GetTableClient(TableName);
```
N.B. Here we are obtaining the table client from the table service client using the credentials that were passed to the table service client.

#### TableClient
The TableClient is used to interact with table entities and perform operations on them.
##### Entities
Entities are similar to rows. An entity has a set of properties, including a **`PartitionKey`** and **`RowKey`** which form the primary key of the entity. A property is a name value pair, similar to a column. Every entity in a table does not need to have the same properties. 

##### Manipulating entities

In order to Create/Update/Merge/Delete entities we need to create a TablesClient first.

```cpp
#include <azure/data/tables.hpp>
...
using namespace Azure::Data::Tables;
const std::string TableName = "sample1";
...
auto tableClient = TableClient::CreateFromConnectionString(..., TableName);
tableServiceClient.CreateTable(TableName);
```

Then we initialize and populate an entity.
```cpp
 // init new entity
  Models::TableEntity entity;
  entity.SetPartitionKey("P1");
  entity.SetRowKey("R1");
  entity.Properties["Name"] = TableEntityProperty("Azure");
  entity.Properties["Product"] = TableEntityProperty("Tables");
```

To create the entity on the server we call the CreateEntity method on the table client.
```cpp
  tableClient.AddEntity(entity);
```

To update the entity, assume we made some changes to the entity, we call the UpdateEntity method on the table client.
```cpp
  tableClient.UpdateEntity(entity);
```

To merge the entity, assume we made some changes to the entity, we call the MergeEntity method on the table client.
```cpp
  tableClient.MergeEntity(entity);
```

To delete the entity, we call the DeleteEntity method on the table client.
```cpp
  tableClient.DeleteEntity(entity);
```


We initialize and populate the entities.
```cpp
Models::TableEntity entity1;
entity1.PartitionKey = "P1";
entity1.RowKey = "R1";
entity1.Properties["Name"] = "Azure";
entity1.Properties["Product"] = "Tables";

Models::TableEntity entity2;
entity2.PartitionKey = "P2";
entity2.RowKey = "R2";
entity2.Properties["Name"] = "Azure";
entity2.Properties["Product"] = "Tables";
```

We create a transaction batch and add the operations to the transaction.
```cpp
// Create a transaction with two steps
std::vector<TransactionStep> steps;
steps.emplace_back(TransactionStep{TransactionActionType::Add, entity});
steps.emplace_back(TransactionStep{TransactionActionType::Add, entity2});
```

We then submit the transaction and check the response.
```cpp
// Check the response
if (!response.Value.Error.HasValue())
{
  std::cout << "Transaction completed successfully." << std::endl;
}
else
{
  std::cout << "Transaction failed with error: " << response.Value.Error.Value().Message
            << std::endl;
}
```

The output of this sample is:
```text
Transaction completed successfully.
```

##### Table Transactions Error

The difference from the previous example is that we are trying to add two entities  with the same PartitionKey and RowKey.
```cpp
// Create two table entities
TableEntity entity;
TableEntity entity2;
entity.SetPartitionKey("P1");
entity.SetRowKey("R1");
...
entity2.SetPartitionKey("P1");
entity2.SetRowKey("R1");
...
```

The rest of the steps are the same as in the previous example.

The output of the sample contains the error message: 

```text
Transaction failed with error: 1:The batch request contains multiple changes with same row key. An entity can appear only once in a batch request.
```
## Next steps

The following sections provide several code snippets covering some of the most common Table tasks, including:

* [Creating and deleting a table](#creating-and-deleting-a-table "Creating and deleting a table")
* [Manipulating entities](#manipulating-entities "Manipulating entities")
* [Table Service Operations](#table-service-operations "Table Service Operations")
* [Table Transactions Success](#table-transactions-success "Table Transactions Success")
* [Table Transactions Error](#table-transactions-error "Table Transactions Error")

## Contributing

See the [C++ Contributing Guide][sdk_contrib] for details on building,
testing, and contributing to these libraries.

See the [Storage Testing Guide][storage_testing] for how to set up storage resources running unit tests.

This project welcomes contributions and suggestions.  Most contributions require
you to agree to a Contributor License Agreement (CLA) declaring that you have
the right to, and actually do, grant us the rights to use your contribution. For
details, visit [cla.microsoft.com][cla].

This project has adopted the [Microsoft Open Source Code of Conduct][coc].
For more information see the [Code of Conduct FAQ][coc_faq]
or contact [opencode@microsoft.com][coc_contact] with any
additional questions or comments.

<!-- LINKS -->
[sdk_contrib]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/CONTRIBUTING.md
[storage_testing]: https://github.com/Azure/azure-sdk-for-cpp/blob/main/sdk/storage/TestingGuide.md
[cla]: https://cla.microsoft.com
[coc]: https://opensource.microsoft.com/codeofconduct/
[coc_faq]: https://opensource.microsoft.com/codeofconduct/faq/
[coc_contact]: mailto:opencode@microsoft.com
[source_code]:https://github.com/Azure/azure-sdk-for-cpp/tree/main/sdk/
[Tables_product_doc]:https://docs.microsoft.com/azure/cosmos-db/table-introduction

[azure_subscription]:https://azure.microsoft.com/free/
[azure_storage_account]:https://docs.microsoft.com/azure/storage/common/storage-account-create?tabs=azure-portal
[azure_cosmos_account]:https://docs.microsoft.com/azure/cosmos-db/create-cosmosdb-resources-portal
[azure_create_cosmos]:https://docs.microsoft.com/azure/cosmos-db/create-cosmosdb-resources-portal
[azure_cli_create_cosmos]:https://docs.microsoft.com/azure/cosmos-db/scripts/cli/table/create
[azure_portal_create_cosmos]:https://docs.microsoft.com/azure/cosmos-db/create-cosmosdb-resources-portal
[azure_portal_create_account]:https://docs.microsoft.com/azure/storage/common/storage-account-create?tabs=azure-portal
[azure_powershell_create_account]:https://docs.microsoft.com/azure/storage/common/storage-account-create?tabs=azure-powershell
[azure_cli_create_account]: https://docs.microsoft.com/azure/storage/common/storage-account-create?tabs=azure-cli
