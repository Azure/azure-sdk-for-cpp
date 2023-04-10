param(
    $JobNames,
    $Project = $env:SYSTEM_TEAMPROJECT,
    $Org = $env:SYSTEM_COLLECTIONID,
    $BuildId = $env:BUILD_BUILDID,
    $AccessToken = $env:SYSTEM_ACCESSTOKEN
)

Set-StrictMode -Version 4.0

if (!($JobNames -is [array])) {
    $JobNames = @($JobNames)
}

$credential = New-Object System.Management.Automation.PSCredential `
    -ArgumentList "pat", (ConvertTo-SecureString $AccessToken -AsPlainText -Force)

$buildTimeline = Invoke-RestMethod `
    -Method Get `
    -Authentication Basic `
    -Credential $credential `
    -Uri "https://dev.azure.com/$Org/$Project/_apis/build/builds/$BuildId/timeline?api-version=5.0"

$hasError = $false

foreach($jobName in $JobNames) {
    $job = $buildTimeline.records `
        | Where-Object { $_.name -eq $jobName }
        | Sort-Object -Property attempt -Descending
        | Select-Object -First 1

    if (!$job) {
        $hasError = $true
        Write-Host "Could not find matching job in timeline: $jobName"
        continue
    }

    if ($job.result -ne "succeeded") {
        $hasError = $true
        Write-Host "Job failed: $jobName"
    }

    Write-Host "Job succeeded: $jobName"
}

if ($hasError) {
    Write-Host "Exiting with error"
    exit 1
}

Write-Host "Evaluated jobs succeeded"
exit 0