[CmdletBinding()]
Param (
    [Parameter()]
    [ValidateNotNullOrEmpty()]
    [string] $Ref = (Get-Content "$PSScriptRoot/../vcpkg-commit.txt"),

    [Parameter(Mandatory = $true)]
    [ValidateNotNullOrEmpty()]
    [string] $Dependencies,

    [Parameter()]
    [ValidateNotNullOrEmpty()]
    [string] $VcpkgPath = "$PSScriptRoot/../../vcpkg"
)

$initialDirectory = Get-Location

try {
    Write-Host "git clone https://github.com/Microsoft/vcpkg $VcpkgPath"
    git clone https://github.com/Microsoft/vcpkg $VcpkgPath
    Write-Host "Set-Location $VcpkgPath"
    Set-Location $VcpkgPath
    Write-Host "git fetch --tags"
    git fetch --tags
    Write-Host "git checkout $Ref"
    git checkout $Ref

    if ($IsWindows) {
        Write-Host ".\bootstrap-vcpkg.bat"
        .\bootstrap-vcpkg.bat
        Write-Host ".\vcpkg.exe install $Dependencies.Split(' ')"
        .\vcpkg.exe install $Dependencies.Split(' ')
    } else {
        Write-Host "./bootstrap-vcpkg.sh"
        ./bootstrap-vcpkg.sh
        Write-Host "./vcpkg install $Dependencies.Split(' ')"
        ./vcpkg install $Dependencies.Split(' ')
    }
} finally {
    Set-Location $initialDirectory
}