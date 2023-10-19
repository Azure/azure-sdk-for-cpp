This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

# umock_c

umock_c is a C mocking library.

## Setup

### Build

- Clone umock_c by:

```
git clone --recursive https://github.com/Azure/umock-c.git
```

- Create a cmake folder under the root of umock-c

- Switch to the cmake folder and run
```
cmake ..
```

If you would like to use installed (by CMake) versions of packages already on your machine:

```
cmake -Duse_installed=ON ../
```

- Build the code for your platform (msbuild for Windows, make for Linux, etc.) by executing in the cmake folder: 

```
cmake --build .
```

### To install umock_c:

```
cmake -Duse_installed=ON ../
```
On Linux:
```
sudo make install
```
On Windows:
```
msbuild /m INSTALL.vcxproj
```

_This requires that ctest and testrunnerswitcher are both installed (through CMake) on your machine._

### Building tests

In order to build the tests use the *run_unittests* cmake option:

```
cmake .. -Drun_unittests:bool=ON
```

## Example

Ever wanted to write something like this in C as a test?

```c
TEST_FUNCTION(my_first_test)
{
    // arrange
    STRICT_EXPECTED_CALL(test_dependency_1_arg(42))
        .SetReturn(44)
        .IgnoreAllArguments();

    // act
    int result = function_under_test();

    // assert
    ASSERT_ARE_EQUAL(int, 44, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}
```

umock_c has way more features than this simple example!

## Dependencies

- umock_c uses ctest as test runner (https://github.com/Azure/azure-ctest.git). ctest is a C test runner that can be run on many platforms as it does not make use of compiler/platform specific code and thus it is easily portable.
- umock_c uses cmake (https://cmake.org/) to generate build files.
- umock_c uses testrunnerswitcher to allow switching between ctest and CppUnitTest for Windows. 

## Documentation

Complete documentation is available [here](doc/umock_c.md).
