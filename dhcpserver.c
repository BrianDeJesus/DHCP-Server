/*
Brian DeJesus
cs436 Networking
DHCP Server
tool: C
Simulates a DHCP server
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#define MAXLINE 80
#define MAX_THREADS 20
#define MAX_OFFERS 10
#define MAC_LEN 12
pthread_t threads[MAX_THREADS];
static int addr_index = 0;
static int mac_index = 0;
char macs[MAX_OFFERS][MAC_LEN] = {" " , " ", " ", " ", " ", " ", " ", " ", " "};

struct thread_argos { //Struct used for multiple thread function arguments
  int sock;
  struct sockaddr_in cli_addr;
  int cli_len;
  char *the_buf;
    };

int check_macs(char *a_mac, char macz[][MAC_LEN]) {  //Check if host already been assigned ip address
  int i;
  for(i = 0; i < MAX_OFFERS; i++) {
    if(strncmp(a_mac, macz[i], MAC_LEN) == 0)
      return 1;
  }
  return 0;
}

void *connection_handler(void *thread_args) {  //Handle requests thread function
  struct thread_argos *the_args = (struct thread_argos*) thread_args;
  int fd = the_args->sock;
  struct sockaddr_in cliaddr = the_args->cli_addr;
  char *buf = the_args->the_buf; // Get buffer from args
  char sees_msg[] = "Server saw the message \n";
  char reject[] = "Mac address already found! \n";
  char dhcp_offer[] = " DHCP offer!";
  char new_ip_addr[] = "192.168.1.11";
  char ack[] = "DHCP ack";
  char address_offer[MAX_OFFERS][20];
  int len = strlen(new_ip_addr);  //Get length of ip_addr
  char int_to_char[1];   //Char holder for new unique last num for new ip
  int buf_len = strlen(buf);

  sprintf(int_to_char, "%i", addr_index); // Convert int to char
  new_ip_addr[len - 1] = int_to_char[0];  // Construct new ip
  strcpy(address_offer[addr_index], new_ip_addr);  // Add to address offer array

  char *client_mac_addr = &buf[buf_len - 12]; //Get client's mac address
  strcat(address_offer[addr_index], dhcp_offer); // Attach offer message with ip offer

  if(client_key(buf, "DHCP discover")) {  //If the client requested an ip
    printf("Client discovery! \n");
    printf("MAC: %s\n", client_mac_addr);
    if(check_macs(client_mac_addr, macs)) {  //If same host tries to restart requesting another ip
      printf("Mac address already found. Terminating now \n");
      sendto (fd, reject, strlen(reject), 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
      exit(-1);
    }
    else {  //If unique mac address
      printf("Successfully added client \n");
      strcpy(macs[mac_index], client_mac_addr); //Place client mac in used mac array
      if(mac_index > MAX_OFFERS) {
        printf("Mac address limit reached. ");
        exit(-1);
      }
      mac_index++;
    }
    if (sendto (fd, address_offer[addr_index], MAXLINE, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr)) < 0) {
        fprintf(stderr, "Error sending datagram message: %x (%s) \n",
                errno, strerror(errno));
        exit( -1);
    }
    if(addr_index >= MAX_OFFERS) { //Check if max amount of offers
      printf("Max offers reached. \n");
      exit(-1);
    }
    printf("addr_index: %i\n", addr_index);
    addr_index++;
    return NULL; //End of send process (already sent a message)
  }
  if(client_key(buf, "DHCP request!")) {
    sendto (fd, ack, strlen(ack), 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
    return NULL;
  }


  sendto(fd, (const void*)sees_msg, strlen(sees_msg), 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));

    return NULL;
}


int client_key(char *msg, char *key) { //Check if client DHCP discover message
  if(strncmp(key, msg, strlen(key)) == 0){
    return 1;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  struct sockaddr_in cliaddr;
  struct ip_mreq group;
  int fd;
  int clilen;
  int thread_no = 0;
  char *ipaddr;
  char *ifaddr;
  int port;

  char buf[80];

  struct thread_argos args;  //Argument struct for thread function
  int i, rc;

    if (argc != 3) { // Check command line args
        fprintf(stderr, "Usage: %s <ipaddr> <port> \n", argv[0]);
        exit (-1);
    }

    ipaddr = argv[1];
    port = atoi(argv[2]);
    ifaddr  = "127.0.0.1";

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
    // Join the multicast group  on the local host interface.
    group.imr_multiaddr.s_addr = inet_addr(ipaddr);
    group.imr_interface.s_addr = inet_addr(ifaddr);
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
    {
        fprintf(stderr, "Error adding multicast group %x (%s) \n",
                errno, strerror(errno));
        close(fd);
        exit(1);
    }

    clilen = sizeof(cliaddr);
    args.cli_len = clilen;
    while(1) {
          if (recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&cliaddr, &clilen) < 0) { // Receive client message
            perror("recvfrom error");
            exit(-1);
          }

            args.cli_addr = cliaddr;
            args.sock = fd;
            args.the_buf = buf;
            printf("The message from multicast server host client is: %s\n", buf);
            rc = pthread_create(&threads[thread_no], NULL, connection_handler, (void*)&args); // Create thread upon client request
        		if(rc) {
        			printf("A request could not be processed\n");
        		}
        		else {
                  if(thread_no + 1 <= MAX_THREADS)
            			thread_no++;
                  else {
                  printf("Too many threads\n");
                  exit(-1);
            		}
              }
            }
      close(fd);

    return 0;
}
