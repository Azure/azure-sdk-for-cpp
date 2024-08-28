cmake_minimum_required(VERSION 3.30.0)

# Remember to replace the path below with the path where you cloned vcpkg
# set(CMAKE_TOOLCHAIN_FILE "path/to/vcpkg/root/scripts/buildsystems/vcpkg.cmake")

project(azure_sample VERSION 0.1.0 LANGUAGES C CXX)

find_package(azure-identity-cpp CONFIG REQUIRED)
find_package(azure-security-keyvault-secrets-cpp CONFIG REQUIRED)

add_executable(azure_sample main.cpp)

target_link_libraries(azure_sample PRIVATE
    Azure::azure-identity
    Azure::azure-security-keyvault-secrets
)