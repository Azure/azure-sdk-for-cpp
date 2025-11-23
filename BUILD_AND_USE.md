# Building and Using Azure Core with CurlOptionsCallback

## Option 1: Build and Install via vcpkg (Recommended)

Since you've made changes to the azure-core library, you'll need to build and install it locally.

### Step 1: Build the Library with Visual Studio

1. Open **Developer PowerShell for VS** (or Developer Command Prompt)
   - In Visual Studio Insiders: `Tools` → `Command Line` → `Developer PowerShell`

2. Navigate to the repository:
   ```powershell
   cd C:\Users\kraman\source\repos\azure-sdk-for-cpp
   ```

3. Configure CMake with Visual Studio generator:
   ```powershell
   cmake -B out\build -S . -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTING=OFF -DBUILD_SAMPLES=OFF
   ```

4. Build the azure-core library:
   ```powershell
   cmake --build out\build --config Debug --target azure-core
   ```
   
   Or for Release:
   ```powershell
   cmake --build out\build --config Release --target azure-core
   ```

5. Install to a local directory:
   ```powershell
   cmake --install out\build --prefix C:\Users\kraman\azure-sdk-local --config Debug
   ```

### Step 2: Use in Your Project

Create a test project to use the modified library:

#### CMakeLists.txt for your project:
```cmake
cmake_minimum_required(VERSION 3.13)
project(TestCurlCallback)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Point to your local installation
set(CMAKE_PREFIX_PATH "C:/Users/kraman/azure-sdk-local")

find_package(azure-core-cpp CONFIG REQUIRED)
find_package(CURL REQUIRED)

add_executable(test_curl_callback test_curl_callback.cpp)

target_link_libraries(test_curl_callback 
    PRIVATE 
    Azure::azure-core
    CURL::libcurl
)
```

#### test_curl_callback.cpp:
```cpp
#include <azure/core/http/curl_transport.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <iostream>
#include <curl/curl.h>

int main()
{
    try
    {
        // Create transport options with custom callback
        Azure::Core::Http::CurlTransportOptions curlOptions;
        
        // Set callback to customize CURL handle
        curlOptions.CurlOptionsCallback = [](void* curlHandle) {
            CURL* handle = static_cast<CURL*>(curlHandle);
            
            // Example 1: Bind to specific network interface
            // curl_easy_setopt(handle, CURLOPT_INTERFACE, "eth0");
            
            // Example 2: Set custom DNS servers
            // curl_easy_setopt(handle, CURLOPT_DNS_SERVERS, "8.8.8.8,8.8.4.4");
            
            // Example 3: Enable verbose output for debugging
            curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
            
            std::cout << "Custom CURL options applied!" << std::endl;
        };

        // Create transport with custom options
        auto transport = std::make_shared<Azure::Core::Http::CurlTransport>(curlOptions);
        
        // Create pipeline
        Azure::Core::Http::Policies::TransportOptions transportPolicyOptions;
        transportPolicyOptions.Transport = transport;
        
        std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
        policies.push_back(
            std::make_unique<Azure::Core::Http::Policies::_internal::TransportPolicy>(
                transportPolicyOptions));
        
        Azure::Core::Http::_internal::HttpPipeline pipeline(policies);
        
        // Make a test request
        Azure::Core::Url url("https://httpbin.org/get");
        Azure::Core::Http::Request request(Azure::Core::Http::HttpMethod::Get, url);
        
        std::cout << "Sending request to " << url.GetAbsoluteUrl() << std::endl;
        
        auto response = pipeline.Send(request, Azure::Core::Context{});
        
        std::cout << "Response status: " 
                  << static_cast<int>(response->GetStatusCode()) 
                  << std::endl;
        
        // Read response body
        auto bodyStream = response->ExtractBodyStream();
        std::vector<uint8_t> bodyBuffer(1024);
        auto bytesRead = bodyStream->Read(bodyBuffer.data(), bodyBuffer.size(), Azure::Core::Context{});
        
        std::cout << "Response body: " 
                  << std::string(bodyBuffer.begin(), bodyBuffer.begin() + bytesRead) 
                  << std::endl;
        
        return 0;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
}
```

## Option 2: Quick Test Using Visual Studio Directly

### Simpler approach for testing:

1. Open Visual Studio Insiders

2. Open the azure-sdk-for-cpp folder:
   - `File` → `Open` → `Folder...`
   - Select `C:\Users\kraman\source\repos\azure-sdk-for-cpp`

3. Visual Studio should detect the CMake project automatically

4. Select a configuration from the dropdown (e.g., `x64-Debug`)

5. Build the azure-core target:
   - Right-click on `CMakeLists.txt` → `Build`
   - Or use `Build` → `Build All`

6. Run the tests:
   - In the Test Explorer, find the new `CurlOptionsCallback` test
   - Run it to verify the functionality works

## Option 3: Create a Standalone Test in the Current Repository

The easiest way to test your changes is to add your test code to the existing test suite (already done in curl_options_test.cpp).

To run it:
1. Build the test project
2. Run: `out\build\sdk\core\azure-core\test\ut\azure-core-test.exe --gtest_filter=CurlTransportOptions.CurlOptionsCallback`

## Network Interface Binding Example

For your multi-NIC scenario, here's how to bind to a specific network interface:

```cpp
curlOptions.CurlOptionsCallback = [](void* curlHandle) {
    CURL* handle = static_cast<CURL*>(curlHandle);
    
    // Bind to specific network interface by name
    curl_easy_setopt(handle, CURLOPT_INTERFACE, "Ethernet 2");
    
    // Or by IP address
    // curl_easy_setopt(handle, CURLOPT_INTERFACE, "192.168.1.100");
};
```

## Next Steps

1. Commit your test changes
2. Build and verify the tests pass
3. Push to your branch
4. Create a pull request

Let me know if you need help with any of these steps!
