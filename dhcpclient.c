#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAXLINE 80

int main (int argc, char *argv[])
{

    struct in_addr localInterface;
    struct sockaddr_in cliaddr;
    char   str[80];
    char rec[60];

    char *ipaddr;
    char *lfaddr;

    int port;
    int fd, n, cli_len;
    int reuse = 1;

    if (argc != 3) {
    	printf("Usage: tcpclient <address> <port> \n");
    	exit(-1);
    }

    ipaddr = argv[1];
    port = atoi(argv[2]);
    lfaddr  = "127.0.0.1";


    /* Create a datagram socket on which to send. */
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        perror("Opening datagram socket error");
        exit(1);
    }


    /* Initialize the group sockaddr structure  */
    memset((char *) &cliaddr, 0, sizeof(cliaddr));

    bzero(&cliaddr, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_addr.s_addr = inet_addr(ipaddr);
    cliaddr.sin_port = htons(port);


    localInterface.s_addr = inet_addr(lfaddr);
    if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
    {
        fprintf(stderr, "Error setting local interface %x (%s) \n",
                errno, strerror(errno));
        exit(1);

    }

    cli_len = sizeof(cliaddr);
    while (1) {
        memset((char *) &str, 0, sizeof(str));
        memset((char *) &rec, 0, sizeof(rec));
        fgets(str, MAXLINE, stdin);
        if (sendto (fd, str, MAXLINE, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr)) < 0) {
            fprintf(stderr, "Error sending datagram message: %x (%s) \n",
                    errno, strerror(errno));
            exit( -1);
        }

        n = recvfrom(fd, rec, MAXLINE, 0, (struct sockaddr *)&cliaddr, &cli_len);

        printf("The message from multicast DHCP server is: %s\n", rec);

  }

    close(fd);
    return 0;
}
