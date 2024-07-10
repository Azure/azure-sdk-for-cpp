# Only show contents of particular vcpkg log files whose potentially sensitive
# information will be redacted by DevOps (if it's included).

# Before adding other expressions to output ensure that those logs will not
# contain sensitive information or that DevOps is properly configured to remove
# sensitive information.

$logFiles = Get-ChildItem -Recurse -Filter *.log
$vcpkgLogFileNames = ('vcpkg-bootstrap.log', 'vcpkg-manifest-install.log')
$filteredLogs = $logFiles.Where({ $_.Name -in $vcpkgLogFileNames })

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

        Write-Host "i =" $i
        Write-Host "vcpkgLogFileNames.Length =" $vcpkgLogFileNames.Length
        if ($i -lt $vcpkgLogFileNames.Length)
        {
            $rawContents = Get-Content $logFile -Raw
            $regexMatches = Select-String "See logs for more information\:\s*(\r|\n|\r\n|\n\r)(\s+(?<logFilePath>\S*)\s*(\r|\n|\r\n|\n\r))+" -input $logFile -AllMatches
            Write-Host "regexMatches.matches.groups.Length =" $regexMatches.matches.groups.Length
            Write-Host "---"
            Write-Host "---"
            Write-Host $regexMatches.matches.groups
            Write-Host "---"
            Write-Host "---"
            foreach ($furtherDetails in $regexMatches.matches.groups.Where({ $_.Name -eq "logFilePath" }))
            {
                Write-Host "furtherDetails.Value = " $furtherDetails.Value
                $filteredLogs += $furtherDetails.Value
            }
        }
    } catch { 
        Write-Host "Could not locate file found using Get-ChildItem $logFile"
    }
}

exit 0
