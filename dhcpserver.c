#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#define MAX_THREADS 10
pthread_t threads[MAX_THREADS];

struct thread_argos {
  int sock;
  struct sockaddr_in cli_addr;
  char *the_buf;
  int cli_len;
    };

void *connection_handler(void *thread_args) {  //Handle requests thread function
  struct thread_argos *the_args = (struct thread_argos*) thread_args;
  int fd = the_args->sock;
  struct sockaddr_in cliaddr = the_args->cli_addr;
  char *buf = the_args->the_buf;
  char *ack = "Server ACK";
  char *dhcp_offer = "DHCP offer!";

  if(client_discover(buf)) {
    printf("Client discover request! \n");
    sendto(fd, (const void*)dhcp_offer, strlen(dhcp_offer), 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
  }
  sendto(fd, (const void*)ack, strlen(ack), 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));

    return NULL;
}


int client_discover(char *msg) { //Check if client DHCP discover message
  char *key = "DHCP discover";
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

  struct thread_argos args;
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
    /* Join the multicast group  on the local host
       interface. IP_ADD_MEMBERSHIP option must be
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
    clilen = sizeof(cliaddr);
    args.cli_len = clilen;
    while(1) {
          if (recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&cliaddr, &clilen) < 0) { // Receive client message
            perror("recvfrom error");
            exit(-1);
          }
            args.cli_addr = cliaddr;
            args.the_buf = buf;
            args.sock = fd;
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
