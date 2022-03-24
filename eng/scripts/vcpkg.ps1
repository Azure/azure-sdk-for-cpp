[CmdletBinding()]
Param (
    [Parameter()]
    [ValidateNotNullOrEmpty()]
    [string] $Ref = (Get-Content "$PSScriptRoot/../vcpkg-commit.txt"),

    [Parameter()]
    [ValidateNotNullOrEmpty()]
    [string] $VcpkgPath = "$PSScriptRoot/../../vcpkg"
)

$initialDirectory = Get-Location

try {
    git clone https://github.com/Microsoft/vcpkg $VcpkgPath
    Set-Location $VcpkgPath
    git fetch --tags
    git checkout $Ref

    if ($IsWindows) {
        .\bootstrap-vcpkg.bat
    } else {
        ./bootstrap-vcpkg.sh
    }
} finally {
    Set-Location $initialDirectory
}
