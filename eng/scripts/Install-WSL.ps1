wsl --install -d Ubuntu-20.04

[console]:OutputEncoding = New-Object System.Text.UnicodeEncoding
$wsl = wsl l -v | out-string

write-host $wsl
while ($wsl -match 'installing') {
	start-sleep -seconds 1
	$wsl = wsl l -v | out-string
	write-host $wsl
}
