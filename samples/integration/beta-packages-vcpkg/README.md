<!-- IN PROGRESS -->
# Getting Beta Releases in Vcpkg

Official vcpkg registry may have beta versions of Azure SDK client libraries, up until a given library gets released as stable. After that, we don't publish post-first-stable beta releases of that library in the official registry.

If you are interested in both stable releases and post-first-stable beta releases, see [Azure SDK Beta Vcpkg Registry](https://github.com/Azure/azure-sdk-vcpkg-betas/). You can update the `AzureVcpkg.cmake` module to use the beta registry.