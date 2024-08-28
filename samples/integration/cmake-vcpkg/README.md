# Develop an Azure C++ App using vcpkg and CMake

## Perquisite 
1. Any Text Editor 
2. A terminal
3. A C++ compiler
    - Windows: [MSVC](https://learn.microsoft.com/en-us/cpp/build/vscpp-step-0-installation?view=msvc-170)
4. [git](https://git-scm.com/downloads)

## Steps 
### Install CMake
- Go to the [CMake Downloads page](https://cmake.org/download/) where you can either download the latest version of the CMake source or an installer for your OS. I'd recommend using the installer for simplicity. When asked select the option to add CMake to your PATH. 
- Once you've downloaded and run through the installer program run the following command to verify CMake has been installed properly:
```bash
cmake --version
```
- You should get an output denoting the currently install version of CMake. Like this: 
```bash
cmake version 3.30.2
```
- If you don't see the expected output you may need to restart your computer. 

### Install vcpkg
- To install vcpkg you'll first need to clone the vcpkg repo. I recommend cloning vcpkg to a central location on your development environment and not in your C++ Project directory. In this example I'll clone vcpkg to my home dir. 
```bash
cd ~
git clone https://github.com/microsoft/vcpkg.git
```
- Once the vcpkg repo is cloned traverse into the new directory and run the bootstrap-vcpkg script. 
  - If you're on Windows run: 
  ```bash
  cd .\vcpkg\
  .\bootstrap-vcpkg.bat
  ```
  - If you're on MacOS or Linux, run: 
  ```bash
  cd vcpkg
  ./bootstrap-vcpkg.sh
  ```
- After bootstrapping vcpkg, add it to your path so you can access the vcpkg executable from your project directory. Remember to replace the path in the command example with the path to the vcpkg directory you cloned earlier.
  - On Windows Powershell, enter: 
  ```bash
  $env:Path = "$env:Path;C:\path\to\vcpkg"
  ```
  - On MacOS or Linux systems, enter: 
  ```bash
  PATH=$PATH:/path/to/vcpkg
  ```
- Traverse back to your project directory and run the following to verify the vcpkg directory was added to your path: 
```bash
vcpkg --version
```
- You should get an output resembling the following (The hash at the end of the line does not need to match exactly):
```bash
vcpkg package management program version 2024-08-01-fd884a0d390d12783076341bd43d77c3a6a15658
```

### Project Setup
- In your terminal, travers back to the root of your project. 
- Let's start by creating our main C++ file. 
  - On Windows Powershell, enter: 
  ```bash
  echo main > main.cpp
  ```
  - On MacOS and Linux, enter:
  ```bash
  touch main.cpp
  ```
- Now lets create our CMakeLists.txt to use with CMake. 
  - On Windows Powershell, enter: 
  ```bash
  echo cmake > CMakeLists.txt
  ```
  - On MacOS and Linux systems, enter: 
  ```bash
  touch CMakeLists.txt
  ```
- We'll also create a `build` directory to store all out build related artifacts.
  - On Windows Powershell, MacOs and Linux systems, enter: 
  ```bash
  mkdir build
  ```
### Setup CMakeLists.txt file
- Now open the `CMakeLists.txt` file in your text editor and delete any contents inside it.
- To set up a extremely basic CMake project, replace the contents of your `CMakeLists.txt` file with the following:
```cmake
cmake_minimum_required(VERSION 3.30.0)
project(azure_sample VERSION 0.1.0 LANGUAGES C CXX)

add_executable(azure_sample main.cpp)
```
- To test our project setup so far lets also open our `main.cpp` file and replace it's contents with the following hello world code: 
```cpp
#include <iostream>

int main() {
    std::cout << "Hello World!"<<std::endl;
    return 0;
}
```
- Now, in the terminal lets run the command to configure our CMake build. 
```bash
cmake -B ./build
```
- You should see the build directory populate with some directories and files. 
- Now lets try building the project with: 
```bash
cmake --build ./build
```
- The output of the build should contain a line stating where the executable that was built was placed. By default it should be in your `build` directory under a new `Debug` directory with the name `azure_sample.exe` if your on Windows or `azure_sample` if your on MacOS or Linux.
- Lets try running our new executable. 
  - If your on Windows Powershell, enter: 
  ```pwsh
  .\build\Debug\azure_sample.exe
  ```
  - If your on MacOS or Linux, enter: 
  ```bash
  ./build/azure_sample
  ```
- You should get the following out put: 
```bash
Hello World!
```

### Install packages with vcpkg
- In this example we'll be using vcpkg in what's known as manifest mode.
- First we'll start by moving back to the root directory of our project, and creating a new vcpkg application with the following command: 
```bash
vcpkg new --application
```
- You should now have a `vcpkg.json` and `vcpkg-configuration.json` file in your project directory with the following contents: 
  - `vcpkg.json`
  ```json
  {}
  ```
  - `vcpkg-configuration.json` *It's okay if the hash on the `baseline` property doesn't match exactly.*
  ```json
  {
    "default-registry": {
      "kind": "git",
      "baseline": "6af584dd59aa5bdba75dae6781ec74614e03e5b9",
      "repository": "https://github.com/microsoft/vcpkg"
    },
    "registries": [
      {
        "kind": "artifact",
        "location": "https://github.com/microsoft/vcpkg-ce-catalog/archive/refs/heads/main.zip",
        "name": "microsoft"
      }
    ]
  }
  ```
- Now we can add the Azure Key Vault and Identity libraries from the Azure SDK for C++ to our project, by entering the following command:
```bash
vcpkg add port azure-identity-cpp azure-security-keyvault-secrets-cpp
```
- Your `vcpkg.json` file should now look like this: 
```json
{
  "dependencies": [
    "azure-identity-cpp",
    "azure-security-keyvault-secrets-cpp"
  ]
}
```

### Integrate CMake with vcpkg

- To integrate CMake with vcpkg we need to add the `vcpkg.cmake` module to our CMake toolchain. You can do this either by setting `CMAKE_TOOLCHAIN_FILE` in your `CMakeLists.txt` file like this: 
```cmake
cmake_minimum_required(VERSION 3.30.0)

# Remember to replace the path below with the path where you cloned vcpkg
set(CMAKE_TOOLCHAIN_FILE "/path/to/vcpkg-root/scripts/buildsystems/vcpkg.cmake")

project(azure_sample VERSION 0.1.0 LANGUAGES C CXX)

add_executable(azure_sample main.cpp)
```
- Or by specifying the toolchain file in the CMake configuration command like this: 
```bash
cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg-root/scripts/buildsystems/vcpkg.cmake -B build
```
- I'd recommend specifying the toolchain file in the `CMakeLists.txt` for now as it will shorten the commands you have to enter.

- We'll also need to tell CMake to find and link our two Azure packages. We can do that by updating our `CMakeLists.txt` to the following: 
```cmake
cmake_minimum_required(VERSION 3.30.0)

# Remember to replace the path below with the path where you cloned vcpkg
set(CMAKE_TOOLCHAIN_FILE "/path/to/vcpkg-root/scripts/buildsystems/vcpkg.cmake")

project(azure_sample VERSION 0.1.0 LANGUAGES C CXX)

find_package(azure-identity-cpp CONFIG REQUIRED)
find_package(azure-security-keyvault-secrets-cpp CONFIG REQUIRED)

add_executable(azure_sample main.cpp)

target_link_libraries(azure_sample PRIVATE
    Azure::azure-identity
    Azure::azure-security-keyvault-secrets
)
```

### Create an Azure Key Vault resource

- To create an Azure Key Vault resouce you'll need an [Azure subscription][azure_sub].
- We'll use the Azure CLI to create our Key Vault resource and authenticate to Azure.
- Go to the [Azure CLI Install page](https://learn.microsoft.com/en-us/cli/azure/install-azure-cli) for instructions on how to install the Azure CLI on your dev environment.
- Use the following command to authenticate with the Azure CLI
```bash
az login
```
- Use the pop up windows to login to Azure.
- Then use the following command to create your Key Vault resource, and remember to replace `<your-resource-group-name>` and `<your-key-vault-name>` with your own, unique names: 

  ```PowerShell
  az keyvault create --resource-group <your-resource-group-name> --name <your-key-vault-name>
  ```
- In the output you should see a list of properties with a `vaultUri` property. We'll set that to an environment variable to be used in our program with the following command: 
  - In a Windows Powershell terminal, enter: 
  ```pwsh
  $env:AZURE_KEYVAULT_URL = "https://<your-key-vault-name>.vault.azure.net/"
  ```
  - In a MacOS or Linux terminal, enter: 
  ```bash
  export AZURE_KEYVAULT_URL="https://<your-key-vault-name>.vault.azure.net/"
  ```
- Finally, you'll need to make sure your Azure account has the proper permissions to work with Key Vault Secrets. You can do this by assigning yourself the "Key Vault Secrets Officer" role on the Access Control (IAM) page of your Key Vault resouce in the Azure portal.

### Add our Azure C++ Code
- Now we'll update our `main.cpp` file with the following code to use Azure Key Vault:
```cpp
#include <azure/identity.hpp> 
#include <azure/keyvault/secrets.hpp> 
#include <iostream>

using namespace Azure::Security::KeyVault::Secrets;

int main(){
    std::cout<<"Starting Program!"<<std::endl;

    try{
        // Set Key Vault URL string
        auto const keyVaultUrl = std::getenv("AZURE_KEYVAULT_URL");

        // Create Defautl Azure Credential to Authenticate. 
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
        std::string valueString = secret.Value.HasValue()? secret.Value.Value() : "NONE RETURNED";
        std::cout<<"Secret is returned with name "<<secret.Name<<" and value "<<valueString<<std::endl;

    }catch (const Azure::Core::RequestFailedException& ex){
        std::cout << std::underlying_type<Azure::Core::Http::HttpStatusCode>::type(ex.StatusCode)<<std::endl;
    }

    std::cout<<"End of Program!"<<std::endl;
}
``` 

### Rebuild and Run

- After updating our `CMakeLists.txt` file we'll want to delete the CMake cache. Lets just delete and recreate the `./build` directory. 
  - In a Windows Powershell terminal, enter: 
  ```pwsh
  Remove-Item -Path .\build\ -Recurse -Force
  ```
  - In a MacOS or Linux terminal, enter: 
  ```bash
  rm -rf ./build
  ```
- Now we'll re-add our `./buid` direcotry by entering the following command: 
```bash
mkdir build
```
- Reconfigure CMake with the following command: 
```bash
cmake -B ./build
```
- Then we'll build our project with the following command: 
```bash
cmake --build ./build
```
- Next we'll run our program: 
  - If your on Windows Powershell, enter: 
  ```pwsh
  .\build\Debug\azure_sample.exe
  ```
  - If your on MacOS or Linux, enter: 
  ```bash
  ./build/azure_sample 
  ```
- The program should output the following: 
```bash
Starting Program!
Secret is returned with name MySampleSecret and value My super secret value
End of Program!
```

### Getting the project ready for git
- To begin collabortaing with others on our Azure C++ project we'll need to change a few things about the way the project has been configured. We need to make sure any file paths we've used are not contained in build files we want to share. 
- Currently our `CMakeLists.txt` file has a path to the `vcpkg.cmake` toolchain file on our local machine.
- Lets remove the `set(CMAKE_TOOLCHAIN_FILE "C:/Users/rgeraghty/vcpkg/scripts/buildsystems/vcpkg.cmake")` line from the `CMakeLists.txt` file so it now looks like this: 
```cmake
cmake_minimum_required(VERSION 3.30.0)

project(azure_sample VERSION 0.1.0 LANGUAGES C CXX)

find_package(azure-identity-cpp CONFIG REQUIRED)
find_package(azure-security-keyvault-secrets-cpp CONFIG REQUIRED)

add_executable(azure_sample main.cpp)

target_link_libraries(azure_sample PRIVATE
    Azure::azure-identity
    Azure::azure-security-keyvault-secrets
)
```
- Now use the following command to create a `CMakeUserPresets.json` file. We'll use this file to store our environment specific configurations.
  - In a Windows Powershell terminal, enter:
  ```pswh
  echo {} > CMakeUserPresets.json
  ```
  - In a MacOS or Linux terminal, enter:
  ```bash
  touch CMakeUserPresets.json
  ```
- Now open the newly created `CMakeUserPresets.json` file and replace its contents with the following:
```json
{
    "version": 3,
    "cmakeMinimumRequired": {
      "major": 3,
      "minor": 30,
      "patch": 0
    },
    "configurePresets": [
      {
        "name": "default",
        "displayName": "Default Config",
        "description": "Default build using Ninja generator",
        "binaryDir": "${sourceDir}/build/",
        "cacheVariables": {
          "CMAKE_BUILD_TYPE": "Debug",
          "CMAKE_TOOLCHAIN_FILE": "path/to/vcpkg/root/scripts/buildsystems/vcpkg.cmake"
        }
      }
    ],
    "buildPresets": [
      {
        "name": "default",
        "configurePreset": "default"
      }
    ]
  }
```
- Replace the value of the `CMAKE_TOOLCHAIN_FILE` property with the path to the vcpkg cmake toolchain file. *Note: this will be the same path we used for this property in our `CMakeLists.txt` file earlier.*
- Now that we're using presets with CMake, our CMake project will configure differently. Lets clean our build directory so the CMake Cache doesn't conflict with our new configureation. 
  - In a Windows Powershell terminal, enter: 
  ```pwsh
  Remove-Item -Path .\build\ -Recurse -Force
  ```
  - In a MacOS or Linux terminal, enter: 
  ```bash
  rm -rf ./build
  ```
- Now we'll re-add our `./buid` direcotry by entering the following command: 
```bash
mkdir build
```
- Now to configure our CMake project will run the following command: 
```bash
cmake --preset default
```
- Then we can build the project with this next command: 
```bash
cmake --build --preset default
```
- Finally we can run our program again with these commands: 
  - If your on Windows Powershell, enter: 
  ```pwsh
  .\build\Debug\azure_sample.exe
  ```
  - If your on MacOS or Linux, enter: 
  ```bash
  ./build/azure_sample 
  ```
- The last thing we need to do to get our project ready for collaboration with git is add a `.gitignore` file that specifies that the `CMakeUserPresets.json` file should not be included in source control. 
- To creat the `.gitignore` file use the following commands: 
  - - If your on Windows Powershell, enter: 
  ```pwsh
  echo ignore > .gitignore
  ```
  - If your on MacOS or Linux, enter: 
  ```bash
  touch .gitignore
  ```
- Then open the `.gitignore` file in your editor and replace its contents with the following 
```
build 
CMakeUserPresets.json
```
- Now you can push your project up to git, and collaboraters can clone it to work on the project with you. They will need to make sure they have properly set up their C++ compiler, CMake, adn vcpkg. They will also need to create their own `CMakeUsersPresets.json` file to point to where they installed the `vcpkg.cmake` module. 

[azure_cli]: https://docs.microsoft.com/cli/azure
[azure_sub]: https://azure.microsoft.com/free/