# HTTP/SOCKS Proxy Tester
# This script tests a proxy by attempting a connection to www.msn.com through a proxy and saves the response.
#
#Compile: gcc -o -pthread proxytester proxytester.c
#
#Usage: Run with following arguments... ./proxytester proxies_file.txt output_file.txt 1-4
#1 = HTTP
#2 = HTTPS
#3 = SOCKS4
#4 = SOCKS5
