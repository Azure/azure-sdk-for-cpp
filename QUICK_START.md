# Quick Start: Testing CurlOptionsCallback in Visual Studio Insiders

## Steps to Build and Test

### 1. Open the Project in Visual Studio Insiders

1. Launch **Visual Studio Insiders**
2. Click `File` → `Open` → `Folder...`
3. Navigate to and select: `C:\Users\kraman\source\repos\azure-sdk-for-cpp`
4. Wait for Visual Studio to load the CMake project (watch the Output pane)

### 2. Select a Build Configuration

In the toolbar, you should see a configuration dropdown. Select one of:
- `x64-Debug` (recommended for testing)
- `x64-static-debug-tests-curl` (includes curl transport and tests)

### 3. Build the Project

Option A - Build Everything:
- `Build` → `Build All` (or Ctrl+Shift+B)

Option B - Build Just azure-core:
- In Solution Explorer, find `CMakeLists.txt` under `sdk/core/azure-core`
- Right-click → `Build`

### 4. Run the Test

Option A - Using Test Explorer:
1. Open Test Explorer: `Test` → `Test Explorer`
2. Wait for tests to be discovered
3. Find `CurlTransportOptions.CurlOptionsCallback`
4. Right-click → `Run`

Option B - Using Terminal:
1. Open Terminal in VS: `View` → `Terminal`
2. Navigate to build output:
   ```powershell
   cd out\build\x64-Debug\sdk\core\azure-core\test\ut
   ```
3. Run the specific test:
   ```powershell
   .\azure-core-test.exe --gtest_filter=CurlTransportOptions.CurlOptionsCallback
   ```

### 5. Verify Your Changes Work

The test should:
- ✅ Invoke the callback
- ✅ Successfully make an HTTP request
- ✅ Return a 200 status code

## Alternative: Use Existing Build Directory

If your `build` directory already has a working configuration:

1. Open **Developer PowerShell for VS Insiders**:
   - In Visual Studio Insiders: `Tools` → `Command Line` → `Developer PowerShell`

2. Navigate and build:
   ```powershell
   cd C:\Users\kraman\source\repos\azure-sdk-for-cpp
   
   # Try to build with existing configuration
   cmake --build build --target azure-core-test --config Debug
   ```

3. Run the test:
   ```powershell
   .\build\sdk\core\azure-core\test\ut\Debug\azure-core-test.exe --gtest_filter=CurlTransportOptions.CurlOptionsCallback
   ```

## Troubleshooting

### Can't find Visual Studio?
- Make sure you're using **Developer PowerShell** (not regular PowerShell)
- It should have Visual Studio environment variables loaded

### Need to reconfigure?
Delete the build/out directories and let Visual Studio recreate them:
```powershell
Remove-Item -Recurse -Force build, out -ErrorAction SilentlyContinue
```
Then reopen the folder in Visual Studio.

## What the Test Does

The new test in `curl_options_test.cpp` demonstrates:

1. **Creates a callback function** that receives the CURL handle
2. **Sets custom CURL options** (like CURLOPT_VERBOSE)
3. **Makes an HTTP request** to verify it works
4. **Verifies the callback was invoked**

You can modify the callback to test network interface binding:

```cpp
curlOptions.CurlOptionsCallback = [](void* curlHandle) {
    CURL* handle = static_cast<CURL*>(curlHandle);
    curl_easy_setopt(handle, CURLOPT_INTERFACE, "192.168.1.100");
    // Or: curl_easy_setopt(handle, CURLOPT_INTERFACE, "Ethernet 2");
};
```
