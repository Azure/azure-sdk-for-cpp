# Only show contents of particular vcpkg log files whose potentially sensitive
# information will be redacted by DevOps (if it's included).

# Before adding other expressions to output ensure that those logs will not
# contain sensitive information or that DevOps is properly configured to remove
# sensitive information.

$logFiles = Get-ChildItem -Recurse -Filter *.log
$filteredLogs = $logFiles.Where({ $_.Name -in ('vcpkg-bootstrap.log', 'vcpkg-manifest-install.log') })

foreach ($logFile in $filteredLogs)
{
    Write-Host "//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////"
    Write-Host "=============================================================================================================================="
    Write-Host "Log file: $logFile"
    Write-Host "=============================================================================================================================="
    Get-Content $logFile | Write-Host
}