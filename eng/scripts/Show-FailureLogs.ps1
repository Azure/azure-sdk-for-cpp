# Only show contents of particular vcpkg log files whose potentially sensitive
# information will be redacted by DevOps (if it's included).

# Before adding other expressions to output ensure that those logs will not
# contain sensitive information or that DevOps is properly configured to remove
# sensitive information.

$logFiles = Get-ChildItem -Recurse -Filter *.log
$filteredLogs = $logFiles.Where({ $_.Name -in ('vcpkg-bootstrap.log', 'vcpkg-manifest-install.log') })

$filteredLogs.FullName | Write-Host

if (!$filteredLogs) {
    Write-Host "No logs found"
    exit 0
}

$filteredLogs = $filteredLogs | select FullName

for ($i = 0; $i -lt $filteredLogs.Length; $i += 1)
{
    $logFile = $filteredLogs[$i]

    Write-Host "//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////"
    Write-Host "=============================================================================================================================="
    Write-Host "Log file: $logFile"
    Write-Host "=============================================================================================================================="
    try {
        Get-Content $logFile | Write-Host

        $rawContents = Get-Content $logFile -Raw
        $regexMatches = Select-String "See logs for more information\:\s*(\r|\n|\r\n|\n\r)(\s+(?<logFilePath>\S*)\s*(\r|\n|\r\n|\n\r))+" -input $logFile -AllMatches
        foreach ($furtherDetails in $regexMatches.matches.groups.Where({ $_.Name -eq "logFilePath" }))
        {
            $filteredLogs += $furtherDetails.Value
        }
    } catch { 
        Write-Host "Could not locate file found using Get-ChildItem $logFile"
    }
}

exit 0
