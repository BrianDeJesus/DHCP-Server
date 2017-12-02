/*
Brian DeJesus
extra credit Client
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define MAXLINE 80

void mac_addr(unsigned char MAC_str[13])
{
    #define HWADDR_len 6
    int s,i;
    struct ifreq ifr;
    s = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, "eth0");
    ioctl(s, SIOCGIFHWADDR, &ifr);
    for (i=0; i<HWADDR_len; i++)
        sprintf(&MAC_str[i*2],"%02X",((unsigned char*)ifr.ifr_hwaddr.sa_data)[i]);
    MAC_str[12]='\0';
}

int main(int argc, char *argv[])
{
    int	socket_fd;
    struct  sockaddr_in servaddr;
    unsigned char mac[13];

    mac_addr(mac);
    puts(mac);

    char str[80]; //buffer string

    if (argc != 3) {
    	printf("Usage: tcpclient <address> <port> \n");
    	exit(-1);
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
    	perror("socket error");
    	exit(-1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));


    if (connect(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
    	perror("connect error");
    	exit(-1);
    }

    while (fgets(str, MAXLINE, stdin) != NULL) {

      write(socket_fd, str, sizeof(str));
      printf("Sent data to server\n");
      memset(str, 0, MAXLINE);
      if (read(socket_fd, str, MAXLINE) == 0) {
          printf("ERROR: server terminated \n");
         break;
       }

      printf("From the SERVER: %s\n", str);

    }

    close(socket_fd);

    return EXIT_SUCCESS;

}
