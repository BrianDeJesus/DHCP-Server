# DHCP server in C

This was an extra credit assignment in my networking class at CSUSM. It was interesting.

Both server and client take two command line arguments: IP multicast address and port number.

**Server side**
```
$ gcc -pthread dhcpserver.c -o serve
```
```
$ ./serve <ip_address> <port>
```

**Client side**
```
$ gcc dhcpclient.c -o client
```
```
$ ./client <ip_address> <port>
```

On client side, start request for ip address from DHCP server by entering DHCP discover