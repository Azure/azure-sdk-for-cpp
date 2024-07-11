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

$filteredLogs = $filteredLogs | foreach {$_.FullName}

for ($i = 0; $i -lt $filteredLogs.Length; $i += 1)
{
    $logFile = $filteredLogs[$i]

    Write-Host "//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////"
    Write-Host "=============================================================================================================================="
    Write-Host "Log file: $logFile"
    Write-Host "=============================================================================================================================="
    try {
        Get-Content $logFile | Write-Host

        # vcpkg logs do reference build log paths after "See logs for more information:". Sometimes there are multiple files to see.
        # And should there be a C++ build error, for example - that is where the error message would be.
        # So, we parse known logs (contained in vcpkgLogFileNames), and see if more logs are mentioned there. If there are extra logs,
        # we add them to the end of the list that we're iterating over, so that the format is the same, and this code gets reused
        # (i.e. formatting, Log file name header, try-catch, and so on).
        if ($i -lt $vcpkgLogFileNames.Length)
        {
            $rawContents = Get-Content $logFile -Raw
            $regexMatches = Select-String "See logs for more information\:\s*(\r|\n|\r\n|\n\r)(\s+(?<logFilePath>\S*)\s*(\r|\n|\r\n|\n\r))+" -input $rawContents -AllMatches
            foreach ($additionalLogFile in $regexMatches.matches.groups.Where({ $_.Name -eq "logFilePath" }))
            {
                $filteredLogs += $additionalLogFile.Value
            }
        }
    } catch { 
        Write-Host "Could not locate file found using Get-ChildItem $logFile"
    }
}

exit 0
