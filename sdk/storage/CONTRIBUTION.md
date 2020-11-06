If you intend to contribute to the project, please make sure you've followed the instructions provided in the [Azure Projects Contribution Guidelines](https://opensource.microsoft.com/collaborate).

## Project Setup on Windows
On Windows, the Azure Storage development team uses Visual Studio so instructions will be tailored to that preference. However, any preferred IDE or other toolset should be usable.

### Tool Dependencies
* Visual Studio 2019 with C++ toolsets.
* [Vcpkg](https://github.com/microsoft/vcpkg).
* Clone the source code from GitHub.

### Test Environment Dependencies

#### Standard RA-GRS Storage Account to run Blob/File service tests
You can leverage [this doc](https://docs.microsoft.com/en-us/azure/storage/common/storage-account-create?tabs=azure-portal) to create Standard RA-GRS Storage Account that can be used to run most of the test cases for Blob/File Share services.
**Please ensure that you selected 'Standard' for performance, as this is to create the standard storage account.**
**Please ensure that you selected `Read-access geo-redundant storage (RA-GRS)` to test the fall-back to secondary feature.**
**Please ensure that in 'Data protection' tab, you need to enable the corresponding features for Storage's tests to pass. Recommendation is to check all boxes.**
**Please ensure that in 'Advanced' tab, Customer-managed keys support are selected for CMK related tests.**

After the Storage account is created, you can go to  Storage Account -> Settings -> Access keys, copy either the connection string in key1 or key2 and set it to system environment variable `STANDARD_STORAGE_CONNECTION_STRING`.

#### DataLake Gen2 enabled Storage Account to run DataLake service tests.
You can leverage [this doc](https://docs.microsoft.com/en-us/azure/storage/blobs/create-data-lake-storage-account) to create storage account to run test cases for DataLake Gen2 service.
**Please ensure that you selected 'General-purpose v2' as account type.**
After the Storage account is created, you can go to Storage Account -> Settings -> Access keys, copy either the connection string in key1 or key2 and set it to system environment variable `ADLS_GEN2_CONNECTION_STRING`.

#### Register an Application to run OAuth related tests.
Please login and go to [this page](https://ms.portal.azure.com/#blade/Microsoft_AAD_RegisteredApps/ApplicationsListBlade) to create register an application.
Choose `Accounts in any organizational directory (Any Azure AD directory - Multitenant)` for multi-tenant access.
In overview, you can find Application (client) ID and Directory(tenant) ID, set it to system environment variable `AAD_CLIENT_ID` and `AAD_TENANT_ID` respectively.
Click certificate & secret, create a new client secret, and after it is created, copy the Value of the client secrete and set it to `AAD_CLIENT_SECRET`.
Now go to the Storage accounts and assign the roles to the application you just created. You can reference [this doc](https://docs.microsoft.com/en-us/azure/active-directory/fundamentals/active-directory-users-assign-role-azure-portal) for more details.
Essential role is `Storage Blob Data Owner`. It will take 5 minutes for Storage service to catch the role assignment to the application.


#### Download & Install
Please reference the [Download & Install](https://github.com/Azure/azure-sdk-for-cpp/tree/master/sdk/storage#download--install) in README.md.

### Build Tests
By default, Storage's tests are built when you build the full repository, you can select `azure-storage-test.exe` in Debug Target in Visual Studio, so that there will be `Build azure-storage-test.exe` in Build tab.

### Run Tests
There are three ways to run tests:

#### Via Command Line Tool.
The default output folder of the tests is: `azure-sdk-for-cpp\out\build\x64-DebugWithTests\sdk\storage`
You will be able to locate `.\azure-storage-test.exe` there. You can use argument `gtest-filter` to run specific test, e.g. `.\azure-storage-test.exe --gtest_filter=FileShareClientTest.CreateDeleteShares`.
Or you can use regex to run a set of tests: `.\azure-storage-test.exe --gtest_filter=FileShare*`

#### Via Visual Studio Debug and Launch Settings.
Click on Debug tab in Visual Studio, select Debug and Launch Settings for azure-storage-test.exe. Add below lines in `configurations` if you want to run tests for File Share service:
```
"args": [
  "--gtest_filter=FileShare*"
]
```
Then click the green arrow pointing to `azure-storage-test.exe` in tool bar. You can set break points in code if you are choosing debug build.

#### Via Visual Studio Test Explorer.
Simply open Test Explorer in View, and build the project. After build is finished you should be able to see all Storage's tests under azure-storage-test.

## Debugging

### With Visual Studio.
As mentioned earlier, you can leverage Visual Studio to debug if you are not running from command line tool.

### With Fiddler
To use Fiddler, you will have to change the code within the transport layer.

#### Fiddler with Curl
Add following code to `sdk/core/azure-core/src/http/curl/curl.cpp`, before line 'auto performResult = curl_easy_perform(newHandle);'

```
  curl_easy_setopt(newConnection->GetHandle(), CURLOPT_PROXY, "127.0.0.1:8888");
  curl_easy_setopt(newConnection->GetHandle(), CURLOPT_SSL_VERIFYPEER, FALSE);
```

#### Fiddler with WinHttp

TODO

## Pull Requests

### Guidelines
The following are the minimum requirements for any pull request that must be met before contributions can be accepted.
* Make sure you've signed the CLA before you start working on any change.
* Discuss any proposed contribution with the team via a GitHub issue **before** starting development.
* Code must be professional quality.
    * You should strive to mimic the style with which we have written the library.
    * Clean, well-commented, well-designed code.
    * Try to limit the number of commits for a feature to 1-2. If you end up having too many we may ask you to squash your changes into fewer commits.
* Changelog.txt needs to be updated describing the new change.
* BreakingChanges.txt contains changes that break backward-compatibility.
* Thoroughly test your feature.

### Testing Features
As you develop a feature, you'll need to write tests to ensure quality. You should also run existing tests related to your change to address any unexpected breaks.

### Branching Policy
Changes should be based on the `dev` branch. We're following [semver](http://semver.org/).
We generally release any breaking changes in the next major version (e.g. 1.0, 2.0) and non-breaking changes in the next minor or major version (e.g. 2.1, 2.2).

### Adding Features for All Platforms
We strive to release each new feature for each of our environments at the same time. Therefore, we ask that all contributions be written for both Window, Linux and MacOS.

### Review Process
We expect all guidelines to be met before accepting a pull request. As such, we will work with you to address issues we find by leaving comments in your code. Please understand that it may take a few iterations before the code is accepted as we maintain high standards on code quality. Once we feel comfortable with a contribution, we will validate the change and accept the pull request.

# Thank you for any contributions!
Please let the team know if you have any questions or concerns about our contribution policy.