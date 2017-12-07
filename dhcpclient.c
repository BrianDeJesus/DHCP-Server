/*
Brian DeJesus
cs436 Networking
DHCP Server
tool: C
Enter: "DHCP discover" without quotes on client side to request an ip address
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
 #include <net/if.h>
#include <sys/ioctl.h>

#define MAXLINE 80
#define HWADDR_len 6

void mac_eth0(unsigned char MAC_str[13]) { //Get mac address
     int s,i;
     struct ifreq ifr;
     s = socket(AF_INET, SOCK_DGRAM, 0);
     strcpy(ifr.ifr_name, "eth0");
     ioctl(s, SIOCGIFHWADDR, &ifr);
     for (i=0; i<HWADDR_len; i++)
         sprintf(&MAC_str[i*2],"%02X",((unsigned char*)ifr.ifr_hwaddr.sa_data)[i]);
     MAC_str[12]='\0';
     close(s);
 }

 int message_match(char *msg, char*key) { //Check if message matches
   if(strncmp(key, msg, strlen(key)) == 0){
     return 1;
   }
   return 0;
 }


int main (int argc, char *argv[])
{

    char mac[13];
    struct in_addr localInterface;
    struct sockaddr_in cliaddr;
    char  str[80];  //String to be sent
    char rec[80];   //Buffer to receive server messages
    char dhcp_offer[] = " DHCP offer!";
    int rec_len;
    char offered_ip[12];


    char *ipaddr;
    char *lfaddr;

    int port;
    int fd, n, cli_len;
    int reuse = 1;

    if (argc != 3) { // Check command line arguments
    	fprintf(stderr, "Usage: %s <ipaddr> <port> \n", argv[0]);
    	exit(-1);
    }

    mac_eth0(mac);  //Get mac address of client
    printf("Obtained MAC addres: %s \n", mac);
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

    char *server_offer;
    cli_len = sizeof(cliaddr);
    while (1) {
        memset((char *) &offered_ip, 0, sizeof(offered_ip)); // Clear buffers
        memset((char *) &str, 0, sizeof(str)); // Clear buffers
        memset((char *) &rec, 0, sizeof(rec));
        fgets(str, MAXLINE, stdin);   // Input message
        strcat(str, mac);         // Attach mac address along with message
        if (sendto (fd, str, MAXLINE, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr)) < 0) {
            fprintf(stderr, "Error sending datagram message: %x (%s) \n",
                    errno, strerror(errno));
            exit( -1);
        }
        if(message_match(str, "DHCP discover")) {
          printf("Press Enter to see if server acknowledged request\n");
        }
        n = recvfrom(fd, rec, MAXLINE, 0, (struct sockaddr *)&cliaddr, &cli_len);
        rec_len = strlen(rec);
        server_offer = &rec[rec_len-12]; //Get servers discover message
        if(message_match(server_offer, dhcp_offer)){
          strncpy(offered_ip, rec, 12);
          offered_ip[12] = '\0';  //Manually set end of string for offered ip
          memset((char *) &str, 0, sizeof(str)); // Clear str buffer
          strcpy(str, "DHCP request!");  //Send server the DHCP request for using ip that was offered
          sendto (fd, str, MAXLINE, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
        }
        printf("The message from multicast DHCP server is: %s\n", rec);

  }

    close(fd);
    return 0;
}
