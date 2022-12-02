# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# cspell: words auxf
wsl sudo apt-get -y install squid
if (!$?) {
	write-error "Failed to launch anonymous server."
}

copy localproxy/squid.conf \\wsl$\Ubuntu-20.04\etc\squid\squid.conf
copy localproxy.passwd/proxypasswd //wsl$/Ubuntu-20.04/tmp/proxypasswd
copy localproxy.passwd/squid.conf \\wsl$\Ubuntu-20.04\tmp\squid.conf

write-host "Verify contents of /etc/squid and /tmp to confirm copy succeeded."
wsl ls -l /etc/squid /tmp

write-host "Launch anonymous squid server."
wsl sudo squid -f /etc/squid/squid.conf
if (!$?) {
	write-error "Failed to launch anonymous server."
}

write-host "Launch authenticated squid server."
wsl sudo squid -f /tmp/squid.conf
if (!$?) {
	write-error "Failed to launch authenticated server."
}

write-host "Dump processes; verify two copies of squid running."
wsl ps auxf