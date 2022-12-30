[console]::OutputEncoding = New-Object System.Text.UnicodeEncoding

# Workaround from https://github.com/actions/runner-images/issues/6844#issuecomment-1367225048
# At the time of this comment (30-12-2022) this only works on Azure DevOps hosted
# agents and doesn't work on 1ES images
Write-Host "wsl --update --web-download"
wsl --update --web-download | Out-String

# Out-String waits for the process to finish
Write-Host "wsl --update --web-download | Out-String"
wsl --update --web-download | Out-String

Write-Host "wsl --version" 
wsl --version

write-host "WSL install of ubuntu."
wsl --install -d Ubuntu-20.04 --web-download

write-host "Launch WSL."
$wsl = wsl -l -v | out-string

write-host $wsl
while ($wsl -notmatch 'Ubuntu-20.04.*running') {
	start-sleep -seconds 1
	$wsl = wsl -l -v | out-string
	write-host $wsl
}
write-host "Ubuntu installed."
