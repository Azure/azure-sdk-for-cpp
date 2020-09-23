#include <iostream>

#include <azure/storage/blobs/blob.hpp>

int main()
{
  using namespace Azure::Storage::Blobs;

  std::string containerName = "sample-container";
  std::string blobName = "sample-blob";
  std::string blobContent;
  blobContent.resize(50 * 1024ULL * 1024, 'x');

  auto containerClient = BlobContainerClient::CreateFromConnectionString(
      std::getenv("STORAGE_CONNECTION_STRING"), containerName);
  try
  {
    containerClient.Create();
  }
  catch (std::runtime_error& e)
  {
    // The container may already exist
    std::cout << e.what() << std::endl;
  }

  BlockBlobClient blobClient = containerClient.GetBlockBlobClient(blobName);

  for (int i = 0; i < 2000; ++i)
  {
    {
      UploadBlockBlobFromOptions options;
      options.Concurrency = 16;
      blobClient.UploadFrom(
          reinterpret_cast<const uint8_t*>(blobContent.data()), blobContent.size(), options);
    }

    {
      DownloadBlobToOptions options;
      options.Concurrency = 16;
      blobClient.DownloadTo(
          reinterpret_cast<uint8_t*>(&blobContent[0]), blobContent.size(), options);
    }
    std::cout << i << std::endl;
  }
}
