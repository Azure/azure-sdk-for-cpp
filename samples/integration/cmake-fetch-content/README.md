# Develop an Azure C++ App using CMake and Fetch Content

This guide provides developers with the necessary steps to install and integrate libraries from the Azure SDK for C++ into their projects using CMake and fetch content. By following the instructions, you can set up your development environment and begin using Azure services in your C++ applications. Whether you're new to Azure or looking to streamline your integration process, this documentation helps you get started quickly and efficiently.

## Prerequisites

- Any Text Editor
- A terminal
- A C++ compiler
- [git](https://git-scm.com/downloads)
- [CMake](https://cmake.org/download/)
- [An Azure subscription](https://azure.microsoft.com/free/)
- [Azure CLI](/cli/azure/install-azure-cli)

## Verify git and CMake installation

To ensure a smooth integration process, it's important to verify that git, and CMake are correctly installed on your system.

1. To verify git is installed properly, run the following command in your terminal:

    ```bash
    git --version
    ```

1. You should get an output denoting the currently installed version for git, like this:

    ```output
    git version <version>
    ```

1. To verify CMake is installed properly, run the following command in your terminal:

    ```bash
    cmake --version
    ```

1. You should get an output denoting the currently installed version of CMake, like this:

    ```output
    cmake version <version>
    ```

## Create an Azure Key Vault resource

This section discusses how to use the Azure CLI to create an Azure Key Vault resource. This Key Vault resource securely stores and manages sensitive information, such as secrets and keys.

1. Use the Azure CLI to login by entering following command in your terminal:

    ```azurecli
    az login
    ```

1. Use the pop-up windows to log in to Azure.
1. After using the pop-up browser window to log in, select the Azure subscription you'd like to use in the terminal.
1. Then use the following command to create your Key Vault resource, and remember to replace `<your-resource-group-name>` and `<your-key-vault-name>` with your own, unique names:

    ```azurecli
    az keyvault create --resource-group <your-resource-group-name> --name <your-key-vault-name>
    ```

1. In the output, you should see a list of properties with a `vaultUri` property. Set that to an environment variable to be used in our program with the following command:

   - On Windows Powershell, enter:

        ```powershell
        $env:AZURE_KEYVAULT_URL = "https://<your-key-vault-name>.vault.azure.net/"
        ```

   - On MacOS or Linux systems, enter:

        ```bash
        export AZURE_KEYVAULT_URL="https://<your-key-vault-name>.vault.azure.net/"
        ```

1. Finally, make sure your Azure account has the proper permissions to work with Key Vault Secrets. You can give yourself the proper permissions by assigning yourself the "Key Vault Secrets Officer" role on the Access Control (IAM) page of your Key Vault resource in the Azure portal. *IAM stands for identity and access management.*

## Set up your project

This section describes the process of creating the necessary folders and files to set up your Azure C++ project.

1. In the root of your project directory, create a **CMakeLists.txt** file. This file is used to configure our CMake project. Add the following code to the **CMakeLists.txt** file:

    ```cmake
    # Specify the minimum version of CMake required to build this project
    cmake_minimum_required(VERSION 3.30.0)
    
    # Set the path to the vcpkg toolchain file
    # Remember to replace the path below with the path where you cloned vcpkg
    set(CMAKE_TOOLCHAIN_FILE "/path/to/vcpkg-root/scripts/buildsystems/vcpkg.cmake")
    
    # Define the project name, version, and the languages used
    project(azure_sample VERSION 0.1.0 LANGUAGES C CXX)
    
    # Find and include the azure-identity-cpp package
    find_package(azure-identity-cpp CONFIG REQUIRED)
    
    # Find and include the azure-security-keyvault-secrets-cpp package
    find_package(azure-security-keyvault-secrets-cpp CONFIG REQUIRED)
    
    # Add an executable target named 'azure_sample' built from the main.cpp source file
    add_executable(azure_sample main.cpp)
    
    # Link the azure-identity and azure-security-keyvault-secrets 
    # libraries to the azure_sample target
    target_link_libraries(azure_sample PRIVATE
        Azure::azure-identity
        Azure::azure-security-keyvault-secrets
    )
    ```

1. In the root of your project directory, create a **main.cpp** file. Add the following code to the **main.cpp** file:

    ```cpp
    #include <azure/identity.hpp>
    #include <azure/keyvault/secrets.hpp>
    #include <iostream>
    
    using namespace Azure::Security::KeyVault::Secrets;
    
    int main()
    {
        try
        {
            // Set Key Vault URL string
            auto const keyVaultUrl = std::getenv("AZURE_KEYVAULT_URL");
    
            // Create Default Azure Credential to Authenticate.
            // It will pick up on our AzureCLI login
            auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();
    
            // Create Key Vault Secret Client
            SecretClient secretClient(keyVaultUrl, credential);
    
            // Create a Secret
            std::string secretName("MySampleSecret");
            std::string secretValue("My super secret value");
            secretClient.SetSecret(secretName, secretValue);
    
            // Get the Secret
            KeyVaultSecret secret = secretClient.GetSecret(secretName).Value;
            std::string valueString = secret.Value.HasValue()
                                          ? secret.Value.Value()
                                          : "NONE RETURNED";
            std::cout << "Secret is returned with name " << secret.Name
                      << " and value " << valueString << std::endl;
        }
        catch (Azure::Core::Credentials::AuthenticationException const &e)
        {
            std::cout << "Authentication Exception happened:" << std::endl
                      << e.what() << std::endl;
            return 1;
        }
        catch (Azure::Core::RequestFailedException const &e)
        {
            std::cout << "Key Vault Secret Client Exception happened:" << std::endl
                      << e.Message << std::endl;
            return 1;
        }
    
        return 0;
    }
    ```

1. Create a **build** directory for the build artifacts.

## Build and run

This section discusses how to configure and build your project using CMake commands, and then run the program to ensure everything is set up correctly. The commands in this section should be run from the root of your project where the `build` directory, `CMakeLists.txt`, and `main.cpp` files are located.

1. To configure CMake, enter the following command:

    ```bash
    cmake -B ./build
    ```

1. To build the project, enter the following command:

    ```bash
    cmake --build ./build
    ```

1. To run the program, enter the following command:

   - On Windows Powershell, enter:

        ```powershell
        .\build\Debug\azure_sample.exe
        ```

   - On MacOS or Linux systems, enter:

        ```bash
        ./build/azure_sample
        ```

1. The program should have the following output:

    ```output
    Secret is returned with name MySampleSecret and value My super secret value
    ```
