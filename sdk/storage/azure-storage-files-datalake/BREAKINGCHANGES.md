# Release History

## 1.0.0-beta.3 (2020-10-13)

* CreateFromConnectionString now accepts unencoded file, path and directory name.
* `ETag` and `LastModified` is now `std::string` instead of `Azure::Core::Nullable<std::string>` in `CreateDirectoryResult`, `CreateFileResult` and `CreatePathResult`.
