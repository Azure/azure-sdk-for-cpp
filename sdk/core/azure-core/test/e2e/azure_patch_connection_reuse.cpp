#include <iostream>

#include "azure/storage/files/datalake/datalake.hpp"

int main()
{
  using namespace Azure::Storage::Files::DataLake;

  std::string fileSystemName = "sample-file-system5";
  std::string fileName = "sample-file";

  auto fileSystemClient = FileSystemClient::CreateFromConnectionString(
      std::getenv("STORAGE_CONNECTION_STRING"), fileSystemName);

  Azure::Core::Http::HttpStatusCode code;
  try
  {
    auto result = fileSystemClient.Create();
    code = result.GetRawResponse().GetStatusCode();
  }
  catch (Azure::Storage::StorageError& e)
  {
    unused(e);
  }
  std::cout << std::endl
            << "-"
            << static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                   code);

  auto fileClient = fileSystemClient.GetFileClient(fileName);

  auto result = fileClient.Create();
  std::cout << std::endl
            << static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                   result.GetRawResponse().GetStatusCode());

  auto resultMore = fileClient.GetAccessControls();
  auto acls = resultMore->Acls;
  std::cout << std::endl
            << static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
                   resultMore.GetRawResponse().GetStatusCode());

  for (int i = 0; i < 3; ++i)
  {
    // This connection cannot be reused, although it returns 200
    auto result = fileClient.SetAccessControl(acls);

    std::cout
        << std::endl
        << static_cast<typename std::underlying_type<Azure::Core::Http::HttpStatusCode>::type>(
               result.GetRawResponse().GetStatusCode());
  }
}
