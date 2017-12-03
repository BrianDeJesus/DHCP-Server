#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


int main(int argc, char *argv[])
{

    struct sockaddr_in servaddr;
    struct ip_mreq group;

    int fd;
    int buflen;

    char *ipaddr;
    char *ifaddr;
    char buf[80];

    int port;
    int reuse = 1;


    if (argc != 3) {
        fprintf(stderr, "Usage: %s <ipaddr> <port> \n", argv[0]);
        exit (-1);
    }

    ipaddr = argv[1];
    port = atoi(argv[2]);
    ifaddr  = "127.0.0.1";

    /* Create a datagram socket on which to receive. */
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        fprintf(stderr, "Error opening datagram socket %x (%s) \n",
                errno, strerror(errno));
        exit( -1);

    }

    /* Enable SO_REUSEADDR to allow multiple instances of this
       application to receive copies of the multicast datagrams. */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
    {
        fprintf(stderr, "Error setting SO_REUSEADDR %x (%s) \n",
                errno, strerror(errno));
        close(fd);

        exit( -1);

    }

    /* Bind to the proper port number with the IP address
       specified as INADDR_ANY. */

    memset((char *) &servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (struct sockaddr*)&servaddr, sizeof(servaddr)))
    {
        fprintf(stderr, "Error binding datagram socket %x (%s) \n",
                errno, strerror(errno));
        close(fd);
        exit(-1);

    }

    /* Join the multicast group  on the local host
       interface. Note that this IP_ADD_MEMBERSHIP option must be
       called for each local interface over which the multicast
       datagrams are to be received. */

    group.imr_multiaddr.s_addr = inet_addr(ipaddr);
    group.imr_interface.s_addr = inet_addr(ifaddr);
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
    {
        fprintf(stderr, "Error adding multicast group %x (%s) \n",
                errno, strerror(errno));
        close(fd);
        exit(1);
    }

    /* Read from the socket. */
    while(1) {
      buflen = sizeof(buf);
      if (read(fd, buf, buflen) < 0)
      {
          fprintf(stderr, "Error reading datagram message %x (%s) \n",
                  errno, strerror(errno));
          close(fd);
          exit(1);

      }

      printf("The message from multicast server is: %s\n", buf);
    }

    return 0;

}
