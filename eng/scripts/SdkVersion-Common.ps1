# NOTE: Update-PkgVersion and Get-PkgVersion relies on these variables and
# functions
$RepoRoot = "${PSScriptRoot}/../.."
$VersionRegex = '(#define AZURE_\w+_VERSION_MAJOR )(?<major>[0-9]+)(\s+#define AZURE_\w+_VERSION_MINOR )(?<minor>[0-9]+)(\s+#define AZURE_\w+_VERSION_PATCH )(?<patch>[0-9]+)(\s+#define AZURE_\w+_VERSION_PRERELEASE )"(?<prerelease>[a-zA-Z0-9.]*)"';

function Get-VersionHppLocaiton ($ServiceDirectory, $PackageName) {
    $versionHppLocation = Get-ChildItem version.hpp -Path "$RepoRoot/sdk/$ServiceDirectory/$PackageName" -Recurse
    Write-Verbose "version.hpp location: $versionHppLocation"

    if (!$versionHppLocation) {
        Write-Warning "Could not locate version.hpp file in sdk/$ServiceDirectory/$PackageName"
    }

    return $versionHppLocation
}
