write-host "WSL install of ubuntu."
wsl --install -d Ubuntu-20.04

write-host "Launch WSL."
[console]::OutputEncoding = New-Object System.Text.UnicodeEncoding
$wsl = wsl -l -v | out-string

write-host $wsl
while ($wsl -notmatch 'Ubuntu-20.04.*running') {
	start-sleep -seconds 1
	$wsl = wsl -l -v | out-string
	write-host $wsl
}
write-host "Ubuntu installed."
