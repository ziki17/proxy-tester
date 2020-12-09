# HTTP/SOCKS Proxy Tester
<b>This script tests a proxy by attempting a connection to ```www.msn.com``` through a proxy and saves the response.</b>

# Setup
You can simply use the following command: ```gcc -pthread proxytester.c -o ProxyTester```

# Usage
- [x] ./ProxyTester proxy_list.txt output_file.txt [PROXY TYPE]

<b>Specifying Proxy Type</b>:
- 1 = HTTP
- 2 = HTTPS
- 3 = SOCKS4
- 4 = SOCKS5
