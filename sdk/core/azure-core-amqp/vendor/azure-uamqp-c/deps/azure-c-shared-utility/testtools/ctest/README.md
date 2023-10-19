This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

# ctest

azure-ctest is a simple portable C test runner.

## Setup

1. Clone **azure-ctest** by:

```
git clone https://github.com/Azure/azure-ctest
```

2. Create a folder called *cmake* (or any name of your choice).

3. Switch to the *cmake* folder and run
```
cmake ..
```

### Build

Switch to the *cmake* folder and run:

```
cmake --build .
```

### Installation and Use
Optionally, you may choose to install azure-ctest on your machine:

1. Switch to the *cmake* folder and run
    ```
    cmake --build . --target install
    ```
    or

    Linux:
    ```
    sudo make install
    ```

    Windows:
    ```
    msbuild /m INSTALL.vcxproj
    ```

2. Use it in your project (if installed)
    ```
    find_package(ctest REQUIRED CONFIG)
    target_link_library(yourlib ctest)
    ```

### Building the tests

In order to build the tests use the *run_unittests* cmake option:

```
cmake .. -Drun_unittests:bool=ON
```

## Example

```c
#include "ctest.h"
#include "SomeUnitUnderTest.h"

CTEST_BEGIN_TEST_SUITE(SimpleTestSuiteOneTest)

CTEST_FUNCTION(Test1)
{
    // arrange

    // act
    int x = SomeFunction();

    // assert
    CTEST_ASSERT_ARE_EQUAL(int, 42, x);
}

CTEST_END_TEST_SUITE(SimpleTestSuiteOneTest)
```
