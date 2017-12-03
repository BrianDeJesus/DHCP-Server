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

    struct sockaddr_in cliaddr;
    struct ip_mreq group;

    int fd;
    int clilen;

    char *ipaddr;
    char *ifaddr;
    char buf[80];

    int port;
    char *ack = "Server ACK";
    socklen_t cliaddr_len;


    if (argc != 3) { // Check command line args
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

    /* Bind to the proper port number with the IP address
       specified as INADDR_ANY. */

    memset((char *) &cliaddr, 0, sizeof(cliaddr));

    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(port);
    cliaddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (struct sockaddr*)&cliaddr, sizeof(cliaddr)))
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
          clilen = sizeof(cliaddr);
          if (recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&cliaddr, &clilen) < 0) {
      			perror("recvfrom error");
      			exit(-1);
      		}

            printf("The message from multicast server host client is: %s\n", buf);
            sendto(fd, (const void*)ack, strlen(ack), 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
      }


    return 0;

}
